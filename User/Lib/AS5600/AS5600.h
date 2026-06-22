/**
 * @file AS5600.h
 * @brief AS5600 Magnetic Rotary Encoder Library สำหรับ CH32V003
 * @version 1.0
 * @date 2026-06-22
 *
 * @details
 * Library สำหรับอ่านตำแหน่งเชิงมุมจาก AS5600 magnetic rotary encoder
 * ผ่าน I2C มีความละเอียด 12-bit (0-4095) หรือ 0.0879 องศา
 *
 * **คุณสมบัติ:**
 * - อ่านมุม 12-bit (0-4095)
 * - อ่านมุมเป็นองศา (0.0-359.9)
 * - ตั้งค่า zero position (ZPOS) และ max position (MPOS)
 * - ตรวจสอบ magnet detection
 * - Automatic gain control (AGC) read
 *
 * **Hardware Connection:**
 * ```
 *   CH32V003          AS5600
 *   PC2 (SCL) ──────> SCL  + 4.7kΩ pull-up
 *   PC1 (SDA) <─────> SDA  + 4.7kΩ pull-up
 *   3.3V ───────────> VCC   (3.0-3.6V)
 *   GND ────────────> GND
 *   --- ────────────> ADDR  (float = addr 0x36)
 * ```
 *
 * @example
 * #include "AS5600.h"
 *
 * AS5600_Instance encoder;
 *
 * int main(void) {
 *     SystemCoreClockUpdate();
 *     Timer_Init();
 *     I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
 *
 *     AS5600_Init(&encoder);
 *
 *     while (1) {
 *         uint16_t angle = AS5600_ReadAngle(&encoder);
 *         float    deg   = AS5600_ReadAngleDegrees(&encoder);
 *         printf("Angle: %u (%0.1f deg)\r\n", angle, deg);
 *         Delay_Ms(100);
 *     }
 * }
 *
 * @note ต้อง init I2C ก่อน
 * @note I2C address: 0x36 (ADDR float) หรือ 0x37 (ADDR GND)
 */

#ifndef __AS5600_H
#define __AS5600_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../SimpleHAL/SimpleHAL.h"
#include <stdint.h>
#include <stdbool.h>

/* ========== I2C Address ========== */

#define AS5600_ADDR         0x36
#define AS5600_ADDR_ALT     0x37

/* ========== AS5600 Registers ========== */

#define AS5600_REG_CONFIG   0x00
#define AS5600_REG_ZPOS_HI  0x01
#define AS5600_REG_ZPOS_LO  0x02
#define AS5600_REG_MPOS_HI  0x03
#define AS5600_REG_MPOS_LO  0x04
#define AS5600_REG_MANG_HI  0x05
#define AS5600_REG_MANG_LO  0x06
#define AS5600_REG_CONF_HI  0x07
#define AS5600_REG_CONF_LO  0x08
#define AS5600_REG_RAW_HI   0x0C
#define AS5600_REG_RAW_LO   0x0D
#define AS5600_REG_ANGLE_HI 0x0E
#define AS5600_REG_ANGLE_LO 0x0F
#define AS5600_REG_STATUS   0x0B
#define AS5600_REG_AGC      0x1A
#define AS5600_REG_MAGN_HI  0x1B
#define AS5600_REG_MAGN_LO  0x1C

/* ========== Status Bits ========== */

#define AS5600_STATUS_MAGNET_DETECTED  0x08
#define AS5600_STATUS_MAGNET_TOO_STRONG 0x10
#define AS5600_STATUS_MAGNET_TOO_WEAK  0x20

/* ========== Status ========== */

typedef enum {
    AS5600_OK            = 0,
    AS5600_ERROR_I2C     = 1,
    AS5600_ERROR_PARAM   = 2,
    AS5600_ERROR_MAGNET  = 3
} AS5600_Status;

/* ========== Magnet Strength ========== */

typedef enum {
    AS5600_MAGNET_WEAK   = 0,
    AS5600_MAGNET_OK     = 1,
    AS5600_MAGNET_STRONG = 2,
    AS5600_MAGNET_ERROR  = 3
} AS5600_MagnetStrength;

/* ========== Instance ========== */

typedef struct {
    uint8_t  i2c_addr;
    uint16_t zero_position;
    uint16_t max_position;
    uint8_t  initialized;
} AS5600_Instance;

/* ========== Function Prototypes ========== */

AS5600_Status AS5600_Init(AS5600_Instance* enc);

uint16_t AS5600_ReadAngle(AS5600_Instance* enc);

float AS5600_ReadAngleDegrees(AS5600_Instance* enc);

uint16_t AS5600_ReadRawAngle(AS5600_Instance* enc);

AS5600_Status AS5600_ReadStatus(AS5600_Instance* enc, uint8_t* status);

AS5600_MagnetStrength AS5600_GetMagnetStrength(AS5600_Instance* enc);

uint8_t AS5600_ReadAGC(AS5600_Instance* enc);

uint16_t AS5600_ReadMagnitude(AS5600_Instance* enc);

AS5600_Status AS5600_SetStartPosition(AS5600_Instance* enc, uint16_t angle);

AS5600_Status AS5600_SetEndPosition(AS5600_Instance* enc, uint16_t angle);

AS5600_Status AS5600_BurnAngle(AS5600_Instance* enc);

#ifdef __cplusplus
}
#endif

#endif /* __AS5600_H */
