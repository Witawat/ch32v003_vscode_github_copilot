/**
 * ============================================================
 * �����ҧ��� 7: PulseIn Measurement (�Ѵ���д�� HC-SR04)
 * ============================================================
 *
 * Ἱ�ѧǧ�� (Circuit Diagram):
 *
 *     HC-SR04                  CH32V003
 *     -------                  --------
 *     VCC (5V) ----> 5V Supply (External)
 *     GND ---------> GND (���)
 *     TRIG --------> PC3 (Digital Output)
 *     ECHO ---+----> PC4 (Digital Input)
 *             |
 *             +--[2k Ohm]--+--[3.3k Ohm]--GND
 *                          |
 *                        PC4
 *
 *     Voltage Divider: ECHO (5V)  2k?  PC4  3.3k?  GND
 *     Vout = 5V ? (3.3k / (2k + 3.3k)) = 5V ? 0.623 = 3.11V
 *     (��ʹ�������Ѻ CH32V003 ��� 3.3V)
 *
 * ============================================================
 * ���Ѿ����Ҵ��ѧ (Expected Results):
 * - Serial Monitor �ʴ����зҧ�ء 500ms
 * - "Distance: XX.X cm"
 * - �Ѵ����䴻���ҳ 2cm - 400cm
 * - ���������´ ?0.3cm
 * ============================================================
 * ����͹ (WARNINGS):
 * - HC-SR04 � ECHO ��� 5V ����ѹ���µ� CH32V003 (3.3V �ҹ��)
 * - �ͧ� Voltage Divider (2k? ��͹ء�� + 3.3k? ��ŧ GND) ����!
 * - ������ Voltage Divider �з��� MCU ������¶���
 * - ���зҧ�٧�ش����ҳ 400cm �ͧ��� timeout 30ms
 * - �ٵ�: ���зҧ (cm) = pulse (us) ? 0.034 / 2
 * - 0.034 = �����������§ (cm/us), ��� 2 �����-��Ѻ
 * ============================================================
 * แผนผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["pinMode(TRIG, OUTPUT)"]
 *     C --> D["pinMode(ECHO, INPUT)"]
 *     D --> E["USART_SimpleInit(115200)"]
 *     E --> F["while(1)"]
 *     F --> G["digitalWrite(TRIG, LOW)"]
 *     G --> H["Delay_Us(2)"]
 *     H --> I["digitalWrite(TRIG, HIGH)"]
 *     I --> J["Delay_Us(10)"]
 *     J --> K["digitalWrite(TRIG, LOW)"]
 *     K --> L["pulseIn(ECHO, HIGH, 30000)"]
 *     L --> M["คำนวณ distanceCm"]
 *     M --> N{"pulseWidth > 0?"}
 *     N -->|"Yes"| O["USART_Print('Distance: XX cm')"]
 *     N -->|"No"| P["USART_Print('Out of range!')"]
 *     O --> Q["Delay_Ms(500)"]
 *     P --> Q
 *     Q --> F
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
/* CH32V003 has no hardware FPU � float/double use software emulation (~800 cycles) */
#include <SimpleHAL.h>   // ����ź���� SimpleHAL ������

// ��˹����͢� HC-SR04
#define TRIG_PIN  PC3   // ��ʧ�ѭ�ҳ Trigger (Output)
#define ECHO_PIN  PC4   // ���Ѻ�ѭ�ҳ Echo (Input - �ҹ Voltage Divider)

int main(void)           // ����ѹ��ѡ�ͧ�����
{
    SystemCoreClockUpdate();
    Timer_Init();
    pinMode(TRIG_PIN, PIN_MODE_OUTPUT); // ��駤Ң� TRIG ໹��ҵ�ص (ʧ����)
    pinMode(ECHO_PIN, PIN_MODE_INPUT);  // ��駤Ң� ECHO ໹�Թ�ص (�Ѻ����)

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT); // ������� USART ��� 115200 baud

    while(1)                 // ǹ�ٻ͹ѹ��
    {
        // --- ʧ���� Trigger ��ѧ HC-SR04 ---
        digitalWrite(TRIG_PIN, LOW);       // ��� TRIG ໹ LOW �͹
        Delay_Us(2);                       // �� 2 microseconds (���ѭ�ҳ�����)

        digitalWrite(TRIG_PIN, HIGH);      // ʧ HIGH 价�� TRIG ໹���� 10us
        Delay_Us(10);                      // HC-SR04 �ͧ��þ��� HIGH ��ҧ��� 10us

        digitalWrite(TRIG_PIN, LOW);       // ��� TRIG ��Ѻ��໹ LOW �����Ѵ����
        // --- �����ʧ Trigger ---

        // --- �Ѵ������ҧ�ͧ���� Echo ---
        uint32_t pulseWidth = pulseIn(ECHO_PIN, HIGH, 30000);
        // pulseIn: �Ѵ������ҧ�ͧ�ѭ�ҳ HIGH ��� ECHO_PIN
        // timeout = 30000 microseconds (30ms)
        // �׹�Ҥ�����ҧ໹ microseconds ���� 0 �� timeout

        // --- �ӹǳ���зҧ ---
        // �ٵ�: distance (cm) = time (us) ? 0.034 / 2
        // pulseWidth = ���ҷ�����§�Թ�ҧ�-��Ѻ
        // 0.034 cm/us = �����������§��ҡ��
        float distanceCm = (float)pulseWidth * 0.034f / 2.0f;
        // ��� 2 ���� pulseWidth ��������-��Ѻ �ͧ�������������

        // --- �ʴ��ŷҧ Serial ---
        if (pulseWidth > 0)                // ��Ǩ�ͺ����� timeout
        {
            USART_Print("Distance: ");     // ʧ�ͤ��� "Distance: "
            USART_PrintNum((int32_t)(distanceCm * 10));  // ʧ�Ţ (x10 �������շȹ���)
            USART_Print(" cm\r\n");        // ʧ˹�� " cm" ������鹺�÷Ѵ���
            // �����ҧ: 150  "150" cm ��û�Ѻ��ا���ʴ��ȹ�����ầ໹�ǹ Integer/Fraction
        }
        else                               // �� timeout (pulseWidth == 0)
        {
            USART_Print("Out of range!\r\n"); // ᨧ���Թ���з���Ѵ�
        }

        Delay_Ms(500);         // ˹ǧ 500ms �͹�Ѵ���駶Ѵ�
                               // Ŵ������������ HC-SR04 �ӧҹ䴤����
    }                            // ����ش while loop
}                                // ����ش����ѹ main
