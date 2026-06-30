/**
 * ============================================================
 * ex03_WWDG_Simple.c
 * โปรแกรมสาธิต WWDG (Window Watchdog) แบบง่าย
 * (Simple WWDG demonstration)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *   CH32V003
 *   ------
 *   PC0 (OUT) ----[220?]----+---- LED ---- GND
 *                            |
 *   PD5 (TX)  ----> USB-UART (RX)
 *   PD6 (RX)  <---- USB-UART (TX)
 *
 *   ไม่ต้องใช้อุปกรณ์อื่นเพิ่ม
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   LED ติดค้าง  WWDG รีเฟรช 5 ครั้ง  หยุดรีเฟรช  MCU รีเซ็ต  เริ่มใหม่
 *   USART แสดง:
 *   "--- WWDG Simple ---"
 *   "Refresh #1: feeding WWDG"
 *   "Refresh #2: feeding WWDG"
 *   "Refresh #3: feeding WWDG"
 *   "Refresh #4: feeding WWDG"
 *   "Refresh #5: feeding WWDG"
 *   "Stop refreshing! Reset in ~5.4ms..."
 *   (MCU reset  ข้อความซ้ำอีกครั้ง)
 * ============================================================
 * คำเตือน (WARNINGS):
 *   WWDG มีข้อจำกัดเรื่อง Window: รีเฟรชเร็วเกินไป (counter > window)
 *     ก็ทำให้รีเซ็ตเช่นกัน! ต้องรีเฟรชเมื่อ window < counter < 0x40
 *     (WWDG has WINDOW constraint: refresh too EARLY also causes reset!)
 *   เวลา timeout สูงสุด ~87ms ที่ prescaler = 8
 *     (Max timeout ~87ms at prescaler 8)
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>                      // รวมไลบรารี SimpleHAL (Include SimpleHAL library)

// --------------------------------------------------------------------------
// ฟังก์ชันหลัก (Main function)
// --------------------------------------------------------------------------

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    // ตัวแปรนับรอบรีเฟรช (Refresh counter)
    uint8_t refreshCount = 0;                // จำนวนครั้งที่รีเฟรช WWDG (Number of WWDG refreshes)

    // ---- ส่วนเริ่มต้น (Initialization) ----
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);                      // เริ่มต้นพอร์ตอนุกรม (Initialize USART)
    pinMode(PC0, PIN_MODE_OUTPUT);          // กำหนด PC0 เป็นเอาต์พุต (Set PC0 as PIN_MODE_OUTPUT)
    digitalWrite(PC0, LOW);                 // เริ่มต้น LED ดับ (Initialize LED off)

    USART_Print("--- WWDG Simple ---\r\n"); // แสดงหัวข้อ (Display title)

    // ---- เริ่มต้น WWDG (Initialize WWDG) ----
    // WWDG_SimpleInit(127, 80)
    // counter = 127 (ค่าเริ่มต้น สูงสุด)  (initial counter, max value)
    // window  = 80  (ต้องรีเฟรชเมื่อ counter < 80)  (must refresh when counter < 80)
    // prescaler ใช้ค่าเริ่มต้น = 8
    WWDG_SimpleInit(0x7F, 80);               // เริ่ม WWDG: counter=127, window=80 (Init WWDG)

    digitalWrite(PC0, HIGH);                // ติด LED แสดงว่าทำงาน (Turn LED on to indicate active)

    USART_Print("WWDG started: counter=127, window=80\r\n");  // แจ้งค่าเริ่มต้น (Notify init values)

    // ---- รีเฟรช WWDG 5 ครั้ง (Refresh WWDG 5 times) ----
    for (refreshCount = 1; refreshCount <= 5; refreshCount++) {  // วน 5 รอบ (Loop 5 times)
        // หน่วงเวลาเล็กน้อยเพื่อให้ counter ลดลงถึงช่วง window (Small delay to let counter enter window range)
        Delay_Ms(1);                         // หน่วง 1ms ให้ counter ลดลง (Delay 1ms for counter to decrease)

        WWDG_Refresh(0x7F);                  // รีเฟรช WWDG (Refresh WWDG)
        USART_Print("Refresh #"); USART_PrintNum(refreshCount); USART_Print(": feeding WWDG\r\n");  // แจ้งรอบที่รีเฟรช (Notify refresh round)
    }

    // ---- หยุดรีเฟรช — WWDG จะรีเซ็ต MCU (Stop refreshing — WWDG will reset MCU) ----
    USART_Print("Stop refreshing! Reset in ~5.4ms...\r\n");  // แจ้งว่าหยุดรีเฟรช (Notify stop refreshing)

    // WWDG จะลดค่าลงเรื่อย ๆ เมื่อถึง 0x3F (63) จะรีเซ็ต (WWDG decrements; when reaching 0x3F (63), resets)
    while (1) {                              // รอ WWDG รีเซ็ต (Wait for WWDG reset)
        // ไม่มีการ WWDG_Feed() อีก (No more WWDG_Feed() calls)
    }
}
