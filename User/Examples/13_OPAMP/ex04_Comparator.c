/**
 * ============================================================
 * Comparator
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *  สัญญาณอินพุต ---------> CHP0 (OPAMP +IN) 
 *                          (แรงดันที่ต้องการเปรียบเทียบ)
 *  
 *  VDD (3.3V)
 *     +
 *     |
 *   [POT] 10kΩ
 *     |
 *     +-------------------> CHN0 (OPAMP -IN)
 *     |                    (แรงดันอ้างอิง Threshold)
 *    GND
 * 
 *  OPAMP PIN_MODE_OUTPUT (digital) ----> PD2 (ADC หรือ GPIO)
 *  หรือ OPAMP PIN_MODE_OUTPUT ---[R 1kΩ]---> ฐานของ NPN Transistor --- ควบคุม LED
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - "Signal > 1.65V -> HIGH" เมื่อสัญญาณสูงกว่า Threshold
 *   - "Signal < 1.65V -> LOW" เมื่อสัญญาณต่ำกว่า Threshold
 *   - เอาต์พุตเป็น Digital (HIGH/LOW) เท่านั้น
 * ============================================================
 * คำเตือน (WARNINGS):
 *   1. เอาต์พุตของ OPAMP ในโหมด Comparator เป็น Digital (HIGH/LOW)
 *      ไม่มีสัญญาณ Analog ตรงกลาง
 *   2. อาจมี Hysteresis เล็กน้อยตามธรรมชาติของ OPAMP
 *      แต่ไม่มากพอสำหรับสัญญาณที่มีสัญญาณรบกวนสูง
 *   3. เมื่อสัญญาณใกล้เคียง Threshold อาจเกิดการสั่น (Oscillation)
 *      แนะนำให้เพิ่ม Positive Feedback (Hysteresis) ด้วย R เพิ่ม
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

    OPAMP_ConfigComparator(OPAMP_CHP0, OPAMP_CHN0);
    OPAMP_Enable();

    USART_Print("OPAMP comparator started. Threshold = pot at CHN0.\r\n");

    if (!OPAMP_IsEnabled())
    {
        USART_Print("OPAMP not available!\r\n");
        while (1) { }
    }

    uint8_t lastOutput = 0;

    while (1)
    {
        uint16_t adcVal = ADC_Read(ADC_CH_PD2);
        uint8_t compOutput = (adcVal > 2047) ? 1 : 0;

        if (compOutput && !lastOutput)
        {
            USART_Print("Signal > Threshold -> HIGH\r\n");
        }
        else if (!compOutput && lastOutput)
        {
            USART_Print("Signal < Threshold -> LOW\r\n");
        }

        lastOutput = compOutput;
        Delay_Ms(100);
    }

    return 0;
}
