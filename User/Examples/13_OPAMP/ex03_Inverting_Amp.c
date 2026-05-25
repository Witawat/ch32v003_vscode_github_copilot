/**
 * ============================================================
 * Inverting Amplifier
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *  สัญญาณ ---[R1] 10kΩ---+-----> CHN0 (OPAMP -IN)
 *                         |
 *  CHP0 (OPAMP +IN) ---- GND (หรือ VDD/2 สำหรับ AC)
 *                         |
 *  OPAMP PIN_MODE_OUTPUT ---[R2] 20kΩ---+
 *  OPAMP PIN_MODE_OUTPUT ----> PD2 (ADC Input)
 * 
 *  Gain = -(R2/R1) = -(20k/10k) = -2.0
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - "Inverting amp: R1=10k, R2=20k -> Gain=-2.0"
 *   - "Vin=1.0V, Vout=-2.0V (inverted)"
 *   - ถ้า Vin=0.5V จะได้ Vout=-1.0V
 * ============================================================
 * คำเตือน (WARNINGS):
 *   1. แรงดันเอาต์พุตของ Inverting Amp เป็นลบเทียบกับขาอ้างอิง!
 *      ถ้าอ้างอิง GND (0V) เอาต์พุตจะต่ำกว่า GND ไม่ได้ (ถูกตัดที่ 0V)
 *   2. สำหรับสัญญาณ AC ควรตั้งแรงดันอ้างอิงที่ VDD/2 (1.65V)
 *      เพื่อให้สัญญาณมี Swing สมมาตร
 *   3. ข้อจำกัด: ด้วย R1=10k, R2=20k Gain=-2 แต่ Vin ไม่ควรเกิน 1.65V
 *      เพราะ Vout จะพยายามไป -3.3V ซึ่งต่ำกว่า GND
 * ============================================================
 */

#include <SimpleHAL.h>

int main(void)
{
    SystemCoreClockUpdate();

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(PD2, PIN_MODE_INPUT);
    ADC_SimpleInit();

    float r1 = 10000.0f;
    float r2 = 20000.0f;

    OPAMP_ConfigInverting(OPAMP_CHP0, OPAMP_CHN0);
    OPAMP_Enable();

    (void)(-(r2 / r1));
    USART_Print("Inverting amp: R1=10k, R2=20k, Gain=-2.0\r\n");

    if (!OPAMP_IsEnabled())
    {
        USART_Print("OPAMP init failed!\r\n");
        while (1) { }
    }

    while (1)
    {
        uint16_t adcVal = ADC_Read(ADC_CH_PD2);
        float vout = (adcVal * 3.3f) / 4095.0f;

        USART_Print("Vin=1.0V, Vout=");
        USART_PrintNum((int32_t)vout);
        USART_Print(".");
        uint16_t frac = (uint16_t)((vout - (int32_t)vout) * 10);
        USART_PrintNum((int32_t)frac);
        USART_Print("V (inverted)\r\n");

        Delay_Ms(500);
    }

    return 0;
}
