/**
 * ============================================================
 * ตัวอยางที่ 6: Shift Register (74HC595 + Knight Rider)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003              74HC595                LEDs
 *     --------              --------               ----
 *     PC0 --- DATA -------> DS (14)    Q0 (15) ---/\/\---|>|--- GND
 *     PC1 --- CLOCK ------> SH_CP (11) Q1 (1)  ---/\/\---|>|--- GND
 *     PC2 --- LATCH ------> ST_CP (12) Q2 (2)  ---/\/\---|>|--- GND
 *                                      Q3 (3)  ---/\/\---|>|--- GND
 *                            VCC (16)  Q4 (4)  ---/\/\---|>|--- GND
 *                            GND (8)   Q5 (5)  ---/\/\---|>|--- GND
 *                            MR (10)   Q6 (6)  ---/\/\---|>|--- GND
 *                            OE (13)   Q7 (7)  ---/\/\---|>|--- GND
 *                                          (ทุกตัว 220 Ohm)
 *
 *     MR (10) ---> 3.3V (reset disable)
 *     OE (13) ---> GND (output enable  = active LOW)
 *
 * ============================================================
 * ผลลัพธที่คาดหวัง (Expected Results):
 * - LEDs 8 ดวงแสดงรูปแบบ Knight Rider (ไลไฟกลับไปกลับมา)
 * - เหมือนไฟ KITT ในรถ Knight Rider
 * - LED วิ่งจากซายไปขวา แลวกลับจากขวามาซาย วนไปเรื่อยๆ
 * ============================================================
 * คำเตือน (WARNINGS):
 * - 74HC595 รองรับไฟ 5V แต CH32V003 เปน 3.3V ซึ่ง OK สำหรับลอจิก
 * - 74HC595 รับ Vih ขั้นต่ำ ~3.15V ที่ 5V VCC - ควรใช VCC=3.3V หรือ 5V
 * - ตองตอ MR (pin 10) ไป VCC เพื่อไมใหรีเซต
 * - ตองตอ OE (pin 13) ไป GND เพื่อเปดใชงานเอาตพุต
 * - อยาลืม C 100nF ระหวาง VCC-GND ใกลๆ 74HC595
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>   // รวมไลบรารี SimpleHAL ทั้งหมด

// กำหนดชื่อขาสำหรับตอ 74HC595 เพื่องายตอการเขาใจ
#define DATA_PIN   PC0   // ขาสงขอมูล (Serial Data Input - DS)
#define CLOCK_PIN  PC1   // ขาสัญญาณนาฬิกา (Shift Clock - SH_CP)
#define LATCH_PIN  PC2   // ขาล็อคขอมูล (Storage/Latch Clock - ST_CP)

int main(void)           // ฟงกชันหลักของโปรแกรม
{
    SystemCoreClockUpdate();
    Timer_Init();
    // ตั้งคาขาที่ตอ 74HC595 ทั้ง 3 ขาเปนเอาตพุต
    pinMode(DATA_PIN,  PIN_MODE_OUTPUT);  // DATA (DS) output
    pinMode(CLOCK_PIN, PIN_MODE_OUTPUT);  // CLOCK (SH_CP) output
    pinMode(LATCH_PIN, PIN_MODE_OUTPUT);  // LATCH (ST_CP) output

    while(1)                 // วนลูปอนันต์
    {
        // Knight Rider Pattern: วิ่งจากซายไปขวา (PC0  PC7)
        // LED ที่ 0 = QA, LED ที่ 7 = QH
        for (int i = 0; i < 8; i++)   // i = 0 ถึง 7 (ซายไปขวา)
        {
            digitalWrite(LATCH_PIN, LOW);  // ตั้ง LATCH เปน LOW เพือเริ่มสงขอมูล
                                           // (ไมตอง latch จนกวาจะสงขอมูลครบ)

            shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, (1 << i));
            // shiftOut: สงขอมูล 1 byte แบบ MSB กอน
            // (1 << i) = สราง bit pattern เชน i=0  00000001, i=1  00000010
            // bit ที่ i จะเปน 1 (LED ดวงนั้นติด) ที่เหลือเปน 0 (ดับ)

            digitalWrite(LATCH_PIN, HIGH); // ตั้ง LATCH เปน HIGH เพือล็อคขอมูล
                                           // ขอมูลจะปรากฏที่ Q0-Q7 ทันที

            Delay_Ms(100);                  // หนวงเวลา 100ms กอนขยับ LED ถัดไป
        }

        // Knight Rider Pattern: วิ่งจากขวามาซาย (PC7  PC0)
        for (int i = 7; i >= 0; i--)  // i = 7 ถึง 0 (ขวามาซาย)
        {
            digitalWrite(LATCH_PIN, LOW);  // เริ่มสงขอมูลไปยัง shift register

            shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, (1 << i));
            // (1 << i) = สราง bit pattern เชน i=7  10000000, i=6  01000000

            digitalWrite(LATCH_PIN, HIGH); // ล็อคขอมูลใหปรากฏที่เอาตพุต

            Delay_Ms(100);                  // หนวงเวลา 100ms
        }
        // เมื่อจบ 2 loops จะกลับไปทำซ้ำอีก (วิ่งไป-มาวนไปเรื่อยๆ)
    }                            // สิ้นสุด while loop
}                                // สิ้นสุดฟงกชัน main
