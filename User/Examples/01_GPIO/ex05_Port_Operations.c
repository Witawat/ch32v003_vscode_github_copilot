/**
 * ============================================================
 * ตัวอยางที่ 5: Port Operations (การดำเนินการระดับพอร์ต)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003
 *     --------
 *     PC0 ----/\/\/\---->|--- GND    (LSB - LED 1)
 *     PC1 ----/\/\/\---->|--- GND    (LED 2)
 *     PC2 ----/\/\/\---->|--- GND    (LED 3)
 *     PC3 ----/\/\/\---->|--- GND    (LED 4)
 *     PC4 ----/\/\/\---->|--- GND    (LED 5)
 *     PC5 ----/\/\/\---->|--- GND    (LED 6)
 *     PC6 ----/\/\/\---->|--- GND    (LED 7)
 *     PC7 ----/\/\/\---->|--- GND    (MSB - LED 8)
 *          (ทุกตัว 220 Ohm)
 *
 *     หรือใช LED bar แบบ 8 ขาแทนก็ได
 *     PC7 PC6 PC5 PC4 PC3 PC2 PC1 PC0
 *     [D7][D6][D5][D4][D3][D2][D1][D0]
 *
 * ============================================================
 * ผลลัพธที่คาดหวัง (Expected Results):
 * - คาบนพอร์ต GPIOC จะเปลียนทุก 500ms ตามลำดับ:
 *   0xAA  0x55  0xF0  0x0F  0xAA  ...
 * - LEDs แสดงรูปแบบเปน 8-bit binary
 * - 0xAA = 10101010, 0x55 = 01010101
 * - 0xF0 = 11110000, 0x0F = 00001111
 * ============================================================
 * คำเตือน (WARNINGS):
 * - portWrite() สงผลกระทบตอทุกขาในพอร์ตพรอมกัน (PC0-PC7)
 * - ตองมั่นใจวาทุกขาที่ใชถูกตั้งเปน OUTPUT mode กอน
 * - portRead() อานคาปจจุบันของทุกขา รวมทั้งขาที่เปน INPUT ดวย
 * - portWrite เขียนทับคา ODR โดยตรง (ไมใช BSRR)
 * - กระแสรวม 8 LEDs ตองไมเกิน 120mA
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>   // รวมไลบรารี SimpleHAL ทั้งหมด

int main(void)           // ฟงกชันหลักของโปรแกรม
{
    SystemCoreClockUpdate();
    Timer_Init();
    // ตั้งคาขา PC0 - PC7 ทั้งหมด 8 ขาใหเปนเอาตพุต
    // ใชการตั้งคาทีละขา เพราะ CH32V003 มีแค GPIOC พรอม 8 ขา
    pinMode(PC0, PIN_MODE_OUTPUT);
    pinMode(PC1, PIN_MODE_OUTPUT);
    pinMode(PC2, PIN_MODE_OUTPUT);
    pinMode(PC3, PIN_MODE_OUTPUT);
    pinMode(PC4, PIN_MODE_OUTPUT);
    pinMode(PC5, PIN_MODE_OUTPUT);
    pinMode(PC6, PIN_MODE_OUTPUT);
    pinMode(PC7, PIN_MODE_OUTPUT);

    // สราง array ของคาที่ตองการแสดงบนพอร์ต
    const uint8_t patterns[] = {0xAA, 0x55, 0xF0, 0x0F};
    // 0xAA = 10101010 (LED คูติด, คี่ดับ)
    // 0x55 = 01010101 (LED คี่ติด, คูดับ)
    // 0xF0 = 11110000 (PC4-PC7 ติด, PC0-PC3 ดับ)
    // 0x0F = 00001111 (PC0-PC3 ติด, PC4-PC7 ดับ)

    uint8_t index = 0;   // ตัวแปรชี้ตำแหนงใน array patterns

    while(1)                 // วนลูปอนันต์
    {
        portWrite(GPIOC, patterns[index]);  // เขียนคา 8-bit ไปยงพอร์ต GPIOC
                                           // portWrite สงผลทันทีทุกขาพรอมกัน
                                           // ไมตองเรียก digitalWrite ทีละขา

        uint8_t currentValue = portRead(GPIOC);  // อานคาปจจุบันของ GPIOC
                                           (void)currentValue;  // ปองกัน compiler warning
                                           // กรณีไมไดนำคาไปใช

        index++;                     // เพิ่ม index เพื่อเลื่อนไปคาถัดไป
        if (index >= 4)              // ถา index เกินขนาด array (0-3)
        {
            index = 0;               // รีเซต index กลับไปที่คาแรก (0xAA)
        }

        Delay_Ms(500);         // หนวงเวลา 500ms กอนเปลียนเปนรูปแบบถัดไป
    }                            // สิ้นสุด while loop
}                                // สิ้นสุดฟงกชัน main
