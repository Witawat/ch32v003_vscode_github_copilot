/**
 * ============================================================
 * ตัวอย่างที่ 1: DS18B20 อ่านอุณหภูมิ (Temperature Reading)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003              DS18B20
 *     --------              -------
 *     PD2 ----/\/\/\-------+ DQ (Data)
 *            4.7kΩ          |
 *                           +--- VCC (3.3V)
 *                           |
 *     GND ----------------+ GND
 * 
 *     การต่อวงจร:
 *     - ขา DQ ของ DS18B20 ต่อกับ PD2 ของ CH32V003
 *     - ตัวต้านทาน 4.7kΩ ดึงขา PD2 ขึ้นไปที่ 3.3V (pull-up)
 *     - VCC ของ DS18B20 ต่อกับ 3.3V
 *     - GND ต่อร่วมกัน
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 * - ทุก 1 วินาที จะแสดงอุณหภูมิที่อ่านได้ทาง USART
 * - "Temperature: 28.50 C" (หรืออุณหภูมิปัจจุบัน)
 * - สามารถใช้นิ้วแตะ DS18B20 เพื่อดูอุณหภูมิเปลี่ยนแปลง
 * ============================================================
 * คำเตือน (WARNINGS):
 * - ต้องมีตัวต้านทาน 4.7kΩ pull-up! ถ้าไม่มี 1-Wire จะไม่ทำงาน!
 * - การแปลงอุณหภูมิใช้เวลาสูงสุด 750ms ต้องรอก่อนอ่านค่า
 * - สูตรคำนวณอุณหภูมิ: Temp = (raw>>4) + ((raw&0x0F)*0.0625)
 * - ต้องใช้ Sensor DS18B20 เท่านั้น (เบอร์อื่นอาจใช้คำสั่งต่างกัน)
 * ============================================================
 */

#include <SimpleHAL.h>    // รวมไลบรารี SimpleHAL ทั้งหมด
#include <stdio.h>        // รวมไลบรารี sprintf สำหรับจัดรูปแบบข้อความ

int main(void)            // ฟังก์ชันหลัก จุดเริ่มต้นโปรแกรม
{
    SystemCoreClockUpdate(); // อัปเดตค่าความถี่สัญญาณนาฬิกา (จำเป็นทุกครั้ง)

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT); // เริ่มต้น USART ที่ 115200 baud

    OneWire_Bus* bus = OneWire_Init(PD2); // เริ่มต้น 1-Wire bus บนขา PD2

    uint8_t scratchpad[9]; // Buffer สำหรับเก็บข้อมูล Scratchpad 9 ไบต์

    while (1)              // วนลูปอนันต์ อ่านอุณหภูมิทุกวินาที
    {
        OneWire_Reset(bus);    // ส่ง Reset pulse เพื่อเริ่มต้นการสื่อสาร
        OneWire_SkipROM(bus);  // ส่ง Skip ROM (0xCC) ข้ามการระบุอุปกรณ์
        OneWire_WriteByte(bus, 0x44); // ส่งคำสั่ง Convert T (0x44) เริ่มแปลงอุณหภูมิ

        Delay_Ms(750);     // รอ 750ms จนกว่าการแปลงจะเสร็จ (DS18B20 ใช้เวลาสูงสุด 750ms)

        OneWire_Reset(bus);    // ส่ง Reset pulse อีกครั้งก่อนอ่านค่า
        OneWire_SkipROM(bus);  // ส่ง Skip ROM เพื่อเลือกอุปกรณ์เดียวบนบัส
        OneWire_WriteByte(bus, 0xBE); // ส่งคำสั่ง Read Scratchpad (0xBE) อ่านค่าอุณหภูมิ

        for (uint8_t i = 0; i < 9; i++) // อ่านข้อมูล Scratchpad ทั้ง 9 ไบต์
        {
            scratchpad[i] = OneWire_ReadByte(bus); // อ่านทีละไบต์จาก 1-Wire bus
        }

        int16_t raw_temp = (int16_t)(scratchpad[1] << 8) | scratchpad[0]; // รวมไบต์ต่ำและสูงเป็นค่า 16-bit signed
        float temperature_c = (raw_temp >> 4) + ((raw_temp & 0x0F) * 0.0625f); // คำนวณอุณหภูมิตามสูตร 12-bit resolution

        char buffer[32];   // Buffer สำหรับข้อความที่ต้องการพิมพ์
        sprintf(buffer, "Temperature: %.2f C\r\n", temperature_c); // จัดรูปแบบข้อความด้วยอุณหภูมิที่อ่านได้
        USART_Print(buffer); // ส่งข้อความไปยัง USART

        Delay_Ms(250);     // หน่วงเวลาเพิ่ม 250ms รวมเป็น 1 วินาทีต่อรอบ
    }                      // สิ้นสุด while loop กลับไปเริ่มรอบใหม่
}                          // สิ้นสุดฟังก์ชัน main
