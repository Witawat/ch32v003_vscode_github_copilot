/**
 * ============================================================
 * ตัวอย่างที่ 3: Battery Monitor
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *     CH32V003              Li-ion Battery (3.7V)
 *     --------              ---------------------
 *     BATT+----------------[10kΩ]----+
 *                                      |
 *     PD2(ADC)-------------------------+
 *                                      |
 *     GND ------------------------[10kΩ]---- GND
 *
 *     Divider ratio 2:1 — max 4.2V → 2.1V at ADC pin
 *
 *     USART Debug:
 *     PD5(TX) ----------> USB-Serial RX
 *     PD6(RX) <---------- USB-Serial TX
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   VDD: 3.28V, Battery: 85%
 * ============================================================
 * คำเตือน (WARNINGS):
 * ADC_GetVDD() calculates VDD from internal Vref
 *          (≈1.2V). This method only works on CH32V003.
 * ============================================================
 */

#include <SimpleHAL.h>

#define BAT_PIN     PD2
#define DIV_RATIO   2.0f
#define MAX_BATT_V  4.2f

int main(void)
{
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    ADC_SimpleInit();

    while(1)
    {
        uint16_t adcVal = ADC_Read(ADC_CH_PD2);
        float vDD = ADC_GetVDD();
        float vDivOut = ADC_ToVoltage(adcVal, vDD);

        float battVoltage = vDivOut * DIV_RATIO;
        uint8_t battPercent = (uint8_t)ADC_GetBatteryPercent(battVoltage, 3.0f, MAX_BATT_V);

        USART_Print("VDD: ");
        USART_PrintNum((int32_t)vDD);
        USART_Print(".");
        USART_PrintNum((uint32_t)((vDD - (int32_t)vDD) * 100));
        USART_Print("V, Battery: ");
        USART_PrintNum(battPercent);
        USART_Print("%\r\n");
        Delay_Ms(1000);
    }
    return 0;
}
