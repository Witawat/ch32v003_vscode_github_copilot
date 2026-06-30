/**
 * @example ex06_Advanced_Control.c
 * @brief OPAMP_Disable + OPAMP_ConfigVoltageFollower — ควบคุม OPAMP แบบประหยัดพลังงาน
 *
 * OPAMP_ConfigVoltageFollower: ตั้งค่า buffer แบบระบุ channel เอง (ไม่ใช้ SimpleInit default)
 * OPAMP_Disable: ปิด OPAMP ชั่วคราวเพื่อประหยัดพลังงาน (~10µA)
 *
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["ADC_SimpleInit()"]
 *     C --> D["USART_SimpleInit()"]
 *     D --> E["pinMode(PC0, OUTPUT)"]
 *     E --> F["OPAMP_ConfigVoltageFollower(CHP1)"]
 *     F --> G["OPAMP_Enable()"]
 *     G --> H["for i=0 to 4"]
 *     H --> I["ADC_Read(ADC_CH_PC4)"]
 *     I --> J["ADC_ToVoltage(raw, 3.3f)"]
 *     J --> K["USART_Print(ADC & V)"]
 *     K --> L["Delay_Ms(300)"]
 *     L --> H
 *     H --> M["OPAMP_Disable()"]
 *     M --> N["digitalWrite(PC0, HIGH)"]
 *     N --> O["Delay_Ms(2000)"]
 *     O --> P["OPAMP_Enable()"]
 *     P --> Q["digitalWrite(PC0, LOW)"]
 *     Q --> R["while(1) Delay_Ms(1000)"]
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    ADC_SimpleInit();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(PC0, PIN_MODE_OUTPUT);

    // 1. OPAMP_ConfigVoltageFollower — ใช้ CHP1 (PC4) แทน default CHP0 (PA2)
    OPAMP_ConfigVoltageFollower(OPAMP_CHP1);  // PC4 input
    OPAMP_Enable();
    USART_Print("OPAMP enabled (CHP1=PC4, voltage follower)\r\n");

    // อ่าน output — 5 ครั้ง
    for (int i = 0; i < 5; i++) {
        uint16_t raw = ADC_Read(ADC_CH_PC4);
        float v = ADC_ToVoltage(raw, 3.3f);
        USART_Print("ADC="); USART_PrintNum(raw);
        USART_Print(" V="); USART_PrintNum((int)(v * 1000)); USART_Print("mV\r\n");
        Delay_Ms(300);
    }

    // 2. OPAMP_Disable — ปิดเพื่อประหยัดพลังงาน ไม่ต้อง init ใหม่
    OPAMP_Disable();
    digitalWrite(PC0, HIGH);
    USART_Print("OPAMP disabled — power saving mode\r\n");

    Delay_Ms(2000);

    // เปิดใหม่
    OPAMP_Enable();
    digitalWrite(PC0, LOW);
    USART_Print("OPAMP re-enabled\r\n");

    while (1) { Delay_Ms(1000); }
}
