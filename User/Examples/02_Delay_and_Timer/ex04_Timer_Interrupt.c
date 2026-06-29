/**
 * ============================================================
 * ������ҧ��� 4: ������ҧ�����ҹ Timer Interrupt (TIM_SimpleInit, TIM_AttachInterrupt, TIM_Start, TIM_Stop)
 * ============================================================
 *
 * �ʴ������ Hardware Timer Interrupt ������Ѻʶҹ� LED �ء 500ms
 *
 * Ἱ�ѧǧ�� (Circuit Diagram):
 *
 *                        +3.3V
 *                         |
 *                        [ ] 220?
 *                         |
 *     PC0 (Output) ----+----->| LED (Blue)
 *                       |
 *                      GND
 *
 *     PC1 (Input) ----[ ]----> GND
 *                    Button
 *                    (Pull-up external)
 *
 * ============================================================
 * ���Ѿ����Ҵ��ѧ (Expected Results):
 * - Timer interrupt �ԧ�ء 500ms  ISR ��Ѻʶҹ� LED
 * - Main loop ����ͧ����������ǡѺ LED (ISR �Ѵ������)
 * - ������ PC1 ���������/��ش timer
 * - �ʴ���÷ӧҹẺ interrupt-driven
 * ============================================================
 * ����͹ (WARNINGS):
 * - TIM1 �Ѵ��駡Ѻ SimplePWM! ����� TIM2 ����Ѻ������ҧ���
 * - Callback �ӧҹ� ISR context - ��ͧ�ӧҹ��������Ƿ���ش!
 * - ������ Delay_Ms() � ISR (Delay �� SysTick interrupt)
 * - �������¡ USART_Print() � ISR (blocking �ҹ�Թ�)
 * - ����� volatile ����Ѻ����÷�����Ѻ ISR
 * - TIM2 �� shared resource - ���ѧ�����ҹ�����Ѻ module ���
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

// === ����� Global (���Ѻ ISR) ===

static volatile uint8_t led_state = 0;   // ʶҹ� LED (volatile ��������¹� ISR)
static uint8_t led_pin = PC0;            // pin PC0 ����Ѻ LED
static uint8_t button_pin = PC1;         // pin PC1 ����Ѻ������

/**
 * @brief Timer Interrupt Callback
 * @details �ѧ��ѹ���١���¡�ء���駷�� timer overflow (�ء 500ms)
 *          �ӧҹ� ISR context - ��ͧ��ЪѺ����Ǵ����!
 *
 * @warning ������ Delay, USART_Print, ���Ϳѧ��ѹ blocking �� 㹹��!
 */
void timer_isr_callback(void)
{
    // === ISR Context: �ӧҹ���Ƿ���ش! ===

    led_state = !led_state;              // ��Ѻʶҹ� LED (�Ǵ����)
    digitalWrite(led_pin, led_state);    // ��¹�����ѧ LED �µç (�� register-level)

    // �����˵�: � ISR ����÷������ҡ���ҹ��
    // ��ҵ�ͧ����觢����� �� flag ���ǨѴ���� main loop ᷹
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

    pinMode(led_pin, PIN_MODE_OUTPUT);         // ��� PC0 �� output ����Ѻ LED
    pinMode(button_pin, PIN_MODE_INPUT_PULLUP);  // ��� PC1 �� input pull-up ����Ѻ����

    // === ��駤�� Timer ===

    // �� TIM2 (���Ѵ��駡Ѻ SimplePWM ����� TIM1)
    TIM_SimpleInit(TIM_2, 2);                  // ��駤�� TIM2 ��� 2Hz (500ms ����ͺ)
    TIM_AttachInterrupt(TIM_2, timer_isr_callback);  // �١ callback �Ѻ timer interrupt
    TIM_Start(TIM_2);                          // �������÷ӧҹ�ͧ timer

    // === �ʴ���ͤ���������� ===

    USART_Print("Timer Interrupt Example\r\n");    // �ʴ����͵�����ҧ
    USART_Print("TIM2 configured at 2Hz (500ms)\r\n");  // �駤������ timer
    USART_Print("Press PC1 button to start/stop\r\n");  // ���Ը���ҹ

    // === ���������Ѻ main loop ===

    uint32_t last_print = 0;                 // �����Ҥ����ش���·������ʶҹ�

    // === Main Loop (�������ش) ===

    while (1)
    {
        // === ��ҹʶҹл����� ===

        // === Non-blocking button handling with state machine ===
        static uint8_t timer_running = 1;
        static uint8_t btn_state = 0;
        static Timer_t btn_timer;

        // State 0: waiting for press
        if (btn_state == 0 && digitalRead(button_pin) == LOW) {
            btn_state = 1;
            Start_Timer(&btn_timer, 50, 0);  // 50ms debounce
        }
        // State 1: debouncing
        if (btn_state == 1 && Is_Timer_Expired(&btn_timer)) {
            if (digitalRead(button_pin) == LOW) {
                btn_state = 2;
                timer_running = !timer_running;
                if (timer_running) {
                    TIM_Start(TIM_2);
                    USART_Print("Button pressed!\r\n");
                    USART_Print("Timer Started\r\n");
                } else {
                    TIM_Stop(TIM_2);
                    digitalWrite(led_pin, LOW);
                    USART_Print("Button pressed!\r\n");
                    USART_Print("Timer Stopped\r\n");
                }
            } else {
                btn_state = 0;
            }
        }
        // State 2: waiting for release
        if (btn_state == 2 && digitalRead(button_pin) == HIGH) {
            btn_state = 0;
        }

        // === Main Loop �ӧҹ���� ===

        // ������ҧ: �����ʶҹзء 2 �Թҷ�
        if (ELAPSED_TIME(last_print, Get_CurrentMs()) >= 2000)
        {
            last_print = Get_CurrentMs();      // �ѻവ����

            // �ʴ�ʶҹ� LED �Ѩ�غѹ (LED �١�Ǻ����� ISR)
            USART_Print("Main loop running, LED state: ");  // �ʴ���ͤ���
            if (led_state)                     // ��� LED �Դ
            {
                USART_Print("ON\r\n");         // �ʴ� ON
            }
            else
            {
                USART_Print("OFF\r\n");        // �ʴ� OFF
            }

            // �ʴ���ҵ�ǹѺ�Ѩ�غѹ
            uint16_t counter = Simple_TIM_GetCounter(TIM_2);  // ��ҹ counter
            USART_Print("TIM2 Counter: ");     // �ʴ���ͤ��� counter
            USART_PrintNum(counter);            // �ʴ���� counter
            USART_Print("\r\n");                // ��鹺�÷Ѵ����
        }

        // ISR �ШѴ��� LED ���ѵ��ѵ� - main loop ����ͧ���!
    }

    // ��觹���������ѹ�֧
    // return 0;
}
