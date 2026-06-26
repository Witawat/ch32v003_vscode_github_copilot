/**
 * ============================================================
 * ตัวอย่างที่ 4: Smart Countdown
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *   OLED SSD1306 (I2C):
 *     VCC -> 3.3V, GND -> GND
 *     SCL -> PC2 + 4.7kΩ pull-up -> 3.3V
 *     SDA -> PC1 + 4.7kΩ pull-up -> 3.3V
 *   Buzzer -> PWM1_CH2(PA1) -> NPN transistor (2N2222) -> Collector -> Buzzer+ -> GND
 *   Button1 (start/pause) -> PC3 -> 10kΩ pull-up -> 3.3V
 *   Button2 (reset) -> PC4 -> 10kΩ pull-up -> 3.3V
 *   USB-Serial: TX=PD5, RX=PD6
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   OLED: "Smart Timer" -> กด Start -> "00:30" -> "00:29" -> ... -> "00:00"
 *   เมื่อหมดเวลา: Buzzer ดัง 1 วินาที + USART "Time's up!"
 * ============================================================
 * คำเตือน (WARNINGS):
 *   - OLED driver อยู่ใน User/Lib/OLED - ติดตาม oled_i2c.h และ oled_graphics.h
 *   - Countdown ใช้ TIM2 - อาจขัดแย้งกับ PWM ถ้าใช้ TIM2 channels
 *   - Buzzer ต้องใช้ transistor ขยายกระแส
 * ============================================================
 */

#include <SimpleHAL.h>

#define BUZZER_PIN PA1
#define BTN1_PIN   PC3
#define BTN2_PIN   PC4

volatile uint8_t btn1State = 0;
volatile uint8_t btn2State = 0;

volatile uint8_t countdownRunning = 0;
volatile uint8_t countdownPaused = 0;
volatile uint16_t remainingSeconds = 30;
volatile uint8_t alarmTriggered = 0;

void Button1_ISR(void) {
    btn1State = 1;
}

void Button2_ISR(void) {
    btn2State = 1;
}

void Countdown_Finished_Callback(void) {
    alarmTriggered = 1;
    countdownRunning = 0;
}

void PrintTime(uint16_t totalSec) {
    USART_Print("Smart Timer [");
    if (totalSec < 10) USART_Print("0");
    USART_PrintNum((int32_t)(totalSec / 60));
    USART_Print(":");
    if (totalSec % 60 < 10) USART_Print("0");
    USART_PrintNum((int32_t)(totalSec % 60));
    USART_Print("]\r");
}

int main(void) {
    SystemCoreClockUpdate();

    Timer_Init();
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);

    PWM_Init(PWM1_CH2, 1000);
    PWM_Start(PWM1_CH2);
    PWM_SetDutyCycle(PWM1_CH2, 0);

    pinMode(BTN1_PIN, PIN_MODE_INPUT_PULLUP);
    pinMode(BTN2_PIN, PIN_MODE_INPUT_PULLUP);

    attachInterrupt(BTN1_PIN, Button1_ISR, FALLING);
    attachInterrupt(BTN2_PIN, Button2_ISR, FALLING);

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    USART_Print("Smart Countdown Ready\r\n");

    Countdown_Init(0, 0, 30);
    Countdown_SetAlarmCallback(Countdown_Finished_Callback);

    while (1) {
        if (btn1State) {
            btn1State = 0;
            Delay_Ms(50);

            if (!countdownRunning) {
                Countdown_Start();
                countdownRunning = 1;
                countdownPaused = 0;
                USART_Print("Countdown started\r\n");
            } else if (!countdownPaused) {
                Countdown_Stop();
                countdownPaused = 1;
                USART_Print("Countdown paused\r\n");
            } else {
                Countdown_Start();
                countdownPaused = 0;
                USART_Print("Countdown resumed\r\n");
            }
        }

        if (btn2State) {
            btn2State = 0;
            Delay_Ms(50);

            Countdown_Reset();
            countdownRunning = 0;
            countdownPaused = 0;
            remainingSeconds = 30;
            alarmTriggered = 0;
            PWM_SetDutyCycle(PWM1_CH2, 0);

            PrintTime(remainingSeconds);
            USART_Print("Countdown reset\r\n");
        }

        if (countdownRunning && !countdownPaused) {
            uint16_t sec = Countdown_GetRemainingSeconds();
            if (sec != remainingSeconds) {
                remainingSeconds = sec;
                PrintTime(remainingSeconds);
            }
        }

        if (alarmTriggered) {
            alarmTriggered = 0;

            USART_Print("Time's Up!\r\n");

            PWM_SetDutyCycle(PWM1_CH2, 50);
            USART_Print("Time's up!\r\n");
            Delay_Ms(1000);
            PWM_SetDutyCycle(PWM1_CH2, 0);
        }
    }
}
