/**
 * ============================================================
 * ตัวอย่างที่ 2: SPI Buffer Transfer
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
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
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   Status Reg: 0x00   (if unprotected)
 *   JEDEC ID  : EF 40 15
 *   [32-byte hex dump of status + ID area]
 * ============================================================
 * คำเตือน (WARNINGS):
 * W25Qxx commands need specific byte sequences —
 *          always follow the datasheet command set.
 * ============================================================
 */

#include <SimpleHAL.h>

#define CS_PIN      PC4
#define CMD_RDSR    0x05
#define CMD_JEDEC   0x9F

int main(void)
{
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);

    pinMode(CS_PIN, PIN_MODE_OUTPUT);
    SPI_SetCSPin(GPIOC, GPIO_Pin_4);

    while(1)
    {
        uint8_t buf[32];
        uint8_t jedec[3];
        uint8_t status;
        uint8_t cmd;

        // --- Read Status Register ---
        SPI_SetCS(0);
        cmd = CMD_RDSR;
        SPI_Write(&cmd, 1);
        SPI_Read(&status, 1, 0xFF);
        SPI_SetCS(1);
        USART_Print("Status Reg: 0x");
        USART_PrintHex(status, 1);
        USART_Print("\r\n");

        // --- Read JEDEC ID ---
        SPI_SetCS(0);
        cmd = CMD_JEDEC;
        SPI_Write(&cmd, 1);
        SPI_TransferBuffer(NULL, jedec, 3);
        SPI_SetCS(1);
        USART_Print("JEDEC ID: ");
        USART_PrintHex(jedec[0], 1);
        USART_Print(" ");
        USART_PrintHex(jedec[1], 1);
        USART_Print(" ");
        USART_PrintHex(jedec[2], 1);
        USART_Print("\r\n");

        // --- Read 32 bytes from address 0x00 ---
        SPI_SetCS(0);
        cmd = 0x03;
        uint8_t addrBytes[3] = {0x00, 0x00, 0x00};
        SPI_Write(&cmd, 1);
        SPI_Write(addrBytes, 3);
        SPI_TransferBuffer(NULL, buf, 32);
        SPI_SetCS(1);

        USART_Print("Hex dump: ");
        for (uint8_t i = 0; i < 32; i++)
        {
            USART_PrintHex(buf[i], 1);
            USART_Print(" ");
        }
        USART_Print("\r\n");

        Delay_Ms(3000);
    }
    return 0;
}
