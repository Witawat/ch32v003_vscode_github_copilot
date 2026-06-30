/**
 * ============================================================
 * ex01_IWDG_Simple.c
 * ������ҸԵ IWDG (Independent Watchdog) Ẻ����
 * (Simple IWDG demonstration)
 * ============================================================
 *
 * Ἱ�ѧǧ�� (Circuit Diagram):
 *
 *   CH32V003
 *   ------
 *   PC0 (OUT) ----[220?]----+---- LED ---- GND
 *                            |
 *   PD5 (TX)  ----> USB-UART (RX)
 *   PD6 (RX)  <---- USB-UART (TX)
 *
 *   ����ͧ���ػ�ó��������
 *
 * ============================================================
 * ���Ѿ����Ҵ��ѧ (Expected Results):
 *   LED ��о�Ժ 3 ���� (IWDG �١ feed �ء����)
 *    ��ش feed  MCU ����  �����������  ��о�Ժ 3 ����  ǹ���
 *   USART �ʴ�:
 *   "--- IWDG Simple ---"
 *   "Blink 1: feeding watchdog"
 *   "Blink 2: feeding watchdog"
 *   "Blink 3: feeding watchdog"
 *   "Stop feeding! Reset in ~1.6s..."
 *   (MCU reset  �������ͤ��������ա����)
 * ============================================================
 * ����͹ (WARNINGS):
 *   ����� IWDG �١�Դ��ҹ���� �лԴ����騹���� MCU ������
 *     (Once IWDG is enabled, it cannot be stopped except by reset)
 *   ������� LSI �üѹ (30-60kHz ���� 40kHz) ���Ҩ�ԧ�Ҩ��Ҵ����͹ ?25%
 *     (LSI frequency varies 30-60kHz, typical 40kHz � actual timeout may differ ?25%)
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit()"]
 *     C --> D["IWDG_SimpleInit(1600)"]
 *     D --> E["for blinkCount=1 to 3"]
 *     E --> F["LED ON"]
 *     F --> G["IWDG_Feed()"]
 *     G --> H["Delay_Ms(200)"]
 *     H --> I["LED OFF"]
 *     I --> J["IWDG_Feed()"]
 *     J --> K["Delay_Ms(200)"]
 *     K --> L{"blinkCount <= 3?"}
 *     L -->|"Yes"| E
 *     L -->|"No"| M["USART_Print(Stop feeding)"]
 *     M --> N["while(1) รอ IWDG reset"]
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
    // ����ùѺ�ͺ (Loop counter)
    uint8_t blinkCount = 0;                  // �ӹǹ���駷�� LED ��о�Ժ (Number of LED blinks)

    // ---- ��ǹ������� (Initialization) ----
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);                      // ������鹾���͹ء�� (Initialize USART)
    pinMode(PC0, PIN_MODE_OUTPUT);          // ��˹� PC0 ����ҵ�ص (Set PC0 as PIN_MODE_OUTPUT)
    digitalWrite(PC0, LOW);                 // ������� LED �Ѻ (Initialize LED off)

    USART_Print("--- IWDG Simple ---\r\n"); // �ʴ���Ǣ�� (Display title)

    // ---- ������� IWDG (Initialize IWDG) ----
    // IWDG_SimpleInit �����������: prescaler=40, reload=255  timeout ~1.6s
    // (IWDG_SimpleInit uses defaults: prescaler=40, reload=255  timeout ~1.6s)
    IWDG_SimpleInit(1600);                       // ����� IWDG ���¤�ҵ�駵� (Initialize IWDG with defaults)
    USART_Print("IWDG started, timeout ~1.6s\r\n");  // ������� IWDG (Notify IWDG start)

    // ---- ��о�Ժ LED 3 ���� ����� feed IWDG (Blink LED 3 times while feeding IWDG) ----
    for (blinkCount = 1; blinkCount <= 3; blinkCount++) {  // ǹ 3 �ͺ (Loop 3 times)
        digitalWrite(PC0, HIGH);            // �Դ LED (LED on)
        USART_Print("Blink "); USART_PrintNum(blinkCount); USART_Print(": feeding watchdog\r\n");  // ���ͺ��� feed watchdog (Notify feeding)
        IWDG_Feed();                         // ���絵�ǹѺ IWDG (Reset IWDG counter) � ��ͧ�ѹ���� (prevents reset)
        Delay_Ms(200);                       // ˹�ǧ 200ms (Delay 200ms)

        digitalWrite(PC0, LOW);             // �Ѻ LED (LED off)
        IWDG_Feed();                         // feed �ա���� (Feed again)
        Delay_Ms(200);                       // ˹�ǧ 200ms (Delay 200ms)
    }

    // ---- ��ش feed � IWDG ������ MCU ��ѧ�ҡ timeout (Stop feeding � IWDG will reset MCU after timeout) ----
    USART_Print("Stop feeding! Reset in ~1.6s...\r\n");  // �������ش feed (Notify stop feeding)
    // ����ͧ IWDG_Feed() �ա (No more IWDG_Feed() calls)

    // IWDG �йѺ�����ѧ ����Ͷ֧ 0 ������ MCU (IWDG counts down; when it hits 0, MCU resets)
    while (1) {                              // �� IWDG ���� (Wait for IWDG reset)
        // �������� ��� Watchdog ���� (Do nothing, let watchdog reset)
        // __NOP(); �繤�������� (NOP � no operation)
    }
}
