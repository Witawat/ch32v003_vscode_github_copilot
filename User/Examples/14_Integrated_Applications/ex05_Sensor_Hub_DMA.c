/**
 * ============================================================
 * ตัวอย่างที่ 5: Sensor Hub with DMA
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *   BMP280 (I2C):
 *     VCC -> 3.3V, GND -> GND
 *     SCL -> PC2 + 4.7kΩ pull-up -> 3.3V
 *     SDA -> PC1 + 4.7kΩ pull-up -> 3.3V
 *     CSB -> 3.3V (I2C mode), SDD -> GND (addr 0x76)
 *   DS18B20 (1-Wire):
 *     VDD -> 3.3V, GND -> GND, DQ -> PD2 + 4.7kΩ pull-up -> 3.3V
 *   W25Qxx (SPI Flash):
 *     VCC -> 3.3V, GND -> GND
 *     CS  -> PC4 (Chip Select)
 *     SCK -> PC5 (Clock)
 *     MOSI -> PC6 (Master Out Slave In)
 *     MISO -> PC7 (Master In Slave Out)
 *   USB-Serial: TX=PD5, RX=PD6
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   Serial output:
 *   "=== Sensor Hub ==="
 *   "I2C(BMP280): 28.5C / 1013.2hPa"
 *   "1-Wire(DS18B20): 28.3C"
 *   "SPI(W25Qxx): W25Q32"
 *   (ส่งผ่าน DMA USART ทั้งหมด)
 * ============================================================
 * คำเตือน (WARNINGS):
 *   - ใช้หลายโปรโตคอลพร้อมกัน กินกระแสสูง
 *   - DMA channels: USART=CH2, SPI=CH4/CH5 - ตรวจสอบไม่ให้ชนกัน
 *   - ต้องแน่ใจว่า DMA ไม่ค้างจากการส่งก่อนเริ่มครั้งใหม่
 * ============================================================
 */

#include <SimpleHAL.h>
#include <string.h>

#define BMP280_ADDR  0x76
#define DS18B20_PIN  PD2
#define W25Q_CS_PIN  PC4
#define W25Q_SPI     SPI1

#define DMA_USART_CH DMA_CH2
#define DMA_SPI_TX_CH DMA_CH4
#define DMA_SPI_RX_CH DMA_CH5

uint8_t dmaBusy = 0;

OneWire_Bus* dsBus;

int16_t dig_T1, dig_T2, dig_T3;
uint16_t dig_P1;
int16_t dig_P2, dig_P3, dig_P4, dig_P5;
int16_t dig_P6, dig_P7, dig_P8, dig_P9;

void BMP280_ReadCal(void) {
    uint8_t buf[24];
    I2C_ReadRegMulti(BMP280_ADDR, 0x88, buf, 24);

    dig_T1 = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
    dig_T2 = (int16_t)((uint16_t)buf[2] | ((uint16_t)buf[3] << 8));
    dig_T3 = (int16_t)((uint16_t)buf[4] | ((uint16_t)buf[5] << 8));
    dig_P1 = (uint16_t)buf[6] | ((uint16_t)buf[7] << 8);
    dig_P2 = (int16_t)((uint16_t)buf[8] | ((uint16_t)buf[9] << 8));
    dig_P3 = (int16_t)((uint16_t)buf[10] | ((uint16_t)buf[11] << 8));
    dig_P4 = (int16_t)((uint16_t)buf[12] | ((uint16_t)buf[13] << 8));
    dig_P5 = (int16_t)((uint16_t)buf[14] | ((uint16_t)buf[15] << 8));
    dig_P6 = (int16_t)((uint16_t)buf[16] | ((uint16_t)buf[17] << 8));
    dig_P7 = (int16_t)((uint16_t)buf[18] | ((uint16_t)buf[19] << 8));
    dig_P8 = (int16_t)((uint16_t)buf[20] | ((uint16_t)buf[21] << 8));
    dig_P9 = (int16_t)((uint16_t)buf[22] | ((uint16_t)buf[23] << 8));
}

int32_t BMP280_GetTemp(void) {
    uint8_t buf[3];
    I2C_ReadRegMulti(BMP280_ADDR, 0xFA, buf, 3);

    int32_t adc_T = ((int32_t)buf[0] << 12) | ((int32_t)buf[1] << 4) | ((int32_t)buf[2] >> 4);
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    int32_t t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;
}

uint32_t BMP280_GetPressure(int32_t t_fine) {
    uint8_t buf[3];
    I2C_ReadRegMulti(BMP280_ADDR, 0xF7, buf, 3);

    int32_t adc_P = ((int32_t)buf[0] << 12) | ((int32_t)buf[1] << 4) | ((int32_t)buf[2] >> 4);

    int64_t var1 = ((int64_t)t_fine) - 128000;
    int64_t var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0) return 0;
    int64_t p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (uint32_t)(p / 256);
}

float DS18B20_ReadTemp(void) {
    OneWire_Reset(dsBus);
    OneWire_SkipROM(dsBus);
    OneWire_WriteByte(dsBus, 0x44);

    OneWire_Reset(dsBus);
    OneWire_SkipROM(dsBus);
    OneWire_WriteByte(dsBus, 0xBE);

    uint8_t tempL = OneWire_ReadByte(dsBus);
    uint8_t tempH = OneWire_ReadByte(dsBus);

    int16_t raw = (int16_t)(((uint16_t)tempH << 8) | tempL);
    return raw * 0.0625f;
}

uint32_t W25Q_ReadID(void) {
    uint8_t txBuf[4], rxBuf[4];

    txBuf[0] = 0x9F;
    txBuf[1] = 0x00;
    txBuf[2] = 0x00;
    txBuf[3] = 0x00;

    digitalWrite(W25Q_CS_PIN, LOW);
    SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);
    SPI_TransferBuffer(txBuf, rxBuf, 4);
    digitalWrite(W25Q_CS_PIN, HIGH);

    return ((uint32_t)rxBuf[0] << 16) | ((uint32_t)rxBuf[1] << 8) | rxBuf[2];
}

const char* W25Q_IDToName(uint32_t id) {
    uint8_t mfr = (id >> 16) & 0xFF;
    uint8_t dev = (id >> 8) & 0xFF;

    if (mfr == 0xEF) {
        if (dev == 0x40) return "W25Q80";
        if (dev == 0x41) return "W25Q16";
        if (dev == 0x42) return "W25Q32";
        if (dev == 0x43) return "W25Q64";
        return "W25Qxx";
    }
    return "Unknown";
}

void DMA_SendString(const char *str) {
    if (dmaBusy) return;

    uint16_t len = (uint16_t)strlen(str);
    DMA_USART_Send(DMA_USART_CH, (uint8_t*)str, len);
    dmaBusy = 1;
}

void DMA_CompleteCallback(DMA_Channel ch) {
    (void)ch;
    dmaBusy = 0;
}

int main(void) {
    SystemCoreClockUpdate();

    Timer_Init();
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
    dsBus = OneWire_Init(DS18B20_PIN);
    pinMode(W25Q_CS_PIN, PIN_MODE_OUTPUT);
    digitalWrite(W25Q_CS_PIN, HIGH);

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    DMA_USART_InitTx(DMA_USART_CH, NULL, 0);
    DMA_SetTransferCompleteCallback(DMA_USART_CH, DMA_CompleteCallback);

    USART_Print("=== Sensor Hub ===\r\n");

    BMP280_ReadCal();
    uint8_t cfg[2] = {0xF4, 0x27};
    I2C_WriteReg(BMP280_ADDR, cfg[0], cfg[1]);

    uint32_t flashID = W25Q_ReadID();

    Delay_Ms(100);

    while (1) {
        int32_t tempBMP = BMP280_GetTemp();
        int32_t t_fine = (tempBMP * 256 + 128) / 5;
        uint32_t pressure = BMP280_GetPressure(t_fine);

        float tempDS = DS18B20_ReadTemp();

        char outputBuf[128];
        uint16_t idx = 0;

        idx += sprintf(&outputBuf[idx], "I2C(BMP280): ");
        idx += sprintf(&outputBuf[idx], "%d.%dC / ", tempBMP / 100, (tempBMP % 100) > 0 ? (tempBMP % 100) : 0);
        idx += sprintf(&outputBuf[idx], "%d.%dhPa", pressure / 100, pressure % 100);
        idx += sprintf(&outputBuf[idx], "\r\n");

        int dsInt = (int)tempDS;
        int dsFrac = (int)((tempDS - dsInt) * 10);
        if (dsFrac < 0) dsFrac = -dsFrac;
        idx += sprintf(&outputBuf[idx], "1-Wire(DS18B20): %d.%dC\r\n", dsInt, dsFrac);

        idx += sprintf(&outputBuf[idx], "SPI(W25Qxx): %s\r\n", W25Q_IDToName(flashID));

        DMA_SendString(outputBuf);
        Delay_Ms(3000);
    }
}
