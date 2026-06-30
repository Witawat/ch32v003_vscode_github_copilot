/**
 * ============================================================
 * ������ҧ��� 1: DS18B20 ��ҹ�س����� (Temperature Reading)
 * ============================================================
 *
 * Ἱ�ѧǧ�� (Circuit Diagram):
 *
 *     CH32V003              DS18B20
 *     --------              -------
 *     PD2 ----/\/\/\-------+ DQ (Data)
 *            4.7k?          |
 *                           +--- VCC (3.3V)
 *                           |
 *     GND ----------------+ GND
 * 
 *     ��õ��ǧ��:
 *     - �� DQ �ͧ DS18B20 ��͡Ѻ PD2 �ͧ CH32V003
 *     - ��ǵ�ҹ�ҹ 4.7k? �֧�� PD2 ���价�� 3.3V (pull-up)
 *     - VCC �ͧ DS18B20 ��͡Ѻ 3.3V
 *     - GND ��������ѹ
 *
 * ============================================================
 * ���Ѿ����Ҵ��ѧ (Expected Results):
 * - �ء 1 �Թҷ� ���ʴ��س����Է����ҹ��ҧ USART
 * - "Temperature: 28.50 C" (�����س����ԻѨ�غѹ)
 * - ����ö������� DS18B20 ���ʹ��س���������¹�ŧ
 * ============================================================
 * ����͹ (WARNINGS):
 * - ��ͧ�յ�ǵ�ҹ�ҹ 4.7k? pull-up! �������� 1-Wire �����ӧҹ!
 * - ����ŧ�س������������٧�ش 750ms ��ͧ�͡�͹��ҹ���
 * - �ٵäӹǳ�س�����: Temp = (raw>>4) + ((raw&0x0F)*0.0625)
 * - ��ͧ�� Sensor DS18B20 ��ҹ�� (��������Ҩ�����觵�ҧ�ѹ)
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["OneWire_Init(PD2)"]
 *     C --> D["USART_SimpleInit()"]
 *     D --> E["while(1)"]
 *     E --> F["OneWire_Reset()"]
 *     F --> G["OneWire_SkipROM()"]
 *     G --> H["OneWire_WriteByte(0x44)"]
 *     H --> I["Delay_Ms(750)"]
 *     I --> J["OneWire_Reset()"]
 *     J --> K["OneWire_SkipROM()"]
 *     K --> L["OneWire_WriteByte(0xBE)"]
 *     L --> M["Read 9 bytes scratchpad"]
 *     M --> N["คำนวณอุณหภูมิ"]
 *     N --> O["USART_Print(temp)"]
 *     O --> P["Delay_Ms(250)"]
 *     P --> E
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

    uint8_t scratchpad[9]; // Buffer ����Ѻ�红����� Scratchpad 9 亵�

    while (1)              // ǹ�ٻ͹ѹ�� ��ҹ�س����Էء�Թҷ�
    {
        OneWire_Reset(bus);    // �� Reset pulse ����������鹡���������
        OneWire_SkipROM(bus);  // �� Skip ROM (0xCC) ��������к��ػ�ó�
        OneWire_WriteByte(bus, 0x44); // �觤���� Convert T (0x44) ������ŧ�س�����

        Delay_Ms(750);     // �� 750ms �����ҡ���ŧ������ (DS18B20 �������٧�ش 750ms)

        OneWire_Reset(bus);    // �� Reset pulse �ա���駡�͹��ҹ���
        OneWire_SkipROM(bus);  // �� Skip ROM �������͡�ػ�ó����Ǻ����
        OneWire_WriteByte(bus, 0xBE); // �觤���� Read Scratchpad (0xBE) ��ҹ����س�����

        for (uint8_t i = 0; i < 9; i++) // ��ҹ������ Scratchpad ��� 9 亵�
        {
            scratchpad[i] = OneWire_ReadByte(bus); // ��ҹ����亵�ҡ 1-Wire bus
        }

        int16_t raw_temp = (int16_t)(scratchpad[1] << 8) | scratchpad[0]; // ���亵�������٧�繤�� 16-bit signed
        float temperature_c = (raw_temp >> 4) + ((raw_temp & 0x0F) * 0.0625f); // �ӹǳ�س����Ե���ٵ� 12-bit resolution

        char buffer[32];   // Buffer ����Ѻ��ͤ�������ͧ��þ����
        sprintf(buffer, "Temperature: %.2f C\r\n", temperature_c); // �Ѵ�ٻẺ��ͤ��������س����Է����ҹ��
        USART_Print(buffer); // �觢�ͤ�����ѧ USART

        Delay_Ms(250);     // ˹�ǧ�������� 250ms ����� 1 �Թҷյ���ͺ
    }                      // ����ش while loop ��Ѻ�������ͺ����
}                          // ����ش�ѧ��ѹ main
