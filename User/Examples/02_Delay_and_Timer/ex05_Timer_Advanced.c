/**
 * ============================================================
 * ������ҧ��� 5: ������ҧ�����ҹ Timer Ẻ Advanced (TIM_AdvancedInit, TIM_SetPrescaler, TIM_SetMode, TIM_GetPeriod, Simple_TIM_GetCounter, TIM_GenerateUpdate)
 * ============================================================
 *
 * �ʴ���û�Ѻ�� Timer Ẻ�����´���¡�õ�駤�� Prescaler ��� Period ���µ��ͧ
 *
 * Ἱ�ѧǧ�� (Circuit Diagram):
 *
 *                        +3.3V
 *                         |
 *                        [ ] 220?
 *                         |
 *     PC0 (Output) ----+----->| LED (Green)
 *                       |
 *                      GND
 *
 *     USART (TX=PD5, RX=PD6) �������� PC ��ҹ USB-Serial
 *
 * ============================================================
 * ���Ѿ����Ҵ��ѧ (Expected Results):
 * - ������� timer ��� 10kHz (PSC=2399, Period=9 @24MHz)
 * - ��ҹ counter � loop ��о��������� counter wrap (overflow)
 * - ����¹ prescaler ��ҧ����������ʴ��������¹�������
 * - �ʴ���Ҥ��������ӹǳ�ҡ�ٵ� SystemCoreClock/((PSC+1)*(Period+1))
 * ============================================================
 * ����͹ (WARNINGS):
 * - ������� = SystemCoreClock / ((prescaler+1) * (period+1))
 * - ��� SystemCoreClock=24MHz: PSC=2399, Period=9  1kHz (�١)
 * - ��� SystemCoreClock=24MHz: PSC=2399, Period=0  10kHz
 * - ��� SystemCoreClock=48MHz (��� HSE): ��� PSC/Period ��ҧ�ҡ 24MHz
 * - TIM_SetPrescaler() �ռ���ͺ�Ѵ���ҹ�� ��ͧ���¡ TIM_GenerateUpdate() ������ѹ��
 * - 16-bit timer: �٧�ش 65535 ����Ѻ PSC ��� Period
 * - ����������ش��� 24MHz: ~0.366Hz (PSC=65535, Period=65535)
 * - ��������٧�ش��� 24MHz: 24MHz (PSC=0, Period=0)
 * ============================================================
 * แผนผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit(115200)"]
 *     C --> D["pinMode(PC0, OUTPUT)"]
 *     D --> E["Print SystemCoreClock"]
 *     E --> F["TIM_AdvancedInit(TIM1, PSC=239, PER=9, UP)"]
 *     F --> G["TIM_Start(TIM1)"]
 *     G --> H["while(1)"]
 *     H --> I["LED blink state machine"]
 *     I --> J["Simple_TIM_GetCounter + TIM_GetPeriod"]
 *     J --> K{"counter < last_counter?"}
 *     K -->|"Yes"| L["Print wrapped + toggle LED"]
 *     L --> M["update last_counter"]
 *     K -->|"No"| M
 *     M --> N{"ELAPSED_TIME >= 2000?"}
 *     N -->|"Yes"| O["Print counter, period, PSC"]
 *     O --> P{"after 5s && !freq_changed?"}
 *     P -->|"Yes"| Q["TIM_SetPrescaler(119)"]
 *     Q --> R["TIM_SetMode(DOWN)"]
 *     R --> S["TIM_GenerateUpdate()"]
 *     S --> T["LED blink 5 times"]
 *     T --> H
 *     P -->|"No"| H
 *     N -->|"No"| H
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

// === ����� Global ===

static uint8_t led_pin = PC0;            // pin PC0 ����Ѻ LED
static uint16_t last_counter = 0;        // �纤�� counter ��͹˹�� (���͵�Ǩ�Ѻ wrap)

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

    pinMode(led_pin, PIN_MODE_OUTPUT);   // ��� PC0 �� output ����Ѻ LED

    // === �ʴ��������к� ===

    USART_Print("Timer Advanced Example\r\n");       // �ʴ����͵�����ҧ
    USART_Print("SystemCoreClock: ");                // �ʴ���ͤ��� clock
    USART_PrintNum(SystemCoreClock);                 // �ʴ�������� CPU
    USART_Print(" Hz\r\n");                          // ˹��� Hz

    // === ��駤�� Timer Ẻ Advanced ===

    // ��� SystemCoreClock=24MHz: ��ͧ��� 10kHz
    // �ٵ�: Frequency = SystemCoreClock / ((prescaler+1) * (period+1))
    // 10000 = 24000000 / ((PSC+1) * (PER+1))
    // (PSC+1) * (PER+1) = 2400
    // ���͡ PSC=239, PER=9  24000000 / (240 * 10) = 10000 Hz = 10kHz
    // (��Ѻ��� LED ��о�Ժ����� - ��繼ŪѴਹ)

    uint16_t prescaler = 239;            // ��� prescaler (PSC)
    uint16_t period = 9;                 // ��� period (ARR)
    TIM_Mode mode = TIM_MODE_UP;         // �����Ѻ���

    // �ʴ���ҷ����
    USART_Print("Initial: PSC=");        // �ʴ���ͤ��� PSC
    USART_PrintNum(prescaler);           // �ʴ���� prescaler
    USART_Print(", Period=");            // �ʴ���ͤ��� Period
    USART_PrintNum(period);              // �ʴ���� period
    USART_Print(", Mode=UP");            // �ʴ�����

    // �ӹǳ��������ԧ
    uint32_t calc_freq = SystemCoreClock / ((uint32_t)(prescaler + 1) * (period + 1));  // �ӹǳ�������
    USART_Print(" -> Frequency: ");      // �ʴ���ͤ��� frequency
    USART_PrintNum(calc_freq);           // �ʴ���Ҥ��������ӹǳ
    USART_Print(" Hz\r\n");              // ˹��� Hz

    // ������� Timer
    TIM_AdvancedInit(TIM_1, prescaler, period, mode);  // ��駤�� timer ���� PSC, Period, Mode
    TIM_Start(TIM_1);                    // �������÷ӧҹ�ͧ timer

    // === Main Loop (�������ش) ===

    uint32_t last_report = 0;            // �����Ҥ����ش���·����§ҹ
    uint8_t freq_changed = 0;            // ʶҹ�����¹����������������ѧ

    while (1)
    {
        // === LED Blink State Machine (non-blocking) ===
        static uint8_t blink_active = 0;
        static uint8_t blink_step = 0;
        static Timer_t blink_timer;

        if (blink_active) {
            if (Is_Timer_Expired(&blink_timer)) {
                if (blink_step < 10) {
                    digitalWrite(led_pin, (blink_step & 1) ? LOW : HIGH);
                    Start_Timer(&blink_timer, 50, 0);
                    blink_step++;
                } else {
                    blink_active = 0;
                    blink_step = 0;
                }
            }
        }
        // === ��ҹ��� Counter �Ѩ�غѹ ===

        uint16_t counter = Simple_TIM_GetCounter(TIM_1);  // ��ҹ��� counter �Ѩ�غѹ�ͧ TIM1
        uint16_t current_period = TIM_GetPeriod(TIM_1);   // ��ҹ��� period �Ѩ�غѹ

        // === ��Ǩ�Ѻ Counter Wrap ===

        // ��� counter ���¡��� last_counter �ʴ�����Դ overflow (wrap around)
        if (counter < last_counter)      // ��Ǩ�Ѻ��� wrap �ͧ counter
        {
            USART_Print("Counter wrapped! ");  // �� wrap
            USART_Print("Period=");             // �ʴ���ͤ��� period
            USART_PrintNum(current_period);     // �ʴ���� period
            USART_Print(", Counter=");          // �ʴ���ͤ��� counter
            USART_PrintNum(counter);            // �ʴ���� counter
            USART_Print("\r\n");                // ��鹺�÷Ѵ����

            digitalWrite(led_pin, !digitalRead(led_pin));  // ��о�Ժ LED �ء���駷�� wrap
        }

        last_counter = counter;          // �ѻവ��� counter ����Ѻ�ͺ�Ѵ�

        // === ��§ҹ��ҷء 2 �Թҷ� ===

        if (ELAPSED_TIME(last_report, Get_CurrentMs()) >= 2000)
        {
            last_report = Get_CurrentMs();  // �ѻവ����

            // �ʴ���� Counter ��� Period �Ѩ�غѹ
            USART_Print("Counter=");         // �ʴ���ͤ��� counter
            USART_PrintNum(counter);         // �ʴ���� counter
            USART_Print(", Period=");        // �ʴ���ͤ��� period
            USART_PrintNum(current_period);  // �ʴ���� period
            USART_Print(", PSC=");           // �ʴ���ͤ��� prescaler
            USART_PrintNum(Simple_TIM_GetPrescaler(TIM_1));  // �ʴ���� prescaler
            USART_Print("\r\n");             // ��鹺�÷Ѵ����

            // === ����¹ Prescaler ��ҧ����� (��ѧ�ҡ 5 �Թҷ��á) ===

            if (!freq_changed && (Get_CurrentMs() > 5000))  // ����ѧ���������¹������Ҽ�ҹ�Թ 5s
            {
                freq_changed = 1;            // ��駤��ʶҹ��������¹����

                // ����¹ Prescaler �ҡ 239  119 (����������� 2 ���)
                // ����: PSC=119, Period=9  24000000/(120*10) = 20000 Hz = 20kHz
                uint16_t new_psc = 119;      // ��� prescaler ����

                USART_Print("\r\n*** Changing Prescaler from ");  // ������¹
                USART_PrintNum(prescaler);    // �ʴ�������
                USART_Print(" to ");           // ����� "to"
                USART_PrintNum(new_psc);       // �ʴ��������
                USART_Print(" ***\r\n");       // �Դ��ͤ���

                TIM_SetPrescaler(TIM_1, new_psc);  // ��駤�� prescaler ����
                TIM_SetMode(TIM_1, TIM_MODE_DOWN);  // ����¹�����繹Ѻŧ (�����ʴ� TIM_SetMode)
                TIM_GenerateUpdate(TIM_1);    // ���ҧ update event ��������������ռŷѹ��

                // �ӹǳ�����������
                uint32_t new_freq = SystemCoreClock / ((uint32_t)(new_psc + 1) * (period + 1));  // �ӹǳ����
                USART_Print("New Frequency: ");  // �ʴ���ͤ���
                USART_PrintNum(new_freq);     // �ʴ���Ҥ����������
                USART_Print(" Hz, Mode=DOWN\r\n\r\n");  // ˹��� Hz �������

                // ��о�Ժ LED ����� ���ͺ͡�������¹ mode
                for (uint8_t i = 0; i < 5; i++)  // ��о�Ժ 5 ����
                {
                    blink_active = 1;
                blink_step = 0;
                Start_Timer(&blink_timer, 50, 0);                 // ˹�ǧ 50ms
                }
            }
                }
            }

    // ��觹���������ѹ�֧
    // return 0;
}
