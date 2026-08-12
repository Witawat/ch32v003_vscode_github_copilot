/**
 * @file Modbus.h
 * @brief Modbus RTU Master Library สำหรับ CH32V003 (USART + DMA)
 * @version 1.0
 * @date 2026-08-10
 *
 * @details
 * Library โปรโตคอล Modbus RTU แบบ Master — CH32V003 เป็นตัวส่งคำสั่ง
 * อ่าน/เขียน register ของอุปกรณ์ slave (PZEM-004T v3, VFD, เซนเซอร์อุตสาหกรรม ฯลฯ)
 *
 * รองรับ Function Code:
 * - 0x01 Read Coils, 0x02 Read Discrete Inputs
 * - 0x03 Read Holding Registers, 0x04 Read Input Registers
 * - 0x05 Write Single Coil, 0x06 Write Single Register
 * - 0x0F Write Multiple Coils, 0x10 Write Multiple Registers
 *
 * **2 โหมดขนส่ง (เลือกตอน Init):**
 * - `MODBUS_TRANSPORT_USART` — ใช้ interrupt ring buffer ของ SimpleUSART
 *   (ใช้งานง่ายสุด รับข้อมูลทุก byte เข้า ISR)
 * - `MODBUS_TRANSPORT_DMA` — DMA RX circular buffer + USART IDLE interrupt
 *   (CPU ไม่ถูก interrupt ทุก byte, จับจบเฟรมด้วย IDLE line)
 *
 * **วงจร (RS-485 เพิ่มเติมตามปกติ):**
 * ```
 *   CH32V003 (Master)         Slave (Modbus RTU)
 *   PD5 (TX) ──────────────> RX
 *   PD6 (RX) <────────────── TX
 *   GND ────────────────────> GND
 *   (ระยะไกล/หลายตัว: ต่อผ่าน MAX485 — DI=PD5, RO=PD6, DE/RE=GPIO)
 * ```
 *
 * **การใช้งาน:**
 * @code
 * Modbus mb;
 * MODBUS_Init(&mb, 1, MODBUS_TRANSPORT_USART, USART_PINS_DEFAULT);
 *
 * uint16_t regs[10];
 * if (MODBUS_ReadHoldingRegisters(&mb, 0x0000, 10, regs) == MODBUS_OK) {
 *     // regs[0..9] — ค่าจาก slave
 * }
 * @endcode
 *
 * @note เรียก SystemCoreClockUpdate() และ Timer_Init() ก่อนเสมอ (ใช้ Get_CurrentMs)
 * @note โหมด DMA ใช้ DMA_CH2 (TX) + DMA_CH3 (RX) — ห้ามชนกับ DMA ของ ADC/อื่นๆ
 *
 * @author CH32V003 Library Team
 */

#ifndef __MODBUS_H
#define __MODBUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../SimpleHAL/SimpleHAL.h"
#include <stdint.h>

/* ========== Configuration ========== */

/** @brief Timeout รอ response จาก slave (ms) */
#ifndef MODBUS_TIMEOUT_MS
#define MODBUS_TIMEOUT_MS       500
#endif

/** @brief Baud rate สำหรับ Modbus RTU (default 9600) */
#ifndef MODBUS_BAUD
#define MODBUS_BAUD             BAUD_9600
#endif

/** @brief ขนาด DMA RX buffer (โหมด DMA) */
#ifndef MODBUS_DMA_RX_SIZE
#define MODBUS_DMA_RX_SIZE      256
#endif

/** @brief ขนาด response data สูงสุด (bytes) — 125 registers x 2 = 250 + CRC */
#ifndef MODBUS_MAX_RESP_DATA
#define MODBUS_MAX_RESP_DATA    250
#endif

/** @brief Broadcast address (เขียนได้ทุกตัว แต่ slave ไม่ตอบกลับ) */
#define MODBUS_ADDR_BROADCAST   0x00

/* ========== Transport Modes ========== */

/**
 * @brief โหมดการขนส่งข้อมูล
 */
typedef enum {
    MODBUS_TRANSPORT_USART = 0,  /**< ใช้ SimpleUSART ring buffer (RXNE interrupt) */
    MODBUS_TRANSPORT_DMA   = 1   /**< DMA RX circular + IDLE interrupt + DMA TX */
} Modbus_Transport;

/* ========== Status ========== */

/**
 * @brief สถานะการทำงาน
 */
typedef enum {
    MODBUS_OK              = 0,  /**< สำเร็จ */
    MODBUS_ERROR_PARAM     = 1,  /**< พารามิเตอร์ไม่ถูกต้อง */
    MODBUS_ERROR_TIMEOUT   = 2,  /**< ไม่ได้รับ response ภายใน timeout */
    MODBUS_ERROR_CRC       = 3,  /**< CRC-16 ไม่ตรง */
    MODBUS_ERROR_RESP      = 4,  /**< response address/function code ไม่ถูกต้อง */
    MODBUS_ERROR_EXCEPT    = 5,  /**< slave ตอบ exception */
    MODBUS_ERROR_BUSY      = 6   /**< กำลังรอ response จากคำขอก่อนหน้า (DMA mode) */
} Modbus_Status;

/* ========== Modbus Exception Codes ========== */

#define MODBUS_EX_ILLEGAL_FUNCTION     0x01  /**< Function code ไม่รองรับ */
#define MODBUS_EX_ILLEGAL_DATA_ADDR    0x02  /**< register address เกินช่วง */
#define MODBUS_EX_ILLEGAL_DATA_VALUE   0x03  /**< ค่าที่เขียนไม่ถูกต้อง */
#define MODBUS_EX_SLAVE_DEVICE_FAILURE 0x04  /**< อุปกรณ์ slave ผิดปกติ */
#define MODBUS_EX_ACKNOWLEDGE          0x05  /**< ยอมรับแต่ยังทำไม่เสร็จ */
#define MODBUS_EX_SLAVE_BUSY           0x06  /**< slave ยุ่ง รอแล้วลองใหม่ */
#define MODBUS_EX_NEGATIVE_ACK         0x07  /**< ปฏิเสธ */
#define MODBUS_EX_MEMORY_PARITY        0x08  /**< parity error ในหน่วยความจำ */

/* ========== Function Codes ========== */

#define MODBUS_FC_READ_COILS            0x01
#define MODBUS_FC_READ_DISCRETE_INPUTS  0x02
#define MODBUS_FC_READ_HOLDING_REG      0x03
#define MODBUS_FC_READ_INPUT_REG        0x04
#define MODBUS_FC_WRITE_SINGLE_COIL     0x05
#define MODBUS_FC_WRITE_SINGLE_REG      0x06
#define MODBUS_FC_WRITE_MULTI_COILS     0x0F
#define MODBUS_FC_WRITE_MULTI_REGS      0x10

/* ========== Instance ========== */

/**
 * @brief Modbus Master Instance
 */
typedef struct {
    uint8_t          slave_addr;    /**< address ของ slave ที่คุยด้วย */
    uint8_t          pin_config;    /**< USART pin configuration */
    Modbus_Transport transport;     /**< โหมดขนส่งที่เลือก */
    uint8_t          initialized;
    Modbus_Status    last_error;    /**< error ของคำขอล่าสุด */
    uint8_t          last_exception;/**< exception code ล่าสุด (เมื่อ last_error=ERROR_EXCEPT) */

    /* ---- internal (transport) ---- */
    volatile uint16_t dma_frame_len;   /**< จำนวน bytes ของเฟรมล่าสุด (DMA mode) */
    volatile uint8_t  dma_frame_ready; /**< 1 = มีเฟรมครบรอประมวลผล (DMA mode) */
    volatile uint16_t dma_last_pos;    /**< ตำแหน่ง DMA เขียนล่าสุดที่ประมวลผลแล้ว (ISR อ่าน, main เขียน) */
} Modbus;

/* ========== Function Prototypes ========== */

/**
 * @brief เริ่มต้น Modbus RTU Master
 * @param mb         ตัวแปร instance
 * @param slave_addr address ของ slave (0x01-0xF7)
 * @param transport  MODBUS_TRANSPORT_USART หรือ MODBUS_TRANSPORT_DMA
 * @param pin_config USART pin: USART_PINS_DEFAULT / USART_PINS_REMAP1 / USART_PINS_REMAP2
 * @return MODBUS_OK หรือ MODBUS_ERROR_PARAM
 * @note เรียก SystemCoreClockUpdate() และ Timer_Init() ก่อนเสมอ
 *
 * @example
 * Modbus mb;
 * MODBUS_Init(&mb, 1, MODBUS_TRANSPORT_USART, USART_PINS_DEFAULT);
 */
Modbus_Status MODBUS_Init(Modbus* mb, uint8_t slave_addr,
                          Modbus_Transport transport, uint8_t pin_config);

/**
 * @brief อ่าน Holding Registers (FC 0x03)
 * @param mb     instance
 * @param reg    register เริ่มต้น (0-based)
 * @param count  จำนวน register (1-125)
 * @param data   buffer รับค่า (ต้องมีขนาด >= count x 2 bytes)
 * @return MODBUS_OK หรือ error
 *
 * @example
 * uint16_t regs[10];
 * if (MODBUS_ReadHoldingRegisters(&mb, 0x0000, 10, regs) == MODBUS_OK) { }
 */
Modbus_Status MODBUS_ReadHoldingRegisters(Modbus* mb, uint16_t reg, uint16_t count, uint16_t* data);

/**
 * @brief อ่าน Input Registers (FC 0x04)
 * @param mb     instance
 * @param reg    register เริ่มต้น (0-based)
 * @param count  จำนวน register (1-125)
 * @param data   buffer รับค่า (ต้องมีขนาด >= count x 2 bytes)
 * @return MODBUS_OK หรือ error
 *
 * @example
 * uint16_t regs[10];
 * if (MODBUS_ReadInputRegisters(&mb, 0x0000, 10, regs) == MODBUS_OK) { }
 */
Modbus_Status MODBUS_ReadInputRegisters(Modbus* mb, uint16_t reg, uint16_t count, uint16_t* data);

/**
 * @brief อ่าน Coils (FC 0x01)
 * @param mb     instance
 * @param coil   coil เริ่มต้น (0-based)
 * @param count  จำนวน coil (1-2000)
 * @param data   buffer รับค่า (bit-packed — 1 bit ต่อ coil, ต้องมีขนาด >= ceil(count/8))
 * @return MODBUS_OK หรือ error
 *
 * @example
 * uint8_t coils[4];  // รองรับ 32 coils
 * if (MODBUS_ReadCoils(&mb, 0, 20, coils) == MODBUS_OK) {
 *     uint8_t coil0 = coils[0] & 0x01;   // coil 0
 *     uint8_t coil5 = (coils[0] >> 5) & 1; // coil 5
 * }
 */
Modbus_Status MODBUS_ReadCoils(Modbus* mb, uint16_t coil, uint16_t count, uint8_t* data);

/**
 * @brief อ่าน Discrete Inputs (FC 0x02)
 * @param mb     instance
 * @param input  input เริ่มต้น (0-based)
 * @param count  จำนวน input (1-2000)
 * @param data   buffer รับค่า (bit-packed, ต้องมีขนาด >= ceil(count/8))
 * @return MODBUS_OK หรือ error
 */
Modbus_Status MODBUS_ReadDiscreteInputs(Modbus* mb, uint16_t input, uint16_t count, uint8_t* data);

/**
 * @brief เขียน Single Coil (FC 0x05)
 * @param mb    instance
 * @param coil  coil ที่ต้องการเขียน (0-based)
 * @param value 0 = OFF, != 0 = ON
 * @return MODBUS_OK หรือ error
 *
 * @example
 * MODBUS_WriteSingleCoil(&mb, 0, 1);  // เปิด coil 0
 */
Modbus_Status MODBUS_WriteSingleCoil(Modbus* mb, uint16_t coil, uint8_t value);

/**
 * @brief เขียน Single Register (FC 0x06)
 * @param mb    instance
 * @param reg   register ที่ต้องการเขียน (0-based)
 * @param value ค่าที่ต้องการเขียน (16-bit)
 * @return MODBUS_OK หรือ error
 *
 * @example
 * MODBUS_WriteSingleRegister(&mb, 0x0000, 1000);  // เขียนค่า 1000 ไป reg 0
 */
Modbus_Status MODBUS_WriteSingleRegister(Modbus* mb, uint16_t reg, uint16_t value);

/**
 * @brief เขียน Multiple Registers (FC 0x10)
 * @param mb    instance
 * @param reg   register เริ่มต้น (0-based)
 * @param count จำนวน register (1-123)
 * @param data  buffer ค่าที่ต้องการเขียน
 * @return MODBUS_OK หรือ error
 */
Modbus_Status MODBUS_WriteMultipleRegisters(Modbus* mb, uint16_t reg, uint16_t count, const uint16_t* data);

/**
 * @brief เขียน Multiple Coils (FC 0x0F)
 * @param mb    instance
 * @param coil  coil เริ่มต้น (0-based)
 * @param count จำนวน coil (1-1968)
 * @param data  buffer ค่า (bit-packed)
 * @return MODBUS_OK หรือ error
 */
Modbus_Status MODBUS_WriteMultipleCoils(Modbus* mb, uint16_t coil, uint16_t count, const uint8_t* data);

/**
 * @brief อ่าน error ของคำขอล่าสุด
 * @param mb instance
 * @return MODBUS_OK หรือ error code
 */
Modbus_Status MODBUS_GetLastError(Modbus* mb);

/**
 * @brief อ่าน exception code ล่าสุด (เมื่อ GetLastError = MODBUS_ERROR_EXCEPT)
 * @param mb instance
 * @return exception code (MODBUS_EX_*)
 */
uint8_t MODBUS_GetLastException(Modbus* mb);

/**
 * @brief แปลง error code เป็นข้อความ
 * @param status error code
 * @return ข้อความอธิบาย error (ใช้กับ USART_Print ได้)
 *
 * @example
 * Modbus_Status st = MODBUS_GetLastError(&mb);
 * USART_Print(MODBUS_StatusStr(st));
 */
const char* MODBUS_StatusStr(Modbus_Status status);

/**
 * @brief คำนวณ CRC-16 Modbus (public — ใช้ตรวจเฟรมเองได้)
 * @param buf ข้อมูล
 * @param len ความยาวข้อมูล (bytes)
 * @return ค่า CRC-16 (little-endian ในเฟรม: byte ต่ำมาก่อน)
 *
 * @example
 * uint16_t crc = MODBUS_CRC16(frame, 6);
 */
uint16_t MODBUS_CRC16(const uint8_t* buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __MODBUS_H */
