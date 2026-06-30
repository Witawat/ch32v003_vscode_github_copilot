/**
 * ============================================================
 * Software I2C Scanner
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003        3.3V          I2C Device
 *     --------         |            ----------
 *     PD2 --[SCL]--/\/\/\---+-- SCL
 *                  4.7kΩ    |
 *     PD3 --[SDA]--/\/\/\---+-- SDA
 *                  4.7kΩ    |
 *     GND                   +-- GND
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   Soft I2C scanning...
 *   Found: 0x23
 *   Found: 0x68
 *   2 device(s) found
 *
 * ============================================================
 * คำเตือน (WARNINGS):
 * - Software I2C ช้ากว่า Hardware I2C และใช้ CPU มากกว่า
 * - ต้องมี Pull-up resistor (4.7kΩ) บนทั้ง SCL และ SDA
 * - ความเร็วสูงสุดประมาณ 100kHz ขึ้นอยู่กับความถี่ CPU
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>
#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include "SimpleI2C_Soft.h"

#define SOFT_SCL_PIN PD2
#define SOFT_SDA_PIN PD3

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    I2C_Soft_Init(SOFT_SCL_PIN, SOFT_SDA_PIN, I2C_SOFT_100KHZ);

    while (1)
    {
        uint8_t count = 0;
        USART_Print("Soft I2C scanning...\r\n");

        for (uint8_t addr = 1; addr < 128; addr++)
        {
            if (I2C_Soft_Write(addr, NULL, 0, 1) == I2C_SOFT_OK)
            {
                USART_Print("Found: 0x");
                USART_PrintHex(addr, 1);
                USART_Print("\r\n");
                count++;
            }
        }

        USART_PrintNum(count);
        USART_Print(" device(s) found\r\n");
        Delay_Ms(3000);
    }
}
