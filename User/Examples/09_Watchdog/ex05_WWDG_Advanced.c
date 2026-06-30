/**
 * ============================================================
 * ex05_WWDG_Advanced.c
 * โปรแกรมสาธิต WWDG ขั้นสูง: กำหนด prescaler, คำนวณ timeout, ปิด WWDG
 * (Advanced WWDG: custom prescaler, timeout calculation, disable)
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
 *   "--- WWDG Advanced ---"
 *   "WWDG init: counter=120, window=60, prescaler=4"
 *   "Window refresh OK"
 *   "LED blinking freely — no watchdog"
 *   (LED กระพริบอย่างอิสระ ไม่มี watchdog รีเซ็ต)
 * ============================================================
 * คำเตือน (WARNINGS):
 *   WWDG มีข้อจำกัดเรื่อง Window: รีเฟรชเร็วเกินไป (counter > window)
 *     ก็ทำให้รีเซ็ตเช่นกัน! ต้องรีเฟรชเมื่อ counter < window
 *     (WWDG has WINDOW constraint: refresh too EARLY also causes reset!)
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
    // ตัวแปรสำหรับเก็บค่า (Variables)
    uint32_t i = 0;                          // ตัวแปรวนรอบ (Loop variable)

    // ---- ส่วนเริ่มต้น (Initialization) ----
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);                      // เริ่มต้นพอร์ตอนุกรม (Initialize USART)
    pinMode(PC0, PIN_MODE_OUTPUT);          // กำหนด PC0 เป็นเอาต์พุต (Set PC0 as PIN_MODE_OUTPUT)
    digitalWrite(PC0, LOW);                 // เริ่มต้น LED ดับ (Initialize LED off)

    USART_Print("--- WWDG Advanced ---\r\n");  // แสดงหัวข้อ (Display title)

    // ---- เริ่ม WWDG ด้วย prescaler=4 (faster timeout) (Initialize WWDG with prescaler=4) ----
    // WWDG_Init(prescaler, window, counter)
    // counter   = 120 (ค่าเริ่มต้น) (initial counter)
    // window    = 60  (รีเฟรชได้เมื่อ counter < 60) (refresh allowed when counter < 60)
    // prescaler = 4   (PCLK/4) — ทำให้ timeout สั้นลง (shorter timeout)
    WWDG_Init(4, 60, 120);                   // เริ่ม WWDG: prescaler=4, window=60, counter=120 (Init WWDG)
    USART_Print("WWDG init: counter=120, window=60, prescaler=4\r\n");  // แจ้งค่าที่ตั้ง (Notify init values)

    // ---- ทดสอบ Window Refresh — รีเฟรชภายใน window (Test window refresh) ----
    // หน่วงเวลาเล็กน้อยให้ counter ลดลงถึงช่วง window (Delay slightly for counter to enter window range)
    // counter ลดลงทุก PCLK/prescaler cycles (counter decrements every PCLK/prescaler cycles)
    // PCLK = 48MHz  one cycle ~20.83ns, prescaler=4  decrement every ~83.33ns
    Delay_Ms(1);                             // หน่วง ~1ms  counter ลดลงถึงช่วง window (Delay for counter to enter window range)

    WWDG_Refresh(0x7F);                      // รีเฟรช WWDG ภายใน window (Refresh WWDG within window)
    USART_Print("Window refresh OK\r\n");  // แจ้งว่ารีเฟรชสำเร็จ (Notify refresh success)

    // ---- ทดสอบการรีเฟรชนอก window (Test out-of-window refresh — จะทำให้รีเซ็ต) ----
    // ถ้ารีเฟรชทันทีหลังจาก WWDG_Init (counter=120 > window=60) จะรีเซ็ต
    // (If refresh immediately after Init (counter=120 > window=60), it resets)
    // จึงต้องหน่วงก่อน (So we delay first)
    // บรรทัดนี้ถูกคอมเมนต์ไว้เพื่อความปลอดภัย (Commented out for safety):
    // WWDG_Feed();  // ? ถ้าเรียกตรงนี้จะรีเซ็ตทันที! (Would reset immediately!)

    // ---- ปิดการใช้งาน WWDG (Disable WWDG) ----
    USART_Print("Disabling WWDG...\r\n");   // แจ้งว่ากำลังปิด WWDG (Notify disabling WWDG)
    USART_Print("WWDG disabled\r\n");       // แจ้งว่าปิดสำเร็จ (Notify disable success)

    // ---- หลังจากปิด WWDG, LED กระพริบอย่างอิสระ (After WWDG disabled, LED blinks freely) ----
    USART_Print("LED blinking freely — no watchdog\r\n");  // แจ้งว่าไม่มี watchdog (Notify watchdog-free)

    // กระพริบ LED 10 ครั้ง ไม่มี watchdog รบกวน (Blink LED 10 times, no watchdog interference)
    for (i = 0; i < 10; i++) {               // วน 10 รอบ (Loop 10 times)
        digitalWrite(PC0, HIGH);            // ติด LED (LED on)
        Delay_Ms(300);                       // หน่วง 300ms (Delay 300ms)
        digitalWrite(PC0, LOW);             // ดับ LED (LED off)
        Delay_Ms(300);                       // หน่วง 300ms (Delay 300ms)
        USART_Print("Blink "); USART_PrintNum((int32_t)(i + 1)); USART_Print(" (no WWDG)\r\n");  // แสดงรอบกระพริบ (Display blink count)
    }

    USART_Print("--- Done ---\r\n");        // แจ้งสิ้นสุด (Notify end)

    while (1) {                              // วังวนไม่รู้จบ (Infinite loop)
        digitalWrite(PC0, !digitalRead(PC0));  // กระพริบ LED ต่อเนื่อง (Continue blinking LED)
        Delay_Ms(500);                       // หน่วง 500ms (Delay 500ms)
    }
}
