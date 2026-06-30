/**
 * ============================================================
 * �����ҧ��� 6: Shift Register (74HC595 + Knight Rider)
 * ============================================================
 *
 * Ἱ�ѧǧ�� (Circuit Diagram):
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
 *                                          (�ء��� 220 Ohm)
 *
 *     MR (10) ---> 3.3V (reset disable)
 *     OE (13) ---> GND (output enable  = active LOW)
 *
 * ============================================================
 * ���Ѿ����Ҵ��ѧ (Expected Results):
 * - LEDs 8 �ǧ�ʴ��ٻẺ Knight Rider (��信�Ѻ仡�Ѻ��)
 * - ����͹� KITT �ö Knight Rider
 * - LED ��觨ҡ���仢�� ��ǡ�Ѻ�ҡ����ҫ�� ǹ��������
 * ============================================================
 * ����͹ (WARNINGS):
 * - 74HC595 �ͧ�Ѻ� 5V � CH32V003 ໹ 3.3V ��� OK ����Ѻ�ͨԡ
 * - 74HC595 �Ѻ Vih ��鹵�� ~3.15V ��� 5V VCC - ���� VCC=3.3V ���� 5V
 * - �ͧ�� MR (pin 10) � VCC ����������૵
 * - �ͧ�� OE (pin 13) � GND ����໴㪧ҹ��ҵ�ص
 * - ������ C 100nF ����ҧ VCC-GND ��� 74HC595
 * ============================================================
 * แผนผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["pinMode(DATA, CLOCK, LATCH, OUTPUT)"]
 *     C --> D["while(1)"]
 *     D --> E["for i = 0 to 7"]
 *     E --> F["digitalWrite(LATCH, LOW)"]
 *     F --> G["shiftOut(MSBFIRST, 1<<i)"]
 *     G --> H["digitalWrite(LATCH, HIGH)"]
 *     H --> I["Delay_Ms(100)"]
 *     I --> J{"i < 8?"}
 *     J -->|"Yes"| E
 *     J -->|"No"| K["for i = 7 to 0"]
 *     K --> L["digitalWrite(LATCH, LOW)"]
 *     L --> M["shiftOut(MSBFIRST, 1<<i)"]
 *     M --> N["digitalWrite(LATCH, HIGH)"]
 *     N --> O["Delay_Ms(100)"]
 *     O --> P{"i >= 0?"}
 *     P -->|"Yes"| K
 *     P -->|"No"| D
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>   // ����ź���� SimpleHAL ������

// ��˹����͢�����Ѻ�� 74HC595 ���ͧ�µ͡�����
#define DATA_PIN   PC0   // ��ʧ����� (Serial Data Input - DS)
#define CLOCK_PIN  PC1   // ���ѭ�ҳ���ԡ� (Shift Clock - SH_CP)
#define LATCH_PIN  PC2   // ����ͤ����� (Storage/Latch Clock - ST_CP)

int main(void)           // ����ѹ��ѡ�ͧ�����
{
    SystemCoreClockUpdate();
    Timer_Init();
    // ��駤Ңҷ��� 74HC595 ��� 3 ��໹��ҵ�ص
    pinMode(DATA_PIN,  PIN_MODE_OUTPUT);  // DATA (DS) output
    pinMode(CLOCK_PIN, PIN_MODE_OUTPUT);  // CLOCK (SH_CP) output
    pinMode(LATCH_PIN, PIN_MODE_OUTPUT);  // LATCH (ST_CP) output

    while(1)                 // ǹ�ٻ͹ѹ��
    {
        // Knight Rider Pattern: ��觨ҡ���仢�� (PC0  PC7)
        // LED ��� 0 = QA, LED ��� 7 = QH
        for (int i = 0; i < 8; i++)   // i = 0 �֧ 7 (���仢��)
        {
            digitalWrite(LATCH_PIN, LOW);  // ��� LATCH ໹ LOW ��������ʧ�����
                                           // (���ͧ latch ����Ҩ�ʧ����Ťú)

            shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, (1 << i));
            // shiftOut: ʧ����� 1 byte Ẻ MSB �͹
            // (1 << i) = ��ҧ bit pattern હ i=0  00000001, i=1  00000010
            // bit ��� i ��໹ 1 (LED �ǧ��鹵Դ) ��������໹ 0 (�Ѻ)

            digitalWrite(LATCH_PIN, HIGH); // ��� LATCH ໹ HIGH �����ͤ�����
                                           // ����Ũл�ҡ���� Q0-Q7 �ѹ��

            Delay_Ms(100);                  // ˹ǧ���� 100ms �͹��Ѻ LED �Ѵ�
        }

        // Knight Rider Pattern: ��觨ҡ����ҫ�� (PC7  PC0)
        for (int i = 7; i >= 0; i--)  // i = 7 �֧ 0 (����ҫ��)
        {
            digitalWrite(LATCH_PIN, LOW);  // �����ʧ�������ѧ shift register

            shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, (1 << i));
            // (1 << i) = ��ҧ bit pattern હ i=7  10000000, i=6  01000000

            digitalWrite(LATCH_PIN, HIGH); // ��ͤ������˻�ҡ������ҵ�ص

            Delay_Ms(100);                  // ˹ǧ���� 100ms
        }
        // ����ͨ� 2 loops �С�Ѻ价ӫ���ա (����-��ǹ��������)
    }                            // ����ش while loop
}                                // ����ش����ѹ main
