/**
 * @file SoftUART.c
 * @brief Software UART Implementation
 */

#include "SoftUART.h"
#include <string.h>
#include <stdio.h>

/* ========== Private Helpers ========== */

static void _delay_bit(SoftUART_Instance* uart) {
    /* bit_time_us = 1,000,000 / baud */
    uint32_t delay = uart->bit_time_us;
    while (delay > 1000) {
        Delay_Us(1000);
        delay -= 1000;
    }
    if (delay > 0) Delay_Us(delay);
}

/* ========== Public API ========== */

SoftUART_Status SoftUART_Init(SoftUART_Instance* uart, uint8_t tx_pin, uint8_t rx_pin, uint32_t baud) {
    if (uart == NULL) return SOFTUART_ERROR;

    uart->tx_pin  = tx_pin;
    uart->rx_pin  = rx_pin;
    uart->baud    = baud;
    uart->bit_time_us = 1000000 / baud;
    uart->rx_head = 0;
    uart->rx_tail = 0;
    uart->rx_count = 0;

    pinMode(tx_pin, PIN_MODE_OUTPUT);
    pinMode(rx_pin, PIN_MODE_INPUT_PULLUP);

    digitalWrite(tx_pin, HIGH);
    Delay_Ms(1);

    uart->initialized = 1;
    return SOFTUART_OK;
}

SoftUART_Status SoftUART_SetBaud(SoftUART_Instance* uart, uint32_t baud) {
    if (uart == NULL || baud == 0) return SOFTUART_ERROR;
    if (baud == 0) return SOFTUART_ERROR;
    uart->baud        = baud;
    uart->bit_time_us = 1000000 / baud;
    return SOFTUART_OK;
}

void SoftUART_WriteByte(SoftUART_Instance* uart, uint8_t data) {
    if (uart == NULL || !uart->initialized) return;

    __disable_irq();

    digitalWrite(uart->tx_pin, LOW);
    _delay_bit(uart);

    for (uint8_t i = 0; i < 8; i++) {
        digitalWrite(uart->tx_pin, (data >> i) & 1);
        _delay_bit(uart);
    }

    digitalWrite(uart->tx_pin, HIGH);
    _delay_bit(uart);

    __enable_irq();
}

SoftUART_Status SoftUART_ReadByte(SoftUART_Instance* uart, uint8_t* data, uint32_t timeout_ms) {
    if (uart == NULL || !uart->initialized || data == NULL) return SOFTUART_ERROR;

    /* Check buffer first */
    if (uart->rx_count > 0) {
        *data = uart->rx_buffer[uart->rx_tail];
        uart->rx_tail = (uart->rx_tail + 1) % SOFTUART_RX_BUF_SIZE;
        uart->rx_count--;
        return SOFTUART_OK;
    }

    /* Polling read: wait for start bit (falling edge) */
    uint32_t start = Get_CurrentMs();

    while (digitalRead(uart->rx_pin) == HIGH) {
        if (ELAPSED_TIME(start, Get_CurrentMs()) >= timeout_ms) {
            return SOFTUART_TIMEOUT;
        }
    }

    /* Wait half bit time to center sampling */
    Delay_Us(uart->bit_time_us / 2);

    uint8_t byte = 0;
    for (uint8_t i = 0; i < 8; i++) {
        _delay_bit(uart);
        byte |= (digitalRead(uart->rx_pin) << i);
    }

    /* Wait for stop bit */
    _delay_bit(uart);

    *data = byte;
    return SOFTUART_OK;
}

uint16_t SoftUART_Available(SoftUART_Instance* uart) {
    if (uart == NULL || !uart->initialized) return 0;
    return uart->rx_count;
}

SoftUART_Status SoftUART_Flush(SoftUART_Instance* uart) {
    if (uart == NULL) return SOFTUART_ERROR;
    uart->rx_head = 0;
    uart->rx_tail = 0;
    uart->rx_count = 0;
    return SOFTUART_OK;
}

void SoftUART_Write(SoftUART_Instance* uart, const uint8_t* data, uint16_t len) {
    if (uart == NULL || !uart->initialized || data == NULL) return;
    for (uint16_t i = 0; i < len; i++) {
        SoftUART_WriteByte(uart, data[i]);
    }
}

void SoftUART_WriteString(SoftUART_Instance* uart, const char* str) {
    if (uart == NULL || !uart->initialized || str == NULL) return;
    while (*str) {
        SoftUART_WriteByte(uart, (uint8_t)*str);
        str++;
    }
}

void SoftUART_Printf(SoftUART_Instance* uart, const char* format, ...) {
    if (uart == NULL || !uart->initialized || format == NULL) return;

    char buf[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    SoftUART_WriteString(uart, buf);
}
