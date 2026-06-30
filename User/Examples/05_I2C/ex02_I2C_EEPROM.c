/**
 * ============================================================
 * ตัวอย่างที่ 2: I2C EEPROM Read/Write
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003        3.3V          AT24Cxx EEPROM
 *     --------         |            --------------
 *     PC2 --[SCL]--/\/\/\---+-- SCL
 *                  4.7kΩ    |
 *     PC1 --[SDA]--/\/\/\---+-- SDA
 *                  4.7kΩ    |
 *                           +-- WP=GND
 *                           +-- A0/A1/A2=GND
 *                           +-- VCC=3.3V
 *     GND                   +-- GND
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   Written "Hello" to EEPROM
 *   Read back: Hello
 * ============================================================
 * คำเตือน (WARNINGS):
 * EEPROM has write cycle time (~5 ms). Delay after
 *          every write operation!
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit(115200)"]
 *     C --> D["I2C_SimpleInit(100kHz)"]
 *     D --> E["while(1)"]
 *     E --> F["I2C_WriteRegMulti(0x50, 0x00, Hello)"]
 *     F --> G["Delay_Ms(10)"]
 *     G --> H["Print Written"]
 *     H --> I["I2C_ReadRegMulti(0x50, 0x00, readBuf)"]
 *     I --> J["Print Read back"]
 *     J --> K["Delay_Ms(3000)"]
 *     K --> E
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

#define EEPROM_ADDR 0x50
#define REG_START   0x00

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_PARTIAL_REMAP);  // PD2=SCL, PD1=SDA � SOP-8 compatible

    while(1)
    {
        uint8_t writeData[] = "Hello";
        uint8_t readBuf[16] = {0};

        I2C_WriteRegMulti(EEPROM_ADDR, REG_START, writeData, sizeof(writeData) - 1);
        Delay_Ms(10);

        USART_Print("Written \"Hello\" to EEPROM\r\n");

        I2C_ReadRegMulti(EEPROM_ADDR, REG_START, readBuf, sizeof(writeData) - 1);
        USART_Print("Read back: ");
        USART_Print((char*)readBuf);
        USART_Print("\r\n");

        Delay_Ms(3000);
    }
    return 0;
}
