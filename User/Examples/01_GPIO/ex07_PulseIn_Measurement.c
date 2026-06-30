/**
 * ============================================================
 * ตัวอยางที่ 7: PulseIn Measurement (วัดระยะดวย HC-SR04)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     HC-SR04                  CH32V003
 *     -------                  --------
 *     VCC (5V) ----> 5V Supply (External)
 *     GND ---------> GND (รวม)
 *     TRIG --------> PC3 (Digital Output)
 *     ECHO ---+----> PC4 (Digital Input)
 *             |
 *             +--[2k Ohm]--+--[3.3k Ohm]--GND
 *                          |
 *                        PC4
 *
 *     Voltage Divider: ECHO (5V)  2k?  PC4  3.3k?  GND
 *     Vout = 5V ? (3.3k / (2k + 3.3k)) = 5V ? 0.623 = 3.11V
 *     (ปลอดภัยสำหรับ CH32V003 ที่ 3.3V)
 *
 * ============================================================
 * ผลลัพธที่คาดหวัง (Expected Results):
 * - Serial Monitor แสดงระยะทางทุก 500ms
 * - "Distance: XX.X cm"
 * - วัดระยะไดประมาณ 2cm - 400cm
 * - ความละเอียด ?0.3cm
 * ============================================================
 * คำเตือน (WARNINGS):
 * - HC-SR04 ใช ECHO ที่ 5V ซึ่งอันตรายตอ CH32V003 (3.3V เทานั้น)
 * - ตองใช Voltage Divider (2k? ตออนุกรม + 3.3k? ตอลง GND) เสมอ!
 * - โดยไมตอ Voltage Divider จะทำให MCU เสียหายถาวร
 * - ระยะทางสูงสุดประมาณ 400cm ตองตั้ง timeout 30ms
 * - สูตร: ระยะทาง (cm) = pulse (us) ? 0.034 / 2
 * - 0.034 = ความเร็วเสียง (cm/us), หาร 2 เพราะไป-กลับ
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
/* CH32V003 has no hardware FPU — float/double use software emulation (~800 cycles) */
#include <SimpleHAL.h>   // รวมไลบรารี SimpleHAL ทั้งหมด

// กำหนดชื่อขา HC-SR04
#define TRIG_PIN  PC3   // ขาสงสัญญาณ Trigger (Output)
#define ECHO_PIN  PC4   // ขารับสัญญาณ Echo (Input - ผาน Voltage Divider)

int main(void)           // ฟงกชันหลักของโปรแกรม
{
    SystemCoreClockUpdate();
    Timer_Init();
    pinMode(TRIG_PIN, PIN_MODE_OUTPUT); // ตั้งคาขา TRIG เปนเอาตพุต (สงพัลส)
    pinMode(ECHO_PIN, PIN_MODE_INPUT);  // ตั้งคาขา ECHO เปนอินพุต (รับพัลส)

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT); // เริ่มตน USART ที่ 115200 baud

    while(1)                 // วนลูปอนันต์
    {
        // --- สงพัลส Trigger ไปยัง HC-SR04 ---
        digitalWrite(TRIG_PIN, LOW);       // ตั้ง TRIG เปน LOW กอน
        Delay_Us(2);                       // รอ 2 microseconds (ใหสัญญาณคงที่)

        digitalWrite(TRIG_PIN, HIGH);      // สง HIGH ไปที่ TRIG เปนเวลา 10us
        Delay_Us(10);                      // HC-SR04 ตองการพัลส HIGH อยางนอย 10us

        digitalWrite(TRIG_PIN, LOW);       // ตั้ง TRIG กลับมาเปน LOW พรอมวัดระยะ
        // --- จบการสง Trigger ---

        // --- วัดความกวางของพัลส Echo ---
        uint32_t pulseWidth = pulseIn(ECHO_PIN, HIGH, 30000);
        // pulseIn: วัดความกวางของสัญญาณ HIGH ที่ ECHO_PIN
        // timeout = 30000 microseconds (30ms)
        // คืนคาความกวางเปน microseconds หรือ 0 ถา timeout

        // --- คำนวณระยะทาง ---
        // สูตร: distance (cm) = time (us) ? 0.034 / 2
        // pulseWidth = เวลาที่เสียงเดินทางไป-กลับ
        // 0.034 cm/us = ความเร็วเสียงในอากาศ
        float distanceCm = (float)pulseWidth * 0.034f / 2.0f;
        // หาร 2 เพราะ pulseWidth คือเวลาไป-กลับ ตองการแคเที่ยวเดียว

        // --- แสดงผลทาง Serial ---
        if (pulseWidth > 0)                // ตรวจสอบวาไมได timeout
        {
            USART_Print("Distance: ");     // สงขอความ "Distance: "
            USART_PrintNum((int32_t)(distanceCm * 10));  // สงเลข (x10 เพื่อใหมีทศนิยม)
            USART_Print(" cm\r\n");        // สงหนวย " cm" พรอมขึ้นบรรทัดใหม
            // ตัวอยาง: 150  "150" cm ควรปรับปรุงใหแสดงทศนิยมโดยแบงเปนสวน Integer/Fraction
        }
        else                               // ถา timeout (pulseWidth == 0)
        {
            USART_Print("Out of range!\r\n"); // แจงวาเกินระยะที่วัดได
        }

        Delay_Ms(500);         // หนวง 500ms กอนวัดครั้งถัดไป
                               // ลดความถี่เพือให HC-SR04 ทำงานไดคงที่
    }                            // สิ้นสุด while loop
}                                // สิ้นสุดฟงกชัน main
