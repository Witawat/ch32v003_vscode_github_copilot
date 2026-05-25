/**
 * ============================================================
 * Voltage Follower
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *                     VDD (3.3V)
 *                       +
 *                       |
 *                     [POT] 10kΩ
 *                       |
 *   CHP0 (OPAMP +IN) ---+--- Wiper (แรงดันปรับได้ 0-3.3V)
 *
 *   OPAMP PIN_MODE_OUTPUT ----------> PD2 (ADC Input)
 *   OPAMP PIN_MODE_OUTPUT ---[R 1kΩ]--->| LED (สำหรับดูสัญญาณคร่าวๆ)
 *
 *   หมายเหตุ: CHP0 และ OPAMP PIN_MODE_OUTPUT ต้องตรวจสอบขาจริงจาก Datasheet
 *             ไม่ใช่ขา GPIO ทั่วไป
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - "Vout ≈ Vin (buffer mode)"
 *   - ปรับ Pot: "Set pot: Vout = 1.65V"
 *   - แรงดันเอาต์พุตจะเท่ากับแรงดันอินพุต (follow)
 * ============================================================
 * คำเตือน (WARNINGS):
 *   1. ขา OPAMP PIN_MODE_OUTPUT ต้องไม่ถูก Short ลง GND เป็นเวลานาน!
 *      อาจทำให้ OPAMP เสียหายได้
 *   2. ตรวจสอบ CH32V003 Datasheet สำหรับตำแหน่งขา OPAMP ที่แน่นอน
 *      OPAMP มีขาของตัวเอง ไม่ใช่ขา GPIO ปกติ
 *   3. ช่วงแรงดันอินพุตต้องอยู่ภายใน Common Mode Range ของ OPAMP
 *      (0V ถึง VDD-0.7V โดยประมาณ)
 * ============================================================
 */

#include <SimpleHAL.h>

int main(void)
{
    SystemCoreClockUpdate();

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(PD2, PIN_MODE_INPUT);
    ADC_SimpleInit();

    OPAMP_SimpleInit(OPAMP_MODE_VOLTAGE_FOLLOWER);
    OPAMP_Enable();

    USART_Print("Vout approx Vin (buffer mode).\r\n");

    if (OPAMP_IsEnabled())
    {
        USART_Print("OPAMP enabled successfully.\r\n");

        while (1)
        {
            uint16_t adcVal = ADC_Read(ADC_CH_PD2);
            float voltage = ADC_ToVoltage(adcVal, 3.3f);

            USART_Print("Set pot: Vout = ");
            USART_PrintNum((int32_t)voltage);
            USART_Print(".");
            uint16_t frac = (uint16_t)((voltage - (int32_t)voltage) * 100);
            USART_PrintNum((int32_t)frac);
            USART_Print("V\r\n");

            Delay_Ms(500);
        }
    }
    else
    {
        USART_Print("OPAMP init failed!\r\n");
    }

    while (1)
    {
    }

    return 0;
}
