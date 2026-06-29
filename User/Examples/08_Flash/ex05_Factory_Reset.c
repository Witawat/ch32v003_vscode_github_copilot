/**
 * ============================================================
 * ex05_Factory_Reset.c
 * โปรแกรมรีเซ็ตค่าโรงงาน — กดปุ่ม 3 วินาทีเพื่อลบ Flash ทั้งหมด
 * (Factory reset — hold button 3 seconds to erase entire Flash)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *   CH32V003
 *   ------
 *   PA1 (OUT) ----[220?]----+---- LED (แดง) ---- GND
 *                            |
 *   PC0 (IN)  ----[10k?]----+---- GND  (ดึงลง)
 *                  |
 *                  +---- ปุ่มกด ---- VCC (3.3V)
 *
 *   PD5 (TX) ----> USB-UART (RX)
 *   PD6 (RX) <---- USB-UART (TX)
 *
 *   เมื่อกดปุ่มค้าง 3 วินาที: Factory reset
 *   LED กระพริบเร็วระหว่าง reset
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   "--- Flash Factory Reset ---"
 *   "System running. Hold button PC0 for 3s to factory reset..."
 *   (กดปุ่มค้าง 3 วินาที  LED กระพริบเร็ว)
 *   "Factory reset: 2 pages erased"
 *   (LED ดับ)
 *   "--- Done ---"
 * ============================================================
 * คำเตือน (WARNINGS):
 *   ? Flash_EraseAll() จะลบทั้งคอนฟิกและข้อมูล — ไม่สามารถกู้คืนได้!
 *     (Flash_EraseAll erases BOTH config and data pages irreversibly!)
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
    // ตัวแปรนับเวลา (Timing variables)
    uint32_t pressCount = 0;                 // ตัวนับจำนวนรอบที่กดปุ่มค้าง (Counter for button hold duration)
    uint8_t  buttonState = 0;                // สถานะปุ่มล่าสุด (Latest button state)

    // ---- ส่วนเริ่มต้น (Initialization) ----
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);                      // เริ่มต้นพอร์ตอนุกรม (Initialize USART)
    pinMode(PC0, PIN_MODE_INPUT_PULLUP);    // กำหนด PC0 เป็นอินพุตดึงขึ้น (Set PC0 as input with pull-up)
    pinMode(PA1, PIN_MODE_OUTPUT);          // กำหนด PA1 เป็นเอาต์พุต (Set PA1 as output)
    digitalWrite(PA1, LOW);                 // เริ่มต้น LED ดับ (Initialize LED off)

    USART_Print("--- Flash Factory Reset ---\r\n");  // แสดงหัวข้อ (Display title)
    Flash_Init();                            // เริ่มต้นโมดูล Flash (Initialize Flash module)

    USART_Print("System running. Hold button PC0 for 3s to factory reset...\r\n");  // แจ้งผู้ใช้ (Prompt user)

    // ---- วังวนหลัก (Main loop) ----
    while (1) {                              // วนรอบไม่สิ้นสุด (Infinite loop)
        buttonState = digitalRead(PC0);     // อ่านสถานะปุ่มกด (Read button state)

        if (buttonState == 0) {              // ถ้ากดปุ่ม (ดึงลง, Active Low) (If button is pressed, active low)
            pressCount++;                    // เพิ่มตัวนับ (Increment counter)
            if (pressCount >= 300) {         // 300 ? ~10ms = ~3000ms (3 seconds)
                USART_Print("Factory reset initiated!\r\n");  // แจ้งเริ่มรีเซ็ต (Notify factory reset start)

                // กระพริบ LED เร็ว ๆ (Fast LED blink)
                for (uint8_t i = 0; i < 10; i++) {  // กระพริบ 10 ครั้ง (Blink 10 times)
                    digitalWrite(PA1, HIGH);  // ติด LED (LED on)
                    Delay_Ms(50);            // หน่วง 50ms (Delay 50ms)
                    digitalWrite(PA1, LOW);  // ดับ LED (LED off)
                    Delay_Ms(50);            // หน่วง 50ms (Delay 50ms)
                }

                // ลบ Flash ทั้งหมด (Erase all Flash)
                Flash_EraseAll();            // ลบข้อมูลทั้งหมด (Erase all data)

                USART_Print("Factory reset: 2 pages erased\r\n");  // แจ้งลบสำเร็จ (Notify erase complete)
                digitalWrite(PA1, LOW);     // ดับ LED (LED off)

                pressCount = 0;              // รีเซ็ตตัวนับ (Reset counter)
            }
        } else {                             // ถ้าปล่อยปุ่ม (If button is released)
            pressCount = 0;                  // รีเซ็ตตัวนับ (Reset counter)
        }

        Delay_Ms(10);                        // หน่วง 10ms เพื่อลดการใช้ซีพียู (Delay 10ms to reduce CPU usage)
    }
}
