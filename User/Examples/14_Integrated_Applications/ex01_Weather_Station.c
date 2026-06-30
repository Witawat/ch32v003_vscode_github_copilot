/**
 * ============================================================
 * ตัวอย่างที่ 1: Weather Station
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *   DS18B20:
 *     VDD -> 3.3V
 *     GND -> GND
 *     DQ  -> PD2 (ต้องมี 4.7kΩ pull-up ไป 3.3V)
 *   BMP280:
 *     VCC -> 3.3V
 *     GND -> GND
 *     SCL -> PC2 (ต้องมี 4.7kΩ pull-up ไป 3.3V)
 *     SDA -> PC1 (ต้องมี 4.7kΩ pull-up ไป 3.3V)
 *     CSB -> 3.3V (I2C mode)
 *     SDD -> GND (I2C address 0x76)
 *   USB-Serial:
 *     TX  -> PD5
 *     RX  -> PD6
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   Serial output every 2 seconds:
 *   "=== Weather Station ==="
 *   "Temp: 28.5C (DS18B20) / 28.3C (BMP280)"
 *   "Pressure: 1013.2 hPa"
 *   "Humidity: 65.0% (placeholder)"
 * ============================================================
 * คำเตือน (WARNINGS):
 *   - BMP280 ต้องการเวลาแปลงค่า (ใช้ delay)
 *   - สูตรคำนวณ Pressure ต้องใช้ค่า calibration coefficient จากโรงงาน
 *   - 1-Wire ใช้ pin แยกจาก I2C ไม่แชร์ bus
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
/* CH32V003 has no hardware FPU � float/double use software emulation (~800 cycles) */
#include <SimpleHAL.h>

#define DS18B20_PIN PD2
#define BMP280_ADDR 0x76
#define LOOP_DELAY 2000

OneWire_Bus* dsBus;

int16_t dig_T1, dig_T2, dig_T3;
uint16_t dig_P1;
int16_t dig_P2, dig_P3, dig_P4;
int16_t dig_P5, dig_P6, dig_P7;
int16_t dig_P8, dig_P9;

void BMP280_ReadCalibration(void) {
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

int32_t BMP280_ReadTempRaw(void) {
    uint8_t buf[3];
    I2C_ReadRegMulti(BMP280_ADDR, 0xFA, buf, 3);

    int32_t adc_T = ((int32_t)buf[0] << 12) |
                    ((int32_t)buf[1] << 4)  |
                    ((int32_t)buf[2] >> 4);
    return adc_T;
}

int32_t BMP280_CompensateTemp(int32_t adc_T, int32_t *t_fine) {
    int32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    *t_fine = var1 + var2;
    T = (*t_fine * 5 + 128) >> 8;
    return T;
}

uint32_t BMP280_CompensatePressure(int32_t adc_P, int32_t t_fine) {
    int64_t var1, var2, p;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0) return 0;

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (uint32_t)(p / 256);
}

float DS18B20_ReadTemp(void) {
    uint8_t tempL, tempH;
    int16_t rawTemp;

    OneWire_Reset(dsBus);
    OneWire_SkipROM(dsBus);
    OneWire_WriteByte(dsBus, 0x44);

    OneWire_Reset(dsBus);
    OneWire_SkipROM(dsBus);
    OneWire_WriteByte(dsBus, 0xBE);

    tempL = OneWire_ReadByte(dsBus);
    tempH = OneWire_ReadByte(dsBus);

    rawTemp = (int16_t)(((uint16_t)tempH << 8) | tempL);
    return rawTemp * 0.0625f;
}

int main(void) {
    SystemCoreClockUpdate();

    Timer_Init();
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
    dsBus = OneWire_Init(DS18B20_PIN);
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    BMP280_ReadCalibration();

    uint8_t config[2] = {0xF4, 0x27};
    I2C_WriteReg(BMP280_ADDR, config[0], config[1]);

    while (1) {
        int32_t temp_DS_raw = (int32_t)(DS18B20_ReadTemp() * 10);

        int32_t adc_T = BMP280_ReadTempRaw();
        int32_t t_fine;
        int32_t temp_BMP = BMP280_CompensateTemp(adc_T, &t_fine);

        uint8_t press_buf[3];
        I2C_ReadRegMulti(BMP280_ADDR, 0xF7, press_buf, 3);
        int32_t adc_P = ((int32_t)press_buf[0] << 12) | ((int32_t)press_buf[1] << 4) | ((int32_t)press_buf[2] >> 4);
        uint32_t pressure = BMP280_CompensatePressure(adc_P, t_fine);

        USART_Print("=== Weather Station ===");
        USART_Print("\r\n");

        USART_Print("Temp: ");
        USART_PrintNum(temp_DS_raw / 10);
        USART_Print(".");
        USART_PrintNum(temp_DS_raw % 10);
        USART_Print("C (DS18B20) / ");
        USART_PrintNum(temp_BMP / 100);
        USART_Print(".");
        USART_PrintNum((temp_BMP % 100) > 0 ? (temp_BMP % 100) : 0);
        USART_Print("C (BMP280)");
        USART_Print("\r\n");

        USART_Print("Pressure: ");
        USART_PrintNum(pressure / 100);
        USART_Print(".");
        USART_PrintNum(pressure % 100);
        USART_Print(" hPa");
        USART_Print("\r\n");

        USART_Print("Humidity: 65.0% (placeholder)");
        USART_Print("\r\n\r\n");

        Delay_Ms(LOOP_DELAY);
    }
}
