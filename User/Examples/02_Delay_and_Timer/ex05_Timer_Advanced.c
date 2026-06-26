/**
 * ============================================================
 * ตัวอย่างที่ 5: ตัวอย่างการใช้งาน Timer แบบ Advanced (TIM_AdvancedInit, TIM_SetPrescaler, TIM_SetMode, TIM_GetPeriod, Simple_TIM_GetCounter, TIM_GenerateUpdate)
 * ============================================================
 *
 * แสดงการปรับแต่ง Timer แบบละเอียดด้วยการตั้งค่า Prescaler และ Period ด้วยตนเอง
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
 *     USART (TX=PD5, RX=PD6) เชื่อมต่อ PC ผ่าน USB-Serial
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 * - เริ่มต้น timer ที่ 10kHz (PSC=2399, Period=9 @24MHz)
 * - อ่าน counter ใน loop และพิมพ์เมื่อ counter wrap (overflow)
 * - เปลี่ยน prescaler กลางโปรแกรมเพื่อแสดงการเปลี่ยนความถี่
 * - แสดงค่าความถี่ที่คำนวณจากสูตร SystemCoreClock/((PSC+1)*(Period+1))
 * ============================================================
 * คำเตือน (WARNINGS):
 * - ความถี่ = SystemCoreClock / ((prescaler+1) * (period+1))
 * - ที่ SystemCoreClock=24MHz: PSC=2399, Period=9 → 1kHz (ถูก)
 * - ที่ SystemCoreClock=24MHz: PSC=2399, Period=0 → 10kHz
 * - ที่ SystemCoreClock=48MHz (ถ้า HSE): ค่า PSC/Period ต่างจาก 24MHz
 * - TIM_SetPrescaler() มีผลในรอบถัดไปเท่านั้น ต้องเรียก TIM_GenerateUpdate() เพื่อใช้ทันที
 * - 16-bit timer: สูงสุด 65535 สำหรับ PSC และ Period
 * - ความถี่ต่ำสุดที่ 24MHz: ~0.366Hz (PSC=65535, Period=65535)
 * - ความถี่สูงสุดที่ 24MHz: 24MHz (PSC=0, Period=0)
 * ============================================================
 */

#include <SimpleHAL.h>

// === ตัวแปร Global ===

static uint8_t led_pin = PC0;            // pin PC0 สำหรับ LED
static uint16_t last_counter = 0;        // เก็บค่า counter ก่อนหน้า (เพื่อตรวจจับ wrap)

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

    pinMode(led_pin, PIN_MODE_OUTPUT);   // ตั้ง PC0 เป็น output สำหรับ LED

    // === แสดงข้อมูลระบบ ===

    USART_Print("Timer Advanced Example\r\n");       // แสดงชื่อตัวอย่าง
    USART_Print("SystemCoreClock: ");                // แสดงข้อความ clock
    USART_PrintNum(SystemCoreClock);                 // แสดงความถี่ CPU
    USART_Print(" Hz\r\n");                          // หน่วย Hz

    // === ตั้งค่า Timer แบบ Advanced ===

    // ที่ SystemCoreClock=24MHz: ต้องการ 10kHz
    // สูตร: Frequency = SystemCoreClock / ((prescaler+1) * (period+1))
    // 10000 = 24000000 / ((PSC+1) * (PER+1))
    // (PSC+1) * (PER+1) = 2400
    // เลือก PSC=239, PER=9 → 24000000 / (240 * 10) = 10000 Hz = 10kHz
    // (ปรับให้ LED กระพริบถี่ขึ้น - เห็นผลชัดเจน)

    uint16_t prescaler = 239;            // ค่า prescaler (PSC)
    uint16_t period = 9;                 // ค่า period (ARR)
    TIM_Mode mode = TIM_MODE_UP;         // โหมดนับขึ้น

    // แสดงค่าที่ตั้ง
    USART_Print("Initial: PSC=");        // แสดงข้อความ PSC
    USART_PrintNum(prescaler);           // แสดงค่า prescaler
    USART_Print(", Period=");            // แสดงข้อความ Period
    USART_PrintNum(period);              // แสดงค่า period
    USART_Print(", Mode=UP");            // แสดงโหมด

    // คำนวณความถี่จริง
    uint32_t calc_freq = SystemCoreClock / ((uint32_t)(prescaler + 1) * (period + 1));  // คำนวณความถี่
    USART_Print(" -> Frequency: ");      // แสดงข้อความ frequency
    USART_PrintNum(calc_freq);           // แสดงค่าความถี่ที่คำนวณ
    USART_Print(" Hz\r\n");              // หน่วย Hz

    // เริ่มต้น Timer
    TIM_AdvancedInit(TIM_1, prescaler, period, mode);  // ตั้งค่า timer ด้วย PSC, Period, Mode
    TIM_Start(TIM_1);                    // เริ่มการทำงานของ timer

    // === Main Loop (ไม่สิ้นสุด) ===

    uint32_t last_report = 0;            // เก็บเวลาครั้งสุดท้ายที่รายงาน
    uint8_t freq_changed = 0;            // สถานะเปลี่ยนความถี่แล้วหรือยัง

    while (1)
    {
        // === อ่านค่า Counter ปัจจุบัน ===

        uint16_t counter = Simple_TIM_GetCounter(TIM_1);  // อ่านค่า counter ปัจจุบันของ TIM1
        uint16_t current_period = TIM_GetPeriod(TIM_1);   // อ่านค่า period ปัจจุบัน

        // === ตรวจจับ Counter Wrap ===

        // ถ้า counter น้อยกว่า last_counter แสดงว่าเกิด overflow (wrap around)
        if (counter < last_counter)      // ตรวจจับการ wrap ของ counter
        {
            USART_Print("Counter wrapped! ");  // แจ้ง wrap
            USART_Print("Period=");             // แสดงข้อความ period
            USART_PrintNum(current_period);     // แสดงค่า period
            USART_Print(", Counter=");          // แสดงข้อความ counter
            USART_PrintNum(counter);            // แสดงค่า counter
            USART_Print("\r\n");                // ขึ้นบรรทัดใหม่

            digitalWrite(led_pin, !digitalRead(led_pin));  // กระพริบ LED ทุกครั้งที่ wrap
        }

        last_counter = counter;          // อัปเดตค่า counter สำหรับรอบถัดไป

        // === รายงานค่าทุก 2 วินาที ===

        if (ELAPSED_TIME(last_report, Get_CurrentMs()) >= 2000)
        {
            last_report = Get_CurrentMs();  // อัปเดตเวลา

            // แสดงค่า Counter และ Period ปัจจุบัน
            USART_Print("Counter=");         // แสดงข้อความ counter
            USART_PrintNum(counter);         // แสดงค่า counter
            USART_Print(", Period=");        // แสดงข้อความ period
            USART_PrintNum(current_period);  // แสดงค่า period
            USART_Print(", PSC=");           // แสดงข้อความ prescaler
            USART_PrintNum(Simple_TIM_GetPrescaler(TIM_1));  // แสดงค่า prescaler
            USART_Print("\r\n");             // ขึ้นบรรทัดใหม่

            // === เปลี่ยน Prescaler กลางโปรแกรม (หลังจาก 5 วินาทีแรก) ===

            if (!freq_changed && (Get_CurrentMs() > 5000))  // ถ้ายังไม่เคยเปลี่ยนและเวลาผ่านเกิน 5s
            {
                freq_changed = 1;            // ตั้งค่าสถานะว่าเปลี่ยนแล้ว

                // เปลี่ยน Prescaler จาก 239 → 119 (เพิ่มความถี่ 2 เท่า)
                // ใหม่: PSC=119, Period=9 → 24000000/(120*10) = 20000 Hz = 20kHz
                uint16_t new_psc = 119;      // ค่า prescaler ใหม่

                USART_Print("\r\n*** Changing Prescaler from ");  // แจ้งเปลี่ยน
                USART_PrintNum(prescaler);    // แสดงค่าเดิม
                USART_Print(" to ");           // คำว่า "to"
                USART_PrintNum(new_psc);       // แสดงค่าใหม่
                USART_Print(" ***\r\n");       // ปิดข้อความ

                TIM_SetPrescaler(TIM_1, new_psc);  // ตั้งค่า prescaler ใหม่
                TIM_SetMode(TIM_1, TIM_MODE_DOWN);  // เปลี่ยนโหมดเป็นนับลง (เพื่อแสดง TIM_SetMode)
                TIM_GenerateUpdate(TIM_1);    // สร้าง update event เพื่อให้ค่าใหม่มีผลทันที

                // คำนวณความถี่ใหม่
                uint32_t new_freq = SystemCoreClock / ((uint32_t)(new_psc + 1) * (period + 1));  // คำนวณใหม่
                USART_Print("New Frequency: ");  // แสดงข้อความ
                USART_PrintNum(new_freq);     // แสดงค่าความถี่ใหม่
                USART_Print(" Hz, Mode=DOWN\r\n\r\n");  // หน่วย Hz และโหมด

                // กระพริบ LED เร็วๆ เพื่อบอกว่าเปลี่ยน mode
                for (uint8_t i = 0; i < 5; i++)  // กระพริบ 5 ครั้ง
                {
                    digitalWrite(led_pin, HIGH);  // เปิด LED
                    Delay_Ms(50);                 // หน่วง 50ms
                    digitalWrite(led_pin, LOW);   // ปิด LED
                    Delay_Ms(50);                 // หน่วง 50ms
                }
            }
        }
    }

    // สิ่งนี้จะไม่มีวันถึง
    // return 0;
}
