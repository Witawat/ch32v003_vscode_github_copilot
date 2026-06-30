/**
 * @example ex06_USART_Remap.c
 * @brief สาธิตการเปลี่ยน Pin Configuration ของ USART ด้วย Remap
 *
 * @details
 * CH32V003 มี USART1 ที่สามารถเปลี่ยน pin TX/RX ได้ 3+1 แบบ:
 * - USART_PINS_DEFAULT   : TX=PD5, RX=PD6 (ใช้ได้ทุกแพ็กเกจ)
 * - USART_PINS_REMAP1    : TX=PD0, RX=PD1 (TSSOP-20/QFN-20 เท่านั้น)
 * - USART_PINS_REMAP2    : TX=PD6, RX=PD5 (ใช้ได้ทุกแพ็กเกจ)
 * - USART_PINS_FULL_REMAP: TX=PD6, RX=PD5 (รวมบิต remap ทั้งสอง)
 *
 * ตัวอย่างนี้สาธิตการเปลี่ยน pin แบบ runtime และส่งข้อความทดสอบ
 *
 * @note USART_PINS_REMAP1 ต้องใช้ PD0 ซึ่งไม่มีใน SOP-8/SOP-16
 *       ถ้าใช้แพ็กเกจ SOP-8/SOP-16 จะเกิด #error ตอนคอมไพล์
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include "SimpleHAL.h"

static void test_pin_config(USART_PinConfig config, const char* name) {
    USART_SimpleInit(BAUD_115200, config);
    USART_Print("\r\n=== ");
    USART_Print(name);
    USART_Print(" ===\r\n");
    USART_Print("CH32V003 USART Remap Test - OK\r\n");
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    pinMode(PC0, PIN_MODE_OUTPUT);

    // 1. Default pins: TX=PD5, RX=PD6
    test_pin_config(USART_PINS_DEFAULT, "DEFAULT  TX=PD5 RX=PD6");
    Delay_Ms(500);

    // 2. Remap2: TX=PD6, RX=PD5 (สลับขั้ว)
    test_pin_config(USART_PINS_REMAP2, "REMAP2   TX=PD6 RX=PD5");
    Delay_Ms(500);

#if CH32V003_HAS_PD0
    // 3. Remap1: TX=PD0, RX=PD1 (เฉพาะ TSSOP-20/QFN-20 มี PD0)
    test_pin_config(USART_PINS_REMAP1, "REMAP1   TX=PD0 RX=PD1");
    Delay_Ms(500);
#endif

    // 4. Full Remap: TX=PD6, RX=PD5
    test_pin_config(USART_PINS_FULL_REMAP, "FULL     TX=PD6 RX=PD5");
    Delay_Ms(500);

    USART_Print("\r\n\r\nAll remap tests complete!\r\n");

    while (1) {
        digitalToggle(PC0);
        Delay_Ms(1000);
    }
}
