/**
 * ============================================================
 * ตัวอย่างที่ 3: SPI Configuration
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
 *   JEDEC @125kHz: EF 40 15
 *   JEDEC @8MHz  : EF 40 15
 *   MSB first: EF  (correct)
 *   LSB first: F7  (bit-reversed)
 * ============================================================
 * คำเตือน (WARNINGS):
 * Some devices require specific SPI mode. W25Qxx
 *          works with MODE0 or MODE3.
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit(115200)"]
 *     C --> D["pinMode(CS, OUTPUT)"]
 *     D --> E["SPI_SetCSPin(PC4)"]
 *     E --> F["while(1)"]
 *     F --> G["SPI_SimpleInit(125kHz)"]
 *     G --> H["readJedec()"]
 *     H --> I["Print JEDEC @125kHz"]
 *     I --> J["SPI_SetSpeed(8MHz)"]
 *     J --> K["readJedec()"]
 *     K --> L["Print JEDEC @8MHz"]
 *     L --> M["SPI_SetBitOrder(MSB_FIRST)"]
 *     M --> N["Read and print MSB"]
 *     N --> O["SPI_SetBitOrder(LSB_FIRST)"]
 *     O --> P["Read and print LSB"]
 *     P --> Q["Delay_Ms(5000)"]
 *     Q --> F
 * ============================================================
 */

#if CH32V003_IS_SOP8
#error "Hardware SPI not available on SOP-8. Use shiftOut/shiftIn instead."
#endif
#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

#define CS_PIN      PC4
#define CMD_JEDEC   0x9F

void readJedec(void);

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    pinMode(CS_PIN, PIN_MODE_OUTPUT);
    SPI_SetCSPin(GPIOC, GPIO_Pin_4);

    while(1)
    {
        // --- Test at 125 kHz ---
        SPI_SimpleInit(SPI_MODE0, SPI_125KHZ, SPI_PINS_DEFAULT);
        USART_Print("JEDEC @125kHz: ");
        readJedec();

        // --- Test at 8 MHz ---
        SPI_SetSpeed(SPI_8MHZ);
        USART_Print("JEDEC @8MHz  : ");
        readJedec();

        // --- Test MSB first ---
        uint8_t cmd;
        uint8_t msbVal, lsbVal;
        SPI_SetBitOrder(SPI_MSB_FIRST);
        SPI_SetCS(0);
        cmd = CMD_JEDEC;
        SPI_Write(&cmd, 1);
        SPI_Read(&msbVal, 1, 0xFF);
        SPI_SetCS(1);
        USART_Print("MSB first: ");
        USART_PrintHex(msbVal, 1);
        USART_Print("\r\n");

        // --- Test LSB first ---
        SPI_SetBitOrder(SPI_LSB_FIRST);
        SPI_SetCS(0);
        cmd = CMD_JEDEC;
        SPI_Write(&cmd, 1);
        SPI_Read(&lsbVal, 1, 0xFF);
        SPI_SetCS(1);
        USART_Print("LSB first: ");
        USART_PrintHex(lsbVal, 1);
        USART_Print("\r\n");

        Delay_Ms(5000);
    }
    return 0;
}

void readJedec()
{
    uint8_t cmd = CMD_JEDEC;
    uint8_t b0, b1, b2;
    SPI_SetCS(0);
    SPI_Write(&cmd, 1);
    SPI_Read(&b0, 1, 0xFF);
    SPI_Read(&b1, 1, 0xFF);
    SPI_Read(&b2, 1, 0xFF);
    SPI_SetCS(1);
    USART_PrintHex(b0, 1);
    USART_Print(" ");
    USART_PrintHex(b1, 1);
    USART_Print(" ");
    USART_PrintHex(b2, 1);
    USART_Print("\r\n");
}
