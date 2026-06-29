/**
 * ============================================================
 * ตัวอย่างที่ 5: สร้างรูปคลื่น PWM ด้วย DMA (DMA PWM Waveform)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003              LED
 *     --------              ---
 *     PD2 (PWM1_CH1) -/\/\/\---->|---- GND
 *                     220 Ohm
 * 
 *     หรือต่อ oscilloscope ที่ PD2 เพื่อดูรูปคลื่น Sine
 *     PD2 = PWM1_CH1 (TIM1 Channel 1)
 *     ตัวต้านทาน 220 Ohm สำหรับ LED (ไม่จำเป็นถ้าใช้ oscilloscope)
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 * - LED จะค่อยๆ สว่างแล้วค่อยๆ ดับ (fade in/out) เป็นรูป Sine
 * - ถ้าใช้ oscilloscope จะเห็นสัญญาณ PWM duty cycle เปลี่ยนแบบ Sine wave
 * - คาบเวลาประมาณ 64 samples * (1/ความถี่ PWM)
 * ============================================================
 * คำเตือน (WARNINGS):
 * - ต้องเปิดใช้งาน TIM1 update DMA: TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE)
 * - Sine table ต้องเป็น uint16_t array ค่าตั้งแต่ 0 ถึง period ของ timer
 * - DMA_CH5 ใช้สำหรับ TIM1 Update (แก้ไข CCR ทุกครั้งที่ timer อัปเดต)
 * - ถ้า LED ไม่เห็น fade ชัด อาจต้องปรับความถี่ PWM หรือขนาด sine table
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
/* CH32V003 has no hardware FPU � float/double use software emulation (~800 cycles) */
#include <SimpleHAL.h>    // รวมไลบรารี SimpleHAL ทั้งหมด
#include <math.h>         // รวมไลบรารี math.h สำหรับฟังก์ชัน sin()

#define SINE_TABLE_SIZE 64  // จำนวนจุดใน Sine lookup table (64 จุดต่อคาบ)
#define PWM_FREQUENCY   1000 // ความถี่ PWM ตั้งไว้ที่ 1kHz

uint16_t sine_table[SINE_TABLE_SIZE]; // Sine lookup table สำหรับ DMA PWM

void generate_sine_table(uint16_t period) // ฟังก์ชันสร้าง Sine table
{
    for (uint16_t i = 0; i < SINE_TABLE_SIZE; i++) // วนลูปสร้างค่าทีละจุด
    {
        double angle = (double)i * 2.0 * 3.14159265 / (double)SINE_TABLE_SIZE; // คำนวณมุม (เรเดียน)
        double sin_val = sin(angle); // คำนวณค่า sine ที่มุมนั้น (ช่วง -1 ถึง 1)
        double duty = (sin_val + 1.0) / 2.0; // ปรับช่วงเป็น 0.0 ถึง 1.0
        sine_table[i] = (uint16_t)(duty * period); // แปลงเป็นค่า duty cycle (0 ถึง period)
    }
}

int main(void)            // ฟังก์ชันหลัก จุดเริ่มต้นโปรแกรม
{
    SystemCoreClockUpdate(); // อัปเดตค่าความถี่สัญญาณนาฬิกา (จำเป็นทุกครั้ง)

    PWM_Init(PWM1_CH1, PWM_FREQUENCY); // เริ่มต้น PWM1_CH1 (PD2) ที่ความถี่ 1000 Hz

    uint16_t period = PWM_GetPeriod(PWM1_CH1); // อ่านค่า period (ARR register) ของ TIM1
    generate_sine_table(period); // สร้าง sine lookup table โดยใช้ period จริง

    uint32_t ccr_addr = DMA_TIM_GetCCRAddress(TIM1, 1); // หา address ของ TIM1->CH1CVR (CCR1)
    DMA_TIM_UpdatePWM(DMA_CH5, TIM1, ccr_addr, sine_table, SINE_TABLE_SIZE, 1); // ตั้งค่า DMA ให้อัปเดต PWM แบบวน循環

    TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE); // เปิดใช้งาน DMA trigger จาก TIM1 update event

    PWM_Start(PWM1_CH1); // เริ่ม PWM PIN_MODE_OUTPUT ที่ PD2

    DMA_Start(DMA_CH5);   // เริ่ม DMA ถ่ายโอน sine table ไปยัง CCR register

    while (1)              // วนลูปอนันต์ (DMA ทำงานเบื้องหลังอัตโนมัติ)
    {
        __NOP();           // No operation: รอให้ DMA สร้าง sine wave ต่อไป
    }                      // สิ้นสุด while loop
}                          // สิ้นสุดฟังก์ชัน main
