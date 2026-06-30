/**
 * ============================================================
 * �����ҧ��� 5: Port Operations (��ô��Թ����дѺ����)
 * ============================================================
 *
 * Ἱ�ѧǧ�� (Circuit Diagram):
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
 *          (�ء��� 220 Ohm)
 *
 *     ����� LED bar Ẻ 8 ��᷹���
 *     PC7 PC6 PC5 PC4 PC3 PC2 PC1 PC0
 *     [D7][D6][D5][D4][D3][D2][D1][D0]
 *
 * ============================================================
 * ���Ѿ����Ҵ��ѧ (Expected Results):
 * - �Һ����� GPIOC �����¹�ء 500ms ����ӴѺ:
 *   0xAA  0x55  0xF0  0x0F  0xAA  ...
 * - LEDs �ʴ��ٻẺ໹ 8-bit binary
 * - 0xAA = 10101010, 0x55 = 01010101
 * - 0xF0 = 11110000, 0x0F = 00001111
 * ============================================================
 * ����͹ (WARNINGS):
 * - portWrite() ʧ�š�з��ͷء��㹾��쵾����ѹ (PC0-PC7)
 * - �ͧ�����ҷء�ҷ��㪶١���໹ OUTPUT mode �͹
 * - portRead() �ҹ�һ��غѹ�ͧ�ء�� �����駢ҷ��໹ INPUT ���
 * - portWrite ��¹�Ѻ�� ODR �µç (��� BSRR)
 * - �������� 8 LEDs �ͧ���Թ 120mA
 * ============================================================
 * แผนผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["pinMode(PC0-PC7, OUTPUT)"]
 *     C --> D["define pattern array"]
 *     D --> E["while(1)"]
 *     E --> F["portWrite(GPIOC, patterns[index])"]
 *     F --> G["portRead(GPIOC)"]
 *     G --> H["index++"]
 *     H --> I{"index >= 4?"}
 *     I -->|"Yes"| J["index = 0"]
 *     I -->|"No"| K["Delay_Ms(500)"]
 *     J --> K
 *     K --> E
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>   // ����ź���� SimpleHAL ������

int main(void)           // ����ѹ��ѡ�ͧ�����
{
    SystemCoreClockUpdate();
    Timer_Init();
    // ��駤Ң� PC0 - PC7 ������ 8 ����໹��ҵ�ص
    // 㪡�õ�駤ҷ��Т� ���� CH32V003 ��� GPIOC ���� 8 ��
    pinMode(PC0, PIN_MODE_OUTPUT);
    pinMode(PC1, PIN_MODE_OUTPUT);
    pinMode(PC2, PIN_MODE_OUTPUT);
    pinMode(PC3, PIN_MODE_OUTPUT);
    pinMode(PC4, PIN_MODE_OUTPUT);
    pinMode(PC5, PIN_MODE_OUTPUT);
    pinMode(PC6, PIN_MODE_OUTPUT);
    pinMode(PC7, PIN_MODE_OUTPUT);

    // ��ҧ array �ͧ�ҷ��ͧ����ʴ�������
    const uint8_t patterns[] = {0xAA, 0x55, 0xF0, 0x0F};
    // 0xAA = 10101010 (LED �ٵԴ, ���Ѻ)
    // 0x55 = 01010101 (LED ���Դ, �ٴѺ)
    // 0xF0 = 11110000 (PC4-PC7 �Դ, PC0-PC3 �Ѻ)
    // 0x0F = 00001111 (PC0-PC3 �Դ, PC4-PC7 �Ѻ)

    uint8_t index = 0;   // ����ê����˹�� array patterns

    while(1)                 // ǹ�ٻ͹ѹ��
    {
        portWrite(GPIOC, patterns[index]);  // ��¹�� 8-bit �§���� GPIOC
                                           // portWrite ʧ�ŷѹ�շء�Ҿ����ѹ
                                           // ���ͧ���¡ digitalWrite ���Т�

        uint8_t currentValue = portRead(GPIOC);  // �ҹ�һ��غѹ�ͧ GPIOC
                                           (void)currentValue;  // �ͧ�ѹ compiler warning
                                           // �ó���䴹Ӥ���

        index++;                     // ���� index ��������͹令ҶѴ�
        if (index >= 4)              // �� index �Թ��Ҵ array (0-3)
        {
            index = 0;               // ��૵ index ��Ѻ价����á (0xAA)
        }

        Delay_Ms(500);         // ˹ǧ���� 500ms �͹���¹໹�ٻẺ�Ѵ�
    }                            // ����ش while loop
}                                // ����ش����ѹ main
