/**
 * ============================================================
 * Non-Inverting Amplifier
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *  สัญญาณอินพุต ----> CHP0 (OPAMP +IN)
 *  
 *  OPAMP PIN_MODE_OUTPUT -----+-----> PD2 (ADC Input)
 *                    |
 *                    [R2] 10kΩ
 *                    |
 *                    +-----> CHN0 (OPAMP -IN)
 *                    |
 *                    [R1] 10kΩ
 *                    |
 *                   GND
 * 
 *  Gain = 1 + (R2/R1) = 1 + (10k/10k) = 2
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - "Non-inverting amp: R1=10k, R2=10k -> Gain=2.0"
 *   - อ่าน Vout ผ่าน ADC -> "Vin=0.5V, Vout=1.0V"
 *   - Vout = Vin * 2.0 (ไม่กลับเฟส)
 * ============================================================
 * คำเตือน (WARNINGS):
 *   1. Gain = 1 + (R2/R1) สำหรับการต่อแบบ Non-Inverting
 *      ถ้า R1 = R2 จะได้ Gain = 2
 *   2. แรงดันเอาต์พุตสูงสุดจำกัดที่ VDD (3.3V) ไม่เกินกว่านี้
 *      ดังนั้น Vin สูงสุด = 3.3V / Gain
 *   3. ต้องใช้รีซิสเตอร์ภายนอก! OPAMP ไม่มี Feedback Network ภายใน
 *   4. เลือกค่า R1, R2 ให้เหมาะสม (โดยทั่วไป 1kΩ - 100kΩ)
 *      ค่าต่ำเกินไปกินกระแสมาก ค่าสูงเกินไปมีสัญญาณรบกวน
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

int main(void)
{
    SystemCoreClockUpdate();

    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(PD2, PIN_MODE_INPUT);
    ADC_SimpleInit();

    OPAMP_ConfigNonInverting(OPAMP_CHP0, OPAMP_CHN0);
    OPAMP_Enable();

    USART_Print("Non-inverting amp: R1=10k, R2=10k, Gain=2.0\r\n");
    USART_Print("(Using fixed-point integer — no float on CH32V003)\r\n");

    if (!OPAMP_IsEnabled())
    {
        USART_Print("OPAMP not enabled!\r\n");
        while (1) { }
    }

    USART_Print("Reading Vout via ADC...\r\n");

    while (1)
    {
        uint16_t adcVal = ADC_Read(ADC_CH_PD2);

        // Fixed-point: Vout(mV) = adcVal * 3300 / 1023  (10-bit ADC, 3.3V ref)
        uint32_t vout_mv = ((uint32_t)adcVal * 3300) / 1023;

        // Gain = 2 (Non-inverting: 1 + R2/R1 = 1 + 10k/10k)
        uint32_t vin_mv = vout_mv / 2;

        USART_Print("Vin=");
        USART_PrintNum(vin_mv / 1000);
        USART_Print(".");
        USART_PrintNum((vin_mv % 1000) / 100);
        USART_Print("V, Vout=");
        USART_PrintNum(vout_mv / 1000);
        USART_Print(".");
        USART_PrintNum((vout_mv % 1000) / 100);
        USART_Print("V (ADC=");
        USART_PrintNum(adcVal);
        USART_Print(")\r\n");

        Delay_Ms(500);
    }

    return 0;
}
