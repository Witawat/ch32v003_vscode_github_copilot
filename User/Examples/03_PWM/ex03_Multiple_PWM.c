/**
 * ============================================================
 * ตัวอย่างที่ 3: Multiple PWM
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003                  LED1
 *     --------                  ----
 *     PD2 ----/\/\/\---->|---- GND
 *     PWM1_CH1  220 Ohm
 *
 *     CH32V003                  LED2
 *     --------                  ----
 *     PC0 ----/\/\/\---->|---- GND
 *     PWM2_CH3  220 Ohm
 *
 *     CH32V003                SERVO SG90
 *     --------                ----------
 *     PA1 ----(Signal)-----> SIGNAL (ส้ม)
 *     PWM1_CH2
 *                               VCC (แดง) ---- +5V ภายนอก
 *                               GND (น้ำตาล) -- GND รวม
 *
 *     หมายเหตุ: TIM1 (PD2, PA1) แชรความถี่ 50Hz, TIM2 (PC0) ใชความถี่ 1000Hz
 *
 * API ที่ใช้:
 *   PWM_Init(), PWM_Start(), PWM_SetDutyCycle() หลายช่อง
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - LED1: ค่อยๆ สว่าง-ดับ (0%->100%->0%)
 *   - LED2: กระพริบ (0%->50%->0%)
 *   - Servo: กวาดช้าๆ
 *   ทุกอย่างทำงานพร้อมกัน!
 * ============================================================
 * คำเตือน (WARNINGS):
 *   - TIM1 channels (PD2, PA1, PC3, PC4) แชร์ความถี่เดียวกัน!
 *   - TIM2 channels (PD4, PD3, PC0, PD7) แชร์ความถี่อีกกลุ่ม!
 *   - PD2 และ PA1 อยู่บน TIM1 ดังนั้นความถี่ต้องเท่ากัน
 * - PC0 อยู่บน TIM2 จึงกำหนดความถี่ต่างหากได้
 * ============================================================
 * แผนผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["PWM_Init(PWM1_CH1, 50)"]
 *     C --> D["PWM_Init(PWM1_CH2, 50)"]
 *     D --> E["PWM_Init(PWM2_CH3, 1000)"]
 *     E --> F["PWM_Start ทั้ง 3 ช่อง"]
 *     F --> G["while(1)"]
 *     G --> H["PWM_SetDutyCycle LED1 fade 0-100%"]
 *     H --> I["PWM_SetDutyCycle LED2 blink 0-50%"]
 *     I --> J["PWM_SetDutyCycle Servo sweep 5-10%"]
 *     J --> K["Delay_Ms(20)"]
 *     K --> G
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

int main(void)
{
    // อัปเดตความถี่ระบบ
    SystemCoreClockUpdate();

    Timer_Init();
    // --- กำหนดค่า PWM ---
    // TIM1: PD2 และ PA1 ใช้ความถี่เดียวกัน (50Hz สำหรับ Servo)
    PWM_Init(PWM1_CH1, 50);   // PD2 - LED1 (50Hz)
    PWM_Init(PWM1_CH2, 50);   // PA1 - Servo (50Hz)

    // TIM2: PC0 ใช้ความถี่ต่างหาก (1000Hz สำหรับ LED2)
    PWM_Init(PWM2_CH3, 1000); // PC0 - LED2

    // --- เริ่ม PWM ทุกช่อง ---
    PWM_Start(PWM1_CH1);
    PWM_Start(PWM1_CH2);
    PWM_Start(PWM2_CH3);

    // ตัวแปรสำหรับปรับค่า
    int led1_duty = 0;
    int led1_dir  = 1;
    int led2_duty = 0;
    int led2_dir  = 1;
    int servo_duty = 5;
    int servo_dir  = 1;

    // วนรอบไม่สิ้นสุด
    while (1)
    {
        // --- LED1: Fade 0-100% ---
        PWM_SetDutyCycle(PWM1_CH1, led1_duty);
        led1_duty = led1_duty + led1_dir;
        if (led1_duty >= 100) led1_dir = -1;
        if (led1_duty <= 0)   led1_dir = 1;

        // --- LED2: Blink 0-50-0% ---
        PWM_SetDutyCycle(PWM2_CH3, led2_duty);
        led2_duty = led2_duty + led2_dir;
        if (led2_duty >= 50) led2_dir = -1;
        if (led2_duty <= 0)  led2_dir = 1;

        // --- Servo: Sweep 5% -> 10% -> 5% ---
        PWM_SetDutyCycle(PWM1_CH2, servo_duty);
        servo_duty = servo_duty + servo_dir;
        if (servo_duty >= 10) servo_dir = -1;
        if (servo_duty <= 5)  servo_dir = 1;

        // หน่วงเวลา 20ms
        Delay_Ms(20);
    }
}
