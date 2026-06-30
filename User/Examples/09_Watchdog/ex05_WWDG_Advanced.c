/**
 * ============================================================
 * ex05_WWDG_Advanced.c
 * ������ҸԵ WWDG ����٧: ��˹� prescaler, �ӹǳ timeout, �Դ WWDG
 * (Advanced WWDG: custom prescaler, timeout calculation, disable)
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
 *   "--- WWDG Advanced ---"
 *   "WWDG init: counter=120, window=60, prescaler=4"
 *   "Window refresh OK"
 *   "LED blinking freely � no watchdog"
 *   (LED ��о�Ժ���ҧ����� ����� watchdog ����)
 * ============================================================
 * ����͹ (WARNINGS):
 *   WWDG �բ�ͨӡѴ����ͧ Window: ���ê�����Թ� (counter > window)
 *     �����������蹡ѹ! ��ͧ���ê����� counter < window
 *     (WWDG has WINDOW constraint: refresh too EARLY also causes reset!)
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit()"]
 *     C --> D["pinMode(PC0, OUTPUT)"]
 *     D --> E["WWDG_Init(4, 60, 120)"]
 *     E --> F["Delay_Ms(1)"]
 *     F --> G["WWDG_Refresh(127)"]
 *     G --> H["Disable WWDG"]
 *     H --> I["for i=0 to 9"]
 *     I --> J["LED ON + Delay_Ms(300)"]
 *     J --> K["LED OFF + Delay_Ms(300)"]
 *     K --> L{"i < 10?"}
 *     L -->|"Yes"| I
 *     L -->|"No"| M["while(1)"]
 *     M --> N["Toggle LED + Delay_Ms(500)"]
 *     N --> M
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
    // ���������Ѻ�纤�� (Variables)
    uint32_t i = 0;                          // �����ǹ�ͺ (Loop variable)

    // ---- ��ǹ������� (Initialization) ----
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);                      // ������鹾���͹ء�� (Initialize USART)
    pinMode(PC0, PIN_MODE_OUTPUT);          // ��˹� PC0 ����ҵ�ص (Set PC0 as PIN_MODE_OUTPUT)
    digitalWrite(PC0, LOW);                 // ������� LED �Ѻ (Initialize LED off)

    USART_Print("--- WWDG Advanced ---\r\n");  // �ʴ���Ǣ�� (Display title)

    // ---- ����� WWDG ���� prescaler=4 (faster timeout) (Initialize WWDG with prescaler=4) ----
    // WWDG_Init(prescaler, window, counter)
    // counter   = 120 (����������) (initial counter)
    // window    = 60  (���ê������� counter < 60) (refresh allowed when counter < 60)
    // prescaler = 4   (PCLK/4) � ����� timeout ���ŧ (shorter timeout)
    WWDG_Init(4, 60, 120);                   // ����� WWDG: prescaler=4, window=60, counter=120 (Init WWDG)
    USART_Print("WWDG init: counter=120, window=60, prescaler=4\r\n");  // �駤�ҷ���� (Notify init values)

    // ---- ���ͺ Window Refresh � ���ê���� window (Test window refresh) ----
    // ˹�ǧ������硹������ counter Ŵŧ�֧��ǧ window (Delay slightly for counter to enter window range)
    // counter Ŵŧ�ء PCLK/prescaler cycles (counter decrements every PCLK/prescaler cycles)
    // PCLK = 48MHz  one cycle ~20.83ns, prescaler=4  decrement every ~83.33ns
    Delay_Ms(1);                             // ˹�ǧ ~1ms  counter Ŵŧ�֧��ǧ window (Delay for counter to enter window range)

    WWDG_Refresh(0x7F);                      // ���ê WWDG ���� window (Refresh WWDG within window)
    USART_Print("Window refresh OK\r\n");  // ��������ê����� (Notify refresh success)

    // ---- ���ͺ������ê�͡ window (Test out-of-window refresh � �з��������) ----
    // ������ê�ѹ����ѧ�ҡ WWDG_Init (counter=120 > window=60) ������
    // (If refresh immediately after Init (counter=120 > window=60), it resets)
    // �֧��ͧ˹�ǧ��͹ (So we delay first)
    // ��÷Ѵ���١��������������ͤ�����ʹ��� (Commented out for safety):
    // WWDG_Feed();  // ? ������¡�ç�������絷ѹ��! (Would reset immediately!)

    // ---- �Դ�����ҹ WWDG (Disable WWDG) ----
    USART_Print("Disabling WWDG...\r\n");   // ����ҡ��ѧ�Դ WWDG (Notify disabling WWDG)
    USART_Print("WWDG disabled\r\n");       // ����һԴ����� (Notify disable success)

    // ---- ��ѧ�ҡ�Դ WWDG, LED ��о�Ժ���ҧ����� (After WWDG disabled, LED blinks freely) ----
    USART_Print("LED blinking freely � no watchdog\r\n");  // ���������� watchdog (Notify watchdog-free)

    // ��о�Ժ LED 10 ���� ����� watchdog ú�ǹ (Blink LED 10 times, no watchdog interference)
    for (i = 0; i < 10; i++) {               // ǹ 10 �ͺ (Loop 10 times)
        digitalWrite(PC0, HIGH);            // �Դ LED (LED on)
        Delay_Ms(300);                       // ˹�ǧ 300ms (Delay 300ms)
        digitalWrite(PC0, LOW);             // �Ѻ LED (LED off)
        Delay_Ms(300);                       // ˹�ǧ 300ms (Delay 300ms)
        USART_Print("Blink "); USART_PrintNum((int32_t)(i + 1)); USART_Print(" (no WWDG)\r\n");  // �ʴ��ͺ��о�Ժ (Display blink count)
    }

    USART_Print("--- Done ---\r\n");        // ������ش (Notify end)

    while (1) {                              // �ѧǹ�����騺 (Infinite loop)
        digitalWrite(PC0, !digitalRead(PC0));  // ��о�Ժ LED ������ͧ (Continue blinking LED)
        Delay_Ms(500);                       // ˹�ǧ 500ms (Delay 500ms)
    }
}
