/**
 * @file AHT10.h
 * @brief AHT10/AHT20 Temperature & Humidity Sensor Library สำหรับ CH32V003
 * @version 1.0
 * @date 2026-06-22
 *
 * @details
 * Library สำหรับอ่านอุณหภูมิและความชื้นจาก AHT10/AHT20 ผ่าน I2C
 * รองรับทั้ง AHT10 และ AHT20 (ใช้คำสั่งเดียวกัน)
 *
 * **คุณสมบัติ:**
 * - อุณหภูมิ: -40 to +85°C, ±0.3°C accuracy
 * - ความชื้น: 0 to 100% RH, ±2% accuracy (AHT20: ±2%, AHT10: ±3%)
 * - ความละเอียด: 14-bit (temp), 12-bit (humidity)
 *
 * **Hardware Connection:**
 * ```
 *   CH32V003          AHT10/AHT20
 *   PC2 (SCL) ──────> SCL  + 4.7kΩ pull-up
 *   PC1 (SDA) <─────> SDA  + 4.7kΩ pull-up
 *   3.3V ───────────> VCC
 *   GND ────────────> GND
 * ```
 *
 * @example
 * #include "AHT10.h"
 *
 * AHT10_Instance sensor;
 *
 * int main(void) {
 *     SystemCoreClockUpdate();
 *     Timer_Init();
 *     I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
 *
 *     if (AHT10_Init(&sensor) == AHT10_OK) {
 *         while (1) {
 *             float temp, hum;
 *             if (AHT10_Read(&sensor, &temp, &hum) == AHT10_OK) {
 *                 printf("T=%.1fC H=%.1f%%\r\n", temp, hum);
 *             }
 *             Delay_Ms(1000);
 *         }
 *     }
 * }
 *
 * @note ต้อง init I2C ก่อน (I2C_SimpleInit)
 * @note I2C address fixed: 0x38
 */

#ifndef __AHT10_H
#define __AHT10_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../SimpleHAL/SimpleHAL.h"
#include <stdint.h>
#include <stdbool.h>

/* ========== I2C Address ========== */

#define AHT10_ADDR  0x38

/* ========== Status ========== */

typedef enum {
    AHT10_OK            = 0,
    AHT10_ERROR_I2C     = 1,
    AHT10_ERROR_PARAM   = 2,
    AHT10_ERROR_BUSY    = 3,
    AHT10_ERROR_CRC     = 4
} AHT10_Status;

/* ========== Instance ========== */

typedef struct {
    uint8_t i2c_addr;
    uint8_t initialized;
    uint8_t version;   /* 0 = unknown, 10 = AHT10, 20 = AHT20 */
} AHT10_Instance;

/* ========== Function Prototypes ========== */

AHT10_Status AHT10_Init(AHT10_Instance* sensor);

AHT10_Status AHT10_Read(AHT10_Instance* sensor, float* temperature, float* humidity);

AHT10_Status AHT10_SoftReset(AHT10_Instance* sensor);

AHT10_Status AHT10_GetStatus(AHT10_Instance* sensor, uint8_t* status);

bool AHT10_IsCalibrated(AHT10_Instance* sensor);

bool AHT10_IsBusy(AHT10_Instance* sensor);

#ifdef __cplusplus
}
#endif

#endif /* __AHT10_H */
