/**
 * ============================================================
 * ตัวอย่างที่ 1: SPI Basic Transfer
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     Option A: Loopback Test
 *     PC6(MOSI) ----+---- PC7(MISO)
 *
 *     Option B: W25Qxx Flash
 *     CH32V003          W25Qxx Flash
 *     --------          -------------
 *     PC4 --[CS]-------- CS
 *     PC5 --[SCK]------- SCK
 *     PC6 --[MOSI]------ MOSI
 *     PC7 --[MISO]------ MISO
 *                       /WP=3.3V, /HOLD=3.3V
 *     3.3V              VCC=3.3V
 *     GND               GND
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results) (loopback):
 *   Loopback: sent 0xAA, received 0xAA
 * ผลลัพธ์ที่คาดหวัง (Expected Results) (W25Qxx):
 *   JEDEC ID: EF 40 15
 * ============================================================
 * คำเตือน (WARNINGS):
 * SPI_SimpleInit does NOT configure CS pin — call
 *          pinMode(CS, PIN_MODE_OUTPUT) and SPI_SetCSPin() yourself.
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

#define CS_PIN PC4

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);

    pinMode(CS_PIN, PIN_MODE_OUTPUT);
    SPI_SetCSPin(GPIOC, GPIO_Pin_4);

    while(1)
    {
        SPI_SetCS(0);
        uint8_t rx = SPI_Transfer(0xAA);
        SPI_SetCS(1);

        USART_Print("Loopback: sent 0xAA, received 0x");
        USART_PrintHex(rx, 1);
        USART_Print("\r\n");

        Delay_Ms(2000);
    }
    return 0;
}
