/**
 * @example ex07_USART_Package_Aware.c
 * @brief สาธิตการเขียนโค้ดที่รองรับทุกแพ็กเกจ — ใช้ CH32V003_PACKAGE
 *
 * @details
 * เปลี่ยนแค่ #define CH32V003_PACKAGE แล้ว rebuild — โค้ดปรับอัตโนมัติ!
 *
 * แพ็กเกจต่าง ๆ:
 * - SOP-8  (PACKAGE_SOP8):    USART_PINS_DEFAULT, USART_PINS_REMAP2
 * - SOP-16 (PACKAGE_SOP16):   USART_PINS_DEFAULT, USART_PINS_REMAP2
 * - TSSOP-20/QFN-20:          ใช้ได้ทุก pin config
 *
 * SOP-8/SOP-16 ไม่มี PD0 → USART_PINS_REMAP1 จะเกิด #error ตอนคอมไพล์
 */

// เปลี่ยนบรรทัดนี้เป็นแพ็กเกจที่คุณใช้:
#define CH32V003_PACKAGE  PACKAGE_TSSOP20
// #define CH32V003_PACKAGE  PACKAGE_SOP16
// #define CH32V003_PACKAGE  PACKAGE_SOP8

#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    // เลือก pin config ตามแพ็กเกจ
#if CH32V003_HAS_PD0
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    USART_Print("Package: TSSOP20/QFN20 — all pin configs available\r\n");
    USART_Print("USART_PINS_DEFAULT : TX=PD5 RX=PD6\r\n");
    USART_Print("USART_PINS_REMAP1  : TX=PD0 RX=PD1\r\n");
    USART_Print("USART_PINS_REMAP2  : TX=PD6 RX=PD5\r\n");
    USART_Print("USART_FULL_REMAP   : TX=PD6 RX=PD5\r\n");
#else
    USART_SimpleInit(BAUD_115200, USART_PINS_REMAP2);
    USART_Print("Package: SOP8/SOP16 — no PD0, using REMAP2\r\n");
    USART_Print("USART_PINS_DEFAULT : TX=PD5 RX=PD6\r\n");
    USART_Print("USART_PINS_REMAP2  : TX=PD6 RX=PD5\r\n");
    USART_Print("(USART_PINS_REMAP1 requires PD0 — not available)\r\n");
#endif

#if CH32V003_IS_SOP8
    USART_Print("\r\nNote: SPI HW and I2C HW not available on SOP-8.\r\n");
    USART_Print("Use shiftOut/shiftIn and SimpleI2C_Soft instead.\r\n");
#elif CH32V003_IS_SOP16
    USART_Print("\r\nNote: SPI HW and I2C HW available with DEFAULT pins only.\r\n");
    USART_Print("SPI_PINS_REMAP and I2C_PINS_REMAP require PD0.\r\n");
#endif

    pinMode(PC0, PIN_MODE_OUTPUT);
    while (1) {
        USART_Print(".");
        digitalToggle(PC0);
        Delay_Ms(1000);
    }
}
