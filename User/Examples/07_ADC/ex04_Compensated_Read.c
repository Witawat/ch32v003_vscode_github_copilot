/**
 * ============================================================
 * ตัวอย่างที่ 4: Compensated Read
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *     CH32V003              Temperature Sensor (LM35/TMP36)
 *     --------              --------------------------------
 *     VDD(3.3V)------------- VCC
 *                            |
 *                          [OUT]
 *                            |
 *     PD2(ADC)---------------+
 *                            |
 *                           GND
 *
 *     Compensation uses VrefInt internal 1.2V reference
 *     to correct for VDD variations
 *
 *     USART Debug:
 *     PD5(TX) ----------> USB-Serial RX
 *     PD6(RX) <---------- USB-Serial TX
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   Raw: 512, Compensated: 1.62V
 *   (compensated voltage is more accurate when VDD varies)
 * ============================================================
 * คำเตือน (WARNINGS):
 * Compensated read uses Vrefint — requires external
 *          Vref measurement for calibration.
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
/* CH32V003 has no hardware FPU � float/double use software emulation (~800 cycles) */
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
        (void)ADC_ReadVrefInt();
        float compVoltage = ADC_ReadVoltageCompensated(ADC_PIN);

        USART_Print("Raw: ");
        USART_PrintNum(adcVal);
        USART_Print(", Compensated: ");
        USART_PrintNum((int32_t)compVoltage);
        USART_Print(".");
        USART_PrintNum((uint32_t)((compVoltage - (int32_t)compVoltage) * 100));
        USART_Print("V\r\n");
        Delay_Ms(500);
    }
    return 0;
}
