/**
 * ============================================================
 * TJC Example 4: Numeric & String Callback
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *   TJC HMI Display (UART):
 *     TX  -> PD6 (MCU RX)
 *     RX  -> PD5 (MCU TX)
 *     VCC -> 5V หรือ 3.3V
 *     GND -> GND
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - ทุก 3 วินาที ส่งคำสั่ง get n0.val และ get t0.txt
 *   - แสดงค่าที่ได้รับทาง Serial
 * ============================================================
 * TJC Editor Setup:
 *   - สร้าง numeric component n0
 *   - สร้าง text component t0
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>
#include <string.h>
#include "TJC.h"

void OnNumeric(uint32_t value) {
    USART_Print("Numeric received: ");
    USART_PrintNum(value);
    USART_Print("\r\n");
}

void OnString(const char *str, uint16_t len) {
    USART_Print("String received (len=");
    USART_PrintNum(len);
    USART_Print("): ");
    USART_Print(str);
    USART_Print("\r\n");

    if (strcmp(str, "ready") == 0) {
        USART_Print(">> TJC is ready!\r\n");
    }
}

void OnPageId(uint8_t page_id) {
    USART_Print("Page changed: ");
    USART_PrintNum(page_id);
    USART_Print("\r\n");
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    TJC_Init(BAUD_115200, USART_PINS_DEFAULT);
    TJC_RegisterNumericCallback(OnNumeric);
    TJC_RegisterStringCallback(OnString);
    TJC_RegisterPageIdCallback(OnPageId);

    Delay_Ms(100);
    USART_Print("=== TJC Numeric & String Callback ===\r\n");

    TJC_SendCommand("page 0");

    uint32_t last_query = 0;

    while (1) {
        TJC_ProcessResponse();

        uint32_t now = Get_CurrentMs();
        if (now - last_query >= 3000) {
            last_query = now;

            USART_Print("--- Querying TJC ---\r\n");
            TJC_SendCommand("get n0.val");
            TJC_SendCommand("get t0.txt");
            TJC_SendCommand("sendme");
        }
    }
}
