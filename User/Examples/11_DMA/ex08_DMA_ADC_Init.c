/**
 * @example ex08_DMA_ADC_Init.c
 * @brief DMA_ADC_Init — ใช้ DMA กับ ADC ต่อเนื่อง
 *
 * ADC continuous conversion → DMA เก็บลง buffer → อ่านค่าเฉลี่ย
 *
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit()"]
 *     C --> D["ADC_SimpleInit()"]
 *     D --> E["DMA_ADC_Init(DMA_CH1, adc_buf, 32, circular)"]
 *     E --> F["ADC_Cmd(ADC1, ENABLE)"]
 *     F --> G["while(1)"]
 *     G --> H["latest = adc_buf[31]"]
 *     H --> I["sum = average of 32 samples"]
 *     I --> J["USART_Print(Latest= X Avg= Y)"]
 *     J --> K["Delay_Ms(500)"]
 *     K --> G
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    ADC_SimpleInit();

    // DMA_ADC_Init — ตั้งค่า DMA สำหรับ ADC continuous (circular mode)
    uint16_t adc_buf[32];
    DMA_ADC_Init(DMA_CH1, adc_buf, 32, 1);  // circular, 32 samples

    // เริ่ม ADC continuous conversion
    ADC_Cmd(ADC1, ENABLE);

    while (1) {
        // อ่านค่าล่าสุดจาก buffer
        uint16_t latest = adc_buf[31];

        // คำนวณค่าเฉลี่ย
        uint32_t sum = 0;
        for (int i = 0; i < 32; i++) sum += adc_buf[i];
        uint16_t avg = (uint16_t)(sum / 32);

        USART_Print("Latest="); USART_PrintNum(latest);
        USART_Print(" Avg="); USART_PrintNum(avg);
        USART_Print("\r\n");
        Delay_Ms(500);
    }
}
