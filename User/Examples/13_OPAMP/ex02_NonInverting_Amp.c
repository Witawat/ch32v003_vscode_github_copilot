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

#include <SimpleHAL.h>

int main(void)
{
    SystemCoreClockUpdate();

    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(PD2, PIN_MODE_INPUT);
    ADC_SimpleInit();

    float r1 = 10000.0f;
    float r2 = 10000.0f;

    OPAMP_ConfigNonInverting(OPAMP_CHP0, OPAMP_CHN0);
    OPAMP_Enable();

    float gain = 1.0f + (r2 / r1);
    USART_Print("Non-inverting amp: R1=10k, R2=10k, Gain=2.0\r\n");

    if (!OPAMP_IsEnabled())
    {
        USART_Print("OPAMP not enabled!\r\n");
        while (1) { }
    }

    USART_Print("Reading Vout via ADC...\r\n");

    while (1)
    {
        uint16_t adcVal = ADC_Read(ADC_CH_PD2);
        float vout = (adcVal * 3.3f) / 4095.0f;
        float vin = vout / gain;

        USART_Print("Vin=");
        USART_PrintNum((int32_t)vin);
        USART_Print(".");
        uint16_t frac = (uint16_t)((vin - (int32_t)vin) * 10);
        USART_PrintNum((int32_t)frac);
        USART_Print("V, Vout=");
        USART_PrintNum((int32_t)vout);
        USART_Print(".");
        frac = (uint16_t)((vout - (int32_t)vout) * 10);
        USART_PrintNum((int32_t)frac);
        USART_Print("V\r\n");

        Delay_Ms(500);
    }

    return 0;
}
