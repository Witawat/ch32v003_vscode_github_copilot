/**
 * ============================================================
 * ex03_WWDG_Simple.c
 * ������ҸԵ WWDG (Window Watchdog) Ẻ����
 * (Simple WWDG demonstration)
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
 *   LED �Դ��ҧ  WWDG ���ê 5 ����  ��ش���ê  MCU ����  ���������
 *   USART �ʴ�:
 *   "--- WWDG Simple ---"
 *   "Refresh #1: feeding WWDG"
 *   "Refresh #2: feeding WWDG"
 *   "Refresh #3: feeding WWDG"
 *   "Refresh #4: feeding WWDG"
 *   "Refresh #5: feeding WWDG"
 *   "Stop refreshing! Reset in ~5.4ms..."
 *   (MCU reset  ��ͤ�������ա����)
 * ============================================================
 * ����͹ (WARNINGS):
 *   WWDG �բ�ͨӡѴ����ͧ Window: ���ê�����Թ� (counter > window)
 *     �����������蹡ѹ! ��ͧ���ê����� window < counter < 0x40
 *     (WWDG has WINDOW constraint: refresh too EARLY also causes reset!)
 *   ���� timeout �٧�ش ~87ms ��� prescaler = 8
 *     (Max timeout ~87ms at prescaler 8)
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit()"]
 *     C --> D["pinMode(PC0, OUTPUT)"]
 *     D --> E["WWDG_SimpleInit(127, 80)"]
 *     E --> F["digitalWrite(PC0, HIGH)"]
 *     F --> G["for refreshCount=1 to 5"]
 *     G --> H["Delay_Ms(1)"]
 *     H --> I["WWDG_Refresh(127)"]
 *     I --> J{"refreshCount <= 5?"}
 *     J -->|"Yes"| G
 *     J -->|"No"| K["Stop refreshing"]
 *     K --> L["while(1) รอ WWDG reset"]
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
    // ����ùѺ�ͺ���ê (Refresh counter)
    uint8_t refreshCount = 0;                // �ӹǹ���駷�����ê WWDG (Number of WWDG refreshes)

    // ---- ��ǹ������� (Initialization) ----
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);                      // ������鹾���͹ء�� (Initialize USART)
    pinMode(PC0, PIN_MODE_OUTPUT);          // ��˹� PC0 ����ҵ�ص (Set PC0 as PIN_MODE_OUTPUT)
    digitalWrite(PC0, LOW);                 // ������� LED �Ѻ (Initialize LED off)

    USART_Print("--- WWDG Simple ---\r\n"); // �ʴ���Ǣ�� (Display title)

    // ---- ������� WWDG (Initialize WWDG) ----
    // WWDG_SimpleInit(127, 80)
    // counter = 127 (���������� �٧�ش)  (initial counter, max value)
    // window  = 80  (��ͧ���ê����� counter < 80)  (must refresh when counter < 80)
    // prescaler ����������� = 8
    WWDG_SimpleInit(0x7F, 80);               // ����� WWDG: counter=127, window=80 (Init WWDG)

    digitalWrite(PC0, HIGH);                // �Դ LED �ʴ���ҷӧҹ (Turn LED on to indicate active)

    USART_Print("WWDG started: counter=127, window=80\r\n");  // �駤��������� (Notify init values)

    // ---- ���ê WWDG 5 ���� (Refresh WWDG 5 times) ----
    for (refreshCount = 1; refreshCount <= 5; refreshCount++) {  // ǹ 5 �ͺ (Loop 5 times)
        // ˹�ǧ������硹���������� counter Ŵŧ�֧��ǧ window (Small delay to let counter enter window range)
        Delay_Ms(1);                         // ˹�ǧ 1ms ��� counter Ŵŧ (Delay 1ms for counter to decrease)

        WWDG_Refresh(0x7F);                  // ���ê WWDG (Refresh WWDG)
        USART_Print("Refresh #"); USART_PrintNum(refreshCount); USART_Print(": feeding WWDG\r\n");  // ���ͺ������ê (Notify refresh round)
    }

    // ---- ��ش���ê � WWDG ������ MCU (Stop refreshing � WWDG will reset MCU) ----
    USART_Print("Stop refreshing! Reset in ~5.4ms...\r\n");  // �������ش���ê (Notify stop refreshing)

    // WWDG ��Ŵ���ŧ������ � ����Ͷ֧ 0x3F (63) ������ (WWDG decrements; when reaching 0x3F (63), resets)
    while (1) {                              // �� WWDG ���� (Wait for WWDG reset)
        // ����ա�� WWDG_Feed() �ա (No more WWDG_Feed() calls)
    }
}
