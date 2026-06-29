/**
 * @file SimpleHAL.h
 * @brief Simple Hardware Abstraction Layer สำหรับ CH32V003
 * @version 1.0
 * @date 2025-12-12
 * 
 * @details
 * Header file รวมสำหรับ Simple HAL Library
 * ใช้งานง่ายแบบ Arduino สำหรับ CH32V003
 * 
 * **รองรับทุกรุ่น CH32V003:**
 * - SOP-8  (CH32V003J4M6) — 6 user pins
 * - SOP-16 (CH32V003A4M6) — 14+ pins
 * - TSSOP-20 (CH32V003F4P6) — ครบ 18 pins
 * - QFN-20 (CH32V003F4U6) — ครบ 18 pins
 * 
 * **การเลือกแพ็กเกจ (สำคัญ!):**
 * ต้องกำหนด CH32V003_PACKAGE ก่อน #include:
 *   #define CH32V003_PACKAGE  PACKAGE_SOP8      // SOP-8
 *   #define CH32V003_PACKAGE  PACKAGE_SOP16     // SOP-16
 *   #define CH32V003_PACKAGE  PACKAGE_TSSOP20   // TSSOP-20 (default)
 *   #define CH32V003_PACKAGE  PACKAGE_QFN20     // QFN-20
 * 
 * **Peripherals ที่รองรับ:**
 * - GPIO: การควบคุม digital I/O และ interrupts
 * - TIM: Timer peripherals พร้อม interrupt callbacks
 * - PWM: PWM output สำหรับควบคุม LED, มอเตอร์, servo
 * - USART: การสื่อสารแบบ serial
 * - I2C: การสื่อสารกับ sensors และ EEPROM
 * - SPI: การสื่อสารความเร็วสูง
 * - ADC: การอ่านค่า analog
 * - Flash: Flash memory storage
 * - IWDG: Independent Watchdog (ป้องกันระบบค้าง)
 * - WWDG: Window Watchdog (ตรวจสอบ timing เข้มงวด)
 * - DMA: Direct Memory Access (ถ่ายโอนข้อมูลความเร็วสูง)
 * 
 * **คุณสมบัติหลัก:**
 * - API แบบ Arduino-style
 * - Pin remapping support
 * - Comments ภาษาไทยครบถ้วน
 * - ใช้ Hardware peripheral (เร็วกว่า Software)
 * 
 * @example
 * #define CH32V003_PACKAGE  PACKAGE_TSSOP20
 * #include <SimpleHAL.h>
 * 
 * int main(void) {
 *     SystemCoreClockUpdate();
 *     Timer_Init();
 *     
 *     // USART
 *     USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
 *     USART_Print("Hello!\r\n");
 *     
 *     // I2C
 *     I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
 *     I2C_WriteReg(0x50, 0x00, 0x55);
 *     
 *     // SPI (ใช้ไม่ได้บน SOP-8 — ดู USART_REMAP2 ใช้ shiftOut/shiftIn แทน)
 *     SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);
 *     uint8_t rx = SPI_Transfer(0xAA);
 * }
 * 
 * @author CH32V003 Simple HAL Team
 * @copyright MIT License
 */

#ifndef __SIMPLE_HAL_H
#define __SIMPLE_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========== Package Selection ========== */
/**
 * @brief เลือกแพ็กเกจของ CH32V003 ที่ใช้ (SOP-8 / SOP-16 / TSSOP-20 / QFN-20)
 *
 * @details ต้องกำหนด CH32V003_PACKAGE ก่อน #include <SimpleHAL.h>:
 *
 *   #define CH32V003_PACKAGE  PACKAGE_SOP8      // SOP-8  (6 pins)
 *   #define CH32V003_PACKAGE  PACKAGE_SOP16     // SOP-16 (14+ pins)
 *   #define CH32V003_PACKAGE  PACKAGE_TSSOP20   // TSSOP-20 (18 pins)
 *   #define CH32V003_PACKAGE  PACKAGE_QFN20     // QFN-20 (18 pins)
 *
 * ถ้าไม่กำหนดจะ default เป็น TSSOP20 (ปลอดภัยสุด ครบทุกฟีเจอร์) พร้อมแสดงคำเตือน
 *
 * @note package macros ต้องเป็น #define constants เพราะใช้ใน #if preprocessor
 */
#ifndef CH32V003_PACKAGE
  #define CH32V003_PACKAGE  PACKAGE_TSSOP20
  #warning "CH32V003_PACKAGE not defined, defaulting to PACKAGE_TSSOP20 (20 pins, all features)."
  #warning "Define CH32V003_PACKAGE before #include <SimpleHAL.h>: PACKAGE_SOP8, PACKAGE_SOP16, PACKAGE_TSSOP20, or PACKAGE_QFN20"
#endif

#define PACKAGE_SOP8    1   /**< SOP-8  (CH32V003J4M6) — 6 user pins, no SPI/I2C HW */
#define PACKAGE_SOP16   2   /**< SOP-16 (CH32V003A4M6) — 14+ pins, no PD0 */
#define PACKAGE_TSSOP20 3   /**< TSSOP-20 (CH32V003F4P6) — all pins, all features */
#define PACKAGE_QFN20   4   /**< QFN-20 (CH32V003F4U6) — all pins, all features */

#define CH32V003_IS_SOP8     (CH32V003_PACKAGE == PACKAGE_SOP8)
#define CH32V003_IS_SOP16    (CH32V003_PACKAGE == PACKAGE_SOP16)
#define CH32V003_IS_TSSOP20  (CH32V003_PACKAGE == PACKAGE_TSSOP20)
#define CH32V003_IS_QFN20    (CH32V003_PACKAGE == PACKAGE_QFN20)

#define CH32V003_HAS_PD0         (!CH32V003_IS_SOP8 && !CH32V003_IS_SOP16)
#define CH32V003_HAS_SPI_HW      (!CH32V003_IS_SOP8)


/* ========== Include All SimpleHAL Libraries ========== */

#include <ch32v00x.h>
#include "SimpleUSART.h" // IWYU pragma: keep
#include "SimpleI2C.h" // IWYU pragma: keep
#include "SimpleSPI.h" // IWYU pragma: keep
#include "SimpleADC.h" // IWYU pragma: keep
#include "SimpleArduino.h" // IWYU pragma: keep
#include "SimpleGPIO.h" // IWYU pragma: keep
#include "SimpleTIM.h" // IWYU pragma: keep
#include "SimpleTIM_Ext.h" // IWYU pragma: keep
#include "SimplePWM.h" // IWYU pragma: keep
#include "SimpleOPAMP.h" // IWYU pragma: keep
#include "SimpleFlash.h" // IWYU pragma: keep
#include "SimpleIWDG.h" // IWYU pragma: keep
#include "SimpleWWDG.h" // IWYU pragma: keep
#include "SimpleDelay.h" // IWYU pragma: keep
#include "Simple1Wire.h" // IWYU pragma: keep
#include "SimpleI2C_Soft.h" // IWYU pragma: keep
#include "SimpleDMA.h" // IWYU pragma: keep
#include "SimplePWR.h" // IWYU pragma: keep

/* ========== Version Information ========== */

#define SIMPLE_HAL_VERSION_MAJOR  1
#define SIMPLE_HAL_VERSION_MINOR  9
#define SIMPLE_HAL_VERSION_PATCH  0


/* ========== Helper Macros ========== */

/**
 * @brief Initialize SimpleHAL (Optional)
 * 
 * @note Auto-initialization is already enabled
 *       Call this only if you need explicit control
 * 
 * @details
 * This function is optional because SimpleHAL components
 * auto-initialize themselves (e.g., SimpleDelay uses constructor)
 * 
 * @example
 * int main(void) {
 *     SimpleHAL_Init();  // Optional
 *     Delay_Ms(1000);
 * }
 */
void SimpleHAL_Init(void);

/* ========== Helper Macros ========== */

/**
 * @brief แปลง GPIO port เป็น RCC peripheral
 */
#ifndef GPIO_TO_RCC_PERIPH
#define GPIO_TO_RCC_PERIPH(port) \
    ((port == GPIOA) ? RCC_APB2Periph_GPIOA : \
     (port == GPIOC) ? RCC_APB2Periph_GPIOC : \
     (port == GPIOD) ? RCC_APB2Periph_GPIOD : 0)
#endif

#ifdef __cplusplus
}
#endif

#endif  // __SIMPLE_HAL_H
