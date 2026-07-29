/**
 * @example ex06_PWM_Remap.c
 * @brief สาธิต PWM Remap API — ตั้งค่ารีแมปล่วงหน้า + analogWrite() auto-init
 *
 * @details
 * PWM_InitRemap() ใช้เปลี่ยน pin ของ PWM output ได้
 * PWM_SetRemap() เก็บค่ารีแมปไว้ แล้ว PWM_Write()/analogWrite() ใช้ตอน auto-init
 *
 * Remap options:
 * - PWM_REMAP_NONE    : default pins — ใช้งานได้เต็มรูปแบบ
 * - PWM_REMAP_PARTIAL1: ⛔ ปิดใช้งาน (no-op ตั้งแต่ v2.1) — ได้ default pin เหมือนไม่ตั้ง remap
 * - PWM_REMAP_PARTIAL2: ⛔ ปิดใช้งาน (no-op ตั้งแต่ v2.1) — ได้ default pin เหมือนไม่ตั้ง remap
 *
 * @warning v2.1: PARTIAL1/PARTIAL2 ถูกปิดใช้งานแล้ว เพราะ pin mapping ของ TIM1/TIM2 partial
 *          remap บน CH32V003 ยังไม่เคยถูกยืนยันกับ datasheet/reference manual — เดิมโค้ดตั้งค่า
 *          AFIO remap register โดยไม่อัปเดต GPIO pin ให้ตรงปลายทางจริง ทำให้สัญญาณ PWM หายทั้ง
 *          จากพิน default และพินที่คิดว่า remap ไปแบบเงียบๆ ตัวอย่างนี้ยัง compile และรันได้ปกติ
 *          แต่ทุก channel ด้านล่างจะได้ pin ตาม default (PWM1_CH1=PD2, PWM2_CH1=PD4) เสมอ
 *          ไม่ว่าจะเรียก PWM_SetRemap/PWM_InitRemap ด้วย remap ใดก็ตาม
 *
 * วิธีใช้งาน:
 *   1. PWM_SetRemap(channel, remap)   — ตั้งรีแมปล่วงหน้า
 *   2. PWM_Write(channel, value)      — auto-init @1kHz + stored remap
 *   หรือ
 *   1. PWM_InitRemap(channel, freq, remap) — init พร้อม remap ทันที
 *   2. PWM_Start(channel) + PWM_SetDutyCycle(channel, percent)
 *
 * @warning PWM_REMAP_FULL ถูกลบออกแล้ว (ใช้พอร์ท PE/PB ที่ไม่มีใน CH32V003)
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    pinMode(PC0, PIN_MODE_OUTPUT);

#if !CH32V003_IS_SOP8
    // === วิธีที่ 1: PWM_SetRemap + PWM_Write (ง่ายสุด!) ===
    // ตั้งรีแมปก่อน แล้ว PWM_Write auto-init ให้เอง
    PWM_SetRemap(PWM1_CH1, PWM_REMAP_PARTIAL1);
    PWM_Write(PWM1_CH1, 128);  // 50% duty @ 1kHz + PARTIAL1 remap
    PWM_Start(PWM1_CH1);

    Delay_Ms(2000);
    PWM_SetDutyCycle(PWM1_CH1, 25);  // ลดเหลือ 25%
    Delay_Ms(2000);
    PWM_Stop(PWM1_CH1);

    // === วิธีที่ 2: PWM_InitRemap + PWM_SetDutyCycle (Manual) ===
    PWM_InitRemap(PWM2_CH1, 1000, PWM_REMAP_PARTIAL2);
    PWM_Start(PWM2_CH1);
    PWM_SetDutyCycle(PWM2_CH1, 75);  // 75% duty
    Delay_Ms(2000);
    PWM_SetDutyCycle(PWM2_CH1, 0);
    PWM_Stop(PWM2_CH1);
#endif

    // === Package-safe: ใช้ channels ที่มีในทุกแพ็กเกจ ===
    if (IS_PWM_VALID_PACKAGE(PWM1_CH1)) {
        PWM_Init(PWM1_CH1, 1000);
        PWM_Start(PWM1_CH1);
    }
    if (IS_PWM_VALID_PACKAGE(PWM2_CH1)) {
        PWM_Init(PWM2_CH1, 1000);
        PWM_Start(PWM2_CH1);
    }

    // Fade LED บน channels ที่ใช้ได้
    while (1) {
        digitalToggle(PC0);
        for (uint8_t i = 0; i <= 100; i += 5) {
            if (IS_PWM_VALID_PACKAGE(PWM1_CH1))
                PWM_SetDutyCycle(PWM1_CH1, i);
            if (IS_PWM_VALID_PACKAGE(PWM2_CH1))
                PWM_SetDutyCycle(PWM2_CH1, 100 - i);
            Delay_Ms(50);
        }
    }
}
