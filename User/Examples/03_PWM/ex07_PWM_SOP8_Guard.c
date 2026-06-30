/**
 * @example ex07_PWM_SOP8_Guard.c
 * @brief สาธิตการตรวจสอบแพ็กเกจก่อนใช้ PWM — IS_PWM_VALID_PACKAGE()
 *
 * @details
 * CH32V003 ในแพ็กเกจ SOP-8 มีเพียง 6 user pins
 * PWM channels ที่ใช้ได้บน SOP-8: PWM1_CH1 (PD2) + PWM2_CH1 (PD4) เท่านั้น!
 *
 * ใช้ macro IS_PWM_VALID_PACKAGE(ch) ตรวจสอบก่อน:
 *   - SOP-8  → ใช้ได้แค่ PWM1_CH1, PWM2_CH1
 *   - อื่น ๆ  → ใช้ได้ทุก channel
 *
 * ตัวอย่างนี้รันได้บนทุกแพ็กเกจ — เปลี่ยน CH32V003_PACKAGE แล้ว rebuild
 */

#define CH32V003_PACKAGE  PACKAGE_SOP8    // ลองเปลี่ยนเป็น PACKAGE_SOP16, PACKAGE_TSSOP20
#include "SimplePWM.h"
#include "SimpleDelay.h"
#include "SimpleUSART.h"

static void try_pwm(PWM_Channel ch, const char* name) {
    if (IS_PWM_VALID_PACKAGE(ch)) {
        PWM_Init(ch, 1000);
        PWM_Start(ch);
        PWM_SetDutyCycle(ch, 30);
        USART_Print("  [OK] ");
    } else {
        USART_Print("  [SKIP] ");
    }
    USART_Print(name);
    USART_Print("\r\n");
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

#if CH32V003_IS_SOP8
    USART_Print("\r\n=== PWM on SOP-8 (6 pins) ===\r\n");
    USART_Print("Only PWM1_CH1(PD2) and PWM2_CH1(PD4) available\r\n\r\n");
#else
    USART_Print("\r\n=== PWM (all channels) ===\r\n\r\n");
#endif

    // ลองเปิดทุก channel — IS_PWM_VALID_PACKAGE จะกรองให้
    try_pwm(PWM1_CH1, "PWM1_CH1 (PD2)");
    try_pwm(PWM1_CH2, "PWM1_CH2 (PA1)");
    try_pwm(PWM1_CH3, "PWM1_CH3 (PC3)");
    try_pwm(PWM1_CH4, "PWM1_CH4 (PC4)");
    try_pwm(PWM2_CH1, "PWM2_CH1 (PD4)");
    try_pwm(PWM2_CH2, "PWM2_CH2 (PD3)");
    try_pwm(PWM2_CH3, "PWM2_CH3 (PC0)");
    try_pwm(PWM2_CH4, "PWM2_CH4 (PD7)");

    USART_Print("\r\nDone! Change CH32V003_PACKAGE and rebuild.\r\n");

    while (1) {
        Delay_Ms(1000);
    }
}
