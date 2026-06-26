/**
 * ============================================================
 * ตัวอย่างที่ 4: PWM Advanced (Polarity, Read Back)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003                  LED
 *     --------                  ---
 *     PD2 ----/\/\/\---->|---- GND
 *     PWM1_CH1  220 Ohm
 *
 *     หรือตอหัววัดออสซิลโลสโคป (Scope Probe) ที่ PD2
 *     เพื่อสังเกตรูปคลื่น PWM และผลของการกลับขั้ว (Polarity)
 *
 * API ที่ใช้:
 *   PWM_AdvancedInit()             // กำหนด prescaler, period, duty_value
 *   PWM_SetPolarity()              // กลับขั้วสัญญาณ
 *   PWM_GetDutyCycle()             // อ่านค่า Duty Cycle ปัจจุบัน
 *   PWM_GetPeriod()                // อ่านค่า Period
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - LED จะดับเมื่อ duty = 100% (เพราะกลับขั้วแล้ว)
 *   - อ่านค่า Duty Cycle และ Period กลับมาได้
 *   - แสดงค่าทาง Serial (ถ้ามี)
 * ============================================================
 * คำเตือน (WARNINGS):
 *   - PWM_AdvancedInit ต้องการ prescaler, period, duty_value ที่คำนวณเอง
 *   - การกลับขั้ว (inverted polarity): duty 0% = LED ติด, duty 100% = LED ดับ
 *   - ค่า duty_value เป็นค่าจริง (0 ถึง period) ไม่ใช่เปอร์เซ็นต์
 * ============================================================
 */

#include <SimpleHAL.h>

int main(void)
{
    SystemCoreClockUpdate();

    Timer_Init();
    uint16_t prescaler = 239;
    uint16_t period    = 99;
    uint16_t duty_value = 50;

    PWM_AdvancedInit(PWM1_CH1, prescaler, period, duty_value);

    PWM_SetPolarity(PWM1_CH1, 1);

    PWM_Start(PWM1_CH1);

    (void)PWM_GetDutyCycle(PWM1_CH1);
    (void)PWM_GetPeriod(PWM1_CH1);

    uint16_t current_duty = 0;
    int dir = 1;

    while (1)
    {
        PWM_SetDutyCycle(PWM1_CH1, current_duty);

        Delay_Ms(50);

        if (dir == 1)
            current_duty++;
        else
            current_duty--;

        if (current_duty >= 100) dir = -1;
        if (current_duty <= 0)   dir = 1;
    }
}
