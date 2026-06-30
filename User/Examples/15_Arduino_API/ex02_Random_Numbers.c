/**
 * ตัวอย่าง: Random Number Generator
 *
 * แสดงการใช้งาน:
 * - randomSeed(seed)  — ตั้งค่า seed สำหรับ PRNG
 * - _randomMax(max)   — สุ่มค่า [0, max-1]
 * - _randomRange(min, max) — สุ่มค่า [min, max-1]
 *
 * ผังวงจร:
 * - PC0 -> LED1
 * - PC1 -> LED2
 *
 * ผลลัพธ์:
 * - สุ่ม LED เปิด/ปิด ทุกครั้งที่สุ่ม
 * - แสดงค่าสุ่มและช่วงทาง USART
 *
 * หมายเหตุ:
 * - PRNG ใช้ LCG glibc-style (1103515245 * seed + 12345) mod 2^31
 * - ใช้ _randomMax() / _randomRange() แทน random() เพราะ stdlib.h
 *   มี long random(void) อยู่แล้ว C จึงใช้ชื่อ random โดยตรงไม่ได้
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART + pinMode x2"]
 *     C --> D["randomSeed()"]
 *     D --> E["while(1)"]
 *     E --> F["_randomMax(10)"]
 *     F --> G{"r1 < 5?"}
 *     G -->|"Yes"| H["LED1 ON, LED2 OFF"]
 *     G -->|"No"| I["LED1 OFF, LED2 ON"]
 *     H --> J["_randomRange(100, 500)"]
 *     I --> J
 *     J --> K["Delay_Ms(delay_ms)"]
 *     K --> E
 * ============================================================
 */
#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(PC0, PIN_MODE_OUTPUT);
    pinMode(PC1, PIN_MODE_OUTPUT);

    randomSeed(Get_CurrentMs());

    USART_Print("=== Random Number Demo ===\r\n");
    USART_Print("--------------------------\r\n");

    while (1) {
        long r1 = _randomMax(10);
        USART_Print("_randomMax(10) = ");
        USART_PrintNum(r1);
        USART_Print("  -> ");
        if (r1 < 5) {
            digitalWrite(PC0, HIGH);
            digitalWrite(PC1, LOW);
            USART_Print("LED1 ON\r\n");
        } else {
            digitalWrite(PC0, LOW);
            digitalWrite(PC1, HIGH);
            USART_Print("LED2 ON\r\n");
        }

        long delay_ms = _randomRange(100, 501);
        USART_Print("_randomRange(100, 500) = ");
        USART_PrintNum(delay_ms);
        USART_Print(" ms\r\n\r\n");

        Delay_Ms(delay_ms);
    }
}
