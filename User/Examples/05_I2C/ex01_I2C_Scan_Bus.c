/**
 * ============================================================
 * ตัวอย่างที่ 1: I2C Bus Scanner
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003        3.3V          I2C Device
 *     --------         |            ----------
 *     PC2 --[SCL]--/\/\/\---+-- SCL
 *                  4.7kΩ    |
 *     PC1 --[SDA]--/\/\/\---+-- SDA
 *                  4.7kΩ    |
 *     GND                   +-- GND
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   Scanning I2C bus...
 *   Found: 0x3C
 *   Found: 0x68
 *   2 device(s) found
 * ============================================================
 * คำเตือน (WARNINGS):
 * MUST have 4.7kΩ pull-up resistors or I2C won't work!
 * Address is 7-bit (0x03-0x77 range typically)
 * ============================================================
 */

#include <SimpleHAL.h>

int main(void)
{
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);

    while(1)
    {
        uint8_t count = 0;
        USART_Print("Scanning I2C bus...\r\n");

        for (uint8_t addr = 1; addr < 128; addr++)
        {
            if (I2C_IsDeviceReady(addr))
            {
                USART_Print("Found: 0x");
                USART_PrintHex(addr, 1);
                USART_Print("\r\n");
                count++;
            }
        }

        USART_PrintNum(count);
        USART_Print(" device(s) found\r\n");
        Delay_Ms(2000);
    }
    return 0;
}
