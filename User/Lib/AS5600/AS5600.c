/**
 * @file AS5600.c
 * @brief AS5600 Magnetic Rotary Encoder Implementation
 */

#include "AS5600.h"

/* ========== Private Helpers ========== */

static uint16_t _read_reg16(uint8_t dev_addr, uint8_t reg_hi) {
    uint8_t reg = reg_hi;
    if (I2C_Write(dev_addr, &reg, 1) != I2C_OK) return 0xFFFF;
    uint8_t buf[2];
    if (I2C_Read(dev_addr, buf, 2) != I2C_OK) return 0xFFFF;
    return ((uint16_t)buf[0] << 8) | buf[1];
}

/* ========== Public API ========== */

AS5600_Status AS5600_Init(AS5600_Instance* enc) {
    if (enc == NULL) return AS5600_ERROR_PARAM;

    enc->i2c_addr     = AS5600_ADDR;
    enc->zero_position = 0;
    enc->max_position  = 0xFFF;
    enc->initialized   = 0;

    if (!I2C_IsDeviceReady(enc->i2c_addr)) {
        return AS5600_ERROR_I2C;
    }

    enc->initialized = 1;
    return AS5600_OK;
}

uint16_t AS5600_ReadAngle(AS5600_Instance* enc) {
    if (enc == NULL || !enc->initialized) return 0;
    return _read_reg16(enc->i2c_addr, AS5600_REG_ANGLE_HI) & 0x0FFF;
}

float AS5600_ReadAngleDegrees(AS5600_Instance* enc) {
    uint16_t angle = AS5600_ReadAngle(enc);
    return (float)angle * 360.0f / 4096.0f;
}

uint16_t AS5600_ReadRawAngle(AS5600_Instance* enc) {
    if (enc == NULL || !enc->initialized) return 0;
    return _read_reg16(enc->i2c_addr, AS5600_REG_RAW_HI) & 0x0FFF;
}

AS5600_Status AS5600_ReadStatus(AS5600_Instance* enc, uint8_t* status) {
    if (enc == NULL || !enc->initialized || status == NULL) return AS5600_ERROR_PARAM;

    uint8_t reg = AS5600_REG_STATUS;
    if (I2C_Write(enc->i2c_addr, &reg, 1) != I2C_OK) return AS5600_ERROR_I2C;
    if (I2C_Read(enc->i2c_addr, status, 1) != I2C_OK) return AS5600_ERROR_I2C;

    return AS5600_OK;
}

AS5600_MagnetStrength AS5600_GetMagnetStrength(AS5600_Instance* enc) {
    if (enc == NULL || !enc->initialized) return AS5600_MAGNET_ERROR;

    uint8_t status = 0;
    if (AS5600_ReadStatus(enc, &status) != AS5600_OK) {
        return AS5600_MAGNET_ERROR;
    }

    if (status & AS5600_STATUS_MAGNET_TOO_WEAK)  return AS5600_MAGNET_WEAK;
    if (status & AS5600_STATUS_MAGNET_TOO_STRONG) return AS5600_MAGNET_STRONG;
    if (status & AS5600_STATUS_MAGNET_DETECTED)   return AS5600_MAGNET_OK;

    return AS5600_MAGNET_ERROR;
}

uint8_t AS5600_ReadAGC(AS5600_Instance* enc) {
    if (enc == NULL || !enc->initialized) return 0;
    return I2C_ReadReg(enc->i2c_addr, AS5600_REG_AGC);
}

uint16_t AS5600_ReadMagnitude(AS5600_Instance* enc) {
    if (enc == NULL || !enc->initialized) return 0;
    return _read_reg16(enc->i2c_addr, AS5600_REG_MAGN_HI) & 0x0FFF;
}

AS5600_Status AS5600_SetStartPosition(AS5600_Instance* enc, uint16_t angle) {
    if (enc == NULL || !enc->initialized) return AS5600_ERROR_PARAM;
    if (angle > 0xFFF) return AS5600_ERROR_PARAM;

    uint8_t data[2] = {(uint8_t)(angle >> 8), (uint8_t)(angle & 0xFF)};

    if (I2C_WriteRegMulti(enc->i2c_addr, AS5600_REG_ZPOS_HI, data, 2) != I2C_OK) {
        return AS5600_ERROR_I2C;
    }

    enc->zero_position = angle;
    return AS5600_OK;
}

AS5600_Status AS5600_SetEndPosition(AS5600_Instance* enc, uint16_t angle) {
    if (enc == NULL || !enc->initialized) return AS5600_ERROR_PARAM;
    if (angle > 0xFFF) return AS5600_ERROR_PARAM;

    uint8_t data[2] = {(uint8_t)(angle >> 8), (uint8_t)(angle & 0xFF)};

    if (I2C_WriteRegMulti(enc->i2c_addr, AS5600_REG_MPOS_HI, data, 2) != I2C_OK) {
        return AS5600_ERROR_I2C;
    }

    enc->max_position = angle;
    return AS5600_OK;
}

AS5600_Status AS5600_BurnAngle(AS5600_Instance* enc) {
    if (enc == NULL || !enc->initialized) return AS5600_ERROR_PARAM;

    /* Write 0x80 to BURN register to burn ZPOS/MPOS to OTP */
    uint8_t cmd = 0x80;
    if (I2C_WriteReg(enc->i2c_addr, 0xFF, cmd) != I2C_OK) {
        return AS5600_ERROR_I2C;
    }

    Delay_Ms(10);
    return AS5600_OK;
}
