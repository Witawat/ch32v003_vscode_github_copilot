/**
 * ============================================================
 * ตัวอยางที่ 4: GPIO Multiple (GPIO หลายขาพรอมกัน)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003
 *     --------
 *     PC0 ----/\/\/\---->|---- GND     (LED 1)
 *            220 Ohm
 *
 *     PC1 ----/\/\/\---->|---- GND     (LED 2)
 *            220 Ohm
 *
 *     PC3 ----/\/\/\---->|---- GND     (LED 3)
 *            220 Ohm
 *
 *     PC4 ----/\/\/\---->|---- GND     (LED 4)
 *            220 Ohm
 *
 * ============================================================
 * ผลลัพธที่คาดหวัง (Expected Results):
 * - LED 4 ดวงทำงานแบบ Running Light (ไฟวิ่ง)
 * - LED 1 → LED 2 → LED 3 → LED 4 → LED 1 → ...
 * - ทีละ 1 ดวงเทานั้นที่ติดในแตละชวงเวลา
 * - ความเร็ว: เปลี่ยนทุก 200ms
 * ============================================================
 * คำเตือน (WARNINGS):
 * - CH32V003 จายกระแสไดสูงสุด 8mA ตอ 1 pin
 * - กระแสรวมของทุก LED ตองไมเกินขีดจำกัดรวมของ GPIO port (~120mA VDD)
 * - 4 LEDs × 8mA = 32mA ยังปลอดภัย แตควรระวังถาเปลียนเปนหลายขา
 * - pinModeMultiple() ตองใช array แบบ const uint8_t arr[] = {...}
 * - digitalWriteMultiple() ตองใช 2 arrays ขนาดเทากัน
 * - ขาม PC2 ถูกละเวนเพราะไมมีในแผนผัง ใช PC3 แทน
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>   // รวมไลบรารี SimpleHAL ทั้งหมด

int main(void)           // ฟงกชันหลักของโปรแกรม
{
    SystemCoreClockUpdate();
    Timer_Init();
    // ประกาศ array ของ pin numbers สำหรับ LED ทั้ง 4 ดวง
    const uint8_t ledPins[] = {PC0, PC1, PC3, PC4};

    // ตั้งคาขา LED ทั้งหมดใหเปนเอาตพุตพรอมกันในคำสั่งเดียว
    pinModeMultiple(ledPins, PIN_MODE_OUTPUT);
    // macro นี้คำนวณจำนวนสมาชิกใน array ใหอัตโนมัติ

    while(1)                 // วนลูปอนันต์
    {
        // Running Light: วนแตละ LED ทีละดวง
        for (int i = 0; i < 4; i++)  // วนจาก LED 0 (PC0) ถึง LED 3 (PC4)
        {
            // สราง array สถานะ: LED ที่ i ติด (HIGH), ที่เหลือดับ (LOW)
            uint8_t ledStates[4] = {LOW, LOW, LOW, LOW};
            ledStates[i] = HIGH;    // ให LED ที่ตำแหนง i ติด

            digitalWriteMultiple(ledPins, ledStates);  // เขียนคาไปยังทุก LED พรอมกัน
                                           // ใช macro ที่คำนวณจำนวนอัตโนมัติ

            Delay_Ms(200);         // หนวงเวลา 200ms กอนเปลยนเปน LED ดวงถัดไป
        }
    }                            // สิ้นสุด while loop
}                                // สิ้นสุดฟงกชัน main
