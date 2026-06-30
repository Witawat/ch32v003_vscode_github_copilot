/**
 * ============================================================
 * ������ҧ��� 7: ������ҧ�����ҹ Countdown Timer (Countdown_Init, Countdown_Start, Countdown_Stop, Countdown_Reset, Countdown_IsFinished, Countdown_SetAlarmCallback, Countdown_GetRemainingSeconds)
 * ============================================================
 *
 * �ʴ������ҹ Countdown Timer ����Ѻ�Ѻ�����ѧ
 *
 * Ἱ�ѧǧ�� (Circuit Diagram):
 *
 *                        +3.3V
 *                         |
 *                        [ ] 220?
 *                         |
 *     PC0 (Output) ----+----->| LED (Red) - Alarm Indicator
 *                       |
 *                      GND
 *
 *     PC1 (Input) ----[ ]----> GND    (Start/Pause)
 *                    Button
 *
 *     (Optional) Buzzer on PWM pin (PA1 or PC3)
 *
 *     USART (TX=PD5, RX=PD6) �������� PC ��ҹ USB-Serial
 *
 *     (Pull-up ���㹢ͧ CH32V003 �١��ҹ��ҹ PIN_MODE_INPUT_PULLUP)
 *
 * ============================================================
 * ���Ѿ����Ҵ��ѧ (Expected Results):
 * - ������� countdown 10 �Թҷ�
 * - ������ PC1  ������Ѻ�����ѧ
 * - USART �ʴ����ҷ������ͷء 1 �Թҷ�
 * - ������������  Alarm callback �ӧҹ  LED ��о�Ժ����
 * - USART �ʴ� "Time's up!"
 * ============================================================
 * ����͹ (WARNINGS):
 * - Countdown �� TIM2 �� base timer ���� (�����ǡѺ Stopwatch)!
 * - ������ Countdown ��� Stopwatch ������ѹ (��駤���� TIM2)
 * - ��ͧ���¡ Countdown_Init() ��͹��ҹ�ѧ��ѹ���
 * - Callback (AlarmCallback) �ӧҹ� main loop context (����� ISR)
 * - Countdown_Stop() ��ش���Ǥ��� (pause) - ������������� Countdown_Start()
 * - Countdown_Reset() ���絡�Ѻ������������������ش��ùѺ
 * - ����ӷ���дѺ milliseconds (resolution 1ms �ҡ TIM2)
 * ============================================================
 * แผนผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit(115200)"]
 *     C --> D["pinMode(PC0, OUTPUT)"]
 *     D --> E["pinMode(PC1, INPUT_PULLUP)"]
 *     E --> F["Countdown_Init(0, 0, 10)"]
 *     F --> G["Countdown_SetAlarmCallback()"]
 *     G --> H["Print instructions"]
 *     H --> I["while(1)"]
 *     I --> J["อ่าน button"]
 *     J --> K{"falling edge?"}
 *     K -->|"Yes"| L["Delay_Ms(50) debounce"]
 *     L --> M{"still LOW?"}
 *     M -->|"Yes"| N{"Countdown_IsRunning?"}
 *     N -->|"Yes"| O["Countdown_Stop + Print Paused"]
 *     N -->|"No"| P{"Countdown_IsFinished?"}
 *     P -->|"Yes"| Q["Countdown_Reset + clear alarm"]
 *     Q --> R["Countdown_Start + Print Running"]
 *     P -->|"No"| R
 *     O --> I
 *     R --> S{"Countdown_IsRunning?"}
 *     S -->|"Yes"| T{"ELAPSED_TIME >= 1000?"}
 *     T -->|"Yes"| U["Print remaining seconds"]
 *     U --> V{"alarm_triggered?"}
 *     T -->|"No"| V
 *     V -->|"Yes"| W["Print Time's up"]
 *     W --> X["LED blink 10 times"]
 *     X --> I
 *     V -->|"No"| I
 *     K -->|"No"| S
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

// === ����� Global ===

static uint8_t led_alarm = PC0;          // pin PC0 ����Ѻ LED alarm indicator
static uint8_t btn_start_pause = PC1;    // pin PC1 ����Ѻ���� Start/Pause
static volatile uint8_t alarm_triggered = 0;  // ʶҹ� alarm (1=�����������)

/**
 * @brief Alarm Callback
 * @details �ѧ��ѹ���١���¡����� countdown �������
 *          ��駤�� flag ������� main loop �Ѵ��õ���
 */
void alarm_callback(void)
{
    // === Alarm �١���¡������������ ===

    alarm_triggered = 1;                 // ��� flag �� main loop ��� alarm �ӧҹ

    // �����˵�: callback ���ӧҹ� context ���� (����� ISR)
    // ����÷ӧҹ����Ƿ���ش
}

/**
 * @brief �ѧ��ѹ��ѡ
 * @return ����� return (loop ����շ������ش)
 */
int main(void)
{
    // === ��������к� ===

    SystemCoreClockUpdate();
    Timer_Init();
    // === ������� USART ===

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);  // ������� USART ��� 115200 baud

    // === ��駤�� GPIO ===

    pinMode(led_alarm, PIN_MODE_OUTPUT);       // ��� PC0 �� output ����Ѻ LED alarm
    pinMode(btn_start_pause, PIN_MODE_INPUT_PULLUP);  // ��� PC1 �� input pull-up ����Ѻ����

    // === ������� Countdown ===

    // ������� countdown 0 �������, 0 �ҷ�, 10 �Թҷ�
    Countdown_Init(0, 0, 10);            // ������� countdown ��� 10 �Թҷ�
    Countdown_SetAlarmCallback(alarm_callback);  // ��� callback ���ж١���¡������������

    // �����˵�: countdown �ѧ���������Ѻ ��ͧ���¡ Countdown_Start() ��͹

    // === �ʴ���ͤ���������� ===

    USART_Print("Countdown Example\r\n");           // �ʴ����͵�����ҧ
    USART_Print("===================\r\n");         // �����
    USART_Print("Countdown: 10 seconds\r\n");       // �����ҷ����
    USART_Print("Press PC1 to Start/Pause\r\n");    // ���Ը���ҹ
    USART_Print("Status: Stopped\r\n");             // ʶҹ��������

    // === ���������Ѻ��÷ӧҹ ===

    uint32_t last_report = 0;            // �����Ҥ����ش���·����§ҹ
    uint8_t button_prev = HIGH;          // ʶҹС�͹˹�Ңͧ���� (����Ѻ edge detection)

    // === Main Loop (�������ش) ===

    while (1)
    {
        // === ��ҹʶҹл��� (Edge Detection) ===

        uint8_t btn_current = digitalRead(btn_start_pause);  // ��ҹʶҹл����Ѩ�غѹ

        // === Button: Start/Pause (��Ǩ�Ѻ Falling Edge) ===

        if (button_prev == HIGH && btn_current == LOW)  // ��һ�����觶١�� (HIGHLOW)
        {
            Delay_Ms(50);                    // ˹�ǧ 50ms ���� debounce
            if (digitalRead(btn_start_pause) == LOW)  // �礫����ҡ���ԧ
            {
                if (Countdown_IsRunning())   // ��� countdown ���ѧ�ӧҹ
                {
                    Countdown_Stop();        // ��ش countdown ���Ǥ��� (pause)
                    USART_Print("Status: Paused\r\n");  // ��ʶҹ� pause
                }
                else                         // ��� countdown ��ش����
                {
                    // �������������� ��� reset ��͹���������
                    if (Countdown_IsFinished())  // ��������������
                    {
                        Countdown_Reset();   // ���絡�Ѻ�� 10 �Թҷ�
                        alarm_triggered = 0; // ��ҧʶҹ� alarm
                        digitalWrite(led_alarm, LOW);  // �Դ LED alarm
                        USART_Print("Countdown Reset to 10s\r\n");  // �� reset
                    }

                    Countdown_Start();       // ����� countdown
                    USART_Print("Status: Running\r\n");  // ��ʶҹ� running
                }
            }
        }

        // === �ѻവʶҹл�������Ѻ�ͺ�Ѵ� ===

        button_prev = btn_current;          // �ѻവʶҹл���

        // === ��§ҹ���ҷ������ͷء 1 �Թҷ� ===

        if (Countdown_IsRunning())           // ��� countdown ���ѧ�ӧҹ
        {
            if (ELAPSED_TIME(last_report, Get_CurrentMs()) >= 1000)  // �ء 1 �Թҷ�
            {
                last_report = Get_CurrentMs();  // �ѻവ����

                // �ʴ����ҷ����������Թҷ�
                uint32_t remaining = Countdown_GetRemainingSeconds();  // ��ҹ�Թҷշ�������
                USART_Print("Remaining: ");      // �ʴ���ͤ���
                USART_PrintNum(remaining);        // �ʴ���ҷ�������
                USART_Print(" seconds\r\n");      // ˹����Թҷ�
            }
        }

        // === ��Ǩ�ͺ Alarm ===

        if (alarm_triggered)                 // ��� alarm �١ trigger
        {
            alarm_triggered = 0;             // ��ҧ flag

            USART_Print("\r\n*** Time's up! ***\r\n");  // ���������
            USART_Print("Countdown finished!\r\n");     // ���������

            // LED ��о�Ժ���� 10 ����
            for (uint8_t i = 0; i < 10; i++)  // ��о�Ժ 10 �ͺ
            {
                digitalWrite(led_alarm, HIGH);  // �Դ LED
                Delay_Ms(100);                  // ˹�ǧ 100ms
                digitalWrite(led_alarm, LOW);   // �Դ LED
                Delay_Ms(100);                  // ˹�ǧ 100ms
            }

            USART_Print("Press PC1 to reset and start again\r\n");  // ���Ը����������
        }
    }

    // ��觹���������ѹ�֧
    // return 0;
}
