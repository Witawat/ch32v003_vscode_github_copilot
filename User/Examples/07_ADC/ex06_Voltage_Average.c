/**
 * @example ex06_Voltage_Average.c
 * @brief ADC_ReadVoltage + ADC_ReadAverage — อ่านแรงดัน + average
 *
 * ADC_ReadVoltage: อ่าน ADC พร้อมแปลงเป็น voltage (V)
 * ADC_ReadAverage: อ่านหลายครั้งหาค่าเฉลี่ย ลด noise
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit(115200)"]
 *     C --> D["ADC_SimpleInit()"]
 *     D --> E["while(1)"]
 *     E --> F["ADC_ReadVoltage(PD2, 3.3f)"]
 *     F --> G["Print Voltage (mV)"]
 *     G --> H["ADC_ReadAverage(PD2, 16)"]
 *     H --> I["ADC_ToVoltage(avg, 3.3f)"]
 *     I --> J["Print Avg (mV) with 16 samples"]
 *     J --> K["ADC_ToPercent(avg)"]
 *     K --> L["Print Percent"]
 *     L --> M["Delay_Ms(500)"]
 *     M --> E
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    ADC_SimpleInit();

    while (1) {
        // ADC_ReadVoltage — อ่าน + แปลงเป็นแรงดัน (V) โดยตรง
        float v = ADC_ReadVoltage(ADC_CH_PD2, 3.3f);
        USART_Print("Voltage: ");
        USART_PrintNum((int)(v * 1000));
        USART_Print(" mV  |  ");

        // ADC_ReadAverage — อ่าน 16 ครั้ง หาค่าเฉลี่ย ลด noise
        uint16_t avg = ADC_ReadAverage(ADC_CH_PD2, 16);
        // แปลงด้วย ADC_ToVoltage (แยก calculation จาก ADC_ReadVoltage)
        float avg_v = ADC_ToVoltage(avg, 3.3f);
        USART_Print("Avg: ");
        USART_PrintNum((int)(avg_v * 1000));
        USART_Print(" mV (16 samples)\r\n");

        // เปอร์เซ็นต์
        float pct = ADC_ToPercent(avg);
        USART_Print("Percent: ");
        USART_PrintNum((int)pct);
        USART_Print("%\r\n");

        Delay_Ms(500);
    }
}
