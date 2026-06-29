/**
 * ============================================================
 * ตัวอย่างที่ 2: Multi-Channel Average
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *     CH32V003              10kΩ Potentiometer      LDR + 10kΩ Divider
 *     --------              ------------------      -------------------
 *     VDD(3.3V)------------[###]---- GND            VDD(3.3V)
 *                           POT                      |
 *                           |                       [LDR]
 *     PD2(ADC)--------------+                        |
 *                                                    +----[10kΩ]---- GND
 *                                                    |
 *     PD5(ADC)---------------------------------------+
 *
 *     Output:
 *     PC0 -----/\/\/\---->|---- GND
 *            220 Ohm      LED
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   CH0: 512 | CH1: 256 | CH2: 768 | Percent: 50%, 25%, 75%
 * ============================================================
 * คำเตือน (WARNINGS):
 * ADC_ReadMultiple reads channels sequentially, not
 *          simultaneously — time skew between channels exists.
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

#define CH_PD2  ADC_CH_PD2
#define CH_PD3  ADC_CH_PD3
#define CH_PD4  ADC_CH_PD4
#define NUM_CH  3

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    ADC_SimpleInit();

    while(1)
    {
        ADC_Channel channels[NUM_CH] = {CH_PD2, CH_PD3, CH_PD4};
        uint16_t values[NUM_CH];

        ADC_ReadMultiple(channels, values, NUM_CH);

        USART_Print("CH0: ");
        USART_PrintNum(values[0]);
        USART_Print(" | CH1: ");
        USART_PrintNum(values[1]);
        USART_Print(" | CH2: ");
        USART_PrintNum(values[2]);
        USART_Print(" | Percent: ");
        USART_PrintNum((int32_t)ADC_ToPercent(values[0]));
        USART_Print("%, ");
        USART_PrintNum((int32_t)ADC_ToPercent(values[1]));
        USART_Print("%, ");
        USART_PrintNum((int32_t)ADC_ToPercent(values[2]));
        USART_Print("%\r\n");

        Delay_Ms(500);
    }
    return 0;
}
