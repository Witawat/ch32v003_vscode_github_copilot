/**
 * ============================================================
 * ������ҧ��� 6: ������ҧ�����ҹ Stopwatch (Stopwatch_Init, Stopwatch_Start, Stopwatch_Stop, Stopwatch_Reset, Stopwatch_GetTimeString, Stopwatch_GetTotalSeconds)
 * ============================================================
 *
 * �ʴ������ҹ Stopwatch ����Ѻ�Ѻ����Ẻ�Ѻ���
 *
 * Ἱ�ѧǧ�� (Circuit Diagram):
 *
 *     Button1 (PC0) ----[ ]----> GND    (Start/Stop)
 *                      Start/Stop
 *
 *     Button2 (PC1) ----[ ]----> GND    (Reset)
 *                      Reset
 *
 *     USART (TX=PD5, RX=PD6) �������� PC ��ҹ USB-Serial
 *
 *     (Pull-up ���㹢ͧ CH32V003 �١��ҹ��ҹ PIN_MODE_INPUT_PULLUP)
 *
 * ============================================================
 * ���Ѿ����Ҵ��ѧ (Expected Results):
 * - �� Button1 (PC0): ������Ѻ���� / ��ش�Ѻ����
 * - �� Button2 (PC1): ���� stopwatch ��Ѻ�� 00:00:00
 * - USART �ʴ�������ٻẺ "HH:MM:SS" ���ͨӹǹ�Թҷշ�����
 * - ��§ҹ���ҷء 100ms ���ͤ��������´�٧
 * ============================================================
 * ����͹ (WARNINGS):
 * - Stopwatch �� TIM2 �� base timer ���� - ������ TIM2 �����ѵ�ػ��ʧ�����
 * - ����� Stopwatch �����Ѻ Countdown (ex07) ������ѹ����� (��駤���� TIM2)
 * - Stopwatch �� TIM2 ��� 1000Hz  resolution 1ms
 * - ��ͧ���¡ Stopwatch_Init() ��͹��ҹ�ѧ��ѹ���
 * - Stopwatch_Reset() ��ش��ùѺ��������� 0
 * - �����٧�ش ~49 �ѹ (overflow �ͧ uint32_t milliseconds)
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

// === ����� Global ===

static uint8_t btn_start_stop = PC0;   // pin PC0 ����Ѻ���� Start/Stop
static uint8_t btn_reset = PC1;        // pin PC1 ����Ѻ���� Reset

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

    // === ��駤�� GPIO ����Ѻ������ ===

    pinMode(btn_start_stop, PIN_MODE_INPUT_PULLUP);  // ��� PC0 �� input pull-up (Button1)
    pinMode(btn_reset, PIN_MODE_INPUT_PULLUP);       // ��� PC1 �� input pull-up (Button2)

    // === ������� Stopwatch ===

    Stopwatch_Init();                   // ��������к� Stopwatch (�� TIM2 ��� 1000Hz)
    // Stopwatch �ѧ���������Ѻ ��ͧ���¡ Stopwatch_Start() ��͹

    // === �ʴ���ͤ���������� ===

    USART_Print("Stopwatch Example\r\n");           // �ʴ����͵�����ҧ
    USART_Print("===================\r\n");         // �����
    USART_Print("PC0: Start/Stop\r\n");            // �駡�÷ӧҹ�ͧ����
    USART_Print("PC1: Reset\r\n");                 // �駡�÷ӧҹ�ͧ����
    USART_Print("Status: Stopped\r\n");            // ʶҹ��������
    USART_Print("Time: 00:00:00\r\n");             // �����������

    // === ���������Ѻ��÷ӧҹ ===

    uint32_t last_display = 0;          // �����Ҥ����ش���·���ѻവ˹�Ҩ�
    uint8_t button1_prev = HIGH;        // ʶҹС�͹˹�Ңͧ Button1 (����Ѻ edge detection)
    uint8_t button2_prev = HIGH;        // ʶҹС�͹˹�Ңͧ Button2 (����Ѻ edge detection)

    // === Main Loop (�������ش) ===

    while (1)
    {
        // === ��ҹʶҹл��� (Edge Detection) ===

        uint8_t btn1_current = digitalRead(btn_start_stop);  // ��ҹʶҹ� Button1 �Ѩ�غѹ
        uint8_t btn2_current = digitalRead(btn_reset);       // ��ҹʶҹ� Button2 �Ѩ�غѹ

        // === Button1: Start/Stop (��Ǩ�Ѻ Falling Edge) ===

                // === Button1: Start/Stop (non-blocking debounce) ===
        static uint8_t btn1_state = 0;
        static Timer_t btn1_timer;

        if (btn1_state == 0 && button1_prev == HIGH && btn1_current == LOW) {
            Start_Timer(&btn1_timer, 50, 0);
            btn1_state = 1;
        }
        if (btn1_state == 1 && Is_Timer_Expired(&btn1_timer)) {
            if (digitalRead(btn_start_stop) == LOW) {
                btn1_state = 2;
                if (Stopwatch_IsRunning()) {
                    Stopwatch_Stop();
                    USART_Print("Status: Paused\r\n");
                } else {
                    Stopwatch_Start();
                    USART_Print("Status: Running\r\n");
                }
            } else {
                btn1_state = 0;
            }
        }
        if (btn1_state == 2 && digitalRead(btn_start_stop) == HIGH) {
            btn1_state = 0;
        }
        // �ѻവʶҹ� Button1
        button2_prev = btn2_current;        // �ѻവʶҹ� Button2

        // === �ʴ����ҷء 100ms ===

        if (ELAPSED_TIME(last_display, Get_CurrentMs()) >= 100)  // �ء 100ms
        {
            last_display = Get_CurrentMs();  // �ѻവ����

            // �ʴ�������ٻẺ HH:MM:SS
            char time_buf[32];               // buffer ����Ѻ�� string ����
            Stopwatch_GetTimeString(time_buf, TIME_FORMAT_HHMMSS, TIME_DISPLAY_NORMALIZED);  // ��ҹ������ string

            USART_Print("Time: ");           // �ʴ���ͤ��� Time
            USART_Print(time_buf);           // �ʴ��������

            // �ʴ����ҷ������˹����Թҷ�
            uint32_t total_sec = Stopwatch_GetTotalSeconds();  // ��ҹ���ҷ��������Թҷ�
            USART_Print(" (");               // �Դǧ���
            USART_PrintNum(total_sec);        // �ʴ��ӹǹ�Թҷ�
            USART_Print(" sec)\r\n");         // �Դǧ�����Т�鹺�÷Ѵ����
        }
    }

    // ��觹���������ѹ�֧
    // return 0;
}
