/**
 * @file AHT10.c
 * @brief AHT10/AHT20 Temperature & Humidity Sensor Implementation
 */

#include "AHT10.h"

#define AHT10_CMD_INIT      0xBE
#define AHT10_CMD_TRIGGER   0xAC
#define AHT10_CMD_SOFTRESET 0xBA
#define AHT10_CMD_STATUS    0x71

#define AHT10_STATUS_BUSY      0x01
#define AHT10_STATUS_CALIBRATE 0x08

static AHT10_Status _send_cmd(AHT10_Instance* sensor, uint8_t cmd, uint8_t* data, uint8_t len) {
    if (len == 0) {
        return (I2C_Write(sensor->i2c_addr, &cmd, 1) == I2C_OK)
               ? AHT10_OK : AHT10_ERROR_I2C;
    }

    uint8_t buf[8];
    buf[0] = cmd;
    for (uint8_t i = 0; i < len; i++) buf[1 + i] = data[i];

    return (I2C_Write(sensor->i2c_addr, buf, 1 + len) == I2C_OK)
           ? AHT10_OK : AHT10_ERROR_I2C;
}

AHT10_Status AHT10_Init(AHT10_Instance* sensor) {
    if (sensor == NULL) return AHT10_ERROR_PARAM;

    sensor->i2c_addr    = AHT10_ADDR;
    sensor->initialized = 0;
    sensor->version     = 0;

    /* Soft reset first */
    AHT10_SoftReset(sensor);
    Delay_Ms(20);

    /* Check if device responds */
    if (!I2C_IsDeviceReady(sensor->i2c_addr)) {
        return AHT10_ERROR_I2C;
    }

    /* Send init command */
    uint8_t init_data[2] = {0x08, 0x00};
    if (_send_cmd(sensor, AHT10_CMD_INIT, init_data, 2) != AHT10_OK) {
        return AHT10_ERROR_I2C;
    }
    Delay_Ms(10);

    sensor->initialized = 1;
    return AHT10_OK;
}

AHT10_Status AHT10_Read(AHT10_Instance* sensor, float* temperature, float* humidity) {
    if (sensor == NULL || !sensor->initialized) return AHT10_ERROR_PARAM;
    if (temperature == NULL && humidity == NULL) return AHT10_ERROR_PARAM;

    /* Trigger measurement */
    uint8_t trig_data[2] = {0x33, 0x00};
    if (_send_cmd(sensor, AHT10_CMD_TRIGGER, trig_data, 2) != AHT10_OK) {
        return AHT10_ERROR_I2C;
    }

    /* Wait for measurement (typical 80ms, max 100ms) */
    Delay_Ms(80);

    /* Check busy status with retry */
    for (uint8_t retry = 0; retry < 10; retry++) {
        uint8_t status = 0;
        if (I2C_ReadRegMulti(sensor->i2c_addr, 0x00, &status, 1) != I2C_OK) {
            return AHT10_ERROR_I2C;
        }
        if (!(status & AHT10_STATUS_BUSY)) break;
        Delay_Ms(10);
    }

    /* Read 6 bytes from sensor */
    uint8_t buf[6] = {0};
    if (I2C_Read(sensor->i2c_addr, buf, 6) != I2C_OK) {
        return AHT10_ERROR_I2C;
    }

    /* Check calibration bit */
    if (!(buf[0] & AHT10_STATUS_CALIBRATE)) {
        return AHT10_ERROR_I2C;
    }

    /* Calculate humidity (20-bit) */
    if (humidity != NULL) {
        uint32_t hum_raw = ((uint32_t)buf[1] << 12) | ((uint32_t)buf[2] << 4) | ((uint32_t)buf[3] >> 4);
        *humidity = (float)hum_raw * 100.0f / 0x100000;
        if (*humidity > 100.0f) *humidity = 100.0f;
        if (*humidity < 0.0f) *humidity = 0.0f;
    }

    /* Calculate temperature (20-bit) */
    if (temperature != NULL) {
        uint32_t temp_raw = (((uint32_t)buf[3] & 0x0F) << 16) | ((uint32_t)buf[4] << 8) | (uint32_t)buf[5];
        *temperature = (float)temp_raw * 200.0f / 0x100000 - 50.0f;
    }

    return AHT10_OK;
}

AHT10_Status AHT10_SoftReset(AHT10_Instance* sensor) {
    if (sensor == NULL) return AHT10_ERROR_PARAM;

    uint8_t cmd = AHT10_CMD_SOFTRESET;
    I2C_Write(sensor->i2c_addr, &cmd, 1);
    Delay_Ms(20);

    return AHT10_OK;
}

AHT10_Status AHT10_GetStatus(AHT10_Instance* sensor, uint8_t* status) {
    if (sensor == NULL || status == NULL) return AHT10_ERROR_PARAM;

    uint8_t cmd = AHT10_CMD_STATUS;
    if (I2C_Write(sensor->i2c_addr, &cmd, 1) != I2C_OK) {
        return AHT10_ERROR_I2C;
    }
    Delay_Ms(1);

    if (I2C_Read(sensor->i2c_addr, status, 1) != I2C_OK) {
        return AHT10_ERROR_I2C;
    }

    return AHT10_OK;
}

bool AHT10_IsCalibrated(AHT10_Instance* sensor) {
    if (sensor == NULL || !sensor->initialized) return false;

    uint8_t status = 0;
    if (AHT10_GetStatus(sensor, &status) != AHT10_OK) return false;

    return (status & AHT10_STATUS_CALIBRATE) != 0;
}

bool AHT10_IsBusy(AHT10_Instance* sensor) {
    if (sensor == NULL || !sensor->initialized) return false;

    uint8_t status = 0;
    if (AHT10_GetStatus(sensor, &status) != AHT10_OK) return false;

    return (status & AHT10_STATUS_BUSY) != 0;
}
