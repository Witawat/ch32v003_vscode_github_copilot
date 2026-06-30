/**
 * ============================================================
 * ������ҧ��� 3: ���͡�ػ�ó����µ�Ǻ���� (Multi-Device Select)
 * ============================================================
 *
 * Ἱ�ѧǧ�� (Circuit Diagram):
 *
 *     CH32V003          DS18B20 #1      DS18B20 #2
 *     --------          ----------      ----------
 *     PD2 ----/\/\/\---+--- DQ          --- DQ
 *            4.7k?     |                |
 *                      +--- VCC (3.3V)  +--- VCC (3.3V)
 *                      |                |
 *     GND ------------+--- GND         --- GND
 * 
 *     ��õ��ǧ��:
 *     - ����͹�Ѻ ex02 �ء��С�� (2x DS18B20 ��ҹ�ѹ�� PD2)
 *     - ��ͧ��Һ ROM address �ͧ�ػ�ó����е�ǡ�͹ (�� ex02 ����)
 *
 * ============================================================
 * ���Ѿ����Ҵ��ѧ (Expected Results):
 * - "Dev1: 28.5C, Dev2: 29.1C"
 * - �ʴ��س����Ԣͧ�ػ�ó���е�������͡��ҹ Match ROM
 * - �س����Ԣͧ�ػ�ó��� 2 ��Ǩ�ᵡ��ҧ�ѹ��硹���
 * ============================================================
 * ����͹ (WARNINGS):
 * - ��ͧ��� ROM address �ͧ�ػ�ó����е�ǡ�͹ (�� Search �ҡ ex02)
 * - ����� ROM address �Դ �ػ�ó��鹨����ͺʹͧ
 * - ��ͧ�� Reset ��� Match ROM �ء���駡�͹�觤������ѧ�ػ�ó�
 * - �س����Ԣͧ��� 2 ��Ǥ�������§�ѹ ��ҵ�ҧ�ѹ�ҡ�ʴ�����ջѭ��
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit()"]
 *     C --> D["OneWire_Init(PD2)"]
 *     D --> E["while(1)"]
 *     E --> F{"OneWire_Select(rom1)"}
 *     F -->|"Yes"| G["Convert T (0x44)"]
 *     G --> H["Delay_Ms(750)"]
 *     H --> I["OneWire_Select(rom1)"]
 *     I --> J["Read Scratchpad (0xBE)"]
 *     J --> K["คำนวณ temp1"]
 *     K --> L{"OneWire_Select(rom2)"}
 *     F -->|"No"| L
 *     L -->|"Yes"| M["Convert T (0x44)"]
 *     M --> N["Delay_Ms(750)"]
 *     N --> O["OneWire_Select(rom2)"]
 *     O --> P["Read Scratchpad (0xBE)"]
 *     P --> Q["คำนวณ temp2"]
 *     L -->|"No"| R["USART_Print(temp1, temp2)"]
 *     Q --> R
 *     R --> S["Delay_Ms(250)"]
 *     S --> E
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
/* CH32V003 has no hardware FPU � float/double use software emulation (~800 cycles) */
#include <SimpleHAL.h>    // ����ź���� SimpleHAL ������
#include <stdio.h>        // ����ź���� sprintf ����Ѻ�Ѵ�ٻẺ��ͤ���

int main(void)            // �ѧ��ѹ��ѡ �ش������������
{
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT); // ������� USART ��� 115200 baud

    OneWire_Bus* bus = OneWire_Init(PD2); // ������� 1-Wire bus ���� PD2

    uint8_t rom1[8] = {0x28, 0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}; // ROM �ͧ DS18B20 ��Ƿ�� 1 (����¹�繤�Ҩ�ԧ)
    uint8_t rom2[8] = {0x28, 0xFF, 0x98, 0x76, 0x54, 0x32, 0x10, 0xDE}; // ROM �ͧ DS18B20 ��Ƿ�� 2 (����¹�繤�Ҩ�ԧ)

    float temp1 = 0.0f;    // ��������س����Ԣͧ�ػ�ó��Ƿ�� 1
    float temp2 = 0.0f;    // ��������س����Ԣͧ�ػ�ó��Ƿ�� 2

    while (1)              // ǹ�ٻ͹ѹ�� ��ҹ�س����Էء�Թҷ�
    {
        if (OneWire_Select(bus, rom1)) // ���͡�ػ�ó��Ƿ�� 1 (�� Reset + Match ROM)
        {
            OneWire_WriteByte(bus, 0x44); // �觤���� Convert T ����ػ�ó��Ƿ�� 1

            Delay_Ms(750);     // �� 750ms ������ŧ�س���������

            OneWire_Select(bus, rom1); // ���͡�ػ�ó��Ƿ�� 1 �ա����
            OneWire_WriteByte(bus, 0xBE); // �觤���� Read Scratchpad

            uint8_t raw_l = OneWire_ReadByte(bus); // ��ҹ亵��Ӣͧ�س�����
            uint8_t raw_h = OneWire_ReadByte(bus); // ��ҹ亵��٧�ͧ�س�����
            int16_t raw1 = (int16_t)(raw_h << 8) | raw_l; // ����繤�� 16-bit signed
            temp1 = (raw1 >> 4) + ((raw1 & 0x0F) * 0.0625f); // �ӹǳ�س�������ͧ��������
        }

        if (OneWire_Select(bus, rom2)) // ���͡�ػ�ó��Ƿ�� 2 (�� Reset + Match ROM)
        {
            OneWire_WriteByte(bus, 0x44); // �觤���� Convert T ����ػ�ó��Ƿ�� 2

            Delay_Ms(750);     // �� 750ms ������ŧ�س���������

            OneWire_Select(bus, rom2); // ���͡�ػ�ó��Ƿ�� 2 �ա����
            OneWire_WriteByte(bus, 0xBE); // �觤���� Read Scratchpad

            uint8_t raw_l = OneWire_ReadByte(bus); // ��ҹ亵��Ӣͧ�س�����
            uint8_t raw_h = OneWire_ReadByte(bus); // ��ҹ亵��٧�ͧ�س�����
            int16_t raw2 = (int16_t)(raw_h << 8) | raw_l; // ����繤�� 16-bit signed
            temp2 = (raw2 >> 4) + ((raw2 & 0x0F) * 0.0625f); // �ӹǳ�س�������ͧ��������
        }

        char buffer[32];         // Buffer ����Ѻ��ͤ���
        sprintf(buffer, "Dev1: %.1fC, Dev2: %.1fC\r\n", temp1, temp2); // �Ѵ�ٻẺ�س����Է�� 2 ���
        USART_Print(buffer);     // ������س����Էҧ USART

        Delay_Ms(250);           // ˹�ǧ���� 250ms ���ú 1 �Թҷյ���ͺ
    }                            // ����ش while loop
}                                // ����ش�ѧ��ѹ main
