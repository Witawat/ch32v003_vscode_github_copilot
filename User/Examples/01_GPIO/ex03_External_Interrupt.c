/**
 * ============================================================
 * �����ҧ��� 3: External Interrupt (�Թ����Ѻ���¹͡)
 * ============================================================
 *
 * Ἱ�ѧǧ�� (Circuit Diagram):
 *
 *     CH32V003                  ����� (Button)
 *     --------                  -------------
 *     PC1 ---+----/\/\/\---- 3.3V
 *            |      10k Ohm    (Pull-up)
 *            |
 *            +---- ����� ---- GND
 *
 *     PC0 ----/\/\/\---->|---- GND
 *            220 Ohm
 *
 * ============================================================
 * ���Ѿ����Ҵ��ѧ (Expected Results):
 * - �ء���駷�衴��� LED ��� PC0 ������¹ʶҹ� (�Դ�Ѻ)
 * - Serial Monitor �ʴ� "Interrupt #1", "#2", "#3"...
 * - 㪡�÷ӧҹẺ Interrupt (���ͧ Polling)
 * ============================================================
 * ����͹ (WARNINGS):
 * - ����ѹ callback � interrupt �ͧ�����зӧҹ���Ƿ���ش
 * - ��������¡ Delay_Ms() ���Ϳ���ѹ��� blocking � ISR
 * - �ͧ��С�ȵ���÷���áѺ ISR ��¤����Ҥѭ volatile
 * - CH32V003 �ͧ�Ѻ EXTI �٧�ش 8 lines �ҹ��
 * - �ҡ���������Թ� �Ҩ�Դ interrupt ��� (debounce)
 * ============================================================
 * แผนผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["pinMode(PC0, OUTPUT)"]
 *     C --> D["pinMode(PC1, INPUT_PULLUP)"]
 *     D --> E["attachInterrupt(PC1, Button_ISR, FALLING)"]
 *     E --> F["USART_SimpleInit(115200)"]
 *     F --> G["while(1)"]
 *     G --> H{"interruptCounter != lastCount?"}
 *     H -->|"Yes"| I["digitalToggle(PC0)"]
 *     I --> J["update lastCount"]
 *     J --> K["USART_Print('Interrupt #...')"]
 *     K --> G
 *     H -->|"No"| G
 *     subgraph ISR["ISR Context"]
 *         L["interruptCounter++"] --> M["return"]
 *     end
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>   // ����ź���� SimpleHAL ������

volatile uint32_t interruptCounter = 0; // ����ùѺ�ӹǹ interrupt ����Դ���
                                         // volatile �ͧ�ѹ compiler optimize ��
                                         // �����ա������¹�ŧ�ҡ ISR

void Button_ISR(void)    // ����ѹ Interrupt Service Routine ����Ѻ�����
{                        // ����ѹ���ж١���¡�ѵ��ѵ�������Դ interrupt ��� PC1
    interruptCounter++;  // �����ҹѺ interrupt ���� 1
                         // (�ӧҹ���� ���ա�� Delay ���� USART � ISR)
}                        // ����ش ISR

int main(void)           // ����ѹ��ѡ�ͧ�����
{
    SystemCoreClockUpdate();
    Timer_Init();
    pinMode(PC0, PIN_MODE_OUTPUT);       // ��駤Ң� PC0 ໹��ҵ�ص (LED)
    pinMode(PC1, PIN_MODE_INPUT_PULLUP); // ��駤Ң� PC1 ໹�Թ�ص Pull-up (�����)

    attachInterrupt(PC1, Button_ISR, FALLING); // ŧ����¹ interrupt ����Ѻ�� PC1
                                         // ���� FALLING: �Դ interrupt ������ѭ�ҳ
                                         // ����¹�ҡ HIGH  LOW (�͹�����)
                                         // ����ѹ Button_ISR �ж١���¡�ء����

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT); // ������� USART ��� 115200 baud

    uint32_t lastCount = 0;  // ������纤ҹѺ���駡͹˹� ���͵�Ǩ�ͺ�������¹�ŧ

    while(1)                 // ǹ�ٻ͹ѹ��
    {
        if (interruptCounter != lastCount)  // ��Ǩ�ͺ�ҤҹѺ interrupt ����¹�ŧ
        {                                    // (�ա���Դ interrupt ���)
            digitalToggle(PC0);              // ��Ѻʶҹ� LED ��� PC0 (�Դ�Ѻ)
                                             // ���ͧ�ҹ�ҡ͹ ����¹�ѹ��

            lastCount = interruptCounter;    // �ѻവ�ҹѺ���ش

            USART_Print("Interrupt #");      // ʧ�ͤ��� "Interrupt #" � Serial
            USART_PrintNum((int32_t)interruptCounter); // ʧ����Ţ�Ѻ interrupt
            USART_Print("\r\n");             // ��鹺�÷Ѵ���
        }

        // ���� Delay_Ms ��ٻ��ѡ ���еͧ����˵ͺʹͧ����
        // �����Ẻ Busy-wait ��»����Ѵ��ѧ�ҹ�㹺ҧ�ó�
    }                            // ����ش while loop
}                                // ����ش����ѹ main
