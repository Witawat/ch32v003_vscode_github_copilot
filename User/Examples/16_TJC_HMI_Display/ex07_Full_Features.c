/**
 * ============================================================
 * TJC Example 7: Full Features (All Callbacks + Sensors)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *   TJC HMI Display (UART):
 *     TX  -> PD6 (MCU RX)
 *     RX  -> PD5 (MCU TX)
 *     VCC -> 5V หรือ 3.3V
 *     GND -> GND
 *   LED:
 *     Anode -> PD3 (ผ่าน resistor 220Ω)
 *     Cathode -> GND
 *   Relay:
 *     IN -> PD4
 *   DHT11:
 *     VCC -> 3.3V
 *     GND -> GND
 *     DATA -> PC0
 *   BMP280 (I2C):
 *     VCC -> 3.3V
 *     GND -> GND
 *     SCL -> PC2 (4.7kΩ pull-up)
 *     SDA -> PC1 (4.7kΩ pull-up)
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - ใช้ TJC ทุกรูปแบบ callback พร้อมกัน
 *   - อ่าน sensor ทุก 2 วินาที ส่งแสดงผลบนจอ
 *   - รับ touch event จากปุ่มบนจอ
 *   - รับ custom command จาก TJC
 *   - จัดการ system event (startup/sleep/wake)
 *   - แสดง error ทาง Serial
 * ============================================================
 * TJC Editor Setup:
 *   Page 0 (Main):
 *     - t0: แสดงอุณหภูมิ
 *     - t1: แสดงความชื้น
 *     - t2: แสดงความกดอากาศ
 *     - n0: แสดงค่า counter
 *     - b0: ปุ่มเพิ่ม counter
 *     - b1: ปุ่มลด counter
 *     - b2: ปุ่มไปหน้า Settings
 *     - Touch Event เปิดทุกปุ่ม
 *     - Global Init: bkcmd=3, sendxy=0
 *
 *   Page 1 (Settings):
 *     - b0: ปุ่มสั่ง LED toggle
 *       prints "led|toggle;"
 *     - b1: ปุ่มสั่ง Relay toggle
 *       prints "relay|toggle;"
 *     - b2: ปุ่มกลับหน้าหลัก
 *       prints "goto|0;"
 *     - b3: ปุ่ม refresh sensor
 *       prints "refresh;"
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "TJC.h"

#define LED_PIN     PD3
#define RELAY_PIN   PD4
#define DHT_PIN     PC0
#define BMP280_ADDR 0x76

static int16_t counter = 0;
static uint8_t led_state = 0;
static uint8_t relay_state = 0;

/* ========== BMP280 Calibration ========== */
static int16_t dig_T1, dig_T2, dig_T3;
static uint16_t dig_P1;
static int16_t dig_P2, dig_P3, dig_P4;
static int16_t dig_P5, dig_P6, dig_P7;
static int16_t dig_P8, dig_P9;

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

int32_t BMP280_CompensateTemp(int32_t adc_T, int32_t *t_fine) {
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    *t_fine = var1 + var2;
    return (*t_fine * 5 + 128) >> 8;
}

uint32_t BMP280_CompensatePressure(int32_t adc_P, int32_t t_fine) {
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

/* ========== DHT11 Reading ========== */
uint8_t DHT11_Read(int16_t *temp_x10, uint16_t *hum_x10) {
    pinMode(DHT_PIN, PIN_MODE_OUTPUT);
    digitalWrite(DHT_PIN, 0);
    Delay_Ms(20);
    digitalWrite(DHT_PIN, 1);
    Delay_Us(30);
    pinMode(DHT_PIN, PIN_MODE_INPUT);

    if (digitalRead(DHT_PIN) != 0) return 1;
    Delay_Us(80);
    if (digitalRead(DHT_PIN) != 1) return 2;
    Delay_Us(80);

    uint8_t data[5] = {0};
    for (uint8_t i = 0; i < 5; i++) {
        for (uint8_t b = 7; b < 255; b--) {
            while (digitalRead(DHT_PIN) == 0);
            Delay_Us(40);
            if (digitalRead(DHT_PIN) == 1) {
                data[i] |= (1 << b);
            }
            while (digitalRead(DHT_PIN) == 1);
        }
    }

    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) return 3;
    *hum_x10 = (data[0] * 10) + data[1];
    *temp_x10 = (data[2] * 10) + data[3];
    return 0;
}

/* ========== System Event ========== */
void OnSystemEvent(uint8_t event_type) {
    switch (event_type) {
        case TJC_RET_STARTUP:
            USART_Print(">> TJC Startup\r\n");
            TJC_SendCommand("bkcmd=3");
            TJC_SendCommand("page 0");
            break;
        case TJC_RET_AUTO_SLEEP:
            USART_Print(">> TJC Sleep\r\n");
            break;
        case TJC_RET_AUTO_WAKE:
            USART_Print(">> TJC Wake\r\n");
            TJC_SendCommand("page 0");
            break;
    }
}

/* ========== Error Callback ========== */
void OnError(uint8_t error_code) {
    if (error_code == TJC_ERR_SUCCESS) return;
    USART_Print("ERR: ");
    USART_Print(TJC_GetErrorString(error_code));
    USART_Print("\r\n");
}

/* ========== Touch Event ========== */
void OnTouch(TJC_TouchEvent_t *event) {
    if (event->event_type != 0x01) return;

    USART_Print("Touch: p=");
    USART_PrintNum(event->page_id);
    USART_Print(" c=");
    USART_PrintNum(event->component_id);
    USART_Print("\r\n");

    if (event->page_id == 0) {
        switch (event->component_id) {
            case 0:
                counter++;
                break;
            case 1:
                counter--;
                break;
            case 2:
                TJC_SendCommand("page 1");
                return;
            default:
                return;
        }
        char buf[32];
        snprintf(buf, sizeof(buf), "n0.val=%d", counter);
        TJC_SendCommand(buf);
    }
}

/* ========== Custom Command ========== */
void OnTJCCommand(TJC_ReceivedCommand_t *cmd) {
    USART_Print("CMD: ");
    USART_Print(cmd->command);
    USART_Print("\r\n");

    if (strcmp(cmd->command, "led") == 0 && cmd->param_count >= 1) {
        if (strcmp(cmd->params[0], "toggle") == 0) {
            led_state = !led_state;
        } else {
            led_state = (uint8_t)atoi(cmd->params[0]);
        }
        digitalWrite(LED_PIN, led_state);
        TJC_SendCommand(led_state ? "t_led.txt=\"ON\"" : "t_led.txt=\"OFF\"");
    }
    else if (strcmp(cmd->command, "relay") == 0 && cmd->param_count >= 1) {
        if (strcmp(cmd->params[0], "toggle") == 0) {
            relay_state = !relay_state;
        } else {
            relay_state = (uint8_t)atoi(cmd->params[0]);
        }
        digitalWrite(RELAY_PIN, relay_state);
        TJC_SendCommand(relay_state ? "t_relay.txt=\"ON\"" : "t_relay.txt=\"OFF\"");
    }
    else if (strcmp(cmd->command, "goto") == 0 && cmd->param_count >= 1) {
        char buf[32];
        snprintf(buf, sizeof(buf), "page %s", cmd->params[0]);
        TJC_SendCommand(buf);
    }
    else if (strcmp(cmd->command, "refresh") == 0) {
        USART_Print(">> Manual sensor refresh\r\n");
    }
}

/* ========== Numeric & String ========== */
void OnNumeric(uint32_t value) {
    USART_Print("NUM: ");
    USART_PrintNum(value);
    USART_Print("\r\n");
}

void OnString(const char *str, uint16_t len) {
    USART_Print("STR: ");
    USART_Print(str);
    USART_Print("\r\n");
    (void)len;
}

/* ========== Sensor Reading ========== */
void UpdateSensorDisplay(void) {
    char buf[48];

    int16_t dht_temp;
    uint16_t dht_hum;
    if (DHT11_Read(&dht_temp, &dht_hum) == 0) {
        snprintf(buf, sizeof(buf), "t0.txt=\"%d.%dC\"",
                 dht_temp / 10, dht_temp % 10);
        TJC_SendCommand(buf);

        snprintf(buf, sizeof(buf), "t1.txt=\"%d%%\"", dht_hum / 10);
        TJC_SendCommand(buf);
    }

    uint8_t press_buf[3];
    I2C_ReadRegMulti(BMP280_ADDR, 0xF7, press_buf, 3);
    int32_t adc_P = ((int32_t)press_buf[0] << 12) |
                    ((int32_t)press_buf[1] << 4) |
                    ((int32_t)press_buf[2] >> 4);

    uint8_t temp_buf[3];
    I2C_ReadRegMulti(BMP280_ADDR, 0xFA, temp_buf, 3);
    int32_t adc_T = ((int32_t)temp_buf[0] << 12) |
                    ((int32_t)temp_buf[1] << 4) |
                    ((int32_t)temp_buf[2] >> 4);

    int32_t t_fine;
    BMP280_CompensateTemp(adc_T, &t_fine);
    uint32_t pressure = BMP280_CompensatePressure(adc_P, t_fine);

    snprintf(buf, sizeof(buf), "t2.txt=\"%d.%dhPa\"",
             (int)(pressure / 100), (int)(pressure % 100));
    TJC_SendCommand(buf);

    USART_Print("Sensor updated\r\n");
}

/* ========== Main ========== */
int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);

    pinMode(LED_PIN, PIN_MODE_OUTPUT);
    pinMode(RELAY_PIN, PIN_MODE_OUTPUT);

    BMP280_ReadCalibration();
    uint8_t config[2] = {0xF4, 0x27};
    I2C_WriteReg(BMP280_ADDR, config[0], config[1]);

    TJC_Init(BAUD_115200, USART_PINS_DEFAULT);
    TJC_RegisterSystemEventCallback(OnSystemEvent);
    TJC_RegisterErrorCallback(OnError);
    TJC_RegisterTouchEventCallback(OnTouch);
    TJC_RegisterCommandCallback(OnTJCCommand);
    TJC_RegisterNumericCallback(OnNumeric);
    TJC_RegisterStringCallback(OnString);

    Delay_Ms(100);
    USART_Print("=== TJC Full Features ===\r\n");

    uint32_t last_sensor = 0;

    while (1) {
        TJC_ProcessResponse();

        uint32_t now = Get_CurrentMs();
        if (now - last_sensor >= 2000) {
            last_sensor = now;
            UpdateSensorDisplay();
        }
    }
}
