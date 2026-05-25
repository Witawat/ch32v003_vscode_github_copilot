/**
 * ============================================================
 * ตัวอย่างที่ 3: Echo Command (USART)
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
 *     CH32V003                  LED
 *     --------                  ---
 *     PC0 ----/\/\/\---->|---- GND
 *     GPIO_OUT  220 Ohm
 *
 *     สั่ง LED=ON / LED=OFF ผาน Serial Monitor
 *     คำสงอื่นจะถูก Echo กลับ
 *
 * API ที่ใช้:
 *   USART_Available()          // ตรวจสอบว่ามีข้อมูลรออ่านหรือไม่
 *   USART_Read()               // อ่าน 1 ไบต์ (blocking!)
 *   USART_Print()              // พิมพ์ข้อความ
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - พิมพ์ "LED=ON" --> LED ที่ PC0 ติดสว่าง
 *   - พิมพ์ "LED=OFF" --> LED ดับ
 *   - พิมพ์อย่างอื่น --> สะท้อนกลับ (echo)
 * ============================================================
 * คำเตือน (WARNINGS):
 *   - USART_Read() เป็นแบบ Blocking! ต้องตรวจสอบ USART_Available() ก่อน
 *   - ตัวอักษรพิมพ์ใหญ่-เล็กมีผล (case-sensitive)
 *   - ต้องตั้ง Serial Monitor เป็น Line Ending mode: "Newline" หรือ "\r\n"
 *   - LED ต้องต่อตัวต้านทาน 220 Ohm เสมอ
 * ============================================================
 */

#include <SimpleHAL.h>

int main(void)
{
    // อัปเดตความถี่ระบบ
    SystemCoreClockUpdate();

    // เริ่มต้น USART: Baud 115200
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    // ตั้งค่า LED ที่ PC0 เป็น PIN_MODE_OUTPUT
    pinMode(PC0, PIN_MODE_OUTPUT);

    // ตัวแปรเก็บข้อความที่รับ
    char cmd[16];
    int  idx = 0;
    char ch;

    // วนรอบไม่สิ้นสุด
    while (1)
    {
        // ตรวจสอบว่ามีข้อมูลเข้ามาหรือไม่
        if (USART_Available() > 0)
        {
            // อ่าน 1 ไบต์ (blocking)
            ch = USART_Read();

            // ถ้าเป็น '\r' หรือ '\n' แสดงว่าจบคำสั่ง
            if ((ch == '\r') || (ch == '\n'))
            {
                // ใส่ null terminator
                cmd[idx] = '\0';

                // เปรียบเทียบคำสั่ง
                if (strcmp(cmd, "LED=ON") == 0)
                {
                    digitalWrite(PC0, HIGH);
                    USART_Print("LED ON\r\n");
                }
                else if (strcmp(cmd, "LED=OFF") == 0)
                {
                    digitalWrite(PC0, LOW);
                    USART_Print("LED OFF\r\n");
                }
                else
                {
                    // Echo กลับ
                    USART_Print("You said: ");
                    USART_Print(cmd);
                    USART_Print("\r\n");
                }

                // รีเซ็ต index
                idx = 0;
            }
            else
            {
                // เก็บตัวอักษรถ้าไม่เกิน buffer
                if (idx < 15)
                {
                    cmd[idx] = ch;
                    idx++;
                }
            }
        }
    }
}
