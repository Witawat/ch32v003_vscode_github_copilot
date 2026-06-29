/**
 * ============================================================
 * ตัวอย่างที่ 7: ตัวอย่างการใช้งาน Countdown Timer (Countdown_Init, Countdown_Start, Countdown_Stop, Countdown_Reset, Countdown_IsFinished, Countdown_SetAlarmCallback, Countdown_GetRemainingSeconds)
 * ============================================================
 *
 * แสดงการใช้งาน Countdown Timer สำหรับนับถอยหลัง
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *                        +3.3V
 *                         |
 *                        [ ] 220?
 *                         |
 *     PC0 (Output) ----+----->| LED (Red) - Alarm Indicator
 *                       |
 *                      GND
 *
 *     PC1 (Input) ----[ ]----> GND    (Start/Pause)
 *                    Button
 *
 *     (Optional) Buzzer on PWM pin (PA1 or PC3)
 *
 *     USART (TX=PD5, RX=PD6) เชื่อมต่อ PC ผ่าน USB-Serial
 *
 *     (Pull-up ภายในของ CH32V003 ถูกใช้งานผ่าน PIN_MODE_INPUT_PULLUP)
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 * - ตั้งเวลา countdown 10 วินาที
 * - กดปุ่ม PC1  เริ่มนับถอยหลัง
 * - USART แสดงเวลาที่เหลือทุก 1 วินาที
 * - เมื่อหมดเวลา  Alarm callback ทำงาน  LED กระพริบเร็ว
 * - USART แสดง "Time's up!"
 * ============================================================
 * คำเตือน (WARNINGS):
 * - Countdown ใช้ TIM2 เป็น base timer ภายใน (เช่นเดียวกับ Stopwatch)!
 * - ห้ามใช้ Countdown และ Stopwatch พร้อมกัน (ทั้งคู่ใช้ TIM2)
 * - ต้องเรียก Countdown_Init() ก่อนใช้งานฟังก์ชันอื่น
 * - Callback (AlarmCallback) ทำงานใน main loop context (ไม่ใช่ ISR)
 * - Countdown_Stop() หยุดชั่วคราว (pause) - เริ่มต่อได้ด้วย Countdown_Start()
 * - Countdown_Reset() รีเซ็ตกลับเป็นเวลาเริ่มต้นและหยุดการนับ
 * - แม่นยำที่ระดับ milliseconds (resolution 1ms จาก TIM2)
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

// === ตัวแปร Global ===

static uint8_t led_alarm = PC0;          // pin PC0 สำหรับ LED alarm indicator
static uint8_t btn_start_pause = PC1;    // pin PC1 สำหรับปุ่ม Start/Pause
static volatile uint8_t alarm_triggered = 0;  // สถานะ alarm (1=หมดเวลาแล้ว)

/**
 * @brief Alarm Callback
 * @details ฟังก์ชันนี้ถูกเรียกเมื่อ countdown หมดเวลา
 *          ตั้งค่า flag เพื่อให้ main loop จัดการต่อไป
 */
void alarm_callback(void)
{
    // === Alarm ถูกเรียกเมื่อหมดเวลา ===

    alarm_triggered = 1;                 // ตั้ง flag แจ้ง main loop ว่า alarm ทำงาน

    // หมายเหตุ: callback นี้ทำงานใน context ปกติ (ไม่ใช่ ISR)
    // แต่ก็ควรทำงานให้ไวที่สุด
}

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

    // === ตั้งค่า GPIO ===

    pinMode(led_alarm, PIN_MODE_OUTPUT);       // ตั้ง PC0 เป็น output สำหรับ LED alarm
    pinMode(btn_start_pause, PIN_MODE_INPUT_PULLUP);  // ตั้ง PC1 เป็น input pull-up สำหรับปุ่ม

    // === เริ่มต้น Countdown ===

    // ตั้งเวลา countdown 0 ชั่วโมง, 0 นาที, 10 วินาที
    Countdown_Init(0, 0, 10);            // เริ่มต้น countdown ที่ 10 วินาที
    Countdown_SetAlarmCallback(alarm_callback);  // ตั้ง callback ที่จะถูกเรียกเมื่อหมดเวลา

    // หมายเหตุ: countdown ยังไม่เริ่มนับ ต้องเรียก Countdown_Start() ก่อน

    // === แสดงข้อความเริ่มต้น ===

    USART_Print("Countdown Example\r\n");           // แสดงชื่อตัวอย่าง
    USART_Print("===================\r\n");         // เส้นแบ่ง
    USART_Print("Countdown: 10 seconds\r\n");       // แจ้งเวลาที่ตั้ง
    USART_Print("Press PC1 to Start/Pause\r\n");    // แจ้งวิธีใช้งาน
    USART_Print("Status: Stopped\r\n");             // สถานะเริ่มต้น

    // === ตัวแปรสำหรับการทำงาน ===

    uint32_t last_report = 0;            // เก็บเวลาครั้งสุดท้ายที่รายงาน
    uint8_t button_prev = HIGH;          // สถานะก่อนหน้าของปุ่ม (สำหรับ edge detection)

    // === Main Loop (ไม่สิ้นสุด) ===

    while (1)
    {
        // === อ่านสถานะปุ่ม (Edge Detection) ===

        uint8_t btn_current = digitalRead(btn_start_pause);  // อ่านสถานะปุ่มปัจจุบัน

        // === Button: Start/Pause (ตรวจจับ Falling Edge) ===

        if (button_prev == HIGH && btn_current == LOW)  // ถ้าปุ่มเพิ่งถูกกด (HIGHLOW)
        {
            Delay_Ms(50);                    // หน่วง 50ms เพื่อ debounce
            if (digitalRead(btn_start_pause) == LOW)  // เช็คซ้ำว่ากดจริง
            {
                if (Countdown_IsRunning())   // ถ้า countdown กำลังทำงาน
                {
                    Countdown_Stop();        // หยุด countdown ชั่วคราว (pause)
                    USART_Print("Status: Paused\r\n");  // แจ้งสถานะ pause
                }
                else                         // ถ้า countdown หยุดอยู่
                {
                    // ถ้าหมดเวลาแล้ว ให้ reset ก่อนเริ่มใหม่
                    if (Countdown_IsFinished())  // ถ้าหมดเวลาแล้ว
                    {
                        Countdown_Reset();   // รีเซ็ตกลับเป็น 10 วินาที
                        alarm_triggered = 0; // ล้างสถานะ alarm
                        digitalWrite(led_alarm, LOW);  // ปิด LED alarm
                        USART_Print("Countdown Reset to 10s\r\n");  // แจ้ง reset
                    }

                    Countdown_Start();       // เริ่ม countdown
                    USART_Print("Status: Running\r\n");  // แจ้งสถานะ running
                }
            }
        }

        // === อัปเดตสถานะปุ่มสำหรับรอบถัดไป ===

        button_prev = btn_current;          // อัปเดตสถานะปุ่ม

        // === รายงานเวลาที่เหลือทุก 1 วินาที ===

        if (Countdown_IsRunning())           // ถ้า countdown กำลังทำงาน
        {
            if (ELAPSED_TIME(last_report, Get_CurrentMs()) >= 1000)  // ทุก 1 วินาที
            {
                last_report = Get_CurrentMs();  // อัปเดตเวลา

                // แสดงเวลาที่เหลือเป็นวินาที
                uint32_t remaining = Countdown_GetRemainingSeconds();  // อ่านวินาทีที่เหลือ
                USART_Print("Remaining: ");      // แสดงข้อความ
                USART_PrintNum(remaining);        // แสดงค่าที่เหลือ
                USART_Print(" seconds\r\n");      // หน่วยวินาที
            }
        }

        // === ตรวจสอบ Alarm ===

        if (alarm_triggered)                 // ถ้า alarm ถูก trigger
        {
            alarm_triggered = 0;             // ล้าง flag

            USART_Print("\r\n*** Time's up! ***\r\n");  // แจ้งหมดเวลา
            USART_Print("Countdown finished!\r\n");     // แจ้งเสร็จสิ้น

            // LED กระพริบเร็ว 10 ครั้ง
            for (uint8_t i = 0; i < 10; i++)  // กระพริบ 10 รอบ
            {
                digitalWrite(led_alarm, HIGH);  // เปิด LED
                Delay_Ms(100);                  // หน่วง 100ms
                digitalWrite(led_alarm, LOW);   // ปิด LED
                Delay_Ms(100);                  // หน่วง 100ms
            }

            USART_Print("Press PC1 to reset and start again\r\n");  // แจ้งวิธีเริ่มใหม่
        }
    }

    // สิ่งนี้จะไม่มีวันถึง
    // return 0;
}
