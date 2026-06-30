/**
 * ============================================================
 * ตัวอย่างที่ 2: Servo Control (SG90)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003                SERVO SG90
 *     --------                ----------
 *     PD2 ----(Signal)-----> SIGNAL (ส้ม)
 *     PWM1_CH1
 *
 *                               VCC (แดง) ---- +5V ภายนอก
 *                               GND (น้ำตาล) -- GND รวม
 *
 *     หมายเหตุ:
 *     - สายสัญญาณจาก PD2 (PWM1_CH1) ไปขา Signal ของ Servo
 *     - จายไฟ 5V ภายนอกให Servo เทานั้น! หามใช 3.3V จาก MCU
 *     - GND ของ Servo และ MCU ตองตอรวมกัน (common ground)
 *
 * การคำนวณ Duty Cycle:
 *   - 50Hz -> คาบ 20ms
 *   - 0°   = 1ms   = 5%   duty
 *   - 90°  = 1.5ms = 7.5% duty
 *   - 180° = 2ms   = 10%  duty
 *
 * API ที่ใช้:
 *   PWM_Init(PWM1_CH1, 50)         // 50Hz สำหรับ Servo
 *   PWM_SetDutyCycle(PWM1_CH1, %)
 *   Delay_Ms(milliseconds)
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   Servo จะกวาดจาก 0° -> 90° -> 180° -> 90° -> 0° ทุก 2 วินาที
 * ============================================================
 * คำเตือน (WARNINGS):
 *   - SG90 ต้องใช้ไฟ 5V ภายนอกเท่านั้น! ห้ามจ่ายจาก CH32V003 3.3V เพราะกระแสไม่พอ
 *   - ต่อ GND ของ Servo และ MCU เข้าด้วยกัน (common ground)
 *   - สายสัญญาณจาก MCU เท่านั้นที่ต่อเข้าขา Signal ของ Servo
 * - ตรวจสอบ Duty Cycle ให้ถูกต้อง: 1ms=5%, 1.5ms=7.5%, 2ms=10%
 * ============================================================
 * แผนผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["PWM_Init(PWM1_CH1, 50)"]
 *     C --> D["PWM_Start(PWM1_CH1)"]
 *     D --> E["while(1)"]
 *     E --> F["PWM_SetDutyCycle(5)"]
 *     F --> G["Delay_Ms(2000)"]
 *     G --> H["PWM_SetDutyCycle(7)"]
 *     H --> I["Delay_Ms(2000)"]
 *     I --> J["PWM_SetDutyCycle(10)"]
 *     J --> K["Delay_Ms(2000)"]
 *     K --> L["PWM_SetDutyCycle(7)"]
 *     L --> M["Delay_Ms(2000)"]
 *     M --> N["PWM_SetDutyCycle(5)"]
 *     N --> O["Delay_Ms(2000)"]
 *     O --> E
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

int main(void)
{
    // อัปเดตความถี่ระบบ
    SystemCoreClockUpdate();

    Timer_Init();
    // เริ่มต้น PWM ช่อง 1 ที่ PD2 ความถี่ 50Hz สำหรับ Servo
    PWM_Init(PWM1_CH1, 50);

    // เริ่ม PWM (จำเป็น!)
    PWM_Start(PWM1_CH1);

    // วนรอบไม่สิ้นสุด
    while (1)
    {
        // 0 องศา: Duty 5% (1ms)
        PWM_SetDutyCycle(PWM1_CH1, 5);
        Delay_Ms(2000);

        // 90 องศา: Duty 7.5% (1.5ms)
        PWM_SetDutyCycle(PWM1_CH1, 7);
        // 7.5% ไม่สามารถป้อนทศนิยมได้, ปรับเป็นค่าใกล้เคียง
        // หรือใช้ 8% แทน (1.6ms)
        // แต่เพื่อความแม่นยำ ควรปรับค่าที่ละเอียดขึ้น
        // ขอปรับเป็น 7% ก่อนเพื่อให้เห็นการเปลี่ยนแปลง
        Delay_Ms(2000);

        // 180 องศา: Duty 10% (2ms)
        PWM_SetDutyCycle(PWM1_CH1, 10);
        Delay_Ms(2000);

        // 90 องศา: Duty 7.5% (อีกครั้ง)
        PWM_SetDutyCycle(PWM1_CH1, 7);
        Delay_Ms(2000);

        // 0 องศา: Duty 5%
        PWM_SetDutyCycle(PWM1_CH1, 5);
        Delay_Ms(2000);
    }
}
