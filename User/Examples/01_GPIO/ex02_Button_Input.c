/**
 * ============================================================
 * ตัวอยางที่ 2: Button Input (อานคาปุมกด)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003                  ปุมกด (Button)
 *     --------                  -------------
 *     PC1 ---+----/\/\/\---- 3.3V
 *            |      10k Ohm    (Pull-up ภายนอก)
 *            |
 *            +---- ปุมกด ---- GND
 *
 *     PC0 ----/\/\/\---->|---- GND
 *            220 Ohm
 *
 *     เมื่อไมกดปุม: PC1 ตอผาน 10k ไป 3.3V → อานได HIGH
 *     เมื่อกดปุม:   PC1 ตอลง GND โดยตรง → อานได LOW (Active LOW)
 *
 * ============================================================
 * ผลลัพธที่คาดหวัง (Expected Results):
 * - Serial Monitor แสดง "Pressed!" เมื่อกดปุม
 * - Serial Monitor แสดง "Released!" เมื่อปลอยปุม
 * - LED ที่ PC0 จะติดเมื่อกดปุม (สถานะเดียวกับปุม)
 * - LED ที่ PC0 จะดับเมื่อปลอยปุม
 * - Baud Rate: 115200, TX=PD5, RX=PD6
 * ============================================================
 * คำเตือน (WARNINGS):
 * - วงจรนี้เปน Active LOW (HIGH เมื่อไมกด, LOW เมื่อกด)
 * - ไมควรลืมใสตัวตานทาน Pull-up (10k) กันกระแสลัดวงจร
 * - ถาใช PIN_MODE_INPUT_PULLUP ภายใน อาจไมตองใชตัวตานทานภายนอก
 * - ตรวจสอบ Debounce ดวย Delay_Ms(50) ในโปรแกรมจริง
 * ============================================================
 */

#include <SimpleHAL.h>   // รวมไลบรารี SimpleHAL ทั้งหมด

int main(void)           // ฟงกชันหลักของโปรแกรม
{
    SystemCoreClockUpdate();  // อัปเดตคาความถี่สัญญาณนาฬิการะบบ

    pinMode(PC0, PIN_MODE_OUTPUT);       // ตั้งคาขา PC0 เปนเอาตพุต (LED)
    pinMode(PC1, PIN_MODE_INPUT_PULLUP); // ตั้งคาขา PC1 เปนอินพุตพรอม Pull-up ภายใน
                                         // (Active LOW: HIGH=ไมกด, LOW=กด)

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT); // เริ่มตน USART ที่ 115200 baud
                                         // ใชขา TX=PD5, RX=PD6 (pin default)

    while(1)                 // วนลูปอนันต์
    {
        uint8_t buttonState = digitalRead(PC1);  // อานคาดิจิทัลจากขา PC1
                                         // คืนคา HIGH (1) หรือ LOW (0)

        digitalWrite(PC0, !buttonState);  // เขียนคาสลับกลับไปที่ LED (Active LOW)
                                         // ถาปุมกด (0) → LED ติด (1)
                                         // ถาปุมไมกด (1) → LED ดับ (0)

        if (buttonState == LOW)          // ตรวจสอบวาปุมถูกกดหรือไม (Active LOW)
        {
            USART_Print("Pressed!\r\n");    // สงขอความ "Pressed!" ไปยัง Serial Monitor
                                         // \r\n = carriage return + new line
        }
        else                             // ถาปุมไมไดถูกกด (buttonState == HIGH)
        {
            USART_Print("Released!\r\n");   // สงขอความ "Released!" ไปยัง Serial Monitor
        }

        Delay_Ms(50);                // หนวงเวลา 50ms เพื้อลดการกระพริบ (debounce)
                                     // และปองกันการสงขอความซ้ำเร็วเกินไป
    }                            // สิ้นสุด while loop
}                                // สิ้นสุดฟงกชัน main
