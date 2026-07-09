/**
 * ============================================================
 * TJC Example 5: System Events & Error Handling
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *   TJC HMI Display (UART):
 *     TX  -> PD6 (MCU RX)
 *     RX  -> PD5 (MCU TX)
 *     VCC -> 5V หรือ 3.3V
 *     GND -> GND
 *   Buzzer:
 *     + -> PD2 (PWM)
 *     - -> GND
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - เมื่อ TJC startup → แสดง "Ready" บนจอ + beep
 *   - เมื่อ TJC sleep → บันทึกสถานะ
 *   - เมื่อ TJC wake → refresh หน้าจอ
 *   - เมื่อ error → แสดงข้อความทาง Serial
 * ============================================================
 * TJC Editor Setup:
 *   - ตั้งค่า thsp=30 (sleep หลัง 30 วินาที)
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>
#include "TJC.h"

#define BUZZER_PIN PD2

static uint8_t tjc_is_awake = 1;

void OnSystemEvent(uint8_t event_type) {
    switch (event_type) {
        case TJC_RET_STARTUP:
            USART_Print(">> TJC Startup!\r\n");
            tjc_is_awake = 1;
            TJC_SendCommand("bkcmd=1");
            TJC_SendCommand("page 0");
            TJC_SendCommand("t0.txt=\"Ready\"");
            TJC_SendCommand("dim=100");
            break;

        case TJC_RET_AUTO_SLEEP:
            USART_Print(">> TJC Sleep\r\n");
            tjc_is_awake = 0;
            break;

        case TJC_RET_AUTO_WAKE:
            USART_Print(">> TJC Wake!\r\n");
            tjc_is_awake = 1;
            TJC_SendCommand("page 0");
            TJC_SendCommand("dim=100");
            break;

        case TJC_RET_SD_UPGRADE:
            USART_Print(">> TJC SD Upgrade\r\n");
            break;
    }
}

void OnError(uint8_t error_code) {
    if (error_code == TJC_ERR_SUCCESS) {
        return;
    }

    USART_Print("TJC Error 0x");
    if (error_code < 0x10) USART_Print("0");
    USART_PrintHex(error_code, 1);
    USART_Print(": ");
    USART_Print(TJC_GetErrorString(error_code));
    USART_Print("\r\n");
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    TJC_Init(BAUD_115200, USART_PINS_DEFAULT);
    TJC_RegisterSystemEventCallback(OnSystemEvent);
    TJC_RegisterErrorCallback(OnError);

    Delay_Ms(100);
    USART_Print("=== TJC System Events ===\r\n");

    TJC_SendCommand("bkcmd=3");
    TJC_SendCommand("thsp=30");

    while (1) {
        TJC_ProcessResponse();

        if (!tjc_is_awake) {
            USART_Print("TJC is sleeping...\r\n");
            Delay_Ms(5000);
        }
    }
}
