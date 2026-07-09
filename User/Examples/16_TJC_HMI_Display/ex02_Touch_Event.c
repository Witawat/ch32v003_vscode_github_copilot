/**
 * ============================================================
 * TJC Example 2: Touch Event Callback
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *   TJC HMI Display (UART):
 *     TX  -> PD6 (MCU RX)
 *     RX  -> PD5 (MCU TX)
 *     VCC -> 5V หรือ 3.3V
 *     GND -> GND
 *   LED:
 *     Anode -> PD3 (ผ่าน resistor 220Ω)
 *     Cathode -> GND
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - กดปุ่ม b0 บนจอ → LED ติด
 *   - ปล่อยปุ่ม b0 → LED ดับ
 *   - กดปุ่ม b1 → ไปหน้า page 1
 * ============================================================
 * TJC Editor Setup:
 *   - สร้างปุ่ม b0 บน page 0
 *   - เปิด Touch Event สำหรับ b0
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>
#include "TJC.h"

#define LED_PIN PD3

void OnTouch(TJC_TouchEvent_t *event) {
    USART_Print("Touch: page=");
    USART_PrintNum(event->page_id);
    USART_Print(" comp=");
    USART_PrintNum(event->component_id);
    USART_Print(" type=");
    USART_PrintNum(event->event_type);
    USART_Print("\r\n");

    if (event->page_id == 0 && event->component_id == 0) {
        if (event->event_type == 0x01) {
            digitalWrite(LED_PIN, 1);
            USART_Print("LED ON\r\n");
        } else {
            digitalWrite(LED_PIN, 0);
            USART_Print("LED OFF\r\n");
        }
    }

    if (event->page_id == 0 && event->component_id == 1) {
        if (event->event_type == 0x01) {
            TJC_SendCommand("page 1");
        }
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(LED_PIN, PIN_MODE_OUTPUT);

    TJC_Init(BAUD_115200, USART_PINS_DEFAULT);
    TJC_RegisterTouchEventCallback(OnTouch);

    Delay_Ms(100);
    USART_Print("=== TJC Touch Event ===\r\n");

    TJC_SendCommand("page 0");

    while (1) {
        TJC_ProcessResponse();
    }
}
