/**
 * ============================================================
 * OPAMP with ADC
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *                     VDD (3.3V)
 *                       +
 *                       |
 *                     [POT] 10kΩ
 *                       |
 *   CHP0 (OPAMP +IN) ---+--- Wiper (ปรับแรงดัน 0-3.3V)
 *   
 *   OPAMP PIN_MODE_OUTPUT ----> (ภายในเชื่อมต่อกับ ADC) ----> อ่านค่าโดย ADC_Read()
 *   หรือภายนอก: OPAMP PIN_MODE_OUTPUT ----> PD2 (ADC Input)
 * 
 *   OPAMP ทำงานในโหมด Buffer (Voltage Follower)
 *   ADC อ่านแรงดันที่ OPAMP ส่งออกมา
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - ปรับ Pot ที่ OPAMP Input
 *   - "OPAMP out via ADC: 1.65V"
 *   - ค่า ADC แปรผันตามแรงดัน OPAMP เอาต์พุต
 * ============================================================
 * คำเตือน (WARNINGS):
 *   1. ไม่ใช่ CH32V003 ทุกรุ่น (revision) ที่รองรับการเชื่อมต่อ OPAMP
 *      เอาต์พุตเข้ากับ ADC โดยตรงภายใน ต้องตรวจสอบ Datasheet ก่อน
 *   2. ถ้าไม่มีเส้นเชื่อมต่อภายใน ให้ต่อสายภายนอกจาก OPAMP PIN_MODE_OUTPUT
 *      ไปยังขา ADC (PD2) แทน
 *   3. OPAMP Buffer Mode มีข้อจำกัดเรื่อง Input Common Mode Range
 *      โดยทั่วไป 0V ถึง VDD-0.7V
 * ============================================================
 */

#include <SimpleHAL.h>

int main(void)
{
    SystemCoreClockUpdate();

    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(PD2, PIN_MODE_INPUT);
    ADC_SimpleInit();

    OPAMP_SimpleInit(OPAMP_MODE_VOLTAGE_FOLLOWER);
    OPAMP_Enable();

    if (!OPAMP_IsEnabled())
    {
        USART_Print("OPAMP init failed!\r\n");
        while (1) { }
    }

    USART_Print("OPAMP buffer ready. Reading via ADC...\r\n");

    while (1)
    {
        uint16_t adcVal = ADC_Read(ADC_CH_PD2);
        float voltage = (adcVal * 3.3f) / 4095.0f;

        USART_Print("OPAMP out via ADC: ");
        USART_PrintNum((int32_t)voltage);
        USART_Print(".");
        uint16_t frac = (uint16_t)((voltage - (int32_t)voltage) * 100);
        USART_PrintNum((int32_t)frac);
        USART_Print("V\r\n");

        Delay_Ms(300);
    }

    return 0;
}
