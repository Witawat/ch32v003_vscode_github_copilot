/**
 * ============================================================
 * Wakeup Pin
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *                    VDD (3.3V)
 *                       +
 *                       |
 *                      [R] 10kΩ
 *                       |
 *  PA0 (Wakeup pin) ----+--- ปุ่มกด --- GND
 * 
 *  PC0 ----[R 220Ω]----->| LED (สีน้ำเงิน) ---- GND
 * 
 *  สามารถปลุกได้ 2 วิธี:
 *    1. AWU ปลุกอัตโนมัติทุก 3 วินาที
 *    2. กดปุ่มที่ PA0 เพื่อปลุกทันที
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - กำหนดค่า AWU ไว้ 3 วินาทีและเปิดใช้งาน Wakeup Pin
 *   - เข้าสู่ Standby ไม่ว่าจะ AWU ปลุก (3 วินาที) หรือกด PA0 ปลุกก็ได้
 *   - LED กระพริบแสดงสถานะทุกครั้งที่ถูกปลุก
 * ============================================================
 * คำเตือน (WARNINGS):
 *   1. หลังจากถูกปลุกจาก Standby register ทั้งหมดจะถูกรีเซ็ตค่าเริ่มต้น
 *      main() จะเริ่มต้นการทำงานใหม่ตั้งแต่ต้น
 *   2. ถ้าต้องการแยกว่าปลุกจากอะไร ต้องใช้ PWR_WasStandbyWakeup()
 *      ร่วมกับการตรวจสอบ PWR_GetWakeupFlag() เพิ่มเติม
 *   3. Wakeup Pin PA0 ต้องต่อ Pull-up หรือ Pull-down ภายนอกตามความเหมาะสม
 * ============================================================
 */

#include <SimpleHAL.h>

int main(void)
{
    SystemCoreClockUpdate();

    Timer_Init();
    pinMode(PC0, PIN_MODE_OUTPUT);
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    if (PWR_WasStandbyWakeup())
    {
        USART_Print("Woke from standby!\r\n");

        for (int i = 0; i < 3; i++)
        {
            digitalWrite(PC0, HIGH);
            Delay_Ms(150);
            digitalWrite(PC0, LOW);
            Delay_Ms(150);
        }
    }
    else
    {
        USART_Print("System start. Configuring AWU & wakeup pin...\r\n");
    }

    PWR_ConfigureAWU(PWR_AWU_PRESCALER_1024, 31);
    PWR_EnableWakeupPin();

    USART_Print("Entering standby. AWU=3s, PA0 can also wake.\r\n");
    Delay_Ms(50);

    PWR_EnterStandbyMode(PWR_ENTRY_WFI);

    return 0;
}
