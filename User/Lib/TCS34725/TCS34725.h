/**
 * @file TCS34725.h
 * @brief TCS34725 Color Sensor Library สำหรับ CH32V003
 * @version 1.0
 * @date 2026-06-22
 *
 * @details
 * Library สำหรับอ่านค่าสี RGBC (Red, Green, Blue, Clear) จาก TCS34725
 * ผ่าน I2C สามารถคำนวณค่า Lux และ Color Temperature ได้
 *
 * **คุณสมบัติ:**
 * - อ่านค่า RGBC raw data
 * - คำนวณ Lux (ความสว่าง)
 * - คำนวณ Color Temperature (อุณหภูมิสี)
 * - ปรับ gain และ integration time ได้
 * - พร้อมใช้งานสำหรับ CH32V003
 *
 * **Hardware Connection:**
 * ```
 *   CH32V003          TCS34725
 *   PC2 (SCL) ──────> SCL  + 4.7kΩ pull-up
 *   PC1 (SDA) <─────> SDA  + 4.7kΩ pull-up
 *   3.3V ───────────> VCC
 *   GND ────────────> GND
 *   GND ────────────> ADDR  (addr = 0x29)
 *   (VCC ──────────> ADDR   addr = 0x49)
 * ```
 *
 * @example
 * #include "TCS34725.h"
 *
 * TCS34725_Instance color;
 *
 * int main(void) {
 *     SystemCoreClockUpdate();
 *     Timer_Init();
 *     I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
 *
 *     TCS34725_Init(&color, TCS34725_GAIN_4X, TCS34725_INTEG_50MS);
 *
 *     while (1) {
 *         uint16_t r, g, b, c;
 *         TCS34725_ReadRGBC(&color, &r, &g, &b, &c);
 *         float lux = TCS34725_GetLux(&color);
 *         printf("RGB(%u,%u,%u) C=%u Lux=%.1f\r\n", r, g, b, c, lux);
 *         Delay_Ms(500);
 *     }
 * }
 *
 * @note ต้อง init I2C ก่อน
 * @note LED pin (สามารถเปิด/ปิด LED ในตัวได้)
 */

#ifndef __TCS34725_H
#define __TCS34725_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../SimpleHAL/SimpleHAL.h"
#include <stdint.h>
#include <stdbool.h>

/* ========== I2C Address ========== */

#define TCS34725_ADDR      0x29
#define TCS34725_ADDR_ALT  0x49

/* ========== TCS34725 Registers ========== */

#define TCS34725_REG_ENABLE    0x80
#define TCS34725_REG_ATIME     0x81
#define TCS34725_REG_WTIME     0x83
#define TCS34725_REG_CONFIG    0x8D
#define TCS34725_REG_CONTROL   0x8F
#define TCS34725_REG_ID        0x92
#define TCS34725_REG_STATUS    0x93
#define TCS34725_REG_CDATA     0x94
#define TCS34725_REG_RDATA     0x96
#define TCS34725_REG_GDATA     0x98
#define TCS34725_REG_BDATA     0x9A

/* ========== Integration Time (ATIME) ========== */

typedef enum {
    TCS34725_INTEG_2_4MS  = 0xFF,
    TCS34725_INTEG_24MS   = 0xF6,
    TCS34725_INTEG_50MS   = 0xEB,
    TCS34725_INTEG_101MS  = 0xD5,
    TCS34725_INTEG_154MS  = 0xC0,
    TCS34725_INTEG_200MS  = 0xB6,
    TCS34725_INTEG_700MS  = 0x00
} TCS34725_IntegrationTime;

/* ========== Gain ========== */

typedef enum {
    TCS34725_GAIN_1X  = 0x00,
    TCS34725_GAIN_4X  = 0x01,
    TCS34725_GAIN_16X = 0x02,
    TCS34725_GAIN_60X = 0x03
} TCS34725_Gain;

/* ========== Status ========== */

typedef enum {
    TCS34725_OK           = 0,
    TCS34725_ERROR_I2C    = 1,
    TCS34725_ERROR_PARAM  = 2,
    TCS34725_ERROR_ID     = 3
} TCS34725_Status;

/* ========== Instance ========== */

typedef struct {
    uint8_t  i2c_addr;
    TCS34725_IntegrationTime integration_time;
    TCS34725_Gain gain;
    uint16_t atime_cycles;
    float    atime_ms;
    uint8_t  initialized;
} TCS34725_Instance;

/* ========== Function Prototypes ========== */

TCS34725_Status TCS34725_Init(TCS34725_Instance* sensor, TCS34725_Gain gain, TCS34725_IntegrationTime time);

TCS34725_Status TCS34725_SetGain(TCS34725_Instance* sensor, TCS34725_Gain gain);

TCS34725_Status TCS34725_SetIntegrationTime(TCS34725_Instance* sensor, TCS34725_IntegrationTime time);

TCS34725_Status TCS34725_Enable(TCS34725_Instance* sensor);

TCS34725_Status TCS34725_Disable(TCS34725_Instance* sensor);

TCS34725_Status TCS34725_ReadRGBC(TCS34725_Instance* sensor, uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c);

float TCS34725_GetLux(TCS34725_Instance* sensor);

uint16_t TCS34725_GetColorTemp(TCS34725_Instance* sensor);

#ifdef __cplusplus
}
#endif

#endif /* __TCS34725_H */
