/**
 * @example ex04_SPI_Remap.c
 * @brief สาธิต SPI Pin Remap (เปลี่ยน pin ของ SPI hardware)
 *
 * @details
 * CH32V003 มี SPI1 ที่เปลี่ยน pin ได้ 2 แบบ:
 * - SPI_PINS_DEFAULT: SCK=PC5, MISO=PC7, MOSI=PC6, NSS=PC4
 * - SPI_PINS_REMAP  : SCK=PD1, MISO=PD2, MOSI=PD3, NSS=PD0
 *
 * ข้อจำกัดต่อแพ็กเกจ:
 * - SOP-8: ใช้ Hardware SPI ไม่ได้เลย (#if !CH32V003_IS_SOP8 ปิดทั้งไฟล์)
 *          ต้องใช้ shiftOut/shiftIn (software SPI) แทน
 * - SOP-16: ใช้ได้เฉพาะ SPI_PINS_DEFAULT (PC4-7)
 * - TSSOP-20/QFN-20: ใช้ได้ทั้ง 2 แบบ
 *
 * ตัวอย่างนี้ทดสอบ SPI loopback (ต่อ MISO-MOSI เข้าหากัน)
 * และสาธิตการสลับระหว่าง pin config
 *
 * @note ปิดการทำงานทั้งหมดบน SOP-8
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include "SimpleHAL.h"

static void test_spi_config(SPI_PinConfig config, const char* name) {
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, config);

    USART_Print("\r\n=== SPI [");
    USART_Print(name);
    USART_Print("] ===\r\n");

    // Loopback test: ส่ง 0x55 แล้วอ่านกลับ (ต้องต่อ MISO กับ MOSI)
    SPI_SetCS(0);
    uint8_t rx = SPI_Transfer(0x55);
    SPI_SetCS(1);

    USART_Print("  Sent: 0x55, Received: 0x");
    USART_PrintHex(rx, 1);
    if (rx == 0x55) {
        USART_Print("\r\n  Loopback OK!\r\n");
    } else {
        USART_Print("\r\n  Loopback FAIL (connect MISO-MOSI?)\r\n");
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    pinMode(PC0, PIN_MODE_OUTPUT);

    // 1. Default pins: PC5-7 + PC4
    test_spi_config(SPI_PINS_DEFAULT, "DEFAULT PC5-7");
    Delay_Ms(500);

#if CH32V003_HAS_PD0
    // 2. Remap: PD1-3 + PD0 (เฉพาะ TSSOP-20/QFN-20)
    test_spi_config(SPI_PINS_REMAP, "REMAP  PD0-3");
    Delay_Ms(500);
#else
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    USART_Print("\r\n=== SPI REMAP [SKIP - PD0 not available on this package] ===\r\n");
#endif

    USART_Print("\r\nSPI remap test complete!\r\n");

    while (1) {
        digitalToggle(PC0);
        Delay_Ms(1000);
    }
}
