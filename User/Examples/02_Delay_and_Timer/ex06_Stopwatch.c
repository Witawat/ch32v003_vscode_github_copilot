/**
 * ============================================================
 * ตัวอย่างที่ 6: ตัวอย่างการใช้งาน Stopwatch (Stopwatch_Init, Stopwatch_Start, Stopwatch_Stop, Stopwatch_Reset, Stopwatch_GetTimeString, Stopwatch_GetTotalSeconds)
 * ============================================================
 *
 * แสดงการใช้งาน Stopwatch สำหรับจับเวลาแบบนับขึ้น
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     Button1 (PC0) ----[ ]----> GND    (Start/Stop)
 *                      Start/Stop
 *
 *     Button2 (PC1) ----[ ]----> GND    (Reset)
 *                      Reset
 *
 *     USART (TX=PD5, RX=PD6) เชื่อมต่อ PC ผ่าน USB-Serial
 *
 *     (Pull-up ภายในของ CH32V003 ถูกใช้งานผ่าน PIN_MODE_INPUT_PULLUP)
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 * - กด Button1 (PC0): เริ่มนับเวลา / หยุดนับเวลา
 * - กด Button2 (PC1): รีเซ็ต stopwatch กลับเป็น 00:00:00
 * - USART แสดงเวลาในรูปแบบ "HH:MM:SS" หรือจำนวนวินาทีทั้งหมด
 * - รายงานเวลาทุก 100ms เพื่อความละเอียดสูง
 * ============================================================
 * คำเตือน (WARNINGS):
 * - Stopwatch ใช้ TIM2 เป็น base timer ภายใน - ห้ามใช้ TIM2 เพื่อวัตถุประสงค์อื่น
 * - การใช้ Stopwatch ร่วมกับ Countdown (ex07) พร้อมกันไม่ได้ (ทั้งคู่ใช้ TIM2)
 * - Stopwatch ใช้ TIM2 ที่ 1000Hz → resolution 1ms
 * - ต้องเรียก Stopwatch_Init() ก่อนใช้งานฟังก์ชันอื่น
 * - Stopwatch_Reset() หยุดการนับและรีเซ็ตเป็น 0
 * - เวลาสูงสุด ~49 วัน (overflow ของ uint32_t milliseconds)
 * ============================================================
 */

#include <SimpleHAL.h>

// === ตัวแปร Global ===

static uint8_t btn_start_stop = PC0;   // pin PC0 สำหรับปุ่ม Start/Stop
static uint8_t btn_reset = PC1;        // pin PC1 สำหรับปุ่ม Reset

/**
 * @brief ฟังก์ชันหลัก
 * @return ไม่มี return (loop ไม่มีที่สิ้นสุด)
 */
int main(void)
{
    // === เริ่มต้นระบบ ===

    SystemCoreClockUpdate();
    Timer_Init();
    // === เริ่มต้น USART ===

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);  // เริ่มต้น USART ที่ 115200 baud

    // === ตั้งค่า GPIO สำหรับปุ่มกด ===

    pinMode(btn_start_stop, PIN_MODE_INPUT_PULLUP);  // ตั้ง PC0 เป็น input pull-up (Button1)
    pinMode(btn_reset, PIN_MODE_INPUT_PULLUP);       // ตั้ง PC1 เป็น input pull-up (Button2)

    // === เริ่มต้น Stopwatch ===

    Stopwatch_Init();                   // เริ่มต้นระบบ Stopwatch (ใช้ TIM2 ที่ 1000Hz)
    // Stopwatch ยังไม่เริ่มนับ ต้องเรียก Stopwatch_Start() ก่อน

    // === แสดงข้อความเริ่มต้น ===

    USART_Print("Stopwatch Example\r\n");           // แสดงชื่อตัวอย่าง
    USART_Print("===================\r\n");         // เส้นแบ่ง
    USART_Print("PC0: Start/Stop\r\n");            // แจ้งการทำงานของปุ่ม
    USART_Print("PC1: Reset\r\n");                 // แจ้งการทำงานของปุ่ม
    USART_Print("Status: Stopped\r\n");            // สถานะเริ่มต้น
    USART_Print("Time: 00:00:00\r\n");             // เวลาเริ่มต้น

    // === ตัวแปรสำหรับการทำงาน ===

    uint32_t last_display = 0;          // เก็บเวลาครั้งสุดท้ายที่อัปเดตหน้าจอ
    uint8_t button1_prev = HIGH;        // สถานะก่อนหน้าของ Button1 (สำหรับ edge detection)
    uint8_t button2_prev = HIGH;        // สถานะก่อนหน้าของ Button2 (สำหรับ edge detection)

    // === Main Loop (ไม่สิ้นสุด) ===

    while (1)
    {
        // === อ่านสถานะปุ่ม (Edge Detection) ===

        uint8_t btn1_current = digitalRead(btn_start_stop);  // อ่านสถานะ Button1 ปัจจุบัน
        uint8_t btn2_current = digitalRead(btn_reset);       // อ่านสถานะ Button2 ปัจจุบัน

        // === Button1: Start/Stop (ตรวจจับ Falling Edge) ===

        if (button1_prev == HIGH && btn1_current == LOW)  // ถ้าปุ่มเพิ่งถูกกด (HIGH→LOW)
        {
            Delay_Ms(50);                    // หน่วง 50ms เพื่อ debounce
            if (digitalRead(btn_start_stop) == LOW)  // เช็คซ้ำว่ากดจริง
            {
                if (Stopwatch_IsRunning())   // ถ้า stopwatch กำลังทำงาน
                {
                    Stopwatch_Stop();        // หยุด stopwatch
                    USART_Print("Status: Paused\r\n");  // แจ้งสถานะ pause
                }
                else                         // ถ้า stopwatch หยุดอยู่
                {
                    Stopwatch_Start();       // เริ่ม stopwatch ต่อ
                    USART_Print("Status: Running\r\n");  // แจ้งสถานะ running
                }
            }
        }

        // === Button2: Reset (ตรวจจับ Falling Edge) ===

        if (button2_prev == HIGH && btn2_current == LOW)  // ถ้าปุ่มเพิ่งถูกกด (HIGH→LOW)
        {
            Delay_Ms(50);                    // หน่วง 50ms เพื่อ debounce
            if (digitalRead(btn_reset) == LOW)  // เช็คซ้ำว่ากดจริง
            {
                Stopwatch_Reset();           // รีเซ็ต stopwatch กลับเป็น 00:00:00
                USART_Print("Status: Reset\r\n");   // แจ้งสถานะ reset
                USART_Print("Time: 00:00:00\r\n");  // แสดงเวลา 0
            }
        }

        // === อัปเดตสถานะปุ่มสำหรับรอบถัดไป ===

        button1_prev = btn1_current;        // อัปเดตสถานะ Button1
        button2_prev = btn2_current;        // อัปเดตสถานะ Button2

        // === แสดงเวลาทุก 100ms ===

        if (ELAPSED_TIME(last_display, Get_CurrentMs()) >= 100)  // ทุก 100ms
        {
            last_display = Get_CurrentMs();  // อัปเดตเวลา

            // แสดงเวลาในรูปแบบ HH:MM:SS
            char time_buf[32];               // buffer สำหรับเก็บ string เวลา
            Stopwatch_GetTimeString(time_buf, TIME_FORMAT_HHMMSS, TIME_DISPLAY_NORMALIZED);  // อ่านเวลาเป็น string

            USART_Print("Time: ");           // แสดงข้อความ Time
            USART_Print(time_buf);           // แสดงค่าเวลา

            // แสดงเวลาทั้งหมดในหน่วยวินาที
            uint32_t total_sec = Stopwatch_GetTotalSeconds();  // อ่านเวลาทั้งหมดเป็นวินาที
            USART_Print(" (");               // เปิดวงเล็บ
            USART_PrintNum(total_sec);        // แสดงจำนวนวินาที
            USART_Print(" sec)\r\n");         // ปิดวงเล็บและขึ้นบรรทัดใหม่
        }
    }

    // สิ่งนี้จะไม่มีวันถึง
    // return 0;
}
