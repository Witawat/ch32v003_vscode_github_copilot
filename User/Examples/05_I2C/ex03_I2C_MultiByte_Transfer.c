/**
 * ============================================================
 * ตัวอย่างที่ 3: I2C Multi-Byte Transfer
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003        3.3V          BH1750 Sensor
 *     --------         |            -------------
 *     PC2 --[SCL]--/\/\/\---+-- SCL
 *                  4.7kΩ    |
 *     PC1 --[SDA]--/\/\/\---+-- SDA
 *                  4.7kΩ    |
 *                           +-- ADDR=GND
 *                           +-- VCC=3.3V
 *     GND                   +-- GND
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   Light: 450 lux
 *   Light: 448 lux   (updated every 500 ms)
 * ============================================================
 * คำเตือน (WARNINGS):
 * BH1750 needs time after power-on (~10 ms) before
 *          first command. Resolution set via command byte.
 * ============================================================
 */

#include <SimpleHAL.h>

#define BH1750_ADDR  0x23
#define CMD_PWR_ON   0x01
#define CMD_HRES     0x10

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);

    uint8_t cmd = CMD_PWR_ON;
    I2C_Write(BH1750_ADDR, &cmd, 1);
    Delay_Ms(10);
    cmd = CMD_HRES;
    I2C_Write(BH1750_ADDR, &cmd, 1);
    Delay_Ms(180);

    while(1)
    {
        uint8_t raw[2] = {0, 0};
        uint16_t lux = 0;

        I2C_Read(BH1750_ADDR, raw, 2);
        lux = ((uint16_t)raw[0] << 8) | raw[1];
        lux = lux / 1.2;

        USART_Print("Light: ");
        USART_PrintNum(lux);
        USART_Print(" lux\r\n");
        Delay_Ms(500);
    }
    return 0;
}
