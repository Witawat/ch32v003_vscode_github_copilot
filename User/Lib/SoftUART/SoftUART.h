/**
 * @file SoftUART.h
 * @brief Software UART (Bit-Bang Serial) Library สำหรับ CH32V003
 * @version 1.0
 * @date 2026-06-22
 *
 * @details
 * Library สำหรับสร้าง USART port เพิ่มเติมด้วย software bit-bang
 * แก้ปัญหาที่ CH32V003 มี USART ฮาร์ดแวร์แค่ 1 ตัว
 *
 * **คุณสมบัติ:**
 * - TX: bit-bang ด้วย Delay_Us (blocking)
 * - RX: polling mode (blocking พร้อม timeout)
 * - RX buffer ในตัว (64 bytes)
 * - รองรับ baud rate 9600 - 115200
 * - ใช้ GPIO pin ใดก็ได้
 *
 * **ข้อจำกัด:**
 * - TX/RX เป็น blocking (ปิด interrupt ชั่วคราวระหว่าง bit-bang)
 * - RX polling อาจพลาดข้อมูลถ้าเรียกไม่ทัน
 * - แนะนำ 9600-38400 baud เพื่อความเสถียร
 *
 * **Hardware Connection:**
 * ```
 *   CH32V003          External Device
 *   TX_Pin (PA2) ───> RX
 *   RX_Pin (PA1) <─── TX
 *   GND ────────────── GND
 * ```
 *
 * @example
 * #include "SoftUART.h"
 *
 * SoftUART_Instance uart;
 * uint8_t rx_buf[64];
 *
 * void main(void) {
 *     SystemCoreClockUpdate();
 *     Timer_Init();
 *
 *     SoftUART_Init(&uart, PA2, PA1, 9600, rx_buf, sizeof(rx_buf));
 *
 *     SoftUART_Printf(&uart, "Hello from SoftUART!\r\n");
 *
 *     while (1) {
 *         uint8_t data;
 *         if (SoftUART_ReadByte(&uart, &data, 100) == SOFTUART_OK) {
 *             SoftUART_WriteByte(&uart, data);
 *         }
 *     }
 * }
 *
 * @note ต้องเรียก SystemCoreClockUpdate() และ Timer_Init() ก่อนใช้
 * @warning ที่ baud rate สูง (>38400) ต้องปิด interrupt หรือใช้ NOP-delay แทน
 */

#ifndef __SOFT_UART_H
#define __SOFT_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../SimpleHAL/SimpleHAL.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

/* ========== Configuration ========== */

#ifndef SOFTUART_RX_BUF_SIZE
#define SOFTUART_RX_BUF_SIZE  64
#endif

/* ========== Status ========== */

typedef enum {
    SOFTUART_OK      = 0,
    SOFTUART_ERROR   = 1,
    SOFTUART_TIMEOUT = 2,
    SOFTUART_PARITY  = 3
} SoftUART_Status;

/* ========== Instance ========== */

typedef struct {
    uint8_t  tx_pin;
    uint8_t  rx_pin;
    uint32_t bit_time_us;
    uint32_t baud;
    uint8_t  rx_buffer[SOFTUART_RX_BUF_SIZE];
    uint16_t rx_head;
    uint16_t rx_tail;
    uint16_t rx_count;
    uint8_t  initialized;
} SoftUART_Instance;

/* ========== Function Prototypes ========== */

SoftUART_Status SoftUART_Init(SoftUART_Instance* uart, uint8_t tx_pin, uint8_t rx_pin, uint32_t baud);

SoftUART_Status SoftUART_SetBaud(SoftUART_Instance* uart, uint32_t baud);

void SoftUART_WriteByte(SoftUART_Instance* uart, uint8_t data);

SoftUART_Status SoftUART_ReadByte(SoftUART_Instance* uart, uint8_t* data, uint32_t timeout_ms);

uint16_t SoftUART_Available(SoftUART_Instance* uart);

SoftUART_Status SoftUART_Flush(SoftUART_Instance* uart);

void SoftUART_Write(SoftUART_Instance* uart, const uint8_t* data, uint16_t len);

void SoftUART_WriteString(SoftUART_Instance* uart, const char* str);

void SoftUART_Printf(SoftUART_Instance* uart, const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __SOFT_UART_H */
