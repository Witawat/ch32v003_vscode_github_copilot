/**
 * ============================================================
 * ตัวอย่างที่ 2: Print Formatted (USART)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003              USB-Serial (CH340G/CP2102/FT232)
 *     --------              --------------------------------
 *     PD5 (TX) ------------> RX
 *     PD6 (RX) <------------ TX
 *     GND ------------------ GND
 *
 *     (เหมือนกับ ex01)
 *     การตอแบบ Cross-connect: MCU-TX -> USB-RX, MCU-RX -> USB-TX
 *     Baud Rate 115200, GND ตองตอรวมกัน
 *
 * API ที่ใช้:
 *   USART_PrintNum(number)         // พิมพ์เลขจำนวนเต็ม (int32_t)
 *   USART_PrintHex(number)         // พิมพ์เลขฐานสิบหก (uint32_t)
 *   USART_WriteByte(byte)          // ส่งข้อมูล 1 ไบต์
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   "Decimal: 12345"
 *   "Hex: 0x3039"
 *   "Negative: -999"
 *   "Byte: 0x55"
 * ============================================================
 * คำเตือน (WARNINGS):
 *   - USART_PrintNum รับค่า int32_t (เลขมีเครื่องหมาย)
 *   - USART_PrintHex รับค่า uint32_t (เลขไม่มีเครื่องหมาย)
 *   - ตรวจสอบให้ Baud Rate ตรงกัน (115200)
 *   - ต่อสาย Cross-connect เหมือน ex01
 * ============================================================
 */

#include <SimpleHAL.h>

int main(void)
{
    // อัปเดตความถี่ระบบ
    SystemCoreClockUpdate();

    Timer_Init();
    // เริ่มต้น USART: Baud 115200
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    // วนรอบไม่สิ้นสุด
    while (1)
    {
        // แสดงเลขจำนวนเต็ม
        USART_Print("Decimal: ");
        USART_PrintNum(12345);
        USART_Print("\r\n");

        // แสดงเลขฐานสิบหก
        USART_Print("Hex: 0x");
        USART_PrintHex(12345, 1);
        USART_Print("\r\n");

        // แสดงเลขลบ
        USART_Print("Negative: ");
        USART_PrintNum(-999);
        USART_Print("\r\n");

        // แสดง 1 ไบต์
        USART_Print("Byte: 0x");
        USART_WriteByte(0x55);
        USART_Print("\r\n");

        // หน่วง 2 วินาที
        Delay_Ms(2000);
    }
}
