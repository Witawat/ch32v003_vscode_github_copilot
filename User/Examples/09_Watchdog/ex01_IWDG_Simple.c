/**
 * ============================================================
 * ex01_IWDG_Simple.c
 * โปรแกรมสาธิต IWDG (Independent Watchdog) แบบง่าย
 * (Simple IWDG demonstration)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *   CH32V003
 *   ------
 *   PC0 (OUT) ----[220Ω]----+---- LED ---- GND
 *                            |
 *   PD5 (TX)  ----> USB-UART (RX)
 *   PD6 (RX)  <---- USB-UART (TX)
 *
 *   ไม่ต้องใช้อุปกรณ์อื่นเพิ่ม
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   LED กระพริบ 3 ครั้ง (IWDG ถูก feed ทุกครั้ง)
 *   → หยุด feed → MCU รีเซ็ต → เริ่มต้นใหม่ → กระพริบ 3 ครั้ง → วนซ้ำ
 *   USART แสดง:
 *   "--- IWDG Simple ---"
 *   "Blink 1: feeding watchdog"
 *   "Blink 2: feeding watchdog"
 *   "Blink 3: feeding watchdog"
 *   "Stop feeding! Reset in ~1.6s..."
 *   (MCU reset → เริ่มข้อความใหม่อีกครั้ง)
 * ============================================================
 * คำเตือน (WARNINGS):
 *   เมื่อ IWDG ถูกเปิดใช้งานแล้ว จะปิดไม่ได้จนกว่า MCU จะรีเซ็ต
 *     (Once IWDG is enabled, it cannot be stopped except by reset)
 *   ความถี่ LSI แปรผัน (30-60kHz ปกติ 40kHz) เวลาจริงอาจคลาดเคลื่อน ±25%
 *     (LSI frequency varies 30-60kHz, typical 40kHz — actual timeout may differ ±25%)
 * ============================================================
 */

#include <SimpleHAL.h>                      // รวมไลบรารี SimpleHAL (Include SimpleHAL library)

// --------------------------------------------------------------------------
// ฟังก์ชันหลัก (Main function)
// --------------------------------------------------------------------------

int main(void)
{
    SystemCoreClockUpdate();                    // อัปเดตความเร็วซีพียู (Update CPU clock speed)

    // ตัวแปรนับรอบ (Loop counter)
    uint8_t blinkCount = 0;                  // จำนวนครั้งที่ LED กระพริบ (Number of LED blinks)

    // ---- ส่วนเริ่มต้น (Initialization) ----
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);                      // เริ่มต้นพอร์ตอนุกรม (Initialize USART)
    pinMode(PC0, PIN_MODE_OUTPUT);          // กำหนด PC0 เป็นเอาต์พุต (Set PC0 as PIN_MODE_OUTPUT)
    digitalWrite(PC0, LOW);                 // เริ่มต้น LED ดับ (Initialize LED off)

    USART_Print("--- IWDG Simple ---\r\n"); // แสดงหัวข้อ (Display title)

    // ---- เริ่มต้น IWDG (Initialize IWDG) ----
    // IWDG_SimpleInit ใช้ค่าเริ่มต้น: prescaler=40, reload=255 → timeout ~1.6s
    // (IWDG_SimpleInit uses defaults: prescaler=40, reload=255 → timeout ~1.6s)
    IWDG_SimpleInit(1600);                       // เริ่ม IWDG ด้วยค่าตั้งต้น (Initialize IWDG with defaults)
    USART_Print("IWDG started, timeout ~1.6s\r\n");  // แจ้งเริ่ม IWDG (Notify IWDG start)

    // ---- กระพริบ LED 3 ครั้ง พร้อม feed IWDG (Blink LED 3 times while feeding IWDG) ----
    for (blinkCount = 1; blinkCount <= 3; blinkCount++) {  // วน 3 รอบ (Loop 3 times)
        digitalWrite(PC0, HIGH);            // ติด LED (LED on)
        USART_Print("Blink "); USART_PrintNum(blinkCount); USART_Print(": feeding watchdog\r\n");  // แจ้งรอบที่ feed watchdog (Notify feeding)
        IWDG_Feed();                         // รีเซ็ตตัวนับ IWDG (Reset IWDG counter) — ป้องกันรีเซ็ต (prevents reset)
        Delay_Ms(200);                       // หน่วง 200ms (Delay 200ms)

        digitalWrite(PC0, LOW);             // ดับ LED (LED off)
        IWDG_Feed();                         // feed อีกครั้ง (Feed again)
        Delay_Ms(200);                       // หน่วง 200ms (Delay 200ms)
    }

    // ---- หยุด feed — IWDG จะรีเซ็ต MCU หลังจาก timeout (Stop feeding — IWDG will reset MCU after timeout) ----
    USART_Print("Stop feeding! Reset in ~1.6s...\r\n");  // แจ้งว่าหยุด feed (Notify stop feeding)
    // ไม่ต้อง IWDG_Feed() อีก (No more IWDG_Feed() calls)

    // IWDG จะนับถอยหลัง เมื่อถึง 0 จะรีเซ็ต MCU (IWDG counts down; when it hits 0, MCU resets)
    while (1) {                              // รอ IWDG รีเซ็ต (Wait for IWDG reset)
        // ไม่ทำอะไร ให้ Watchdog รีเซ็ต (Do nothing, let watchdog reset)
        // __NOP(); เป็นคำสั่งเปล่า (NOP — no operation)
    }
}
