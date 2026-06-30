/**
 * ตัวอย่าง: Basic Arduino Aliases
 *
 * แสดงการใช้งาน macro aliases:
 * - millis()  ~ Get_CurrentMs()
 * - micros()  ~ Get_CurrentUs()
 * - delay()   ~ Delay_Ms()
 * - delayMicroseconds() ~ Delay_Us()
 * - interrupts()  ~ __enable_irq()
 * - noInterrupts() ~ __disable_irq()
 *
 * ผังวงจร:
 * - PC0 -> LED ( anode ผ่าน resistor 220 Ohm )
 *
 * ผลลัพธ์:
 * - LED กระพริบทุก 500ms (ใช้ millis() แบบ non-blocking)
 * - แสดง time stamp และระยะเวลา blink ทาง USART ทุก 2 วินาที
 * - วัด latency ของ delay() และ delayMicroseconds()
 *
 * คำเตือน:
 * - ต้องเรียก SystemCoreClockUpdate() ก่อนใช้ millis/micros/delay
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["pinMode(LED, OUTPUT)"]
 *     C --> D["while(1)"]
 *     D --> E["digitalToggle(LED)"]
 *     E --> F["delay(500)"]
 *     F --> G["USART_Print(millis())"]
 *     G --> H["delay(500)"]
 *     H --> D
 * ============================================================
 */
#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(PC0, PIN_MODE_OUTPUT);

    USART_Print("=== Arduino Basic Aliases Demo ===\r\n");
    USART_Print("\r\n");

    // วัด latency ของ delay() ด้วย micros()
    uint32_t t_start = micros();
    delay(1);
    uint32_t t_end = micros();
    USART_Print("delay(1) duration: ");
    USART_PrintNum(t_end - t_start);
    USART_Print(" us (expect ~1000)\r\n");

    // วัด latency ของ delayMicroseconds()
    t_start = micros();
    delayMicroseconds(10);
    t_end = micros();
    USART_Print("delayMicroseconds(10) duration: ");
    USART_PrintNum(t_end - t_start);
    USART_Print(" us\r\n\r\n");

    uint32_t prev_ms = 0;
    uint32_t blink_count = 0;

    while (1) {
        uint32_t now = millis();
        if (now - prev_ms >= 500) {
            prev_ms = now;
            blink_count++;
            digitalToggle(PC0);

            interrupts();
            USART_Print("Blink #");
            USART_PrintNum(blink_count);
            USART_Print(" at ");
            USART_PrintNum(now);
            USART_Print(" ms\r\n");
            noInterrupts();
        }
    }
}
