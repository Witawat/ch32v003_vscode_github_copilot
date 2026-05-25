/**
 * ============================================================
 * ตัวอย่างที่ 3: ตัวอย่างการอ่านค่าเวลา (Get_CurrentMs, Get_CurrentUs, Get_TickMicros, ELAPSED_TIME)
 * ============================================================
 *
 * แสดงการอ่านค่าจาก millis/micros timer และคำนวณ elapsed time
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     USART (TX=PD5, RX=PD6) เชื่อมต่อกับ PC ผ่าน USB-Serial
 *
 *                        +3.3V
 *                         |
 *                        [ ] 220Ω
 *                         |
 *     PC0 (Output) ----+----->| LED (Yellow)
 *                       |
 *                      GND
 *
 *     PC ---[USB-Serial]--- PD5 (TX)
 *                          PD6 (RX)
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 * - USART พิมพ์ "Milliseconds: XXXX, Microseconds: XXXXXX" ทุก 1 วินาที
 * - แสดง ELAPSED_TIME ที่แม่นยำระหว่างเหตุการณ์ต่างๆ
 * - LED กระพริบทุกครั้งที่พิมพ์ข้อความ
 * - วัดเวลาที่ใช้ในการทำงานแต่ละส่วนของโค้ด
 * ============================================================
 * คำเตือน (WARNINGS):
 * - Get_CurrentUs() overflow หลังจาก ~71 นาที (2^32 microseconds)
 * - Get_CurrentMs() overflow หลังจาก ~49 วัน (2^32 milliseconds)
 * - ควรใช้ ELAPSED_TIME macro แทนการลบตรงๆ เพื่อป้องกัน overflow error
 * - Get_TickMicros() คืนค่า 0-999 (เฉพาะ microseconds ใน tick ปัจจุบัน)
 * - Get_CurrentUs() ปิด interrupt ชั่วคราวเพื่อความแม่นยำ
 * - ค่าเวลามีไว้สำหรับเปรียบเทียบ ไม่ใช่เวลาจริงของโลก
 * ============================================================
 */

#include <SimpleHAL.h>

// === ตัวแปร Global ===

static uint8_t led_pin = PC0;          // กำหนด pin LED ที่ PC0
static uint32_t loop_count = 0;        // ตัวนับจำนวนรอบของ main loop

/**
 * @brief ฟังก์ชันหลัก
 * @return ไม่มี return (loop ไม่มีที่สิ้นสุด)
 */
int main(void)
{
    // === เริ่มต้นระบบ ===

    SystemCoreClockUpdate();            // อัปเดตความถี่ระบบ (จำเป็นก่อนใช้ Timer)

    // === เริ่มต้น Timer และ USART ===

    Timer_Init();                       // เริ่มต้นระบบ SysTick timer (1ms resolution)
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);  // เริ่มต้น USART ที่ 115200 baud (TX=PD5, RX=PD6)

    // === ตั้งค่า GPIO ===

    pinMode(led_pin, PIN_MODE_OUTPUT);  // ตั้ง PC0 เป็น output สำหรับ LED

    // === แสดงข้อความเริ่มต้น ===

    USART_Print("Time Reading Example\r\n");     // แสดงชื่อตัวอย่าง
    USART_Print("========================\r\n");  // เส้นแบ่ง

    // === ตัวแปรสำหรับ Time Reading ===

    uint32_t last_report = 0;           // เก็บเวลาครั้งสุดท้ายที่รายงานค่า
    uint32_t start_time = 0;            // เก็บเวลาเริ่มต้น
    uint32_t elapsed = 0;               // เก็บเวลาที่ผ่านไป

    // === เริ่มจับเวลา ===

    start_time = Get_CurrentMs();       // บันทึกเวลาเริ่มต้น (milliseconds)
    uint32_t start_us = Get_CurrentUs();  // บันทึกเวลาเริ่มต้น (microseconds)

    // === Main Loop (ไม่สิ้นสุด) ===

    while (1)
    {
        // === อ่านค่าปัจจุบัน ===

        uint32_t current_ms = Get_CurrentMs();       // อ่านเวลา millisecond ปัจจุบัน
        uint32_t current_us = Get_CurrentUs();       // อ่านเวลา microsecond ปัจจุบัน
        uint32_t tick_us = Get_TickMicros();         // อ่าน microseconds ใน tick ปัจจุบัน (0-999)

        // === รายงานค่าทุก 1 วินาที ===

        if (ELAPSED_TIME(last_report, current_ms) >= 1000)  // ถ้าผ่านไป 1 วินาที
        {
            last_report = current_ms;               // อัปเดตเวลาครั้งสุดท้าย

            // กระพริบ LED ทุกครั้งที่รายงาน
            digitalWrite(led_pin, HIGH);             // เปิด LED
            Delay_Ms(50);                            // หน่วง 50ms (เพื่อให้เห็นกระพริบ)
            digitalWrite(led_pin, LOW);              // ปิด LED

            // แสดงค่าเวลาผ่าน USART
            USART_Print("Milliseconds: ");           // แสดงข้อความ milliseconds
            USART_PrintNum(current_ms);              // แสดงค่า millisecond ปัจจุบัน
            USART_Print(", Microseconds: ");         // แสดงข้อความ microseconds
            USART_PrintNum(current_us);              // แสดงค่า microsecond ปัจจุบัน
            USART_Print("\r\n");                     // ขึ้นบรรทัดใหม่

            // แสดง Tick Microseconds
            USART_Print("TickMicros: ");             // แสดงข้อความ TickMicros
            USART_PrintNum(tick_us);                 // แสดงค่า TickMicros
            USART_Print(" (within current SysTick)\r\n");  // คำอธิบายเพิ่มเติม

            // แสดง Elapsed Time ตั้งแต่เริ่มต้น
            elapsed = ELAPSED_TIME(start_time, current_ms);  // คำนวณ elapsed time (overflow-safe)
            USART_Print("Elapsed since start: ");    // แสดงข้อความ elapsed
            USART_PrintNum(elapsed);                 // แสดงค่า elapsed milliseconds
            USART_Print(" ms\r\n");                  // หน่วย milliseconds

            // แสดง Elapsed Time ใน microseconds
            uint32_t elapsed_us = current_us - start_us;  // คำนวณ elapsed microseconds
            USART_Print("Elapsed micros: ");         // แสดงข้อความ elapsed micros
            USART_PrintNum(elapsed_us);              // แสดงค่า elapsed microseconds
            USART_Print(" us\r\n");                  // หน่วย microseconds

            // แสดง Loop Count
            loop_count++;                            // เพิ่มจำนวนรอบ
            USART_Print("Loop count: ");             // แสดงข้อความ loop count
            USART_PrintNum(loop_count);              // แสดงค่า loop count
            USART_Print("\r\n\r\n");                 // ขึ้นบรรทัดใหม่ 2 บรรทัด
        }

        // === วัดเวลาที่ใช้ในการทำงานบางส่วน ===

        // ตัวอย่าง: วัดว่า USART_Print ใช้เวลานานเท่าไหร่
        uint32_t t1 = Get_CurrentUs();               // บันทึกเวลาก่อน
        USART_Print("");                             // ส่งข้อความว่าง
        uint32_t t2 = Get_CurrentUs();               // บันทึกเวลาหลัง
        uint32_t usart_time = t2 - t1;               // คำนวณเวลาที่ใช้ (microseconds)

        // ถ้ามากกว่า 100us ให้รายงาน (แสดงทุกครั้งที่เกิน threshold)
        if (usart_time > 100)                        // ถ้า USART ใช้เวลาเกิน 100us
        {
            USART_Print("Note: USART took ");       // แจ้งเตือน
            USART_PrintNum(usart_time);              // แสดงเวลาที่ใช้
            USART_Print(" us\r\n");                  // หน่วย microseconds
        }
    }

    // สิ่งนี้จะไม่มีวันถึง
    // return 0;
}
