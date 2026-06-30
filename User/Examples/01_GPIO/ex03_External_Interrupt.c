/**
 * ============================================================
 * ตัวอยางที่ 3: External Interrupt (อินเทอรรับตภายนอก)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003                  ปุมกด (Button)
 *     --------                  -------------
 *     PC1 ---+----/\/\/\---- 3.3V
 *            |      10k Ohm    (Pull-up)
 *            |
 *            +---- ปุมกด ---- GND
 *
 *     PC0 ----/\/\/\---->|---- GND
 *            220 Ohm
 *
 * ============================================================
 * ผลลัพธที่คาดหวัง (Expected Results):
 * - ทุกครั้งที่กดปุม LED ที่ PC0 จะเปลี่ยนสถานะ (ติดดับ)
 * - Serial Monitor แสดง "Interrupt #1", "#2", "#3"...
 * - ใชการทำงานแบบ Interrupt (ไมตอง Polling)
 * ============================================================
 * คำเตือน (WARNINGS):
 * - ฟงกชัน callback ใน interrupt ตองสั้นและทำงานเร็วที่สุด
 * - ไมควรเรียก Delay_Ms() หรือฟงกชันที่ blocking ใน ISR
 * - ตองประกาศตัวแปรที่แชรกับ ISR ดวยคําสําคัญ volatile
 * - CH32V003 รองรับ EXTI สูงสุด 8 lines เทานั้น
 * - ถากดปุมเร็วเกินไป อาจเกิด interrupt ซ้ำ (debounce)
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>   // รวมไลบรารี SimpleHAL ทั้งหมด

volatile uint32_t interruptCounter = 0; // ตัวแปรนับจำนวน interrupt ที่เกิดซึ้น
                                         // volatile ปองกัน compiler optimize คา
                                         // เพราะมีการเปลี่ยนแปลงจาก ISR

void Button_ISR(void)    // ฟงกชัน Interrupt Service Routine สำหรับปุมกด
{                        // ฟงกชันนี้จะถูกเรียกอัตโนมัติเมื่อเกิด interrupt ที่ PC1
    interruptCounter++;  // เพิ่มคานับ interrupt ทีละ 1
                         // (ทำงานเร็ว ไมมีการ Delay หรือ USART ใน ISR)
}                        // สิ้นสุด ISR

int main(void)           // ฟงกชันหลักของโปรแกรม
{
    SystemCoreClockUpdate();
    Timer_Init();
    pinMode(PC0, PIN_MODE_OUTPUT);       // ตั้งคาขา PC0 เปนเอาตพุต (LED)
    pinMode(PC1, PIN_MODE_INPUT_PULLUP); // ตั้งคาขา PC1 เปนอินพุต Pull-up (ปุมกด)

    attachInterrupt(PC1, Button_ISR, FALLING); // ลงทะเบียน interrupt สำหรับขา PC1
                                         // โหมด FALLING: เกิด interrupt เมื่อสัญญาณ
                                         // เปลี่ยนจาก HIGH  LOW (ตอนกดปุม)
                                         // ฟงกชัน Button_ISR จะถูกเรียกทุกครั้ง

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT); // เริ่มตน USART ที่ 115200 baud

    uint32_t lastCount = 0;  // ตัวแปรเก็บคานับครั้งกอนหนา เพื่อตรวจสอบการเปลี่ยนแปลง

    while(1)                 // วนลูปอนันต์
    {
        if (interruptCounter != lastCount)  // ตรวจสอบวาคานับ interrupt เปลี่ยนแปลง
        {                                    // (มีการเกิด interrupt ใหม)
            digitalToggle(PC0);              // สลับสถานะ LED ที่ PC0 (ติดดับ)
                                             // ไมตองอานคากอน เปลี่ยนทันที

            lastCount = interruptCounter;    // อัปเดตคานับลาสุด

            USART_Print("Interrupt #");      // สงขอความ "Interrupt #" ไป Serial
            USART_PrintNum((int32_t)interruptCounter); // สงตัวเลขนับ interrupt
            USART_Print("\r\n");             // ขึ้นบรรทัดใหม
        }

        // ไมมี Delay_Ms ในลูปหลัก เพราะตองการใหตอบสนองเร็ว
        // การรอแบบ Busy-wait ชวยประหยัดพลังงานไดในบางกรณี
    }                            // สิ้นสุด while loop
}                                // สิ้นสุดฟงกชัน main
