/**
 * ============================================================
 * ex04_WWDG_Interrupt.c
 * โปรแกรมสาธิต WWDG Interrupt (Early Wakeup Interrupt — EWI)
 * (WWDG Early Wakeup Interrupt demonstration)
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
 *   เมื่อ counter ถึง 0x40, EWI fires → callback พิมพ์:
 *     "Early warning! Refreshing..."
 *   → รีเฟรช WWDG → ไม่มีการรีเซ็ต
 *   USART แสดงวนไปเรื่อย ๆ:
 *   "--- WWDG Interrupt ---"
 *   "WWDG with interrupt started"
 *   "Early warning! Refreshing..."
 *   "Early warning! Refreshing..."
 *   ...
 * ============================================================
 * คำเตือน (WARNINGS):
 *   Interrupt แค่เตือน — ยังต้องรีเฟรช WWDG ในช่วง window เพื่อป้องกันรีเซ็ต
 *     (The interrupt only warns — must still refresh within window to avoid reset)
 *   ต้องเพิ่ม WWDG_IRQHandler ใน ch32v00x_it.c
 *     โดยให้เรียก WWDG_IRQHandler_Callback()
 *     (Must add WWDG_IRQHandler in ch32v00x_it.c calling WWDG_IRQHandler_Callback())
 * ============================================================
 */

#include <SimpleHAL.h>                      // รวมไลบรารี SimpleHAL (Include SimpleHAL library)

// --------------------------------------------------------------------------
// ตัวแปรและค่าคงที่ (Variables and constants)
// --------------------------------------------------------------------------

static volatile uint8_t earlyWarningFired = 0;  // ตัวแปรบอกว่า EWI เกิดขึ้นแล้ว (Flag for EWI occurrence)

// --------------------------------------------------------------------------
// ฟังก์ชัน callback สำหรับ WWDG interrupt (Callback function for WWDG interrupt)
// --------------------------------------------------------------------------

void WWDG_EarlyWarningCallback(void)         // ฟังก์ชันนี้ถูกเรียกเมื่อ EWI เกิดขึ้น (Called when EWI fires)
{
    earlyWarningFired = 1;                   // ตั้ง flag ว่าเกิด EWI (Set flag that EWI occurred)
    USART_Print("Early warning! Refreshing...\r\n");  // แสดงข้อความเตือน (Display warning message)

    // รีเฟรช WWDG เพื่อป้องกันรีเซ็ต (Refresh WWDG to prevent reset)
    WWDG_Refresh(0x7F);                      // รีเฟรช WWDG (Refresh WWDG)
}

// --------------------------------------------------------------------------
// ฟังก์ชันหลัก (Main function)
// --------------------------------------------------------------------------

int main(void)
{
    SystemCoreClockUpdate();                    // อัปเดตความเร็วซีพียู (Update CPU clock speed)

    // ---- ส่วนเริ่มต้น (Initialization) ----
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);                      // เริ่มต้นพอร์ตอนุกรม (Initialize USART)
    pinMode(PC0, PIN_MODE_OUTPUT);          // กำหนด PC0 เป็นเอาต์พุต (Set PC0 as PIN_MODE_OUTPUT)
    digitalWrite(PC0, HIGH);                // ติด LED แสดงว่าทำงาน (Turn LED on to indicate active)

    USART_Print("--- WWDG Interrupt ---\r\n");  // แสดงหัวข้อ (Display title)

    // ---- เริ่ม WWDG พร้อม Interrupt (Initialize WWDG with Interrupt) ----
    WWDG_SetCallback(WWDG_EarlyWarningCallback);      // ตั้ง callback สำหรับ Early Wakeup (Set callback for EWI)
    WWDG_InitWithInterrupt(0x7F, 0x50, 8);            // เริ่ม WWDG: counter=127, window=80, prescaler=8

    USART_Print("WWDG with interrupt started\r\n");  // แจ้งว่าเริ่ม WWDG แบบมี Interrupt (Notify interrupt mode started)

    // ---- วังวนหลัก (Main loop) ----
    while (1) {                              // วนรอบไม่สิ้นสุด (Infinite loop)
        if (earlyWarningFired == 1) {        // ถ้าเกิด EWI (If EWI occurred)
            earlyWarningFired = 0;           // รีเซ็ต flag (Reset flag)
        }

        // กระพริบ LED ช้า ๆ แสดงว่า MCU ยังทำงาน (Slow blink to show MCU is alive)
        digitalWrite(PC0, !digitalRead(PC0));  // กลับสถานะ LED (Toggle LED)
        Delay_Ms(500);                       // หน่วง 500ms (Delay 500ms)
    }
}
