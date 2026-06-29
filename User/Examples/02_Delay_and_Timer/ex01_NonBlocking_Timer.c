/**
 * ============================================================
 * ตัวอย่างที่ 1: ตัวอย่างการใช้งาน Non-blocking Timer (Timer_t, Start_Timer, Is_Timer_Expired, Stop_Timer)
 * ============================================================
 *
 * แสดงการใช้ Non-blocking Timer เพื่อควบคุม LED โดยไม่บล็อกการทำงานของ main loop
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *                        +3.3V
 *                         |
 *                        [ ] 220Ω
 *                         |
 *     PC0 (Output) ----+----->| LED (Green)
 *                       |
 *                      GND
 *
 *     PC1 (Input) ----[ ]----> GND
 *                    Button
 *                    (Pull-up external)
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 * - LED บน PC0 กระพริบทุก 500ms โดยไม่บล็อก main loop
 * - Main loop แสดง "loop" ผ่าน USART ทุก 100ms แสดงว่าไม่ถูกบล็อก
 * - กดปุ่ม PC1 จะหยุด/เริ่ม การกระพริบ LED
 * - แสดงประสิทธิภาพของ non-blocking timer
 * ============================================================
 * คำเตือน (WARNINGS):
 * - ต้องเรียก Start_Timer() ก่อนใช้ Is_Timer_Expired() เสมอ
 * - Timer นี้ใช้ millis counter จาก SysTick ไม่ใช่ hardware timer จริง
 * - ความแม่นยำขึ้นอยู่กับ SysTick interrupt ที่ 1ms
 * - ไม่ควรใช้ timer ค่า 0ms (จะทำงานไม่ถูกต้อง)
 * - ถ้าใช้ repeat=0 ต้อง Start_Timer ใหม่ทุกครั้งที่หมดเวลา
 * ============================================================
 */

#include <SimpleHAL.h>

// === ตัวแปร Global ===

static Timer_t led_timer;          // ตัวแปร timer สำหรับควบคุม LED
static Timer_t debounce_timer;     // ตัวแปร timer สำหรับ debounce ปุ่ม (non-blocking)
static uint8_t led_state = 0;      // สถานะ LED ปัจจุบัน (0=OFF, 1=ON)
static uint8_t timer_running = 1;  // สถานะการทำงานของ timer (1=กำลังทำงาน)
static uint8_t btn_pending = 0;    // flag: มีปุ่มกดรอประมวลผล

/**
 * @brief ฟังก์ชันหลัก
 * @return ไม่มี return (loop ไม่มีที่สิ้นสุด)
 */
int main(void)
{
    // === เริ่มต้นระบบ ===

    SystemCoreClockUpdate();            // อัปเดตความถี่ระบบ (จำเป็นก่อนใช้ Delay/Timer)

    // === เริ่มต้น Timer และ USART ===

    Timer_Init();                       // เริ่มต้นระบบ SysTick timer (1ms resolution)
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);  // เริ่มต้น USART ที่ 115200 baud (TX=PD5, RX=PD6)

    // === ตั้งค่า GPIO ===

    pinMode(PC0, PIN_MODE_OUTPUT);      // ตั้ง PC0 เป็น output สำหรับ LED
    pinMode(PC1, PIN_MODE_INPUT_PULLUP);  // ตั้ง PC1 เป็น input pull-up สำหรับปุ่มกด

    // === เริ่มต้น Timer ===

    Start_Timer(&led_timer, 500, 1);    // เริ่ม timer ความถี่ 500ms, repeat=1 (ทำงานซ้ำ)
    timer_running = 1;                  // ตั้งค่าสถานะ timer ว่ากำลังทำงาน

    // === ตัวแปรแสดงเวลา ===

    uint32_t last_print = 0;            // เก็บเวลาครั้งสุดท้ายที่พิมพ์ "loop"

    // === Main Loop (ไม่สิ้นสุด) ===

    while (1)
    {
        // === อ่านสถานะปุ่มกด (non-blocking debounce) ===

        if (digitalRead(PC1) == LOW) {  // ตรวจจับปุ่มถูกกด
            if (!debounce_timer.active) {
                Start_Timer(&debounce_timer, 50, 0);  // เริ่มนับ 50ms debounce
            }
            if (Is_Timer_Expired(&debounce_timer) && !btn_pending) {
                btn_pending = 1;  // flag: รอปล่อยปุ่มก่อนประมวลผล
            }
        } else {
            // ปุ่มถูกปล่อย → ประมวลผล (หลังจาก debounce 50ms ผ่านแล้ว)
            if (btn_pending) {
                btn_pending = 0;
                if (timer_running) {
                    Stop_Timer(&led_timer);
                    timer_running = 0;
                    digitalWrite(PC0, LOW);
                    USART_Print("Timer Stopped\r\n");
                } else {
                    Start_Timer(&led_timer, 500, 1);
                    timer_running = 1;
                    USART_Print("Timer Started\r\n");
                }
            }
            // รีเซ็ต debounce timer เมื่อปล่อยปุ่ม
            if (debounce_timer.active) {
                Stop_Timer(&debounce_timer);
            }
        }

        // === ตรวจสอบ Non-blocking Timer ===

        if (timer_running)              // ถ้า timer กำลังทำงาน
        {
            if (Is_Timer_Expired(&led_timer))  // ตรวจสอบว่า Timer หมดเวลาหรือไม่ (ไม่บล็อก)
            {
                led_state = !led_state;  // สลับสถานะ LED
                digitalWrite(PC0, led_state);  // เขียนค่าไปยัง LED

                // แจ้งสถานะผ่าน USART
                if (led_state)
                {
                    USART_Print("LED ON\r\n");  // LED เปิด
                }
                else
                {
                    USART_Print("LED OFF\r\n");  // LED ปิด
                }
            }
        }

        // === แสดง Non-blocking Behavior ===

        if (ELAPSED_TIME(last_print, Get_CurrentMs()) >= 100)  // ทุก 100ms (ไม่บล็อก)
        {
            last_print = Get_CurrentMs();  // อัปเดตเวลา
            USART_Print("loop\r\n");       // พิมพ์ "loop" แสดงว่า main loop ยังทำงานปกติ
        }
    }

    // สิ่งนี้จะไม่มีวันถึง
    // return 0;
}
