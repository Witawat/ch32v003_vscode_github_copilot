/**
 * @file TCS34725.c
 * @brief TCS34725 Color Sensor Implementation
 */

#include "TCS34725.h"

/* ========== Private Helpers ========== */

static float _calc_atime_ms(TCS34725_IntegrationTime atime) {
    return (256.0f - (float)atime) * 2.4f;
}

static uint16_t _calc_atime_cycles(TCS34725_IntegrationTime atime) {
    return 256 - atime;
}

/* ========== Public API ========== */

TCS34725_Status TCS34725_Init(TCS34725_Instance* sensor, TCS34725_Gain gain, TCS34725_IntegrationTime time) {
    if (sensor == NULL) return TCS34725_ERROR_PARAM;

    sensor->i2c_addr        = TCS34725_ADDR;
    sensor->gain            = gain;
    sensor->integration_time = time;
    sensor->atime_cycles    = _calc_atime_cycles(time);
    sensor->atime_ms        = _calc_atime_ms(time);
    sensor->initialized     = 0;

    if (!I2C_IsDeviceReady(sensor->i2c_addr)) {
        return TCS34725_ERROR_I2C;
    }

    /* Verify device ID */
    uint8_t id = I2C_ReadReg(sensor->i2c_addr, TCS34725_REG_ID);
    if (id != 0x44 && id != 0x4D) {
        return TCS34725_ERROR_ID;
    }

    /* Set integration time */
    I2C_WriteReg(sensor->i2c_addr, TCS34725_REG_ATIME, (uint8_t)time);

    /* Set gain */
    I2C_WriteReg(sensor->i2c_addr, TCS34725_REG_CONTROL, (uint8_t)gain);

    /* Enable device */
    I2C_WriteReg(sensor->i2c_addr, TCS34725_REG_ENABLE, 0x03);  /* PON + AEN */

    Delay_Ms(3);

    sensor->initialized = 1;
    return TCS34725_OK;
}

TCS34725_Status TCS34725_SetGain(TCS34725_Instance* sensor, TCS34725_Gain gain) {
    if (sensor == NULL || !sensor->initialized) return TCS34725_ERROR_PARAM;

    sensor->gain = gain;
    if (I2C_WriteReg(sensor->i2c_addr, TCS34725_REG_CONTROL, (uint8_t)gain) != I2C_OK) {
        return TCS34725_ERROR_I2C;
    }
    return TCS34725_OK;
}

TCS34725_Status TCS34725_SetIntegrationTime(TCS34725_Instance* sensor, TCS34725_IntegrationTime time) {
    if (sensor == NULL || !sensor->initialized) return TCS34725_ERROR_PARAM;

    sensor->integration_time = time;
    sensor->atime_cycles     = _calc_atime_cycles(time);
    sensor->atime_ms         = _calc_atime_ms(time);

    if (I2C_WriteReg(sensor->i2c_addr, TCS34725_REG_ATIME, (uint8_t)time) != I2C_OK) {
        return TCS34725_ERROR_I2C;
    }
    return TCS34725_OK;
}

TCS34725_Status TCS34725_Enable(TCS34725_Instance* sensor) {
    if (sensor == NULL || !sensor->initialized) return TCS34725_ERROR_PARAM;
    I2C_WriteReg(sensor->i2c_addr, TCS34725_REG_ENABLE, 0x03);
    return TCS34725_OK;
}

TCS34725_Status TCS34725_Disable(TCS34725_Instance* sensor) {
    if (sensor == NULL || !sensor->initialized) return TCS34725_ERROR_PARAM;
    I2C_WriteReg(sensor->i2c_addr, TCS34725_REG_ENABLE, 0x00);
    return TCS34725_OK;
}

TCS34725_Status TCS34725_ReadRGBC(TCS34725_Instance* sensor, uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c) {
    if (sensor == NULL || !sensor->initialized) return TCS34725_ERROR_PARAM;
    if (r == NULL && g == NULL && b == NULL && c == NULL) return TCS34725_ERROR_PARAM;

    /* Wait for integration time + margin */
    Delay_Ms((uint32_t)sensor->atime_ms + 10);

    uint8_t buf[8];
    if (I2C_ReadRegMulti(sensor->i2c_addr, TCS34725_REG_CDATA, buf, 8) != I2C_OK) {
        return TCS34725_ERROR_I2C;
    }

    if (c) *c = (uint16_t)buf[1] << 8 | buf[0];
    if (r) *r = (uint16_t)buf[3] << 8 | buf[2];
    if (g) *g = (uint16_t)buf[5] << 8 | buf[4];
    if (b) *b = (uint16_t)buf[7] << 8 | buf[6];

    return TCS34725_OK;
}

float TCS34725_GetLux(TCS34725_Instance* sensor) {
    if (sensor == NULL || !sensor->initialized) return 0.0f;

    uint16_t r, g, b, c;
    if (TCS34725_ReadRGBC(sensor, &r, &g, &b, &c) != TCS34725_OK) {
        return 0.0f;
    }

    if (c == 0) return 0.0f;

    /* Gain compensation */
    float gain_val;
    switch (sensor->gain) {
        case TCS34725_GAIN_4X:  gain_val = 4.0f;  break;
        case TCS34725_GAIN_16X: gain_val = 16.0f; break;
        case TCS34725_GAIN_60X: gain_val = 60.0f; break;
        default:                gain_val = 1.0f;  break;
    }

    /* Integration time compensation */
    float atime_comp = 256.0f / sensor->atime_cycles;

    /* Lux calculation (TAOS formula) */
    float cpl = (atime_comp * gain_val) / 60.0f;
    float lux = ((float)c * 1.0f - (float)r * 1.5f - (float)g * 0.5f) / cpl;

    if (lux < 0.0f) lux = 0.0f;
    return lux;
}

uint16_t TCS34725_GetColorTemp(TCS34725_Instance* sensor) {
    if (sensor == NULL || !sensor->initialized) return 0;

    uint16_t r, g, b, c;
    if (TCS34725_ReadRGBC(sensor, &r, &g, &b, &c) != TCS34725_OK) {
        return 0;
    }

    if (r == 0 || g == 0 || b == 0) return 0;

    /* Color temperature calculation */
    float r_ratio = (float)r / (float)g;
    float b_ratio = (float)b / (float)g;

    float ct = 0.0f;
    if (b_ratio > 0.0f && r_ratio > 0.0f) {
        float x = (r_ratio - b_ratio);
        ct = 449.0f * x * x * x + 3525.0f * x * x + 6823.3f * x + 5520.33f;
    }

    if (ct < 1000.0f) ct = 1000.0f;
    if (ct > 15000.0f) ct = 15000.0f;

    return (uint16_t)ct;
}
