/**
 * @example ex09_Error_Handling.c
 * @brief สาธิตการตรวจสอบ Pin และ Error Handling — ใช้ IS_VALID_PIN, IS_ADC_PIN, IS_PWM_PIN
 *
 * @details
 * SimpleHAL ตรวจสอบ pin 2 ระดับ:
 * 1. Compile-time: analogRead/analogWrite ใช้ _Static_assert — ใช้ pin ผิด → compile error
 * 2. Runtime: pinMode/digitalWrite คืนค่าเงียบถ้า pin ไม่มีในแพ็กเกจ
 *
 * วิธีตรวจจับ:
 * - ใช้ IS_VALID_PIN() ก่อน pinMode
 * - ใช้ IS_ADC_PIN() ก่อน analogRead
 * - ใช้ IS_PWM_PIN() ก่อน analogWrite
 *
 * ตัวอย่างนี้เลือกใช้ SOP-8 — เปลี่ยน CH32V003_PACKAGE เพื่อดูผลต่าง
 */

#define CH32V003_PACKAGE  PACKAGE_SOP8
#include "SimpleHAL.h"

static uint8_t safe_pinMode(uint8_t pin, GPIO_PinMode mode) {
    if (!IS_VALID_PIN(pin)) {
        USART_Print("  [SKIP] pinMode(");
        USART_PrintNum(pin);
        USART_Print(") — pin not available\r\n");
        return 0;
    }
    pinMode(pin, mode);
    return 1;
}

static uint8_t safe_analogRead(uint8_t pin) {
    if (!IS_ADC_PIN(pin)) {
        USART_Print("  [SKIP] analogRead(");
        USART_PrintNum(pin);
        USART_Print(") — not ADC pin\r\n");
        return 0;
    }
    // ใช้ _analogRead_impl() โดยตรง (ตัวแปร pin → runtime validation)
    uint16_t val = _analogRead_impl(pin);
    USART_Print("  [OK]   analogRead(");
    USART_PrintNum(pin);
    USART_Print(") = ");
    USART_PrintNum(val);
    USART_Print("\r\n");
    return val;
}

static void safe_analogWrite(uint8_t pin, uint8_t val) {
    if (!IS_PWM_PIN(pin)) {
        USART_Print("  [SKIP] analogWrite(");
        USART_PrintNum(pin);
        USART_Print(") — not PWM pin\r\n");
        return;
    }
    _analogWrite_impl(pin, val);
    USART_Print("  [OK]   analogWrite(");
    USART_PrintNum(pin);
    USART_Print(", ");
    USART_PrintNum(val);
    USART_Print(")\r\n");
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

#if CH32V003_IS_SOP8
    USART_Print("\r\n=== Error Handling Test [SOP-8 — 6 pins] ===\r\n\r\n");
#else
    USART_Print("\r\n=== Error Handling Test ===\r\n\r\n");
#endif

    // 1. pinMode — SOP-8 มี PD1,PD4,PD5,PD6,PC1,PC2
    USART_Print("--- pinMode Validation ---\r\n");
    safe_pinMode(PD5, PIN_MODE_OUTPUT);  // OK — SOP-8 มี
    safe_pinMode(PD6, PIN_MODE_OUTPUT);  // OK
    safe_pinMode(PA1, PIN_MODE_OUTPUT);  // SKIP — SOP-8 ไม่มี
    safe_pinMode(PD0, PIN_MODE_OUTPUT);  // SKIP — SOP-8/SOP-16 ไม่มี
    safe_pinMode(PC4, PIN_MODE_OUTPUT);  // SKIP — SOP-8 ไม่มี

    // 2. ADC — SOP-8 มี PD4,PD5,PD6 (3 channels)
    USART_Print("\r\n--- ADC Pin Validation ---\r\n");
    ADC_SimpleInit();  // เปิดเฉพาะ channels ที่มีในแพ็กเกจนี้
    safe_analogRead(PD5);   // OK
    safe_analogRead(PD7);   // SKIP — PD7 ไม่ใช่ ADC pin!
    safe_analogRead(PC0);   // SKIP — PC0 ไม่ใช่ ADC

    // 3. PWM — SOP-8 มี PWM1_CH1(PD2) + PWM2_CH1(PD4) เท่านั้น
    USART_Print("\r\n--- PWM Pin Validation ---\r\n");
    if (IS_PWM_VALID_PACKAGE(PWM1_CH1)) {
        USART_Print("  [OK]   PWM1_CH1 (PD2) — available\r\n");
        PWM_Init(PWM1_CH1, 1000);
        PWM_Start(PWM1_CH1);
        PWM_SetDutyCycle(PWM1_CH1, 50);
    }
    if (IS_PWM_VALID_PACKAGE(PWM2_CH1)) {
        USART_Print("  [OK]   PWM2_CH1 (PD4) — available\r\n");
        PWM_Init(PWM2_CH1, 1000);
        PWM_Start(PWM2_CH1);
        PWM_SetDutyCycle(PWM2_CH1, 25);
    }
    if (!IS_PWM_VALID_PACKAGE(PWM1_CH2)) {
        USART_Print("  [SKIP] PWM1_CH2 (PA1) — not available on this package\r\n");
    }

    // 4. analogWrite — runtime validation
    USART_Print("\r\n--- analogWrite Validation ---\r\n");
    safe_analogWrite(PA1, 128);  // OK — PA1 รองรับ PWM
    safe_analogWrite(PA2, 128);  // SKIP
    safe_analogWrite(PD7, 255);  // OK
    safe_analogWrite(PC1, 100);  // SKIP

    USART_Print("\r\n=== Done! Change CH32V003_PACKAGE and rebuild ===\r\n");

    while (1) { Delay_Ms(1000); }
}
