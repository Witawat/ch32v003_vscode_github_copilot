/**
 * @file MAX31855.c
 * @brief MAX31855 / MAX6675 Thermocouple Implementation
 */

#include "MAX31855.h"

/* ========== Private ========== */

static uint32_t _spi_read32(MAX31855_Instance* therm) {
    uint8_t buf[4] = {0};
    digitalWrite(therm->cs_pin, LOW);
    SPI_TransferBuffer(NULL, buf, 4);
    digitalWrite(therm->cs_pin, HIGH);
    return ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) |
           ((uint32_t)buf[2] << 8)  | (uint32_t)buf[3];
}

static uint16_t _spi_read16(MAX31855_Instance* therm) {
    uint8_t buf[2] = {0};
    digitalWrite(therm->cs_pin, LOW);
    SPI_TransferBuffer(NULL, buf, 2);
    digitalWrite(therm->cs_pin, HIGH);
    return ((uint16_t)buf[0] << 8) | buf[1];
}

/* ========== Public API ========== */

void MAX31855_Init(MAX31855_Instance* therm, uint8_t cs_pin) {
    if (therm == NULL) return;

    therm->cs_pin       = cs_pin;
    therm->initialized  = 0;

    pinMode(cs_pin, PIN_MODE_OUTPUT);
    digitalWrite(cs_pin, HIGH);
    Delay_Ms(1);

    therm->initialized = 1;
}

float MAX31855_ReadTemp(MAX31855_Instance* therm) {
    if (therm == NULL || !therm->initialized) return 0.0f;

    uint32_t raw = _spi_read32(therm);

    if (raw & 0x08) {
        /* Fault condition, return error indicator */
        return 0.0f;
    }

    int16_t temp_raw = (int16_t)((raw >> 18) & 0x3FFF);

    if (temp_raw & 0x2000) {
        temp_raw |= 0xC000;
    }

    return (float)temp_raw * 0.25f;
}

float MAX31855_ReadInternalTemp(MAX31855_Instance* therm) {
    if (therm == NULL || !therm->initialized) return 0.0f;

    uint32_t raw = _spi_read32(therm);
    int16_t temp_raw = (int16_t)((raw >> 4) & 0xFFF);

    if (temp_raw & 0x800) {
        temp_raw |= 0xF000;
    }

    return (float)temp_raw * 0.0625f;
}

MAX31855_Status MAX31855_GetFault(MAX31855_Instance* therm, uint8_t* fault) {
    if (therm == NULL || !therm->initialized || fault == NULL) return MAX31855_ERROR;

    uint32_t raw = _spi_read32(therm);
    *fault = (uint8_t)(raw & 0x0F);

    if (*fault & MAX31855_FAULT_OC)  return MAX31855_FAULT_OC;
    if (*fault & MAX31855_FAULT_SCG) return MAX31855_FAULT_SCG;
    if (*fault & MAX31855_FAULT_SCV) return MAX31855_FAULT_SCV;

    return MAX31855_OK;
}

bool MAX31855_IsThermocoupleOpen(MAX31855_Instance* therm) {
    uint8_t fault;
    MAX31855_GetFault(therm, &fault);
    return (fault & MAX31855_FAULT_OC) != 0;
}

bool MAX31855_IsShortedToGND(MAX31855_Instance* therm) {
    uint8_t fault;
    MAX31855_GetFault(therm, &fault);
    return (fault & MAX31855_FAULT_SCG) != 0;
}

bool MAX31855_IsShortedToVCC(MAX31855_Instance* therm) {
    uint8_t fault;
    MAX31855_GetFault(therm, &fault);
    return (fault & MAX31855_FAULT_SCV) != 0;
}

float MAX6675_ReadTemp(MAX31855_Instance* therm) {
    if (therm == NULL || !therm->initialized) return 0.0f;

    uint16_t raw = _spi_read16(therm);

    if (raw & 0x04) {
        /* Thermocouple open fault */
        return 0.0f;
    }

    int16_t temp_raw = (int16_t)((raw >> 3) & 0xFFF);

    if (temp_raw & 0x800) {
        temp_raw |= 0xF000;
    }

    return (float)temp_raw * 0.25f;
}
