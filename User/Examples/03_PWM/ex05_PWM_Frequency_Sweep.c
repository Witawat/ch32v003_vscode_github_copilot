/**
 * ============================================================
 * ตัวอย่างที่ 5: PWM Frequency Sweep (Buzzer)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003                  2N2222
 *     --------                  ------
 *     PD2 ----/\/\/\----------- BASE
 *     PWM1_CH1  1k Ohm           |
 *                              COLLECTOR ----(+)---- VCC (5V)
 *                                            |
 *                                          Piezo
 *                                            |
 *                              EMITTER ------(-)---- GND
 *                                          Buzzer
 *
 *     วงจรขับ Buzzer ดวยทรานซิสเตอร 2N2222
 *     - PWM ที่ Duty 50% ให Square Wave เหมาะที่สุดสำหรับ Buzzer
 *     - ตองใชทรานซิสเตอรเพราะ Buzzer กินกระแสเกิน MCU จายได
 *
 * API ที่ใช้:
 *   PWM_Init()                    // เริ่มต้น PWM
 *   PWM_SetFrequency()            // เปลี่ยนความถี่
 *   PWM_SetDutyCycle(50)          // 50% Duty สำหรับ Square Wave
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   Buzzer จะเปลี่ยนความถี่: 500Hz -> 1kHz -> 2kHz -> 4kHz -> 2kHz -> 1kHz -> 500Hz
 *   แต่ละความถี่ค้าง 1 วินาที
 *   เสียงจะสูงขึ้นและต่ำลงตามความถี่
 * ============================================================
 * คำเตือน (WARNINGS):
 *   - Piezo Buzzer ต้องใช้ทรานซิสเตอร์ 2N2222 ขับเพราะกินกระแสเกิน MCU จ่ายได้
 *   - Duty = 50% ให้สัญญาณ Square Wave ที่ดีที่สุดสำหรับ Buzzer
 *   - PWM_SetFrequency() เปลี่ยนความถี่ทุกช่องใน TIMER เดียวกัน!
 *     (PWM1_CH1 และ PWM1_CH2 อยู่บน TIM1 ดังนั้นจะเปลี่ยนทั้งคู่)
 * - ความถี่ที่มนุษย์ได้ยินประมาณ 20Hz - 20kHz
 * ============================================================
 * แผนผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["PWM_Init(PWM1_CH1, 500)"]
 *     C --> D["PWM_SetDutyCycle(50)"]
 *     D --> E["PWM_Start(PWM1_CH1)"]
 *     E --> F["while(1)"]
 *     F --> G["PWM_SetFrequency(500)"]
 *     G --> H["Delay_Ms(1000)"]
 *     H --> I["PWM_SetFrequency(1000)"]
 *     I --> J["Delay_Ms(1000)"]
 *     J --> K["PWM_SetFrequency(2000)"]
 *     K --> L["Delay_Ms(1000)"]
 *     L --> M["PWM_SetFrequency(4000)"]
 *     M --> N["Delay_Ms(1000)"]
 *     N --> O["PWM_SetFrequency(2000)"]
 *     O --> P["Delay_Ms(1000)"]
 *     P --> Q["PWM_SetFrequency(1000)"]
 *     Q --> R["Delay_Ms(1000)"]
 *     R --> S["PWM_SetFrequency(500)"]
 *     S --> T["Delay_Ms(1000)"]
 *     T --> F
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

int main(void)
{
    // อัปเดตความถี่ระบบ
    SystemCoreClockUpdate();

    Timer_Init();
    // เริ่มต้น PWM ช่อง 1 ที่ PD2 ความถี่ 500Hz
    PWM_Init(PWM1_CH1, 500);

    // ตั้งค่า Duty Cycle 50% สำหรับ Square Wave
    PWM_SetDutyCycle(PWM1_CH1, 50);

    // เริ่ม PWM
    PWM_Start(PWM1_CH1);

    // วนรอบไม่สิ้นสุด
    while (1)
    {
        // 500 Hz - เสียงทุ้ม
        PWM_SetFrequency(PWM1_CH1, 500);
        Delay_Ms(1000);

        // 1 kHz - เสียงกลาง
        PWM_SetFrequency(PWM1_CH1, 1000);
        Delay_Ms(1000);

        // 2 kHz - เสียงสูง
        PWM_SetFrequency(PWM1_CH1, 2000);
        Delay_Ms(1000);

        // 4 kHz - เสียงสูงมาก
        PWM_SetFrequency(PWM1_CH1, 4000);
        Delay_Ms(1000);

        // 2 kHz
        PWM_SetFrequency(PWM1_CH1, 2000);
        Delay_Ms(1000);

        // 1 kHz
        PWM_SetFrequency(PWM1_CH1, 1000);
        Delay_Ms(1000);

        // 500 Hz
        PWM_SetFrequency(PWM1_CH1, 500);
        Delay_Ms(1000);
    }
}
