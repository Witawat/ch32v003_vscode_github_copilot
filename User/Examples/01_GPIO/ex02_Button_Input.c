/**
 * ============================================================
 * ตัวอย่างที่ 2: Button Input (อ่านค่าปุ่ม)
 * ============================================================
 *
 * ผังวงจร (Circuit Diagram):
 *
 *     CH32V003                  ปุ่มกด (Button)
 *     --------                  -------------
 *     PC1 ---+----/\/\/\---- 3.3V
 *            |      10k Ohm    (Pull-up ภายนอก)
 *            |
 *            +---- ปุ่มกด ---- GND
 *
 *     PC0 ----/\/\/\---->|---- GND
 *            220 Ohm
 *
 *     สถานะปกติ: PC1 ผ่าน 10k ไป 3.3V => อ่าน HIGH
 *     ขณะกดปุ่ม:   PC1 ลง GND โดยตรง => อ่าน LOW (Active LOW)
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 * - Serial Monitor แสดง "Pressed!" ขณะกดปุ่ม
 * - Serial Monitor แสดง "Released!" ขณะปล่อยปุ่ม
 * - LED ที่ PC0 ติดค้างขณะกดปุ่ม (สถานะตรงข้ามกับปุ่ม)
 * - LED ที่ PC0 ดับเมื่อปล่อยปุ่ม
 * - Baud Rate: 115200, TX=PD5, RX=PD6
 * ============================================================
 * คำเตือน (WARNINGS):
 * - วงจรนี้เป็น Active LOW (HIGH ไม่กด, LOW กดปุ่ม)
 * - ต้องมีตัวต้านทาน Pull-up (10k) ภายนอกตามวงจร
 * - ใช้ PIN_MODE_INPUT_PULLUP หาก ไม่ต้องการใช้ตัวต้านทานภายนอก
 * - ตรวจสอบ Debounce ด้วย ELAPSED_TIME แบบ non-blocking
 * ============================================================
 * แผนผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["pinMode(PC0, OUTPUT)"]
 *     C --> D["pinMode(PC1, INPUT_PULLUP)"]
 *     D --> E["USART_SimpleInit(115200)"]
 *     E --> F["while(1)"]
 *     F --> G{"ELAPSED_TIME >= 50?"}
 *     G -->|"Yes"| H["digitalRead(PC1)"]
 *     H --> I["digitalWrite(PC0, !buttonState)"]
 *     I --> J{"buttonState == LOW?"}
 *     J -->|"Yes"| K["USART_Print('Pressed!')"]
 *     J -->|"No"| L["USART_Print('Released!')"]
 *     K --> F
 *     L --> F
 *     G -->|"No"| F
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>   // เรียกใช้ไลบรารี SimpleHAL ทั้งหมด

int main(void)           // ฟังก์ชันหลักของโปรแกรม
{
    SystemCoreClockUpdate();
    Timer_Init();
    pinMode(PC0, PIN_MODE_OUTPUT);       // ตั้งค่าขา PC0 เป็นขา Output (LED)
    pinMode(PC1, PIN_MODE_INPUT_PULLUP); // ตั้งค่าขา PC1 เป็นขา Input แบบ Pull-up ภายใน
                                         // (Active LOW: HIGH=ปล่อย, LOW=กด)

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT); // เริ่มต้น USART ที่ 115200 baud
                                         // ใช้ TX=PD5, RX=PD6 (pin default)

    static uint32_t last_loop = 0;

    while(1)                 // วงวนไม่รู้จบ
    {
        if (ELAPSED_TIME(last_loop, Get_CurrentMs()) >= 50) {
            last_loop = Get_CurrentMs();

            uint8_t buttonState = digitalRead(PC1);  // อ่านค่าดิจิตอลจากขา PC1
                                             // เป็น HIGH (1) หรือ LOW (0)

            digitalWrite(PC0, !buttonState);  // เขียนค่าตรงข้ามกับค่า LED (Active LOW)
                                             // ถ้ากดปุ่ม (0) => LED ติด (1)
                                             // ถ้าไม่กดปุ่ม (1) => LED ดับ (0)

            if (buttonState == LOW)          // ตรวจสอบว่ากำลังกดปุ่มอยู่ (Active LOW)
            {
                USART_Print("Pressed!\r\n");    // ส่งข้อความ "Pressed!" ไปยัง Serial Monitor
                                             // \r\n = carriage return + new line
            }
            else                             // ถ้าไม่ได้กดปุ่ม (buttonState == HIGH)
            {
                USART_Print("Released!\r\n");   // ส่งข้อความ "Released!" ไปยัง Serial Monitor
            }
        }
    }                            // จบ while loop
}                                // จบฟังก์ชัน main
