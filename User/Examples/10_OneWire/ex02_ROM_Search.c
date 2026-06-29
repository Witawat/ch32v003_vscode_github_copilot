/**
 * ============================================================
 * ตัวอย่างที่ 2: ค้นหา ROM ของอุปกรณ์ 1-Wire (ROM Search)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003          DS18B20 #1      DS18B20 #2
 *     --------          ----------      ----------
 *     PD2 ----/\/\/\---+--- DQ          --- DQ
 *            4.7kΩ     |                |
 *                      +--- VCC (3.3V)  +--- VCC (3.3V)
 *                      |                |
 *     GND ------------+--- GND         --- GND
 * 
 *     การต่อวงจร:
 *     - DS18B20 ทั้ง 2 ตัวต่อขนานกันบนเส้น 1-Wire bus (PD2) เดียวกัน
 *     - ใช้ตัวต้านทาน 4.7kΩ เพียงตัวเดียวต่อระหว่าง PD2 และ VCC 3.3V
 *     - VCC ของทั้ง 2 ตัวต่อกับ 3.3V, GND ต่อร่วมกัน
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 * - "Searching 1-Wire bus..."
 * - "Device 1: 28-FF-12-34-56-78-9A-BC" (ROM แรกที่พบ)
 * - "Device 2: 28-FF-98-76-54-32-10-DE" (ROM ที่สองที่พบ)
 * - "Found 2 device(s)"
 * ============================================================
 * คำเตือน (WARNINGS):
 * - OneWire_ReadROM() ใช้ได้เฉพาะตอนที่มีอุปกรณ์เดียวบนบัสเท่านั้น! ใช้ Search สำหรับหลายอุปกรณ์
 * - ROM[0] = Family code (0x28 สำหรับ DS18B20), ROM[7] = CRC checksum
 * - ต้องใช้ OneWire_ResetSearch() ก่อนเริ่มค้นหาทุกครั้ง
 * - ถ้าไม่มีอุปกรณ์ต่อ จะขึ้น "Found 0 device(s)"
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>    // รวมไลบรารี SimpleHAL ทั้งหมด
#include <stdio.h>        // รวมไลบรารี sprintf สำหรับจัดรูปแบบข้อความ

int main(void)            // ฟังก์ชันหลัก จุดเริ่มต้นโปรแกรม
{
    SystemCoreClockUpdate(); // อัปเดตค่าความถี่สัญญาณนาฬิกา (จำเป็นทุกครั้ง)

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT); // เริ่มต้น USART ที่ 115200 baud

    OneWire_Bus* bus = OneWire_Init(PD2); // เริ่มต้น 1-Wire bus บนขา PD2

    USART_Print("Searching 1-Wire bus...\r\n"); // บอกผู้ใช้ว่ากำลังค้นหาอุปกรณ์

    OneWire_ResetSearch(bus); // รีเซ็ตสถานะการค้นหา (ต้องเรียกก่อนเริ่ม Search)

    uint8_t device_count = 0; // ตัวนับจำนวนอุปกรณ์ที่พบ

    while (OneWire_Search(bus)) // วนลูปค้นหาอุปกรณ์ทีละตัวจนกว่าจะไม่มีอุปกรณ์เหลือ
    {
        uint8_t rom[8];      // Buffer สำหรับเก็บ ROM address ที่พบ
        OneWire_GetAddress(bus, rom); // ดึง ROM address จาก bus instance

        uint8_t crc = OneWire_CRC8(rom, 7); // คำนวณ CRC8 จาก 7 ไบต์แรก
        if (crc == rom[7])   // ตรวจสอบว่า CRC ตรงกับไบต์สุดท้ายของ ROM หรือไม่
        {
            device_count++;  // เพิ่มจำนวนอุปกรณ์ที่ตรวจสอบ CRC ถูกต้อง
            char buffer[48]; // Buffer สำหรับข้อความ
            sprintf(buffer, "Device %d: %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\r\n",
                    device_count, rom[0], rom[1], rom[2], rom[3],
                    rom[4], rom[5], rom[6], rom[7]); // จัดรูปแบบ ROM address
            USART_Print(buffer); // พิมพ์ ROM address ที่พบ
        }
    }

    char result[24];         // Buffer สำหรับข้อความสรุป
    sprintf(result, "Found %d device(s)\r\n", device_count); // จัดรูปแบบข้อความ
    USART_Print(result);     // บอกจำนวนอุปกรณ์ที่พบทั้งหมด

    while (1) { }            // หยุดโปรแกรมที่ตรงนี้ (รออย่างเดียว)
}                            // สิ้นสุดฟังก์ชัน main
