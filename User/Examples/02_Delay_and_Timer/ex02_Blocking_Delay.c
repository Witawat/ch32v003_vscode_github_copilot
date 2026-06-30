/**
 * ============================================================
 * ตัวอย่างที่ 2: ตัวอย่างการใช้งาน Blocking Delay (Delay_Ms, Delay_Us)
 * ============================================================
 *
 * แสดงการใช้ Delay_Ms() และ Delay_Us() แบบ blocking เพื่อควบคุม LED
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *                        +3.3V
 *                         |
 *                        [ ] 220Ω
 *                         |
 *     PC0 (Output) ----+----->| LED (Red)
 *                       |
 *                      GND
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 * - LED ON 200ms → OFF 100ms → ON 300ms → OFF 1s (sequence แบบแม่นยำ)
 * - LED กระพริบ 1us จำนวน 10 cycles (เร็วเกินไปสำหรับตามองเห็น)
 * - แสดงความแม่นยำของ Delay_Us ที่ระดับ microseconds
 * - ระหว่าง delay ระบบจะไม่ตอบสนองต่อการกดปุ่มหรือ interrupt อื่น
 * ============================================================
 * คำเตือน (WARNINGS):
 * - Delay_Ms() และ Delay_Us() บล็อกการทำงานทั้งหมด ไม่สามารถทำงานอื่นได้ระหว่าง delay
 * - ไม่สามารถประมวลผล interrupt อื่นได้ระหว่าง delay (ยกเว้น SysTick)
 * - ถ้าต้องการให้ระบบตอบสนองตลอดเวลา ให้ใช้ Non-blocking Timer แทน
 * - Delay_Us(0) หรือ Delay_Ms(0) จะ return ทันทีโดยไม่หน่วง
 * - การใช้ delay นานๆ จะทำให้ CPU ทำงานว่างเปล่า (waste cycles)
 * - ไม่เหมาะสำหรับงานที่ต้องการ real-time response
 * ============================================================
 * แผนผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit(115200)"]
 *     C --> D["pinMode(PC0, OUTPUT)"]
 *     D --> E["Print header"]
 *     E --> F["while(1)"]
 *     F --> G["digitalWrite(HIGH) + Delay_Ms(200)"]
 *     G --> H["digitalWrite(LOW) + Delay_Ms(100)"]
 *     H --> I["digitalWrite(HIGH) + Delay_Ms(300)"]
 *     I --> J["digitalWrite(LOW) + Delay_Ms(1000)"]
 *     J --> K["Print 'Testing Delay_Us'"]
 *     K --> L["for i = 0 to 9"]
 *     L --> M["digitalWrite(HIGH) + Delay_Us(1)"]
 *     M --> N["digitalWrite(LOW) + Delay_Us(1)"]
 *     N --> O{"i < 10?"}
 *     O -->|"Yes"| L
 *     O -->|"No"| F
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

// === ตัวแปร Global ===

static uint8_t led_pin = PC0;          // กำหนด pin LED ที่ PC0

/**
 * @brief ฟังก์ชันหลัก
 * @return ไม่มี return (loop ไม่มีที่สิ้นสุด)
 */
int main(void)
{
    // === เริ่มต้นระบบ ===

    SystemCoreClockUpdate();            // อัปเดตความถี่ระบบ (จำเป็นก่อนใช้ Delay)

    // === เริ่มต้น Timer และ USART ===

    Timer_Init();                       // เริ่มต้นระบบ SysTick timer (จำเป็นสำหรับ Delay)
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);  // เริ่มต้น USART ที่ 115200 baud

    // === ตั้งค่า GPIO ===

    pinMode(led_pin, PIN_MODE_OUTPUT);  // ตั้ง PC0 เป็น output สำหรับ LED

    // === แสดงข้อความเริ่มต้น ===

    USART_Print("Blocking Delay Example\r\n");  // แสดงชื่อตัวอย่าง
    USART_Print("LED Sequence Start!\r\n");     // แจ้งเริ่มทำงาน

    // === Main Loop (ไม่สิ้นสุด) ===

    while (1)
    {
        // === Sequence 1: LED ON 200ms, OFF 100ms ===

        USART_Print("Sequence: ON 200ms\r\n");  // แจ้งสถานะผ่าน USART
        digitalWrite(led_pin, HIGH);             // เปิด LED (ON)
        Delay_Ms(200);                          // หน่วง 200ms (blocking)

        USART_Print("Sequence: OFF 100ms\r\n"); // แจ้งสถานะผ่าน USART
        digitalWrite(led_pin, LOW);              // ปิด LED (OFF)
        Delay_Ms(100);                          // หน่วง 100ms (blocking)

        // === Sequence 2: LED ON 300ms, OFF 1000ms ===

        USART_Print("Sequence: ON 300ms\r\n");  // แจ้งสถานะผ่าน USART
        digitalWrite(led_pin, HIGH);             // เปิด LED (ON)
        Delay_Ms(300);                          // หน่วง 300ms (blocking)

        USART_Print("Sequence: OFF 1000ms\r\n"); // แจ้งสถานะผ่าน USART
        digitalWrite(led_pin, LOW);              // ปิด LED (OFF)
        Delay_Ms(1000);                         // หน่วง 1000ms หรือ 1 วินาที (blocking)

        // === แสดงความแม่นยำระดับ Microseconds ===

        USART_Print("Testing Delay_Us precision: toggling LED every 1us for 10 cycles\r\n");
        USART_Print("(Too fast for human eye - shows microsecond capability)\r\n");

        // กระพริบ LED ทุก 1us จำนวน 10 รอบ (เร็วมาก)
        for (uint8_t i = 0; i < 10; i++)        // วนลูป 10 ครั้ง
        {
            digitalWrite(led_pin, HIGH);         // เปิด LED
            Delay_Us(1);                         // หน่วง 1 microsecond (blocking)
            digitalWrite(led_pin, LOW);          // ปิด LED
            Delay_Us(1);                         // หน่วง 1 microsecond (blocking)
        }

        USART_Print("Delay_Us precision test complete!\r\n\r\n");  // แจ้งเสร็จสิ้น

        // === หมายเหตุ: ระหว่าง Delay CPU จะถูกบล็อกทั้งหมด ===

        // ถ้าต้องการให้ main loop ทำงานต่อเนื่องโดยไม่ถูกบล็อก
        // ให้ใช้ non-blocking timer แทน (ดู ex01)
    }

    // สิ่งนี้จะไม่มีวันถึง
    // return 0;
}
