/**
 * ============================================================
 * ex05_Factory_Reset.c
 * ��������絤���ç�ҹ � ������ 3 �Թҷ�����ź Flash ������
 * (Factory reset � hold button 3 seconds to erase entire Flash)
 * ============================================================
 *
 * Ἱ�ѧǧ�� (Circuit Diagram):
 *
 *   CH32V003
 *   ------
 *   PA1 (OUT) ----[220?]----+---- LED (ᴧ) ---- GND
 *                            |
 *   PC0 (IN)  ----[10k?]----+---- GND  (�֧ŧ)
 *                  |
 *                  +---- ������ ---- VCC (3.3V)
 *
 *   PD5 (TX) ----> USB-UART (RX)
 *   PD6 (RX) <---- USB-UART (TX)
 *
 *   ����͡�������ҧ 3 �Թҷ�: Factory reset
 *   LED ��о�Ժ���������ҧ reset
 *
 * ============================================================
 * ���Ѿ����Ҵ��ѧ (Expected Results):
 *   "--- Flash Factory Reset ---"
 *   "System running. Hold button PC0 for 3s to factory reset..."
 *   (��������ҧ 3 �Թҷ�  LED ��о�Ժ����)
 *   "Factory reset: 2 pages erased"
 *   (LED �Ѻ)
 *   "--- Done ---"
 * ============================================================
 * ����͹ (WARNINGS):
 *   ? Flash_EraseAll() ��ź��駤͹�ԡ��Т����� � �������ö���׹��!
 *     (Flash_EraseAll erases BOTH config and data pages irreversibly!)
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit()"]
 *     C --> D["pinMode(PC0, INPUT_PULLUP)"]
 *     D --> E["pinMode(PA1, OUTPUT)"]
 *     E --> F["Flash_Init()"]
 *     F --> G["while(1)"]
 *     G --> H{"button pressed?"}
 *     H -->|"No"| I["pressCount = 0"]
 *     I --> J["Delay_Ms(10)"]
 *     J --> G
 *     H -->|"Yes"| K["pressCount++"]
 *     K --> L{"pressCount >= 300?"}
 *     L -->|"No"| J
 *     L -->|"Yes"| M["Blink LED 10 times"]
 *     M --> N["Flash_EraseAll()"]
 *     N --> O["pressCount = 0"]
 *     O --> J
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>                      // ����ź���� SimpleHAL (Include SimpleHAL library)

// --------------------------------------------------------------------------
// �ѧ��ѹ��ѡ (Main function)
// --------------------------------------------------------------------------

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    // ����ùѺ���� (Timing variables)
    uint32_t pressCount = 0;                 // ��ǹѺ�ӹǹ�ͺ��衴������ҧ (Counter for button hold duration)
    uint8_t  buttonState = 0;                // ʶҹл�������ش (Latest button state)

    // ---- ��ǹ������� (Initialization) ----
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);                      // ������鹾���͹ء�� (Initialize USART)
    pinMode(PC0, PIN_MODE_INPUT_PULLUP);    // ��˹� PC0 ���Թ�ص�֧��� (Set PC0 as input with pull-up)
    pinMode(PA1, PIN_MODE_OUTPUT);          // ��˹� PA1 ����ҵ�ص (Set PA1 as output)
    digitalWrite(PA1, LOW);                 // ������� LED �Ѻ (Initialize LED off)

    USART_Print("--- Flash Factory Reset ---\r\n");  // �ʴ���Ǣ�� (Display title)
    Flash_Init();                            // ������������ Flash (Initialize Flash module)

    USART_Print("System running. Hold button PC0 for 3s to factory reset...\r\n");  // �駼���� (Prompt user)

    // ---- �ѧǹ��ѡ (Main loop) ----
    while (1) {                              // ǹ�ͺ�������ش (Infinite loop)
        buttonState = digitalRead(PC0);     // ��ҹʶҹл����� (Read button state)

        if (buttonState == 0) {              // ��ҡ����� (�֧ŧ, Active Low) (If button is pressed, active low)
            pressCount++;                    // ������ǹѺ (Increment counter)
            if (pressCount >= 300) {         // 300 ? ~10ms = ~3000ms (3 seconds)
                USART_Print("Factory reset initiated!\r\n");  // ����������� (Notify factory reset start)

                // ��о�Ժ LED ���� � (Fast LED blink)
                for (uint8_t i = 0; i < 10; i++) {  // ��о�Ժ 10 ���� (Blink 10 times)
                    digitalWrite(PA1, HIGH);  // �Դ LED (LED on)
                    Delay_Ms(50);            // ˹�ǧ 50ms (Delay 50ms)
                    digitalWrite(PA1, LOW);  // �Ѻ LED (LED off)
                    Delay_Ms(50);            // ˹�ǧ 50ms (Delay 50ms)
                }

                // ź Flash ������ (Erase all Flash)
                Flash_EraseAll();            // ź�����ŷ����� (Erase all data)

                USART_Print("Factory reset: 2 pages erased\r\n");  // ��ź����� (Notify erase complete)
                digitalWrite(PA1, LOW);     // �Ѻ LED (LED off)

                pressCount = 0;              // ���絵�ǹѺ (Reset counter)
            }
        } else {                             // ��һ���»��� (If button is released)
            pressCount = 0;                  // ���絵�ǹѺ (Reset counter)
        }

        Delay_Ms(10);                        // ˹�ǧ 10ms ����Ŵ�����վ��� (Delay 10ms to reduce CPU usage)
    }
}
