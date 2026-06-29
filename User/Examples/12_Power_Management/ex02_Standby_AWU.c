/**
 * ============================================================
 * Standby with AWU
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *  PC0 ----[R 220Ω]----->| LED (สีเขียว) ---- GND
 * 
 *  USART1: PA9 (TX), PA10 (RX) --- CP2102 --- PC
 * 
 *  ไม่ต้องใช้ปุ่มกดเพราะ AWU จะปลุกเองอัตโนมัติทุก 5 วินาที
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - "System starting..." พร้อม LED กระพริบ
 *   - เข้าสู่ Standby นาน 5 วินาที
 *   - AWU ปลุก -> ระบบ RESTART (เหมือนกดรีเซ็ต)
 *   - "Woke from standby" แสดงผลซ้ำทุกครั้งที่ปลุก
 * ============================================================
 * คำเตือน (WARNINGS):
 *   1. โหมด Standby สูญเสีย RAM ทั้งหมด! ระบบเริ่มต้นใหม่ตั้งแต่ต้น
 *      ควรบันทึกข้อมูลสำคัญลง Flash ก่อนเข้าสู่ Standby!
 *   2. AWU ใช้นาฬิกา LSI (ความถี่ประมาณ 128kHz) 
 *      เวลาจริงอาจคลาดเคลื่อนได้ถึง ±25%
 *   3. หลังปลุกจาก Standby ระบบจะรีเซ็ตเหมือนกดปุ่มรีเซ็ต
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

int main(void)
{
    SystemCoreClockUpdate();

    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(PC0, PIN_MODE_OUTPUT);

    if (PWR_WasStandbyWakeup())
    {
        USART_Print("Woke from standby!\r\n");

        for (int i = 0; i < 5; i++)
        {
            digitalWrite(PC0, HIGH);
            Delay_Ms(100);
            digitalWrite(PC0, LOW);
            Delay_Ms(100);
        }
    }
    else
    {
        USART_Print("System starting...\r\n");
    }

    digitalWrite(PC0, HIGH);
    Delay_Ms(500);
    digitalWrite(PC0, LOW);

    USART_Print("Entering standby for 5 seconds...\r\n");
    Delay_Ms(50);

    PWR_ConfigureAWU(PWR_AWU_PRESCALER_1024, 31);
    PWR_EnterStandbyMode(PWR_ENTRY_WFI);

    return 0;
}
