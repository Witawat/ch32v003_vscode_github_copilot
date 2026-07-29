/**
 * @file SimpleUSART.h
 * @brief Simple USART Library สำหรับ CH32V003 แบบ Arduino-style
 * @version 1.0
 * @date 2025-12-12
 * 
 * @details
 * Library นี้ห่อหุ้ม Hardware USART ให้ใช้งานง่ายแบบ Arduino
 * รองรับ pin remapping และมีฟังก์ชันพื้นฐานสำหรับการสื่อสาร
 * 
 * **คุณสมบัติ:**
 * - เริ่มต้นใช้งานได้ 1 บรรทัด
 * - รองรับ 3 pin configurations
 * - ฟังก์ชัน print แบบ Arduino
 * - รองรับการอ่านแบบ blocking และ non-blocking
 * 
 * @example
 * // ตัวอย่างการใช้งาน
 * #include "SimpleUSART.h"
 * 
 * int main(void) {
 *     // เริ่มต้น USART ที่ 115200 baud, ใช้ default pins
 *     USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
 *     
 *     // ส่งข้อความ
 *     USART_Print("Hello World!\r\n");
 *     USART_PrintNum(12345);
 *     
 *     // อ่านข้อมูล
 *     if(USART_Available()) {
 *         uint8_t data = USART_Read();
 *     }
 * }
 * 
 * @note ต้องเรียก SystemCoreClockUpdate() และ Delay_Init() ก่อนใช้งาน
 */

#ifndef __SIMPLE_USART_H
#define __SIMPLE_USART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x_usart.h>
#include <stdint.h>
#include "SimpleHAL.h"

/* ========== Configuration ========== */

#ifndef USART_RX_BUFFER_SIZE
#define USART_RX_BUFFER_SIZE  64  /**< ขนาด RX ring buffer (bytes) — #define ค่าใหม่ก่อน include ถ้าต้องการเปลี่ยน */
#endif

/* ========== Enumerations ========== */

/**
 * @brief ค่า Baud Rate ที่รองรับ
 */
typedef enum {
    BAUD_9600   = 9600,      /**< 9600 baud */
    BAUD_19200  = 19200,     /**< 19200 baud */
    BAUD_38400  = 38400,     /**< 38400 baud */
    BAUD_57600  = 57600,     /**< 57600 baud */
    BAUD_115200 = 115200,    /**< 115200 baud (แนะนำ) */
    BAUD_230400 = 230400,    /**< 230400 baud */
    BAUD_460800 = 460800     /**< 460800 baud */
} USART_BaudRate;

/**
 * @brief การเลือก Pin Configuration
 * 
 * @details Pin mapping สำหรับ USART1:
 * - USART_PINS_DEFAULT: TX=PD5, RX=PD6
 * - USART_PINS_REMAP1:  TX=PD0, RX=PD1
 * - USART_PINS_REMAP2:  TX=PD6, RX=PD5
 */
typedef enum {
    USART_PINS_DEFAULT = 0,  /**< Default pins: TX=PD5, RX=PD6 (ใช้ได้ทุกแพ็กเกจ) */
#if CH32V003_HAS_PD0
    USART_PINS_REMAP1  = 1,  /**< Remap 1: TX=PD0, RX=PD1 (ต้องมี PD0 — TSSOP-20/QFN-20 เท่านั้น) */
#endif
    USART_PINS_REMAP2  = 2,  /**< Remap 2: TX=PD6, RX=PD5 (ใช้ได้ทุกแพ็กเกจ) */
    USART_PINS_FULL_REMAP = 3  /**< Full Remap: TX=PD6, RX=PD5 (รวมบิต REMAP1+REMAP2) */
} USART_PinConfig;

/* ========== Function Prototypes ========== */

/**
 * @brief เริ่มต้นการใช้งาน USART
 * @param baud อัตราความเร็ว baud rate
 * @param pin_config การเลือก pin configuration
 * 
 * @note ฟังก์ชันนี้จะ:
 *       1. เปิด Clock สำหรับ USART และ GPIO
 *       2. ตั้งค่า Pin remapping ตามที่เลือก
 *       3. ตั้งค่า USART (8N1, no flow control)
 *       4. เปิดใช้งาน USART พร้อม RX interrupt + ring buffer (กัน byte หายถ้า
 *          โปรแกรมอ่านไม่ทัน — hardware buffer มีแค่ 1 byte)
 * @warning ฟังก์ชันนี้ define USART1_IRQHandler() เอง — ห้ามใช้ร่วมกับ library
 *          อื่นที่ต้องการ owns USART1 IRQ เอง (เช่น TJC) เพราะมี ISR ได้แค่ตัวเดียว
 *          ต่อ interrupt vector
 *
 * @example
 * USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
 */
void USART_SimpleInit(USART_BaudRate baud, USART_PinConfig pin_config);

/**
 * @brief ส่งข้อความแบบ string
 * @param str pointer ไปยัง null-terminated string
 * 
 * @example
 * USART_Print("Hello World!\r\n");
 */
void USART_Print(const char* str);

/**
 * @brief ส่งตัวเลขแบบ decimal
 * @param num ตัวเลขที่ต้องการส่ง (signed 32-bit)
 * 
 * @example
 * USART_PrintNum(12345);   // ส่ง "12345"
 * USART_PrintNum(-999);    // ส่ง "-999"
 */
void USART_PrintNum(int32_t num);

/**
 * @brief ส่งตัวเลขแบบ hexadecimal
 * @param num ตัวเลขที่ต้องการส่ง (unsigned 32-bit)
 * @param uppercase ใช้ตัวพิมพ์ใหญ่ (A-F) หรือเล็ก (a-f)
 * 
 * @example
 * USART_PrintHex(0xFF, 1);    // ส่ง "0xFF"
 * USART_PrintHex(255, 0);     // ส่ง "0xff"
 */
void USART_PrintHex(uint32_t num, uint8_t uppercase);

/**
 * @brief ส่ง 1 byte
 * @param data ข้อมูล 1 byte ที่ต้องการส่ง
 * 
 * @example
 * USART_WriteByte(0x55);
 */
void USART_WriteByte(uint8_t data);

/**
 * @brief ตรวจสอบว่ามีข้อมูลรอรับหรือไม่
 * @return 1 = มีข้อมูล, 0 = ไม่มีข้อมูล
 * 
 * @example
 * if(USART_Available()) {
 *     uint8_t data = USART_Read();
 * }
 */
uint8_t USART_Available(void);

/**
 * @brief อ่านข้อมูล 1 byte (blocking)
 * @return ข้อมูล 1 byte ที่อ่านได้
 * 
 * @note ฟังก์ชันนี้จะรอจนกว่าจะมีข้อมูล
 * 
 * @example
 * uint8_t data = USART_Read();
 */
uint8_t USART_Read(void);

/**
 * @brief อ่านข้อมูลหลาย bytes
 * @param buffer pointer ไปยัง buffer สำหรับเก็บข้อมูล
 * @param length จำนวน bytes ที่ต้องการอ่าน
 * @return จำนวน bytes ที่อ่านได้จริง
 * 
 * @example
 * uint8_t buffer[10];
 * uint16_t len = USART_ReadBytes(buffer, 10);
 */
uint16_t USART_ReadBytes(uint8_t* buffer, uint16_t length);

/**
 * @brief ล้างข้อมูลใน receive buffer
 * 
 * @example
 * USART_Flush();
 */
void USART_Flush(void);

/**
 * @brief Hook เรียกจาก USART1_IRQHandler() ทุกครั้งที่ได้รับ byte ใหม่ (v2.1)
 * @param data byte ที่เพิ่งรับมา (เติมลง ring buffer ของ SimpleUSART ไปแล้วก่อนเรียก hook นี้)
 *
 * @details
 * Default implementation ไม่ทำอะไร (`__attribute__((weak))`) — override ได้โดย define
 * ฟังก์ชันชื่อเดียวกันแบบไม่ใส่ weak ในไฟล์ของคุณเอง (compiler จะเลือกตัวที่ไม่ weak แทน)
 * ใช้กรณีต้องการ "แอบดู" byte ที่เข้ามาโดยไม่ต้องเขียน `USART1_IRQHandler()` เอง — เพราะ
 * `USART1_IRQHandler()` ถูก SimpleUSART.c เป็นเจ้าของแล้ว (มี ISR ได้แค่ตัวเดียวต่อ vector)
 * ตัวอย่าง: `User/Lib/TJC/TJC.c` ใช้กลไกนี้แทนการ define `USART1_IRQHandler()` ของตัวเอง
 *
 * @example
 * void USART_RxByteHook(uint8_t data) {
 *     my_parser_feed(data);
 * }
 */
void USART_RxByteHook(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif  // __SIMPLE_USART_H
