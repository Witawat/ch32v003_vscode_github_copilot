/**
 * ============================================================
 * ตัวอย่างที่ 1: Hello World (USART)
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
 *     การตอแบบ Cross-connect:
 *     MCU-TX -> USB-RX, MCU-RX -> USB-TX
 *     ตั้ง Baud Rate 115200 ใน Serial Monitor
 *     GND ตองตอรวมกันเสมอ!
 *
 * API ที่ใช้:
 *   USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT)
 *   USART_Print("ข้อความ")
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   บน Serial Monitor จะแสดง "Hello from CH32V003!" ทุก 1 วินาที
 * ============================================================
 * คำเตือน (WARNINGS):
 *   - PD5 = TX, PD6 = RX (default pins)
 *   - ต่อแบบ Cross-connect: MCU-TX -> USB-RX, MCU-RX -> USB-TX
 *   - ตั้ง Baud Rate ใน Serial Monitor ให้ตรงกัน (115200)
 *   - ตรวจสอบให้ GND ต่อร่วมกัน
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

int main(void)
{
    // อัปเดตความถี่ระบบ
    SystemCoreClockUpdate();

    Timer_Init();
    // เริ่มต้น USART: Baud Rate 115200, ใช้พินเริ่มต้น (PD5=TX, PD6=RX)
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    // วนรอบไม่สิ้นสุด
    while (1)
    {
        // พิมพ์ข้อความ Hello World ทาง Serial
        USART_Print("Hello from CH32V003!\r\n");

        // รอ 1 วินาที
        Delay_Ms(1000);
    }
}
