/**
 * ============================================================
 * ex04_WWDG_Interrupt.c
 * ������ҸԵ WWDG Interrupt (Early Wakeup Interrupt � EWI)
 * (WWDG Early Wakeup Interrupt demonstration)
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
 *   ����� counter �֧ 0x40, EWI fires  callback �����:
 *     "Early warning! Refreshing..."
 *    ���ê WWDG  ����ա������
 *   USART �ʴ�ǹ������� �:
 *   "--- WWDG Interrupt ---"
 *   "WWDG with interrupt started"
 *   "Early warning! Refreshing..."
 *   "Early warning! Refreshing..."
 *   ...
 * ============================================================
 * ����͹ (WARNINGS):
 *   Interrupt ����͹ � �ѧ��ͧ���ê WWDG 㹪�ǧ window ���ͻ�ͧ�ѹ����
 *     (The interrupt only warns � must still refresh within window to avoid reset)
 *   ��ͧ���� WWDG_IRQHandler � ch32v00x_it.c
 *     ��������¡ WWDG_IRQHandler_Callback()
 *     (Must add WWDG_IRQHandler in ch32v00x_it.c calling WWDG_IRQHandler_Callback())
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit()"]
 *     C --> D["pinMode(PC0, OUTPUT)"]
 *     D --> E["WWDG_SetCallback()"]
 *     E --> F["WWDG_InitWithInterrupt(127, 80, 8)"]
 *     F --> G["digitalWrite(PC0, HIGH)"]
 *     G --> H["while(1)"]
 *     H --> I{"earlyWarningFired?"}
 *     I -->|"Yes"| J["Reset flag"]
 *     I -->|"No"| K["Toggle LED"]
 *     J --> K
 *     K --> L["Delay_Ms(500)"]
 *     L --> H
 *     F --> M["EWI fires (counter=0x40)"]
 *     M --> N["WWDG_EarlyWarningCallback()"]
 *     N --> O["Set flag + WWDG_Refresh(127)"]
 *     O --> H
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>                      // ����ź���� SimpleHAL (Include SimpleHAL library)

// --------------------------------------------------------------------------
// �������Ф�Ҥ���� (Variables and constants)
// --------------------------------------------------------------------------

static volatile uint8_t earlyWarningFired = 0;  // ����ú͡��� EWI �Դ������� (Flag for EWI occurrence)

// --------------------------------------------------------------------------
// �ѧ��ѹ callback ����Ѻ WWDG interrupt (Callback function for WWDG interrupt)
// --------------------------------------------------------------------------

void WWDG_EarlyWarningCallback(void)         // �ѧ��ѹ���١���¡����� EWI �Դ��� (Called when EWI fires)
{
    earlyWarningFired = 1;                   // ��� flag ����Դ EWI (Set flag that EWI occurred)
    USART_Print("Early warning! Refreshing...\r\n");  // �ʴ���ͤ�����͹ (Display warning message)

    // ���ê WWDG ���ͻ�ͧ�ѹ���� (Refresh WWDG to prevent reset)
    WWDG_Refresh(0x7F);                      // ���ê WWDG (Refresh WWDG)
}

// --------------------------------------------------------------------------
// �ѧ��ѹ��ѡ (Main function)
// --------------------------------------------------------------------------

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    // ---- ��ǹ������� (Initialization) ----
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);                      // ������鹾���͹ء�� (Initialize USART)
    pinMode(PC0, PIN_MODE_OUTPUT);          // ��˹� PC0 ����ҵ�ص (Set PC0 as PIN_MODE_OUTPUT)
    digitalWrite(PC0, HIGH);                // �Դ LED �ʴ���ҷӧҹ (Turn LED on to indicate active)

    USART_Print("--- WWDG Interrupt ---\r\n");  // �ʴ���Ǣ�� (Display title)

    // ---- ����� WWDG ����� Interrupt (Initialize WWDG with Interrupt) ----
    WWDG_SetCallback(WWDG_EarlyWarningCallback);      // ��� callback ����Ѻ Early Wakeup (Set callback for EWI)
    WWDG_InitWithInterrupt(0x7F, 0x50, 8);            // ����� WWDG: counter=127, window=80, prescaler=8

    USART_Print("WWDG with interrupt started\r\n");  // ���������� WWDG Ẻ�� Interrupt (Notify interrupt mode started)

    // ---- �ѧǹ��ѡ (Main loop) ----
    while (1) {                              // ǹ�ͺ�������ش (Infinite loop)
        if (earlyWarningFired == 1) {        // ����Դ EWI (If EWI occurred)
            earlyWarningFired = 0;           // ���� flag (Reset flag)
        }

        // ��о�Ժ LED ��� � �ʴ���� MCU �ѧ�ӧҹ (Slow blink to show MCU is alive)
        digitalWrite(PC0, !digitalRead(PC0));  // ��Ѻʶҹ� LED (Toggle LED)
        Delay_Ms(500);                       // ˹�ǧ 500ms (Delay 500ms)
    }
}
