/**
 * @file I2CScan.c
 * @brief I2C Address Scanner Library Implementation
 * @version 1.0
 * @date 2026-04-25
 */

#include "I2CScan.h"
#include <ch32v00x_i2c.h>

/* ========== Private Helpers ========== */

/**
 * @brief รอ I2C flag พร้อม timeout แบบ custom (ไม่ขึ้นกับ SimpleI2C)
 * @return 1 ถ้า event เกิดขึ้น, 0 ถ้า timeout
 */
static uint8_t _WaitFlag(uint32_t event, uint32_t timeout_us) {
    while (!I2C_CheckEvent(I2C1, event)) {
        if (timeout_us-- == 0) return 0;
        Delay_Us(1);
    }
    return 1;
}

/**
 * @brief รอจนกว่า bus จะว่าง พร้อม timeout
 * @return 1 ถ้า bus ว่าง, 0 ถ้า timeout
 */
static uint8_t _WaitBusFree(uint32_t timeout_us) {
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) {
        if (timeout_us-- == 0) return 0;
        Delay_Us(1);
    }
    return 1;
}

/* ========== Public Functions ========== */

/**
 * @brief Probe I2C address เพียง address เดียว
 */
uint8_t I2C_Probe(uint8_t addr) {
    uint32_t timeout_us = (uint32_t)I2C_SCAN_TIMEOUT_MS * 1000U;
    uint8_t found = 0;

    /* 1. รอ bus ว่าง */
    if (!_WaitBusFree(timeout_us)) {
        return 0;
    }

    /* 2. ส่ง START */
    I2C_GenerateSTART(I2C1, ENABLE);
    if (!_WaitFlag(I2C_EVENT_MASTER_MODE_SELECT, timeout_us)) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        return 0;
    }

    /* 3. ส่ง address พร้อม Write bit */
    I2C_Send7bitAddress(I2C1, (uint8_t)(addr << 1), I2C_Direction_Transmitter);

    /* 4. รอ ACK หรือ NACK */
    timeout_us = (uint32_t)I2C_SCAN_TIMEOUT_MS * 1000U;
    while (timeout_us--) {
        /* ACK ได้รับ — device มีอยู่ */
        if (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
            found = 1;
            break;
        }
        /* NACK ได้รับ — ไม่มี device */
        if (I2C_GetFlagStatus(I2C1, I2C_FLAG_AF)) {
            I2C_ClearFlag(I2C1, I2C_FLAG_AF);
            found = 0;
            break;
        }
        Delay_Us(1);
    }

    /* 5. ส่ง STOP เสมอ */
    I2C_GenerateSTOP(I2C1, ENABLE);

    /* รอ STOP เสร็จสิ้น */
    _WaitBusFree((uint32_t)I2C_SCAN_TIMEOUT_MS * 1000U);

    return found;
}

/* ========== Private Print Helpers ========== */

/** ส่งตัวเลข hex 2 หลัก (lowercase) */
static void _PrintHex2(uint8_t val) {
    const char hex[] = "0123456789abcdef";
    char buf[3];
    buf[0] = hex[(val >> 4) & 0x0F];
    buf[1] = hex[val & 0x0F];
    buf[2] = '\0';
    I2C_SCAN_PRINT(buf);
}

/* ========== Scan Function ========== */

/**
 * @brief Scan I2C bus และแสดงผลเป็นตารางผ่าน USART
 */
void I2CScan_Run(void) {
    uint8_t found_count = 0;

    /* Header */
    I2C_SCAN_PRINT("I2C_SCAN: Starting I2C scan...\r\n");
    I2C_SCAN_PRINT("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");

    /* Scan address 0x00–0x7F */
    for (uint8_t row = 0; row < 8; row++) {
        /* Row label: 00:, 10:, ... , 70: */
        _PrintHex2((uint8_t)(row << 4));
        I2C_SCAN_PRINT(": ");

        for (uint8_t col = 0; col < 16; col++) {
            uint8_t addr = (uint8_t)((row << 4) | col);

            /* Skip reserved addresses (0x00–0x07 และ 0x78–0x7F) */
            if (addr < 0x08 || addr > 0x77) {
                I2C_SCAN_PRINT("   ");
                continue;
            }

            if (I2C_Probe(addr)) {
                /* แสดง address ที่พบ */
                I2C_SCAN_PRINT(" ");
                _PrintHex2(addr);
                found_count++;
            } else {
#if I2C_SCAN_SHOW_EMPTY
                I2C_SCAN_PRINT(" --");
#else
                I2C_SCAN_PRINT("   ");
#endif
            }
        }

        I2C_SCAN_PRINT("\r\n");
    }

    /* Footer */
    I2C_SCAN_PRINT("Scan finished. Found ");
    I2C_SCAN_PRINT_NUM((int32_t)found_count);
    I2C_SCAN_PRINT(" device(s).\r\n");
}
