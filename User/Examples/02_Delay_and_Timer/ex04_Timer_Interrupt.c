/**
 * ============================================================
 * ตัวอย่างที่ 4: ตัวอย่างการใช้งาน Timer Interrupt (TIM_SimpleInit, TIM_AttachInterrupt, TIM_Start, TIM_Stop)
 * ============================================================
 *
 * แสดงการใช้ Hardware Timer Interrupt เพื่อสลับสถานะ LED ทุก 500ms
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *                        +3.3V
 *                         |
 *                        [ ] 220Ω
 *                         |
 *     PC0 (Output) ----+----->| LED (Blue)
 *                       |
 *                      GND
 *
 *     PC1 (Input) ----[ ]----> GND
 *                    Button
 *                    (Pull-up external)
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 * - Timer interrupt ยิงทุก 500ms → ISR สลับสถานะ LED
 * - Main loop ไม่ต้องทำอะไรเกี่ยวกับ LED (ISR จัดการให้)
 * - กดปุ่ม PC1 เพื่อเริ่ม/หยุด timer
 * - แสดงการทำงานแบบ interrupt-driven
 * ============================================================
 * คำเตือน (WARNINGS):
 * - TIM1 ขัดแย้งกับ SimplePWM! ให้ใช้ TIM2 สำหรับตัวอย่างนี้
 * - Callback ทำงานใน ISR context - ต้องทำงานให้เสร็จไวที่สุด!
 * - ห้ามใช้ Delay_Ms() ใน ISR (Delay ใช้ SysTick interrupt)
 * - ห้ามเรียก USART_Print() ใน ISR (blocking นานเกินไป)
 * - ควรใช้ volatile สำหรับตัวแปรที่แชร์กับ ISR
 * - TIM2 เป็น shared resource - ระวังการใช้งานร่วมกับ module อื่น
 * ============================================================
 */

#include <SimpleHAL.h>

// === ตัวแปร Global (แชร์กับ ISR) ===

static volatile uint8_t led_state = 0;   // สถานะ LED (volatile เพราะเปลี่ยนใน ISR)
static uint8_t led_pin = PC0;            // pin PC0 สำหรับ LED
static uint8_t button_pin = PC1;         // pin PC1 สำหรับปุ่มกด

/**
 * @brief Timer Interrupt Callback
 * @details ฟังก์ชันนี้ถูกเรียกทุกครั้งที่ timer overflow (ทุก 500ms)
 *          ทำงานใน ISR context - ต้องกระชับและรวดเร็ว!
 *
 * @warning ห้ามใช้ Delay, USART_Print, หรือฟังก์ชัน blocking ใดๆ ในนี้!
 */
void timer_isr_callback(void)
{
    // === ISR Context: ทำงานเร็วที่สุด! ===

    led_state = !led_state;              // สลับสถานะ LED (รวดเร็ว)
    digitalWrite(led_pin, led_state);    // เขียนค่าไปยัง LED โดยตรง (ใช้ register-level)

    // หมายเหตุ: ใน ISR ไม่ควรทำอะไรมากกว่านี้
    // ถ้าต้องการส่งข้อมูล ใช้ flag แล้วจัดการใน main loop แทน
}

/**
 * @brief ฟังก์ชันหลัก
 * @return ไม่มี return (loop ไม่มีที่สิ้นสุด)
 */
int main(void)
{
    // === เริ่มต้นระบบ ===

    SystemCoreClockUpdate();            // อัปเดตความถี่ระบบ

    // === เริ่มต้น USART ===

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);  // เริ่มต้น USART ที่ 115200 baud

    // === ตั้งค่า GPIO ===

    pinMode(led_pin, PIN_MODE_OUTPUT);         // ตั้ง PC0 เป็น output สำหรับ LED
    pinMode(button_pin, PIN_MODE_INPUT_PULLUP);  // ตั้ง PC1 เป็น input pull-up สำหรับปุ่ม

    // === ตั้งค่า Timer ===

    // ใช้ TIM2 (ไม่ขัดแย้งกับ SimplePWM ที่ใช้ TIM1)
    TIM_SimpleInit(TIM_2, 2);                  // ตั้งค่า TIM2 ที่ 2Hz (500ms ต่อรอบ)
    TIM_AttachInterrupt(TIM_2, timer_isr_callback);  // ผูก callback กับ timer interrupt
    TIM_Start(TIM_2);                          // เริ่มการทำงานของ timer

    // === แสดงข้อความเริ่มต้น ===

    USART_Print("Timer Interrupt Example\r\n");    // แสดงชื่อตัวอย่าง
    USART_Print("TIM2 configured at 2Hz (500ms)\r\n");  // แจ้งความถี่ timer
    USART_Print("Press PC1 button to start/stop\r\n");  // แจ้งวิธีใช้งาน

    // === ตัวแปรสำหรับ main loop ===

    uint32_t last_print = 0;                 // เก็บเวลาครั้งสุดท้ายที่พิมพ์สถานะ

    // === Main Loop (ไม่สิ้นสุด) ===

    while (1)
    {
        // === อ่านสถานะปุ่มกด ===

        if (digitalRead(button_pin) == LOW)  // ถ้ากดปุ่ม (active LOW)
        {
            Delay_Ms(50);                    // หน่วง 50ms เพื่อ debounce
            if (digitalRead(button_pin) == LOW)  // เช็คซ้ำว่ากดจริง
            {
                // ตรวจสอบสถานะ timer ปัจจุบันโดยใช้ TIM_GetPeriod เพื่อดูว่ากำลังทำงาน
                // (ไม่มี TIM_IsRunning ดังนั้นเราจะใช้ตัวแปรอื่น)

                USART_Print("Button pressed!\r\n");  // แจ้งผ่าน USART

                // หยุด timer (ถัดไปจะใช้ตัวแปร flag แทน)
                if (TIM_GetPeriod(TIM_2) > 0)  // ถ้า timer ยังทำงาน (period > 0)
                {
                    TIM_Stop(TIM_2);           // หยุด timer
                    digitalWrite(led_pin, LOW);  // ปิด LED
                    USART_Print("Timer Stopped\r\n");  // แจ้งหยุด
                }
                else
                {
                    TIM_Start(TIM_2);          // เริ่ม timer ใหม่
                    USART_Print("Timer Started\r\n");  // แจ้งเริ่ม
                }

                while (digitalRead(button_pin) == LOW);  // รอปล่อยปุ่ม
            }
        }

        // === Main Loop ทำงานอื่นๆ ===

        // ตัวอย่าง: พิมพ์สถานะทุก 2 วินาที
        if (ELAPSED_TIME(last_print, Get_CurrentMs()) >= 2000)
        {
            last_print = Get_CurrentMs();      // อัปเดตเวลา

            // แสดงสถานะ LED ปัจจุบัน (LED ถูกควบคุมโดย ISR)
            USART_Print("Main loop running, LED state: ");  // แสดงข้อความ
            if (led_state)                     // ถ้า LED เปิด
            {
                USART_Print("ON\r\n");         // แสดง ON
            }
            else
            {
                USART_Print("OFF\r\n");        // แสดง OFF
            }

            // แสดงค่าตัวนับปัจจุบัน
            uint16_t counter = Simple_TIM_GetCounter(TIM_2);  // อ่าน counter
            USART_Print("TIM2 Counter: ");     // แสดงข้อความ counter
            USART_PrintNum(counter);            // แสดงค่า counter
            USART_Print("\r\n");                // ขึ้นบรรทัดใหม่
        }

        // ISR จะจัดการ LED โดยอัตโนมัติ - main loop ไม่ต้องยุ่ง!
    }

    // สิ่งนี้จะไม่มีวันถึง
    // return 0;
}
