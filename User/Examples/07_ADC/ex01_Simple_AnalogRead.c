/**
 * ============================================================
 * ตัวอย่างที่ 1: Simple Analog Read
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *     CH32V003              10kΩ Potentiometer
 *     --------              ------------------
 *     VDD(3.3V)------------[###]---- GND
 *                           POT
 *                           |
 *     PD2(ADC)--------------+
 *
 *     Optional Output:
 *     PC0 -----/\/\/\---->|---- GND
 *            220 Ohm      LED
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   ADC: 512, Voltage: 1.65V
 *   (turning pot changes values 0-1023, 0-3.3V)
 * ============================================================
 * คำเตือน (WARNINGS):
 * ADC-capable pins: PA1, PA2, PC4, PD2-PD6 only!
 *          PD7 does NOT support ADC.
 * analogRead() macro has compile-time pin validation.
 * ============================================================
 */

#include <SimpleHAL.h>

#define ADC_PIN PD2

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    ADC_SimpleInit();

    while(1)
    {
        uint16_t adcVal = ADC_Read(ADC_CH_PD2);
        float voltage = ADC_ToVoltage(adcVal, 3.3f);

        USART_Print("ADC: ");
        USART_PrintNum(adcVal);
        USART_Print(", Voltage: ");
        USART_PrintNum((int32_t)voltage);
        USART_Print(".");
        USART_PrintNum((uint32_t)((voltage - (int32_t)voltage) * 100));
        USART_Print("V\r\n");
        Delay_Ms(500);
    }
    return 0;
}
