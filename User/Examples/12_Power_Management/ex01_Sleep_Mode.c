/**
 * ============================================================
 * Sleep Mode
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *                    VDD (3.3V)
 *                       +
 *                       |
 *                      [R] 10kΩ
 *                       |
 *  PC1 (อินเทอร์รัพท์) --+--- ปุ่มกด --- GND
 * 
 *  PC0 ----[R 220Ω]----->| LED (สีแดง) ---- GND
 * 
 *  USART1: PA9 (TX), PA10 (RX) --- CP2102 --- PC
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - LED ติดสว่างก่อนเข้าสู่โหมด Sleep
 *   - กระแสไฟลดลงขณะอยู่ในโหมด Sleep
 *   - กดปุ่มที่ PC1 เพื่อปลุก -> LED กระพริบ 3 ครั้ง
 *   - USART แสดง "Entering sleep..." / "Woke up!"
 * ============================================================
 * คำเตือน (WARNINGS):
 *   1. ในโหมด Sleep, RAM ยังคงรักษาข้อมูลไว้, peripheral ยังทำงานต่อเนื่อง
 *   2. Interrupt ทั้งหมดสามารถปลุก CPU จากโหมด Sleep ได้
 *   3. ต้องกำหนดค่า EXTI บนขา PC1 ก่อนเข้าสู่โหมด Sleep เพื่อให้สามารถปลุกได้
 * ============================================================
 */

#include <SimpleHAL.h>

void WakeupCallback(void)
{
}

int main(void)
{
    SystemCoreClockUpdate();

    Timer_Init();
    pinMode(PC0, PIN_MODE_OUTPUT);
    pinMode(PC1, PIN_MODE_INPUT_PULLUP);
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    attachInterrupt(PC1, WakeupCallback, RISING);

    digitalWrite(PC0, HIGH);
    USART_Print("Entering sleep...\r\n");

    while (1)
    {
        PWR_EnterSleepMode(PWR_ENTRY_WFI);

        USART_Print("Woke up!\r\n");

        for (int i = 0; i < 3; i++)
        {
            digitalWrite(PC0, HIGH);
            Delay_Ms(200);
            digitalWrite(PC0, LOW);
            Delay_Ms(200);
        }

        USART_Print("Entering sleep...\r\n");
        Delay_Ms(100);
    }

    return 0;
}
