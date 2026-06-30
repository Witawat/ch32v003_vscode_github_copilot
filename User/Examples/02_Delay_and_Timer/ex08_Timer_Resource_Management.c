/**
 * @example ex08_Timer_Resource_Management.c
 * @brief หลีกเลี่ยง Timer Resource Conflict — เมื่อ TIM1/TIM2 ถูกใช้โดย PWM
 *
 * @details
 * CH32V003 มี 2 hardware timers:
 *   TIM1 — ใช้โดย SimplePWM (PWM1_CH1-CH4) หรือ SimpleTIM
 *   TIM2 — ใช้โดย SimplePWM (PWM2_CH1-CH4) หรือ SimpleTIM หรือ SimpleTIM_Ext
 *
 * ถ้าใช้ PWM1_CH1 (TIM1) และต้องการ timer interrupt สำหรับงานอื่น:
 *   → ใช้ SysTick (SimpleDelay) แทน TIM2
 *   → ใช้ Timer_t (Start_Timer + Is_Timer_Expired) สำหรับ non-blocking timing
 *
 * ตัวอย่างนี้:
 *   TIM1 → PWM LED fade (auto hardware)
 *   SysTick → Non-blocking timer สำหรับ USART print ทุก 2 วิ
 *   SysTick → Non-blocking timer สำหรับ LED toggle ทุก 0.5 วิ
 *
 * ไม่มีการใช้ TIM2 เลย — หลีกเลี่ยง conflict 100%
 *
 * ============================================================
 * แผนผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit(115200)"]
 *     C --> D["pinMode(PC0, OUTPUT)"]
 *     D --> E["PWM_Init(PWM1_CH1, 1000)"]
 *     E --> F["PWM_Start(PWM1_CH1)"]
 *     F --> G["Start_Timer x3 (print, led, pwm)"]
 *     G --> H["while(1)"]
 *     H --> I{"Is_Timer_Expired(pwm)?"}
 *     I -->|"Yes"| J["pwm_val += dir, check bounds"]
 *     J --> K["PWM_SetDutyCycle(pwm_val)"]
 *     I -->|"No"| L{"Is_Timer_Expired(led)?"}
 *     K --> L
 *     L -->|"Yes"| M["digitalToggle(PC0)"]
 *     L -->|"No"| N{"Is_Timer_Expired(print)?"}
 *     M --> N
 *     N -->|"Yes"| O["Print PWM% status"]
 *     N -->|"No"| P["yield()"]
 *     O --> P
 *     P --> H
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(PC0, PIN_MODE_OUTPUT);

    // TIM1 → PWM LED fade (ใช้ TIM1 hardware)
    PWM_Init(PWM1_CH1, 1000);  // PD2, 1kHz
    PWM_Start(PWM1_CH1);

    // SysTick → Non-blocking timers (ไม่ใช้ TIM1/TIM2 เลย!)
    Timer_t print_timer;
    Timer_t led_timer;
    Timer_t pwm_timer;

    Start_Timer(&print_timer, 2000, 1);  // ทุก 2 วิ — แจ้งสถานะ
    Start_Timer(&led_timer, 500, 1);     // ทุก 0.5 วิ — LED toggle
    Start_Timer(&pwm_timer, 50, 1);      // ทุก 50ms — PWM step

    uint8_t pwm_val = 0;
    int8_t pwm_dir = 5;

    USART_Print("\r\n=== Timer Resource Management ===\r\n");
    USART_Print("TIM1 = PWM (PD2 fade)\r\n");
    USART_Print("SysTick = 3 non-blocking timers\r\n");
    USART_Print("TIM2 = free (available for user)\r\n\r\n");

    while (1) {
        // PWM fade — ใช้ SysTick timing แทน TIM2 interrupt
        if (Is_Timer_Expired(&pwm_timer)) {
            pwm_val += pwm_dir;
            if (pwm_val >= 100 || pwm_val <= 0) pwm_dir = -pwm_dir;
            PWM_SetDutyCycle(PWM1_CH1, pwm_val);
        }

        // LED toggle
        if (Is_Timer_Expired(&led_timer)) {
            digitalToggle(PC0);
        }

        // Status print
        if (Is_Timer_Expired(&print_timer)) {
            USART_Print("PWM=");
            USART_PrintNum(pwm_val);
            USART_Print("%  TIM2=free  SysTick=OK\r\n");
        }

        // yield() — เรียก IWDG feed ถ้าใช้ IWDG
        yield();
    }
}
