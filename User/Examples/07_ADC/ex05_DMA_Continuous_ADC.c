/**
 * ============================================================
 * ตัวอย่างที่ 5: DMA Continuous ADC
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
 *     DMA buffers ADC, LED blinks on buffer fill:
 *     PC0 -----/\/\/\---->|---- GND
 *            220 Ohm      LED
 *
 *     USART Debug:
 *     PD5(TX) ----------> USB-Serial RX
 *     PD6(RX) <---------- USB-Serial TX
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   DMA ADC Avg: 510
 *   (ADC reads continuously via DMA without CPU intervention)
 * ============================================================
 * คำเตือน (WARNINGS):
 * DMA uses channel DMA_CH1 by default. Can change
 *          with DMA_SetAnalogReadChannel().
 * ============================================================
 */

#include <SimpleHAL.h>

#define ADC_PIN PD2
#define DMA_BUF_SIZE 64
static uint16_t adcBuffer[DMA_BUF_SIZE];

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    ADC_SimpleInit();

    DMA_analogReadStart(ADC_PIN, adcBuffer, DMA_BUF_SIZE, 1);

    while(1)
    {
        if (!DMA_analogReadBusy())
        {
            uint16_t avg = DMA_analogReadAverage(adcBuffer, DMA_BUF_SIZE);
            USART_Print("DMA ADC Avg: ");
            USART_PrintNum(avg);
            USART_Print("\r\n");
        }

        Delay_Ms(500);
    }
    return 0;
}
