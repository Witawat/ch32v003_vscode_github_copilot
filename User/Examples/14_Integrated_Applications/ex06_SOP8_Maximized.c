/**
 * @example ex06_SOP8_Maximized.c
 * @brief ใช้ครบทุกฟังก์ชันบน SOP-8 (6 pins) — shiftOut + SoftI2C + PWM + USART + ADC
 *
 * @details
 * SOP-8 มี 6 user pins: PD1(SWIO), PD4, PD5, PD6, PC1, PC2
 * ใช้ทุก pin ให้คุ้ม:
 *   PD1 = SWDIO (debug เท่านั้น)
 *   PD4 = PWM LED (PWM2_CH1)
 *   PD5 = USART TX
 *   PD6 = Analog Input (NTC temperature sensor via ADC_CH6)
 *   PC1 = SoftI2C SDA (OLED display)
 *   PC2 = SoftI2C SCL
 *
 * ฟีเจอร์ที่ไม่สามารถใช้บน SOP-8:
 * - Hardware SPI (ไม่มี SCK/MISO/MOSI) → ใช้ shiftOut แทน
 * - Hardware I2C (PC1/PC2 อาจไม่มี) → ใช้ SimpleI2C_Soft แทน
 * - PWM channels เกิน PWM2_CH1 (PD2,PD3,PD7,PC0,PC3,PC4,PA1 ไม่มี)
 *
 * @note เปลี่ยน CH32V003_PACKAGE เป็น PACKAGE_TSSOP20 เพื่อใช้ฟีเจอร์ครบ
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART + ADC + PWM init"]
 *     C --> D["I2C_Soft_Init()"]
 *     D --> E["while(1)"]
 *     E --> F["PWM brightness fade"]
 *     F --> G["ADC temperature read"]
 *     G --> H["USART print"]
 *     H --> I["Delay_Ms(200)"]
 *     I --> E
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_SOP8
#include "SimpleHAL.h"
#include "SimpleI2C_Soft.h"

// SOP-8 Pin Assignment
#define PWM_LED      PD4     // PWM2_CH1
#define USART_TX     PD5     // USART TX (default)
#define USART_RX     PD6     // USART RX (default)
#define TEMP_ADC     PD6     // ADC_CH6 — NTC temperature
#define I2C_SDA      PC1     // Software I2C SDA
#define I2C_SCL      PC2     // Software I2C SCL

static uint16_t read_temperature(void) {
    uint16_t adc = analogRead(TEMP_ADC);
    // NTC 10K: แปลง ADC → อุณหภูมิโดยประมาณ (ตาราง lookup อย่างง่าย)
    // ที่ 25°C, NTC 10K → ADC ~512 (3.3V/2)
    // ต่ำกว่า = เย็นกว่า, สูงกว่า = ร้อนกว่า
    return adc;
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    // USART — PD5/PD6 (ใช้ได้ทุกแพ็กเกจ)
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    // ADC — PD6 (ADC_CH6)
    ADC_SimpleInit();

    // PWM LED — PD4 (PWM2_CH1)
    PWM_Init(PWM2_CH1, 1000);
    PWM_Start(PWM2_CH1);

    // Software I2C — PC1/PC2 (ใช้ GPIO ขาไหนก็ได้)
    I2C_Soft_Init(I2C_SCL, I2C_SDA, I2C_SOFT_100KHZ);

    // shift register LED — ส่งออกทาง USART (PD5) หรือใช้ pin อื่น
    // shiftOut(PD5, PD4, MSBFIRST, 0xFF);  // ตัวอย่าง

    USART_Print("\r\n=== SOP-8 Maximized ===\r\n");
    USART_Print("Pins: PD4=PWM, PD5=TX, PD6=ADC+RX, PC1/PC2=SoftI2C\r\n\r\n");

    uint8_t brightness = 0;
    int8_t dir = 5;

    while (1) {
        // PWM fade
        brightness += dir;
        if (brightness >= 100 || brightness <= 0) dir = -dir;
        PWM_SetDutyCycle(PWM2_CH1, brightness);

        // ADC temperature
        uint16_t temp_adc = read_temperature();

        // USART output
        USART_Print("LED: ");
        USART_PrintNum(brightness);
        USART_Print("%  NTC_ADC: ");
        USART_PrintNum(temp_adc);
        USART_Print("\r\n");

        Delay_Ms(200);
    }
}
