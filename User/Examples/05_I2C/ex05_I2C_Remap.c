/**
 * @example ex05_I2C_Remap.c
 * @brief สาธิตการเปลี่ยน Pin Configuration ของ I2C ด้วย Remap
 *
 * @details
 * CH32V003 มี I2C1 ที่เปลี่ยน pin ได้ 3 แบบ:
 * - I2C_PINS_DEFAULT  : SCL=PC2, SDA=PC1 (default)
 * - I2C_PINS_PARTIAL_REMAP : SCL=PC2, SDA=PC1 (partial, pin mapping ต้องตรวจ datasheet)
 * - I2C_PINS_REMAP    : SCL=PD0, SDA=PD1 (TSSOP-20/QFN-20 เท่านั้น)
 *
 * ตัวอย่าง: สแกน I2C bus ด้วย config ที่ต่างกัน
 *
 * @note I2C_PINS_REMAP ต้องใช้ PD0 ซึ่งไม่มีใน SOP-8/SOP-16 → #error
 *       ถ้าใช้แพ็กเกจเล็ก ให้ใช้ SimpleI2C_Soft แทน
 * @note ต้องต่อ pull-up resistor 4.7kΩ ที่ SDA และ SCL
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include "SimpleHAL.h"

static void scan_bus(I2C_PinConfig config, const char* name) {
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    I2C_SimpleInit(I2C_100KHZ, config);

    USART_Print("\r\n=== Scan I2C [");
    USART_Print(name);
    USART_Print("] ===\r\n");

    uint8_t devices[16];
    uint8_t count = I2C_Scan(devices, 16);

    if (count == 0) {
        USART_Print("  No devices found (check pull-up resistors)\r\n");
    } else {
        for (uint8_t i = 0; i < count; i++) {
            USART_Print("  Found: 0x");
            USART_PrintHex(devices[i], 1);
            USART_Print("\r\n");
        }
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    pinMode(PC0, PIN_MODE_OUTPUT);

    // 1. Default pins: PC2=SCL, PC1=SDA
    scan_bus(I2C_PINS_DEFAULT, "DEFAULT PC2/PC1");
    Delay_Ms(500);

    // 2. Partial Remap
    scan_bus(I2C_PINS_PARTIAL_REMAP, "PARTIAL PC2/PC1");
    Delay_Ms(500);

#if CH32V003_HAS_PD0
    // 3. Full Remap: PD0=SCL, PD1=SDA (เฉพาะ TSSOP-20/QFN-20)
    scan_bus(I2C_PINS_REMAP, "FULL   PD0/PD1");
    Delay_Ms(500);
#endif

    USART_Print("\r\nI2C remap test complete!\r\n");

    while (1) {
        digitalToggle(PC0);
        Delay_Ms(1000);
    }
}
