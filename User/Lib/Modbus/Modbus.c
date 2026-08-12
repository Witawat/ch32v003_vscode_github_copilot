/**
 * @file Modbus.c
 * @brief Modbus RTU Master Library Implementation — โปรโตคอลหลัก
 */

#include "Modbus.h"
#include "Modbus_transport.h"

/* ========== Private: CRC-16 (Modbus) ========== */

uint16_t MODBUS_CRC16(const uint8_t* buf, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)buf[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

/* ========== Private: สร้าง/ส่ง/รับ ADU ========== */

/* Static buffers (RAM ~510B) — ใช้ static เพราะ stack ของ CH32V003 มีแค่ 256 bytes
 * หมายเหตุ: โปรแกรมเดียวใช้ Modbus master ได้ตัวเดียว (USART1 มีตัวเดียว) → ปลอดภัย */
static uint8_t s_req[MODBUS_MAX_RESP_DATA + 8];
static uint8_t s_frame[3 + MODBUS_MAX_RESP_DATA];
static uint8_t s_pdu[6 + 246];   /* write multi: FC + addr + cnt + bcnt + data (สูงสุด 123 regs) */

/**
 * @brief ส่ง PDU (Protocol Data Unit) ไป slave และรับ response
 * @param mb        instance
 * @param pdu       PDU (function code + data)
 * @param pdu_len   ความยาว PDU
 * @param resp_data buffer รับ payload ของ response (เฉพาะ data — ไม่รวม addr/FC/CRC)
 * @param resp_max  ขนาดสูงสุดของ buffer
 * @param resp_len  [out] จำนวน bytes payload ที่รับได้
 * @return MODBUS_Status
 */
static Modbus_Status _modbus_request(Modbus* mb, const uint8_t* pdu, uint16_t pdu_len,
                                     uint8_t* resp_data, uint16_t resp_max,
                                     uint16_t* resp_len) {
    if (mb == NULL || !mb->initialized || pdu == NULL || pdu_len < 1) {
        if (mb != NULL) mb->last_error = MODBUS_ERROR_PARAM;
        return MODBUS_ERROR_PARAM;
    }

    /* ADU = [ADDR][PDU][CRC_LO][CRC_HI] */
    uint8_t* req = s_req;
    req[0] = mb->slave_addr;
    for (uint16_t i = 0; i < pdu_len; i++) req[1 + i] = pdu[i];
    uint16_t crc = MODBUS_CRC16(req, 1 + pdu_len);
    req[1 + pdu_len]     = (uint8_t)(crc & 0xFF);
    req[1 + pdu_len + 1] = (uint8_t)(crc >> 8);
    uint16_t req_len = 1 + pdu_len + 2;

    /* ล้าง RX ก่อนส่ง (ทิ้งข้อมูลค้างเก่า) แล้วส่งคำขอ */
    MBT_FlushRx(mb);
    MBT_SendBytes(mb, req, req_len);

    /* timeout วัดรวมทั้งคำขอ — เริ่มนับก่อนรับ response byte แรก */
    uint32_t deadline = Get_CurrentMs() + MODBUS_TIMEOUT_MS;

    /* รับ response — header 3 bytes: [ADDR][FC][BYTE_COUNT] */
    uint8_t header[3];
    Modbus_Status st = MBT_ReadByte(mb, &header[0], deadline);
    if (st != MODBUS_OK) return st;
    st = MBT_ReadByte(mb, &header[1], deadline);
    if (st != MODBUS_OK) return st;
    st = MBT_ReadByte(mb, &header[2], deadline);
    if (st != MODBUS_OK) return st;

    /* ตรวจ address */
    if (header[0] != mb->slave_addr) return MODBUS_ERROR_RESP;

    /* ตรวจ exception response (FC | 0x80) */
    if (header[1] == (pdu[0] | 0x80)) {
        mb->last_exception = header[2];
        return MODBUS_ERROR_EXCEPT;
    }

    /* ตรวจ function code echo */
    if (header[1] != pdu[0]) return MODBUS_ERROR_RESP;

    if (pdu[0] <= 0x04) {
        /* Read (01/02/03/04): ตามด้วย [DATA byte_count][CRC 2 bytes] */
        if (resp_data == NULL || resp_len == NULL) {
            mb->last_error = MODBUS_ERROR_PARAM;
            return MODBUS_ERROR_PARAM;
        }
        uint8_t byte_count = header[2];
        if (byte_count == 0 || byte_count > resp_max) return MODBUS_ERROR_RESP;

        uint8_t crc_b[2];
        for (uint16_t i = 0; i < byte_count; i++) {
            st = MBT_ReadByte(mb, &resp_data[i], deadline);
            if (st != MODBUS_OK) return st;
        }
        st = MBT_ReadByte(mb, &crc_b[0], deadline);
        if (st != MODBUS_OK) return st;
        st = MBT_ReadByte(mb, &crc_b[1], deadline);
        if (st != MODBUS_OK) return st;

        /* CRC ครอบทั้ง ADU: [ADDR][FC][BYTE_COUNT][DATA...] */
        uint8_t* frame = s_frame;
        frame[0] = header[0];
        frame[1] = header[1];
        frame[2] = header[2];
        for (uint16_t i = 0; i < byte_count; i++) frame[3 + i] = resp_data[i];
        uint16_t calc_crc = MODBUS_CRC16(frame, 3 + byte_count);
        uint16_t recv_crc = (uint16_t)crc_b[0] | ((uint16_t)crc_b[1] << 8);
        if (calc_crc != recv_crc) return MODBUS_ERROR_CRC;

        *resp_len = byte_count;
    } else {
        /* Write (05/06/0F/10): response 8 bytes = echo PDU + CRC
         * header[2] คือ byte แรกของ echo — รับส่วนที่เหลือ 5 bytes + CRC 2 bytes */
        uint8_t echo[6];
        echo[0] = header[0];
        echo[1] = header[1];
        echo[2] = header[2];
        for (uint8_t i = 3; i < 6; i++) {
            st = MBT_ReadByte(mb, &echo[i], deadline);
            if (st != MODBUS_OK) return st;
        }
        uint8_t crc_b[2];
        st = MBT_ReadByte(mb, &crc_b[0], deadline);
        if (st != MODBUS_OK) return st;
        st = MBT_ReadByte(mb, &crc_b[1], deadline);
        if (st != MODBUS_OK) return st;

        if (MODBUS_CRC16(echo, 6) != ((uint16_t)crc_b[0] | ((uint16_t)crc_b[1] << 8))) {
            return MODBUS_ERROR_CRC;
        }
        if (resp_len != NULL) *resp_len = 0;
    }

    return MODBUS_OK;
}

/* ========== Public ========== */

Modbus_Status MODBUS_Init(Modbus* mb, uint8_t slave_addr,
                          Modbus_Transport transport, uint8_t pin_config) {
    if (mb == NULL) return MODBUS_ERROR_PARAM;
    if (slave_addr == 0x00 || slave_addr > 0xF7) {
        mb->last_error = MODBUS_ERROR_PARAM;
        return MODBUS_ERROR_PARAM;
    }

    mb->slave_addr      = slave_addr;
    mb->transport       = transport;
    mb->pin_config      = pin_config;
    mb->initialized     = 0;
    mb->last_error      = MODBUS_OK;
    mb->last_exception  = 0;
    mb->dma_frame_len   = 0;
    mb->dma_frame_ready = 0;
    mb->dma_last_pos    = 0;

    MBT_Init(mb);
    mb->initialized = 1;
    return MODBUS_OK;
}

Modbus_Status MODBUS_ReadHoldingRegisters(Modbus* mb, uint16_t reg, uint16_t count, uint16_t* data) {
    if (mb == NULL || data == NULL || count < 1 || count > 125) {
        if (mb != NULL) mb->last_error = MODBUS_ERROR_PARAM;
        return MODBUS_ERROR_PARAM;
    }

    uint8_t pdu[5];
    pdu[0] = MODBUS_FC_READ_HOLDING_REG;
    pdu[1] = (uint8_t)(reg >> 8);
    pdu[2] = (uint8_t)(reg & 0xFF);
    pdu[3] = (uint8_t)(count >> 8);
    pdu[4] = (uint8_t)(count & 0xFF);

    uint8_t raw[MODBUS_MAX_RESP_DATA];
    uint16_t raw_len = 0;
    mb->last_error = _modbus_request(mb, pdu, 5, raw, sizeof(raw), &raw_len);
    if (mb->last_error != MODBUS_OK) return mb->last_error;
    if (raw_len != count * 2) return (mb->last_error = MODBUS_ERROR_RESP);

    for (uint16_t i = 0; i < count; i++) {
        data[i] = ((uint16_t)raw[i * 2] << 8) | raw[i * 2 + 1];
    }
    return mb->last_error;
}

Modbus_Status MODBUS_ReadInputRegisters(Modbus* mb, uint16_t reg, uint16_t count, uint16_t* data) {
    if (mb == NULL || data == NULL || count < 1 || count > 125) {
        if (mb != NULL) mb->last_error = MODBUS_ERROR_PARAM;
        return MODBUS_ERROR_PARAM;
    }

    uint8_t pdu[5];
    pdu[0] = MODBUS_FC_READ_INPUT_REG;
    pdu[1] = (uint8_t)(reg >> 8);
    pdu[2] = (uint8_t)(reg & 0xFF);
    pdu[3] = (uint8_t)(count >> 8);
    pdu[4] = (uint8_t)(count & 0xFF);

    uint8_t raw[MODBUS_MAX_RESP_DATA];
    uint16_t raw_len = 0;
    mb->last_error = _modbus_request(mb, pdu, 5, raw, sizeof(raw), &raw_len);
    if (mb->last_error != MODBUS_OK) return mb->last_error;
    if (raw_len != count * 2) return (mb->last_error = MODBUS_ERROR_RESP);

    for (uint16_t i = 0; i < count; i++) {
        data[i] = ((uint16_t)raw[i * 2] << 8) | raw[i * 2 + 1];
    }
    return mb->last_error;
}

Modbus_Status MODBUS_ReadCoils(Modbus* mb, uint16_t coil, uint16_t count, uint8_t* data) {
    if (mb == NULL || data == NULL || count < 1 || count > 2000) {
        if (mb != NULL) mb->last_error = MODBUS_ERROR_PARAM;
        return MODBUS_ERROR_PARAM;
    }

    uint8_t pdu[5];
    pdu[0] = MODBUS_FC_READ_COILS;
    pdu[1] = (uint8_t)(coil >> 8);
    pdu[2] = (uint8_t)(coil & 0xFF);
    pdu[3] = (uint8_t)(count >> 8);
    pdu[4] = (uint8_t)(count & 0xFF);

    uint8_t byte_count = (uint8_t)((count + 7) / 8);
    uint8_t raw[MODBUS_MAX_RESP_DATA];
    uint16_t raw_len = 0;
    mb->last_error = _modbus_request(mb, pdu, 5, raw, sizeof(raw), &raw_len);
    if (mb->last_error != MODBUS_OK) return mb->last_error;
    if (raw_len != byte_count) return (mb->last_error = MODBUS_ERROR_RESP);

    for (uint8_t i = 0; i < byte_count; i++) data[i] = raw[i];
    return mb->last_error;
}

Modbus_Status MODBUS_ReadDiscreteInputs(Modbus* mb, uint16_t input, uint16_t count, uint8_t* data) {
    if (mb == NULL || data == NULL || count < 1 || count > 2000) {
        if (mb != NULL) mb->last_error = MODBUS_ERROR_PARAM;
        return MODBUS_ERROR_PARAM;
    }

    uint8_t pdu[5];
    pdu[0] = MODBUS_FC_READ_DISCRETE_INPUTS;
    pdu[1] = (uint8_t)(input >> 8);
    pdu[2] = (uint8_t)(input & 0xFF);
    pdu[3] = (uint8_t)(count >> 8);
    pdu[4] = (uint8_t)(count & 0xFF);

    uint8_t byte_count = (uint8_t)((count + 7) / 8);
    uint8_t raw[MODBUS_MAX_RESP_DATA];
    uint16_t raw_len = 0;
    mb->last_error = _modbus_request(mb, pdu, 5, raw, sizeof(raw), &raw_len);
    if (mb->last_error != MODBUS_OK) return mb->last_error;
    if (raw_len != byte_count) return (mb->last_error = MODBUS_ERROR_RESP);

    for (uint8_t i = 0; i < byte_count; i++) data[i] = raw[i];
    return mb->last_error;
}

Modbus_Status MODBUS_WriteSingleCoil(Modbus* mb, uint16_t coil, uint8_t value) {
    if (mb == NULL) return MODBUS_ERROR_PARAM;

    uint8_t pdu[5];
    pdu[0] = MODBUS_FC_WRITE_SINGLE_COIL;
    pdu[1] = (uint8_t)(coil >> 8);
    pdu[2] = (uint8_t)(coil & 0xFF);
    pdu[3] = value ? 0xFF : 0x00;
    pdu[4] = 0x00;

    mb->last_error = _modbus_request(mb, pdu, 5, NULL, 0, NULL);
    return mb->last_error;
}

Modbus_Status MODBUS_WriteSingleRegister(Modbus* mb, uint16_t reg, uint16_t value) {
    if (mb == NULL) return MODBUS_ERROR_PARAM;

    uint8_t pdu[5];
    pdu[0] = MODBUS_FC_WRITE_SINGLE_REG;
    pdu[1] = (uint8_t)(reg >> 8);
    pdu[2] = (uint8_t)(reg & 0xFF);
    pdu[3] = (uint8_t)(value >> 8);
    pdu[4] = (uint8_t)(value & 0xFF);

    mb->last_error = _modbus_request(mb, pdu, 5, NULL, 0, NULL);
    return mb->last_error;
}

Modbus_Status MODBUS_WriteMultipleRegisters(Modbus* mb, uint16_t reg, uint16_t count, const uint16_t* data) {
    if (mb == NULL || data == NULL || count < 1 || count > 123) {
        if (mb != NULL) mb->last_error = MODBUS_ERROR_PARAM;
        return MODBUS_ERROR_PARAM;
    }

    uint8_t* pdu = s_pdu;
    pdu[0] = MODBUS_FC_WRITE_MULTI_REGS;
    pdu[1] = (uint8_t)(reg >> 8);
    pdu[2] = (uint8_t)(reg & 0xFF);
    pdu[3] = (uint8_t)(count >> 8);
    pdu[4] = (uint8_t)(count & 0xFF);
    pdu[5] = (uint8_t)(count * 2);
    for (uint16_t i = 0; i < count; i++) {
        pdu[6 + i * 2]     = (uint8_t)(data[i] >> 8);
        pdu[6 + i * 2 + 1] = (uint8_t)(data[i] & 0xFF);
    }

    mb->last_error = _modbus_request(mb, pdu, 6 + count * 2, NULL, 0, NULL);
    return mb->last_error;
}

Modbus_Status MODBUS_WriteMultipleCoils(Modbus* mb, uint16_t coil, uint16_t count, const uint8_t* data) {
    if (mb == NULL || data == NULL || count < 1 || count > 1968) {
        if (mb != NULL) mb->last_error = MODBUS_ERROR_PARAM;
        return MODBUS_ERROR_PARAM;
    }

    uint8_t byte_count = (uint8_t)((count + 7) / 8);
    uint8_t* pdu = s_pdu;
    pdu[0] = MODBUS_FC_WRITE_MULTI_COILS;
    pdu[1] = (uint8_t)(coil >> 8);
    pdu[2] = (uint8_t)(coil & 0xFF);
    pdu[3] = (uint8_t)(count >> 8);
    pdu[4] = (uint8_t)(count & 0xFF);
    pdu[5] = byte_count;
    for (uint8_t i = 0; i < byte_count; i++) pdu[6 + i] = data[i];

    mb->last_error = _modbus_request(mb, pdu, 6 + byte_count, NULL, 0, NULL);
    return mb->last_error;
}

Modbus_Status MODBUS_GetLastError(Modbus* mb) {
    if (mb == NULL) return MODBUS_ERROR_PARAM;
    return mb->last_error;
}

uint8_t MODBUS_GetLastException(Modbus* mb) {
    if (mb == NULL) return 0;
    return mb->last_exception;
}

const char* MODBUS_StatusStr(Modbus_Status status) {
    switch (status) {
        case MODBUS_OK:            return "OK";
        case MODBUS_ERROR_PARAM:   return "Param Error";
        case MODBUS_ERROR_TIMEOUT: return "Timeout";
        case MODBUS_ERROR_CRC:     return "CRC Error";
        case MODBUS_ERROR_RESP:    return "Bad Response";
        case MODBUS_ERROR_EXCEPT:  return "Slave Exception";
        case MODBUS_ERROR_BUSY:    return "Busy";
        default:                   return "Unknown";
    }
}
