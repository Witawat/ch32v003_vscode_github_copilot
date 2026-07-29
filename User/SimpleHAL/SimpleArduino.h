/**
 * @file SimpleArduino.h
 * @brief Arduino-compatible API aliases และ utility functions สำหรับ CH32V003
 * @version 1.0
 * @date 2026-06-16
 *
 * @details
 * รวมฟังก์ชันและ macro ที่มีชื่อตรงกับ Arduino standard API
 * เพื่อให้ผู้ที่เคยใช้ Arduino สามารถย้ายมาใช้ CH32V003 ได้สะดวก
 *
 * **Arduino Aliases (macros):**
 * - millis(), micros() — เวลาปัจจุบัน
 * - delay(), delayMicroseconds() — blocking delay
 * - interrupts(), noInterrupts() — จัดการ global interrupt
 *
 * **Utility Functions:**
 * - digitalPinToInterrupt() — แปลง pin number เป็น EXTI line
 * - randomSeed(), random() — PRNG (Pseudo-Random Number Generator)
 * - yield() — cooperative multitasking
 * - dtostrf() — float/double to string (lightweight)
 *
 * **USART Print Extensions (optional):**
 * - USART_Println(), USART_PrintlnNum(), USART_PrintlnHex()
 * - USART_PrintFloat(), USART_PrintlnFloat()
 *
 * @note เปิด/ปิด USART print extensions ด้วย:
 *       #define ENABLE_USART_PRINTLN  1   // or 0
 *       #define ENABLE_USART_PRINTFLOAT 1 // or 0
 *
 * @author CH32V003 Library Team
 */
#ifndef __SIMPLE_ARDUINO_H
#define __SIMPLE_ARDUINO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ========== Arduino Time Aliases ========== */

/**
 * @brief อ่านเวลาปัจจุบันในหน่วย milliseconds
 * @return จำนวน ms ตั้งแต่เริ่มระบบ (overflow ทุก ~49.7 วัน)
 * @note alias ของ Get_CurrentMs()
 */
#define millis()            Get_CurrentMs()

/**
 * @brief อ่านเวลาปัจจุบันในหน่วย microseconds
 * @return จำนวน us ตั้งแต่เริ่มระบบ (overflow ทุก ~71.6 นาที)
 * @note alias ของ Get_CurrentUs()
 */
#define micros()            Get_CurrentUs()

/**
 * @brief หน่วงเวลาแบบ blocking ในหน่วย milliseconds
 * @param ms จำนวน ms ที่ต้องการหน่วง
 * @note alias ของ Delay_Ms()
 */
#define delay(ms)           Delay_Ms(ms)

/**
 * @brief หน่วงเวลาแบบ blocking ในหน่วย microseconds
 * @param us จำนวน us ที่ต้องการหน่วง
 * @note alias ของ Delay_Us()
 */
#define delayMicroseconds(us) Delay_Us(us)

/* ========== Interrupt Aliases ========== */

/**
 * @brief เปิด global interrupt (enable IRQ)
 * @note alias ของ __enable_irq()
 */
#define interrupts()        __enable_irq()

/**
 * @brief ปิด global interrupt (disable IRQ)
 * @note alias ของ __disable_irq()
 */
#define noInterrupts()      __disable_irq()

/* ========== Pin Interrupt Mapping ========== */

/**
 * @brief แปลง GPIO pin number เป็น EXTI interrupt line
 * @param pin หมายเลข pin (PA1-PA2, PC0-PC7, PD2-PD7)
 * @return EXTI line number (0-7), หรือ 0xFF ถ้า pin ไม่รองรับ interrupt
 *
 * @example
 * attachInterrupt(digitalPinToInterrupt(PC1), my_isr, FALLING);
 *
 * @note CH32V003 มี EXTI 8 lines (0-7) แชร์กันทุก pins
 */
uint8_t digitalPinToInterrupt(uint8_t pin);

/* ========== Random Number Generator ========== */

/**
 * @brief ตั้งค่า seed สำหรับ random()
 * @param seed ค่าเริ่มต้นสำหรับ PRNG
 *
 * @example
 * randomSeed(analogRead(PA2));  // ใช้ ADC noise เป็น seed
 */
void randomSeed(uint32_t seed);

/**
 * @brief สุ่มตัวเลขในช่วง [0, max-1]
 * @param max ค่าสูงสุด (ไม่รวม)
 * @return ตัวเลขสุ่มในช่วง [0, max-1]
 *
 * @note ใช้ _randomMax(max) แทน random(max) เพราะ stdlib.h มี long random(void) อยู่แล้ว
 *       ตอน compile C จึงใช้ชื่อ random โดยตรงไม่ได้
 *
 * @example
 * uint8_t r = _randomMax(10);  // ได้ค่า 0-9
 */
long _randomMax(long max);

/**
 * @brief สุ่มตัวเลขในช่วง [min, max-1]
 * @param min ค่าต่ำสุด (รวม)
 * @param max ค่าสูงสุด (ไม่รวม)
 * @return ตัวเลขสุ่มในช่วง [min, max-1]
 *
 * @example
 * uint8_t r = _randomRange(10, 20);  // ได้ค่า 10-19
 */
long _randomRange(long min, long max);

/**
 * @brief (Optional) Arduino-style random(max) / random(min,max) macro
 * @note ปิดไว้เป็นค่าเริ่มต้น เพราะ stdlib.h มี `long random(void)` อยู่แล้ว —
 *       ถ้าไฟล์นี้ include stdlib.h ด้วยจะชื่อชนกัน เปิดใช้เฉพาะไฟล์ที่ไม่ต้อง
 *       ใช้ stdlib random() โดย #define ENABLE_ARDUINO_RANDOM_MACRO ก่อน include
 *
 * @example
 * #define ENABLE_ARDUINO_RANDOM_MACRO
 * #include "SimpleArduino.h"
 * ...
 * uint8_t r1 = random(10);       // [0, 9]   -> _randomMax(10)
 * uint8_t r2 = random(10, 20);   // [10, 19] -> _randomRange(10, 20)
 */
#ifdef ENABLE_ARDUINO_RANDOM_MACRO
  #define _RANDOM_PICK(_1, _2, NAME, ...) NAME
  #define random(...) _RANDOM_PICK(__VA_ARGS__, _randomRange, _randomMax)(__VA_ARGS__)
#endif

/* ========== yield() — Cooperative Multitasking ========== */

/**
 * @brief ให้ CPU ทำงานอื่น (cooperative multitasking)
 *
 * ใช้ใน busy loop หรือ delay เพื่อให้ระบบทำงานพื้นหลัง:
 * - IWDG_Feed() (ถ้า IWDG ถูก init แล้ว)
 * - เตรียมรองรับ USART background processing, timer callbacks
 *
 * @example
 * while (1) {
 *     if (Is_Timer_Expired(&my_timer)) {
 *         digitalToggle(PC0);
 *     }
 *     yield();  // ให้ระบบทำงานพื้นหลัง
 * }
 */
void yield(void);

/**
 * @brief แจ้ง SimpleArduino ว่า IWDG ถูกเปิดแล้ว — yield() จะเรียก IWDG_Feed()
 *        ถูกเรียกอัตโนมัติจาก SimpleIWDG ตอน init
 */
void arduino_SetIWDGActive(void);

/* ========== dtostrf() — Float to String (Lightweight) ========== */

/**
 * @brief แปลง float/double เป็น string (ไม่พึ่ง sprintf)
 * @param val ค่าที่ต้องการแปลง
 * @param width ความกว้างต่ำสุดของ string (pad ด้วย space ทางซ้าย)
 * @param precision จำนวนตำแหน่งทศนิยม
 * @param buf output buffer (ต้องมีขนาดเพียงพอ)
 * @return pointer ไปยัง buf
 *
 * @note ขนาด buffer ที่แนะนำ: width + 2 bytes
 * @note ใช้ integer arithmetic ล้วน ไม่ต้องใช้ FPU
 *
 * @example
 * char buf[16];
 * dtostrf(3.14159, 6, 2, buf);  // "  3.14"
 * USART_Print(buf);
 */
char* dtostrf(double val, int width, unsigned int precision, char* buf);

/* ========== USART Print Extensions (Optional) ========== */

#if !defined(ENABLE_USART_PRINTLN)
#define ENABLE_USART_PRINTLN  0
#endif

#if !defined(ENABLE_USART_PRINTFLOAT)
#define ENABLE_USART_PRINTFLOAT 0
#endif

#if ENABLE_USART_PRINTLN
/**
 * @brief ส่งข้อความ string ตามด้วย newline
 * @param str ข้อความที่ต้องการส่ง
 * @note ต่อ "\r\n" อัตโนมัติ
 */
void USART_Println(const char* str);

/**
 * @brief ส่งตัวเลขตามด้วย newline
 * @param num ตัวเลข (signed 32-bit)
 */
void USART_PrintlnNum(int32_t num);

/**
 * @brief ส่งเลขฐาน 16 ตามด้วย newline
 * @param num ตัวเลข (unsigned 32-bit)
 * @param uppercase ใช้ตัวพิมพ์ใหญ่ (1) หรือเล็ก (0)
 */
void USART_PrintlnHex(uint32_t num, uint8_t uppercase);
#endif

#if ENABLE_USART_PRINTFLOAT
/**
 * @brief ส่ง float/decimal number
 * @param val ค่าที่ต้องการส่ง
 * @param decimal_places จำนวนตำแหน่งทศนิยม
 */
void USART_PrintFloat(float val, uint8_t decimal_places);

#if ENABLE_USART_PRINTLN
/**
 * @brief ส่ง float/decimal number ตามด้วย newline
 * @param val ค่าที่ต้องการส่ง
 * @param decimal_places จำนวนตำแหน่งทศนิยม
 */
void USART_PrintlnFloat(float val, uint8_t decimal_places);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SIMPLE_ARDUINO_H */
