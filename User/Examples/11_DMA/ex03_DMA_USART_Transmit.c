/**
 * ============================================================
 * ������ҧ��� 3: �觢����ż�ҹ USART ���� DMA (DMA USART Transmit)
 * ============================================================
 *
 * Ἱ�ѧǧ�� (Circuit Diagram):
 *
 *     CH32V003              USB-Serial          LED
 *     --------              ----------          ---
 *     PD5 (TX) ----------- RX
 *     PD6 (RX) ----------- TX
 *     GND    -------------- GND
 *     PC0 ----/\/\/\---->|---- GND
 *            220 Ohm
 * 
 *     LED ��� PC0 �ʴ���� main loop �ѧ�ӧҹ�����ҧ DMA �����͹������
 *
 * ============================================================
 * ���Ѿ����Ҵ��ѧ (Expected Results):
 * - "Hello from DMA!" ��ҡ��� Serial Monitor
 * - LED ��� PC0 ��о�Ժ��� DMA ���ѧ�����͹������ (non-blocking)
 * - main loop �ӧҹ������ͧ����ͧ�� DMA
 * ============================================================
 * ����͹ (WARNINGS):
 * - DMA_USART_Send() ��Ẻ blocking (�ͨ�����)
 * - DMA_USART_Transmit() ��Ẻ non-blocking
 * - ��ͧ���¡ USART_SimpleInit() ��͹�� DMA_USART functions
 * - DMA_USART_InitTx() ��ͧ���¡��͹ DMA_USART_Transmit() ����
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit()"]
 *     C --> D["pinMode(PC0, OUTPUT)"]
 *     D --> E["DMA_USART_InitTx(DMA_CH2, tx_buffer, 64)"]
 *     E --> F["DMA_USART_Transmit(DMA_CH2, message, len)"]
 *     F --> G{"DMA complete within 5s?"}
 *     G -->|"No"| H["USART_Print(DMA timeout)"]
 *     G -->|"Yes"| I["DMA_USART_Send(DMA_CH2, blocking, 22)"]
 *     H --> J["USART_Print(example complete)"]
 *     I --> J
 *     J --> K["while(1) toggle LED every 500ms"]
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>    // ����ź���� SimpleHAL ������
#include <string.h>       // ����ź���� string.h ����Ѻ strlen

int main(void)            // �ѧ��ѹ��ѡ �ش������������
{
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT); // ������� USART ��� 115200 baud
    pinMode(PC0, PIN_MODE_OUTPUT); // ��駤�� PC0 �� output ����Ѻ LED �ʴ�ʶҹ�

    uint8_t tx_buffer[64]; // Buffer ����Ѻ DMA TX (��Ҵ�������)
    DMA_USART_InitTx(DMA_CH2, tx_buffer, sizeof(tx_buffer)); // ������� DMA USART TX �� DMA_CH2

    const char* message = "Hello from DMA!\r\n"; // ��ͤ�������ͧ����觼�ҹ DMA
    uint16_t msg_len = strlen(message); // �ӹǳ������Ǣ�ͤ���

    DMA_USART_Transmit(DMA_CH2, (const uint8_t*)message, msg_len); // �觢�ͤ���Ẻ non-blocking

    uint32_t blink_count = 0; // ��ǹѺ�ͺ��á�о�Ժ LED

    uint32_t timeout = 5000;  // 5s timeout (�ѹ DMA �Դ��ҧ)
    while (DMA_GetStatus(DMA_CH2) != DMA_STATUS_COMPLETE && timeout > 0)
    {
        blink_count++;       // �����ӹǹ�ͺ
        digitalWrite(PC0, blink_count % 2); // ��о�Ժ LED ���¡����� modulo 2
        Delay_Ms(50);        // ˹�ǧ 50ms �����繡�á�о�Ժ�Ѵਹ
        timeout -= 50;
    }
    if (timeout == 0) {
        USART_Print("DMA timeout!\r\n");
    }

    Delay_Ms(500);           // ˹�ǧ 500ms ��͹���ͺẺ blocking

    DMA_USART_Send(DMA_CH2, (const uint8_t*)"Blocking send done!\r\n", 22); // ��Ẻ blocking �ͨ�����

    USART_Print("DMA USART example complete\r\n"); // ������ͤ�������ش���� USART ����

    while (1)                // ǹ�ٻ͹ѹ��
    {
        digitalWrite(PC0, !digitalRead(PC0)); // ��Ѻʶҹ� LED (toggle)
        Delay_Ms(500);       // ˹�ǧ 500ms ����ͺ
    }                        // ����ش while loop
}                            // ����ش�ѧ��ѹ main
