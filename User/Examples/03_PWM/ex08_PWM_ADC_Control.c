/**
 * @example ex08_PWM_ADC_Control.c
 * @brief Cross-module: ADC (Potentiometer) → PWM (LED Brightness)
 *
 * @details
 * อ่านค่า ADC จาก potentiometer (PD2) → แปลง 0-1023 เป็น 0-100% → PWM LED (PC0)
 * แสดงค่าผ่าน USART แบบ real-time
 *
 * ต่อวงจร:
 *   Potentiometer: VCC → POT → GND, wiper → PD2
 *   LED: PC0 → 220Ω → LED → GND
 *   USART: PD5=TX, PD6=RX
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    // เริ่มต้น PWM และ ADC
    PWM_Init(PWM2_CH3, 1000);  // PC0, 1kHz
    PWM_Start(PWM2_CH3);
    ADC_SimpleInit();

    USART_Print("\r\n=== ADC → PWM Control ===\r\n");
    USART_Print("Turn potentiometer to adjust LED brightness\r\n");
    USART_Print("ADC(0-1023) → Duty(0-100%) → PWM(PC0)\r\n\r\n");

    while (1) {
        // อ่าน ADC จาก PD2 (potentiometer)
        uint16_t adc = analogRead(PD2);  // 0-1023

        // แปลงเป็น duty cycle (0-100%)
        uint8_t duty = (uint8_t)(((uint32_t)adc * 100) / 1023);

        // ตั้ง PWM
        PWM_SetDutyCycle(PWM2_CH3, duty);

        // แสดงผล
        USART_Print("ADC: ");
        USART_PrintNum(adc);
        USART_Print(" -> Duty: ");
        USART_PrintNum(duty);
        USART_Print("%\r\n");

        Delay_Ms(100);  // 10Hz update
    }
}

/**
 * ============================================================
 * แผนผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit(115200)"]
 *     C --> D["PWM_Init(PWM2_CH3, 1000)"]
 *     D --> E["PWM_Start(PWM2_CH3)"]
 *     E --> F["ADC_SimpleInit()"]
 *     F --> G["Print header"]
 *     G --> H["while(1)"]
 *     H --> I["analogRead(PD2)"]
 *     I --> J["แปลง ADC 0-1023 -> duty 0-100%"]
 *     J --> K["PWM_SetDutyCycle(duty)"]
 *     K --> L["Print ADC + Duty"]
 *     L --> M["Delay_Ms(100)"]
 *     M --> H
 * ============================================================
 */
