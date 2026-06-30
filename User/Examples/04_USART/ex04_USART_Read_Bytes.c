/**
 * ============================================================
 * ตัวอย่างที่ 4: USART Read Bytes
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003              USB-Serial
 *     --------              ----------
 *     PD5 (TX) ------------> RX
 *     PD6 (RX) <------------ TX
 *     GND ------------------ GND
 *
 *     (เหมือนกับ ex01)
 *     สงขอความ "Send 5 bytes:" แลวรอรับขอมูล 5 ไบต
 *
 * API ที่ใช้:
 *   USART_ReadBytes(buffer, length)    // อ่านข้อมูลตามจำนวนที่กำหนด
 *   USART_Print("ข้อความ")              // พิมพ์ข้อความ
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - MCU ส่ง "Send 5 bytes:" ไปยัง Serial Monitor
 *   - รอรับข้อมูล 5 ไบต์จากผู้ใช้
 *   - เมื่อครบ 5 ไบต์ แสดง "Received: XX XX XX XX XX"
 * ============================================================
 * คำเตือน (WARNINGS):
 *   - USART_ReadBytes() เป็น blocking จนกว่าจะได้รับข้อมูลครบตามจำนวน
 *   - ต้องส่งให้ครบ 5 ไบต์ ไม่เช่นนั้นโปรแกรมจะค้างอยู่ที่บรรทัดนี้
 *   - ควรใช้ร่วมกับ USART_Available() ถ้าต้องการไม่ให้ blocking
 *   - แสดงผลเป็นเลขฐานสิบหก 2 หลัก
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit(115200)"]
 *     C --> D["while(1)"]
 *     D --> E["USART_Print(Send 5 bytes: )"]
 *     E --> F["USART_ReadBytes(buffer, 5)"]
 *     F --> G["USART_Print(Received: )"]
 *     G --> H["for i = 0 to 4"]
 *     H --> I{"buffer[i] < 16?"}
 *     I -->|"Yes"| J["Print leading 0"]
 *     I -->|"No"| K["USART_PrintHex(buffer[i])"]
 *     J --> K
 *     K --> L["Print space if not last"]
 *     L --> H
 *     H --> M["USART_Print(\\r\\n)"]
 *     M --> D
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

int main(void)
{
    // อัปเดตความถี่ระบบ
    SystemCoreClockUpdate();

    Timer_Init();
    // เริ่มต้น USART: Baud 115200
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    // บัฟเฟอร์สำหรับรับข้อมูล 5 ไบต์
    uint8_t buffer[5];
    int i;

    // วนรอบไม่สิ้นสุด
    while (1)
    {
        // ส่งข้อความขอให้ผู้ใช้ส่งข้อมูล
        USART_Print("Send 5 bytes:\r\n");

        // รอรับข้อมูล 5 ไบต์ (blocking จนกว่าจะครบ)
        USART_ReadBytes(buffer, 5);

        // แสดงผลข้อมูลที่รับมา
        USART_Print("Received: ");

        // วนลูปแสดงทีละไบต์
        for (i = 0; i < 5; i++)
        {
            // แสดงในรูปเลขฐานสิบหก 2 หลัก
            if (buffer[i] < 16)
            {
                USART_Print("0");
            }
            USART_PrintHex(buffer[i], 1);

            // เว้นวรรคระหว่างไบต์
            if (i < 4)
            {
                USART_Print(" ");
            }
        }

        // ขึ้นบรรทัดใหม่
        USART_Print("\r\n");
    }
}
