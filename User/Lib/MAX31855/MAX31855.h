/**
 * @file MAX31855.h
 * @brief MAX31855 / MAX6675 Thermocouple Library สำหรับ CH32V003
 * @version 1.0
 * @date 2026-06-22
 *
 * @details
 * Library สำหรับอ่านอุณหภูมิจาก thermocouple ผ่าน MAX31855 หรือ MAX6675
 * ผ่าน SPI
 *
 * **คุณสมบัติ:**
 * - รองรับ MAX31855 (K/J/N/T/S/E/B type thermocouple)
 * - รองรับ MAX6675 (K-type thermocouple)
 * - อ่าน internal temperature (cold-junction compensation)
 * - ตรวจสอบ fault (open circuit, short to GND, short to VCC)
 *
 * **MAX31855 Specifications:**
 * - Resolution: 0.25°C (thermocouple), 0.0625°C (internal)
 * - Range: -270°C to +1800°C (ขึ้นกับ thermocouple type)
 * - SPI interface, 14-bit signed temperature data
 *
 * **MAX6675 Specifications:**
 * - Resolution: 0.25°C
 * - Range: 0°C to +1024°C
 * - SPI interface, 12-bit signed temperature data
 *
 * **Hardware Connection:**
 * ```
 *   CH32V003          MAX31855
 *   PC5 (SCK)  ──────> SCK
 *   PC7 (MISO) <────── SO   (Slave Out = data to MCU)
 *   PC6 (MOSI) ─────── n.c. (ไม่ต้องต่อ)
 *   GPIO_Pin  ────────> CS   (user-defined chip select)
 *   3.3V ────────────> VCC
 *   GND ─────────────> GND
 * ```
 *
 * @example
 * #include "MAX31855.h"
 *
 * MAX31855_Instance therm;
 *
 * int main(void) {
 *     SystemCoreClockUpdate();
 *     Timer_Init();
 *     SPI_SimpleInit(SPI_MODE1, SPI_2MHZ, SPI_PINS_DEFAULT);
 *
 *     MAX31855_Init(&therm, PC0);  // CS = PC0
 *
 *     while (1) {
 *         float temp = MAX31855_ReadTemp(&therm);
 *         float internal = MAX31855_ReadInternalTemp(&therm);
 *
 *         uint8_t fault;
 *         MAX31855_GetFault(&therm, &fault);
 *
 *         printf("TC: %.2fC  Int: %.2fC  Fault: 0x%02X\r\n",
 *                temp, internal, fault);
 *         Delay_Ms(500);
 *     }
 * }
 *
 * @note ต้อง init SPI ก่อน
 * @note MAX31855 ใช้ SPI Mode 0 (CPOL=0, CPHA=0)
 * @note MAX6675 ใช้ SPI Mode 0 หรือ 1 ขืนกับรุ่น
 */

#ifndef __MAX31855_H
#define __MAX31855_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../SimpleHAL/SimpleHAL.h"
#include <stdint.h>
#include <stdbool.h>

/* ========== Status ========== */

typedef enum {
    MAX31855_OK               = 0,
    MAX31855_ERROR            = 1,
    /* Distinct names from the MAX31855_FAULT_* bitmask macros below —
     * previously these enum members shared names with the macros, so the
     * macros silently won and MAX31855_STATUS_FAULT_OC (0x01) became
     * indistinguishable from MAX31855_ERROR (see LIB_AUDIT.md #4) */
    MAX31855_STATUS_FAULT_OC  = 2,
    MAX31855_STATUS_FAULT_SCG = 3,
    MAX31855_STATUS_FAULT_SCV = 4
} MAX31855_Status;

/* ========== Fault Bits ========== */

#define MAX31855_FAULT_OC  0x01
#define MAX31855_FAULT_SCG 0x02
#define MAX31855_FAULT_SCV 0x04
#define MAX31855_FAULT_ANY 0x08

/* ========== Instance ========== */

typedef struct {
    uint8_t cs_pin;
    uint8_t initialized;
} MAX31855_Instance;

/* ========== Function Prototypes ========== */

void MAX31855_Init(MAX31855_Instance* therm, uint8_t cs_pin);

float MAX31855_ReadTemp(MAX31855_Instance* therm);

float MAX31855_ReadInternalTemp(MAX31855_Instance* therm);

MAX31855_Status MAX31855_GetFault(MAX31855_Instance* therm, uint8_t* fault);

bool MAX31855_IsThermocoupleOpen(MAX31855_Instance* therm);

bool MAX31855_IsShortedToGND(MAX31855_Instance* therm);

bool MAX31855_IsShortedToVCC(MAX31855_Instance* therm);

float MAX6675_ReadTemp(MAX31855_Instance* therm);

#ifdef __cplusplus
}
#endif

#endif /* __MAX31855_H */
