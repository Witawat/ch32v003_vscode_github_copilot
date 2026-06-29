/**
 * ============================================================
 * ตัวอย่างที่ 3: เลือกอุปกรณ์หลายตัวบนบัส (Multi-Device Select)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003          DS18B20 #1      DS18B20 #2
 *     --------          ----------      ----------
 *     PD2 ----/\/\/\---+--- DQ          --- DQ
 *            4.7k?     |                |
 *                      +--- VCC (3.3V)  +--- VCC (3.3V)
 *                      |                |
 *     GND ------------+--- GND         --- GND
 * 
 *     การต่อวงจร:
 *     - เหมือนกับ ex02 ทุกประการ (2x DS18B20 ขนานกันบน PD2)
 *     - ต้องทราบ ROM address ของอุปกรณ์แต่ละตัวก่อน (ใช้ ex02 ค้นหา)
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 * - "Dev1: 28.5C, Dev2: 29.1C"
 * - แสดงอุณหภูมิของอุปกรณ์ทีละตัวโดยเลือกผ่าน Match ROM
 * - อุณหภูมิของอุปกรณ์ทั้ง 2 ตัวจะแตกต่างกันเล็กน้อย
 * ============================================================
 * คำเตือน (WARNINGS):
 * - ต้องรู้ ROM address ของอุปกรณ์แต่ละตัวก่อน (ใช้ Search จาก ex02)
 * - ถ้าใช้ ROM address ผิด อุปกรณ์นั้นจะไม่ตอบสนอง
 * - ต้องส่ง Reset และ Match ROM ทุกครั้งก่อนส่งคำสั่งไปยังอุปกรณ์
 * - อุณหภูมิของทั้ง 2 ตัวควรใกล้เคียงกัน ถ้าต่างกันมากแสดงว่ามีปัญหา
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
/* CH32V003 has no hardware FPU — float/double use software emulation (~800 cycles) */
#include <SimpleHAL.h>    // รวมไลบรารี SimpleHAL ทั้งหมด
#include <stdio.h>        // รวมไลบรารี sprintf สำหรับจัดรูปแบบข้อความ

int main(void)            // ฟังก์ชันหลัก จุดเริ่มต้นโปรแกรม
{
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT); // เริ่มต้น USART ที่ 115200 baud

    OneWire_Bus* bus = OneWire_Init(PD2); // เริ่มต้น 1-Wire bus บนขา PD2

    uint8_t rom1[8] = {0x28, 0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}; // ROM ของ DS18B20 ตัวที่ 1 (เปลี่ยนเป็นค่าจริง)
    uint8_t rom2[8] = {0x28, 0xFF, 0x98, 0x76, 0x54, 0x32, 0x10, 0xDE}; // ROM ของ DS18B20 ตัวที่ 2 (เปลี่ยนเป็นค่าจริง)

    float temp1 = 0.0f;    // ตัวแปรเก็บอุณหภูมิของอุปกรณ์ตัวที่ 1
    float temp2 = 0.0f;    // ตัวแปรเก็บอุณหภูมิของอุปกรณ์ตัวที่ 2

    while (1)              // วนลูปอนันต์ อ่านอุณหภูมิทุกวินาที
    {
        if (OneWire_Select(bus, rom1)) // เลือกอุปกรณ์ตัวที่ 1 (ส่ง Reset + Match ROM)
        {
            OneWire_WriteByte(bus, 0x44); // ส่งคำสั่ง Convert T ให้อุปกรณ์ตัวที่ 1

            Delay_Ms(750);     // รอ 750ms ให้การแปลงอุณหภูมิเสร็จ

            OneWire_Select(bus, rom1); // เลือกอุปกรณ์ตัวที่ 1 อีกครั้ง
            OneWire_WriteByte(bus, 0xBE); // ส่งคำสั่ง Read Scratchpad

            uint8_t raw_l = OneWire_ReadByte(bus); // อ่านไบต์ต่ำของอุณหภูมิ
            uint8_t raw_h = OneWire_ReadByte(bus); // อ่านไบต์สูงของอุณหภูมิ
            int16_t raw1 = (int16_t)(raw_h << 8) | raw_l; // รวมเป็นค่า 16-bit signed
            temp1 = (raw1 >> 4) + ((raw1 & 0x0F) * 0.0625f); // คำนวณอุณหภูมิเป็นองศาเซลเซียส
        }

        if (OneWire_Select(bus, rom2)) // เลือกอุปกรณ์ตัวที่ 2 (ส่ง Reset + Match ROM)
        {
            OneWire_WriteByte(bus, 0x44); // ส่งคำสั่ง Convert T ให้อุปกรณ์ตัวที่ 2

            Delay_Ms(750);     // รอ 750ms ให้การแปลงอุณหภูมิเสร็จ

            OneWire_Select(bus, rom2); // เลือกอุปกรณ์ตัวที่ 2 อีกครั้ง
            OneWire_WriteByte(bus, 0xBE); // ส่งคำสั่ง Read Scratchpad

            uint8_t raw_l = OneWire_ReadByte(bus); // อ่านไบต์ต่ำของอุณหภูมิ
            uint8_t raw_h = OneWire_ReadByte(bus); // อ่านไบต์สูงของอุณหภูมิ
            int16_t raw2 = (int16_t)(raw_h << 8) | raw_l; // รวมเป็นค่า 16-bit signed
            temp2 = (raw2 >> 4) + ((raw2 & 0x0F) * 0.0625f); // คำนวณอุณหภูมิเป็นองศาเซลเซียส
        }

        char buffer[32];         // Buffer สำหรับข้อความ
        sprintf(buffer, "Dev1: %.1fC, Dev2: %.1fC\r\n", temp1, temp2); // จัดรูปแบบอุณหภูมิทั้ง 2 ค่า
        USART_Print(buffer);     // พิมพ์อุณหภูมิทาง USART

        Delay_Ms(250);           // หน่วงเวลา 250ms ให้ครบ 1 วินาทีต่อรอบ
    }                            // สิ้นสุด while loop
}                                // สิ้นสุดฟังก์ชัน main
