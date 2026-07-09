/**
 * ============================================================
 * TJC Example 1: Basic Commands
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *   TJC HMI Display (UART):
 *     TX  -> PD6 (MCU RX)
 *     RX  -> PD5 (MCU TX)
 *     VCC -> 5V หรือ 3.3V (ตามสเปคจอ)
 *     GND -> GND
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - จอแสดง page 0
 *   - ข้อความ "Hello TJC!" ใน t0
 *   - ค่าตัวเลข 42 ใน n0
 *   - Progress bar j0 ที่ 75%
 *   - ความสว่างหน้าจอ 50%
 * ============================================================
 * คำเตือน (WARNINGS):
 *   - ต้องตั้งค่า bkcmd=3 ใน TJC Editor เพื่อรับ response
 *   - ต้องเพิ่ม TJC_UART_IRQHandler() ใน USART1_IRQHandler
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>
#include "TJC.h"

void OnError(uint8_t error_code) {
    if (error_code != TJC_ERR_SUCCESS) {
        USART_Print("TJC Error: ");
        USART_Print(TJC_GetErrorString(error_code));
        USART_Print("\r\n");
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    TJC_Init(BAUD_115200, USART_PINS_DEFAULT);
    TJC_RegisterErrorCallback(OnError);

    Delay_Ms(100);

    USART_Print("=== TJC Basic Commands ===\r\n");

    TJC_SendCommand("bkcmd=3");
    TJC_SendCommand("page 0");
    TJC_SendCommand("t0.txt=\"Hello TJC!\"");
    TJC_SendCommand("n0.val=42");
    TJC_SendCommand("j0.val=75");
    TJC_SendCommand("dim=50");

    TJC_SendCommand("vis b0,1");
    Delay_Ms(2000);
    TJC_SendCommand("vis b0,0");

    TJC_SendCommand("click b0,1");
    Delay_Ms(100);
    TJC_SendCommand("click b0,0");

    while (1) {
        TJC_ProcessResponse();
    }
}
