/**
 * ============================================================
 * ex02_IWDG_Advanced.c
 * ������ҸԵ IWDG Ẻ��˹�����ͧ �������Ǩ�ͺ���˵�����
 * (Advanced IWDG with custom config and reset cause detection)
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
 * ============================================================
 * ���Ѿ����Ҵ��ѧ (Expected Results):
 *   �ٵ�����á: "Clean boot"  feed IWDG 5 �Թҷ�  ��ش feed  ����
 *   �ٵ���駷���ͧ: "Watchdog reset! Last timeout: 2000ms"
 *   (First boot: "Clean boot"  feed 5s  stop  reset)
 *   (Second boot: "Watchdog reset! Last timeout: 2000ms")
 * ============================================================
 * ����͹ (WARNINGS):
 *   IWDG_WasResetCause() ��Ǩ�ͺ RCC flag � ��ͧź��ѧ��ҹ
 *     ���ͻ�ͧ�ѹ��õ�Ǩ�Ѻ���㹺ٵ�Ѵ�
 *     (Clear flag after reading to avoid false detection on next boot)
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit()"]
 *     C --> D["IWDG_WasResetCause()"]
 *     D --> E{"IWDG reset?"}
 *     E -->|"Yes"| F["USART_Print(Watchdog reset)"]
 *     E -->|"No"| G["USART_Print(Clean boot)"]
 *     F --> H["IWDG_ClearResetFlag()"]
 *     G --> H
 *     H --> I["IWDG_Init(64, 1249)"]
 *     I --> J["IWDG_GetTimeout()"]
 *     J --> K["IWDG_IsBusy()"]
 *     K --> L["for feedCount=0 to 24"]
 *     L --> M["Toggle LED + IWDG_Feed()"]
 *     M --> N["Delay_Ms(200)"]
 *     N --> O{"feedCount < 25?"}
 *     O -->|"Yes"| L
 *     O -->|"No"| P["Stop feeding"]
 *     P --> Q["while(1) รอ IWDG reset"]
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
    // ����õ�ҧ � (Variables)
    uint8_t  resetCause = 0;                 // ���˵ء������ (Reset cause flag)
    uint32_t timeoutMs   = 0;                // ��� timeout ���ӹǳ�� (Calculated timeout in ms)
    uint8_t  feedCount   = 0;                // ��ǹѺ�ͺ��� feed (Feed counter)
    uint8_t  isBusy      = 0;                // ʶҹ� IWDG ���ѧ�ӧҹ (IWDG busy flag)

    // ---- ��ǹ������� (Initialization) ----
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);                      // ������鹾���͹ء�� (Initialize USART)
    pinMode(PC0, PIN_MODE_OUTPUT);          // ��˹� PC0 ����ҵ�ص (Set PC0 as PIN_MODE_OUTPUT)
    digitalWrite(PC0, LOW);                 // ������� LED �Ѻ (Initialize LED off)

    USART_Print("--- IWDG Advanced ---\r\n");  // �ʴ���Ǣ�� (Display title)

    // ---- ��Ǩ�ͺ���˵ء������ (Check reset cause) ----
    resetCause = IWDG_WasResetCause();       // ��Ǩ�ͺ������絨ҡ IWDG ������� (Check if IWDG caused reset)
    if (resetCause == 1) {                   // ������絨ҡ IWDG (If reset from IWDG)
        USART_Print("Watchdog reset! Last timeout: "); USART_PrintNum((int32_t)IWDG_GetTimeout(IWDG_PRESCALER_64, 1249)); USART_Print("ms\r\n");  // ��������絨ҡ IWDG �������� timeout (Notify IWDG reset with timeout)
        IWDG_ClearResetFlag();               // ź flag �����������Ǩ�Ѻ���㹺ٵ˹�� (Clear flag to avoid false detection)
    } else {                                 // ����繡�úٵ���� (If normal boot)
        USART_Print("Clean boot\r\n");      // ����Һٵ���� (Notify clean boot)
    }

    // ---- ����� IWDG ���¤�ҷ���˹��ͧ (Initialize IWDG with custom config) ----
    // prescaler=32, reload=1249  timeout ? (32*1249)/40000 = 0.9992s ? 1000ms
    // ���� 2000ms (prescaler=64, reload=1249)
    IWDG_Init(64, 1249);                     // ����� IWDG: prescaler=64, reload=1249  ~2000ms (Init IWDG)
    USART_Print("IWDG initialized: prescaler=64, reload=1249\r\n");  // �駤��������� (Notify init params)

    timeoutMs = IWDG_GetTimeout(IWDG_PRESCALER_64, 1249);           // �֧��� timeout ���ӹǳ�� (Get calculated timeout)
    USART_Print("Calculated timeout: "); USART_PrintNum((int32_t)timeoutMs); USART_Print("ms\r\n");  // �ʴ���� timeout (Display timeout)

    // ---- ��Ǩ�ͺʶҹ� IWDG (Check IWDG status) ----
    isBusy = IWDG_IsBusy();                  // ��Ǩ�ͺ��� IWDG ���ѧ���������� (Check if IWDG is busy)
    USART_Print("IWDG busy status: "); USART_PrintNum(isBusy); USART_Print("\r\n");  // �ʴ�ʶҹ� busy (Display busy status)

    // ---- Feed IWDG 5 �Թҷ� (Feed IWDG for 5 seconds) ----
    USART_Print("Feeding watchdog for 5s...\r\n");  // ���������� feed 5 �Թҷ� (Notify feeding for 5s)
    for (feedCount = 0; feedCount < 25; feedCount++) {  // 25 �ͺ ? 200ms = 5 �Թҷ� (25 cycles ? 200ms = 5s)
        digitalWrite(PC0, !digitalRead(PC0));  // ��Ѻʶҹ� LED (Toggle LED)
        IWDG_Feed();                         // ���絵�ǹѺ IWDG (Reset IWDG counter)
        Delay_Ms(200);                       // ˹�ǧ 200ms (Delay 200ms)
    }

    // ---- ��ش feed � �� IWDG ���� (Stop feeding � wait for IWDG reset) ----
    USART_Print("Stop feeding! IWDG will reset in ~"); USART_PrintNum((int32_t)timeoutMs); USART_Print("ms...\r\n");  // �������ش feed ������ (Notify stop feeding, reset pending)
    digitalWrite(PC0, LOW);                 // �Ѻ LED (Turn LED off)

    // IWDG �йѺ�����ѧ������� MCU (IWDG counts down and resets MCU)
    while (1) {                              // �ѧǹ������ (Wait for reset)
        // ����ա�� feed �ա � IWDG ������ MCU (No more feeding � IWDG will reset MCU)
    }
}
