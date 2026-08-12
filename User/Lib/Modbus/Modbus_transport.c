/**
 * @file Modbus_transport.c
 * @brief Transport layer ของ Modbus RTU — USART (ring buffer) และ DMA (circular + IDLE)
 *
 * โหมด DMA:
 * - RX: DMA_CH3 circular buffer + USART IDLE interrupt (จับจบเฟรม)
 * - TX: DMA_CH2 (DMA_USART_Send — blocking)
 * - ต้อง disable RXNE interrupt หลัง USART_SimpleInit (กัน ring buffer แย่ง byte กับ DMA)
 * - override USART_IdleHook() (weak ใน SimpleUSART) — ห้ามมีตัว override อื่นในโปรเจกต์
 */

#include "Modbus_transport.h"
#include "../../SimpleHAL/SimpleDMA.h"

/* ========== โหมด DMA — สถานะ ========== */

static uint8_t  s_dma_rx[MODBUS_DMA_RX_SIZE];       /* DMA RX circular buffer */
static volatile uint16_t s_dma_pos;                 /* ตำแหน่ง DMA เขียนล่าสุด (0..SIZE-1) */
static uint8_t  s_capture[MODBUS_MAX_RESP_DATA + 8];/* สำเนาเฟรมที่รับได้ (กันข้อมูลถูกเขียนทับ) */
static volatile uint16_t s_capture_len;             /* ISR เขียน, main อ่าน */
static volatile uint16_t s_capture_pos;             /* ISR รีเซ็ต, main ใช้ */

/** @brief instance ที่ใช้งานโหมด DMA (มีได้ตัวเดียว — USART1 มีตัวเดียว) */
static Modbus* s_dma_owner;

/**
 * @brief Override weak hook ของ SimpleUSART — เรียกจาก ISR เมื่อพบ IDLE line
 * @note ทำงานใน ISR — ห้าม blocking / ห้ามใช้ USART_* ที่ block
 */
void USART_IdleHook(void) {
    if (s_dma_owner == NULL) return;

    s_dma_pos = (uint16_t)DMA_USART_GetReceivedCount(DMA_CH3, MODBUS_DMA_RX_SIZE);

    if (!s_dma_owner->dma_frame_ready) {
        uint16_t flen = (uint16_t)(
            (s_dma_pos - s_dma_owner->dma_last_pos + MODBUS_DMA_RX_SIZE) %
            MODBUS_DMA_RX_SIZE);
        if (flen > 0) {
            /* คัดลอกเฟรมออกจาก circular buffer ทันทีใน ISR (เฟรมสั้น < 300B) */
            if (flen > sizeof(s_capture)) flen = (uint16_t)sizeof(s_capture);
            uint16_t start_pos = s_dma_owner->dma_last_pos;
            for (uint16_t i = 0; i < flen; i++) {
                s_capture[i] = s_dma_rx[(start_pos + i) % MODBUS_DMA_RX_SIZE];
            }
            s_capture_len = flen;
            s_capture_pos = 0;
            s_dma_owner->dma_frame_ready = 1;
        }
    }
}

/* ========== โหมด USART — ฟังก์ชันช่วย ========== */

static Modbus_Status _usart_read_byte(uint8_t* byte, uint32_t deadline) {
    while (!USART_Available()) {
        if ((int32_t)(Get_CurrentMs() - deadline) >= 0) {
            return MODBUS_ERROR_TIMEOUT;
        }
    }
    *byte = USART_Read();
    return MODBUS_OK;
}

/* ========== Public (dispatch ตาม transport) ========== */

void MBT_Init(Modbus* mb) {
    /* เปิด USART 9600 8N1 + pin remap + NVIC (ทำทั้ง 2 โหมด) */
    USART_SimpleInit(MODBUS_BAUD, (USART_PinConfig)mb->pin_config);

    if (mb->transport == MODBUS_TRANSPORT_DMA) {
        /* RX: DMA_CH3 circular + IDLE interrupt */
        DMA_USART_InitRx(DMA_CH3, s_dma_rx, MODBUS_DMA_RX_SIZE, 1);
        DMA_Start(DMA_CH3);

        /* TX: DMA_CH2 */
        DMA_USART_InitTx(DMA_CH2, s_dma_rx, MODBUS_DMA_RX_SIZE);

        /* ปิด RXNE interrupt (ring buffer) — กัน SimpleUSART แย่ง byte กับ DMA */
        USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
        /* เปิด IDLE interrupt — จับจบเฟรม */
        USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

        s_dma_owner = mb;
        mb->dma_last_pos = (uint16_t)DMA_USART_GetReceivedCount(DMA_CH3, MODBUS_DMA_RX_SIZE);
        mb->dma_frame_ready = 0;
        s_capture_len = 0;
        s_capture_pos = 0;
    }
}

void MBT_FlushRx(Modbus* mb) {
    if (mb->transport == MODBUS_TRANSPORT_DMA) {
        /* จำตำแหน่งปัจจุบัน — เฟรมถัดไป = bytes ที่เข้ามาหลังจากนี้ */
        mb->dma_frame_ready = 0;
        mb->dma_last_pos = (uint16_t)DMA_USART_GetReceivedCount(DMA_CH3, MODBUS_DMA_RX_SIZE);
        s_capture_len = 0;
        s_capture_pos = 0;
    } else {
        USART_Flush();
    }
}

void MBT_SendBytes(Modbus* mb, const uint8_t* data, uint16_t len) {
    if (mb->transport == MODBUS_TRANSPORT_DMA) {
        DMA_USART_Send(DMA_CH2, data, len);
        /* รอ byte สุดท้ายเลื่อนออกจาก shift register ครบ (DMA TC ≠ USART TC) */
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    } else {
        for (uint16_t i = 0; i < len; i++) USART_WriteByte(data[i]);
        /* รอ TX ครบจริงก่อนล้าง RX — กัน DE/RE สลับเร็วกว่า byte สุดท้ายออก */
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
        USART_Flush();
    }
}

Modbus_Status MBT_ReadByte(Modbus* mb, uint8_t* byte, uint32_t deadline) {
    if (mb->transport == MODBUS_TRANSPORT_DMA) {
        /* รอ IDLE interrupt คัดลอกเฟรมเข้า s_capture ก่อน */
        if (s_capture_pos >= s_capture_len) {
            while (!mb->dma_frame_ready) {
                if ((int32_t)(Get_CurrentMs() - deadline) >= 0) {
                    mb->dma_frame_ready = 0;
                    return MODBUS_ERROR_TIMEOUT;
                }
            }
        }
        if (s_capture_pos >= s_capture_len) {
            mb->dma_frame_ready = 0;
            return MODBUS_ERROR_TIMEOUT;  /* เฟรมสั้นกว่าที่คาด — malformed */
        }
        *byte = s_capture[s_capture_pos++];

        /* อ่านครบแล้ว — advance ตำแหน่งก่อน clear flag (กัน race: ถ้า IDLE
         * มาคั่นระหว่าง advance กับ clear — ISR เห็น ready==1 → ไม่ capture) */
        if (s_capture_pos >= s_capture_len) {
            mb->dma_last_pos = (uint16_t)(
                (mb->dma_last_pos + s_capture_len) % MODBUS_DMA_RX_SIZE);
            mb->dma_frame_ready = 0;
        }
        return MODBUS_OK;
    }

    /* โหมด USART — poll ring buffer */
    return _usart_read_byte(byte, deadline);
}
