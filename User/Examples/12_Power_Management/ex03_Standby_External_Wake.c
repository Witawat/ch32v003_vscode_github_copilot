/**
 * ============================================================
 * Standby External Wake
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *                    VDD (3.3V)
 *                       +
 *                       |
 *                      [R] 10kΩ (Pull-up ภายนอก)
 *                       |
 *  PA0 (Wakeup pin) ----+--- ปุ่มกด --- GND
 * 
 *  PC0 ----[R 220Ω]----->| LED (สีเหลือง) ---- GND
 * 
 *  USART1: PA9 (TX), PA10 (RX) --- CP2102 --- PC
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - "Entering standby. Press PA0 to wake..."
 *   - กดปุ่มที่ PA0 -> ระบบรีสตาร์ท
 *   - "Woke by external wakeup pin!" แสดงผล
 * ============================================================
 * คำเตือน (WARNINGS):
 *   1. มีเฉพาะขา PA0 เท่านั้นที่รองรับฟังก์ชัน Wakeup Pin สำหรับโหมด Standby
 *      ขาอื่นๆ ต้องใช้ EXTI เพื่อปลุกจาก Sleep เท่านั้น ไม่สามารถปลุกจาก Standby ได้
 *   2. ระบบจะรีเซ็ตหลังจาก Standby - นี่คือการเริ่มต้นใหม่ทั้งหมด ไม่ใช่การกลับมาทำงานต่อ
 *   3. RAM และ register ทั้งหมดถูกรีเซ็ต ไม่มีการคงค่าตัวแปรใดๆ ไว้
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

int main(void)
{
    SystemCoreClockUpdate();

    Timer_Init();
    pinMode(PC0, PIN_MODE_OUTPUT);
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    if (PWR_WasStandbyWakeup())
    {
        USART_Print("Woke by external wakeup pin!\r\n");

        for (int i = 0; i < 3; i++)
        {
            digitalWrite(PC0, HIGH);
            Delay_Ms(300);
            digitalWrite(PC0, LOW);
            Delay_Ms(300);
        }
    }
    else
    {
        USART_Print("Entering standby. Press PA0 to wake...\r\n");
        Delay_Ms(100);
    }

    digitalWrite(PC0, HIGH);
    Delay_Ms(500);
    digitalWrite(PC0, LOW);

    PWR_EnableWakeupPin();
    PWR_EnterStandbyMode(PWR_ENTRY_WFI);

    return 0;
}
