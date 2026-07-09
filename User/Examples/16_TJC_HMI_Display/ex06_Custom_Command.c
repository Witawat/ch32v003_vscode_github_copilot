/**
 * ============================================================
 * TJC Example 6: Custom Command (TJC → MCU)
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
 *     IN -> PD4 (ผ่าน resistor 1kΩ + transistor)
 *   Buzzer (PWM):
 *     + -> PD2
 *     - -> GND
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - กดปุ่ม "led|1" บนจอ → LED PD3 ติด
 *   - กดปุ่ม "led|0" บนจอ → LED PD3 ดับ
 *   - กดปุ่ม "relay|1" บนจอ → Relay PD4 เปิด
 *   - เลื่อน slider "pwm|50" → ตั้ง PWM 50%
 *   - กดปุ่ม "goto|1" → ไปหน้า page 1
 *   - กดปุ่ม "status" → MCU ตอบกลับสถานะ
 * ============================================================
 * TJC Editor Setup (Touch Press Event ของแต่ละปุ่ม):
 *
 *   ปุ่ม LED ON:
 *     prints "led|1;"
 *
 *   ปุ่ม LED OFF:
 *     prints "led|0;"
 *
 *   ปุ่ม Relay ON:
 *     prints "relay|1;"
 *
 *   ปุ่ม Relay OFF:
 *     prints "relay|0;"
 *
 *   Slider (h0) ค่าเปลี่ยน:
 *     prints "pwm|"
 *     prints h0.val,0
 *     prints ";"
 *
 *   ปุ่มไปหน้า 1:
 *     prints "goto|1;"
 *
 *   ปุ่มขอสถานะ:
 *     prints "status;"
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "TJC.h"

#define LED_PIN    PD3
#define RELAY_PIN  PD4
#define BUZZER_PWM PWM1_CH1

static uint16_t current_pwm_duty = 0;

void OnTJCCommand(TJC_ReceivedCommand_t *cmd) {
    USART_Print("CMD: ");
    USART_Print(cmd->command);
    for (uint8_t i = 0; i < cmd->param_count; i++) {
        USART_Print(" | ");
        USART_Print(cmd->params[i]);
    }
    USART_Print("\r\n");

    if (strcmp(cmd->command, "led") == 0 && cmd->param_count >= 1) {
        uint8_t state = (uint8_t)atoi(cmd->params[0]);
        digitalWrite(LED_PIN, state);
        TJC_SendCommand(state ? "t_led.txt=\"ON\"" : "t_led.txt=\"OFF\"");
        USART_Print(state ? ">> LED ON\r\n" : ">> LED OFF\r\n");
    }
    else if (strcmp(cmd->command, "relay") == 0 && cmd->param_count >= 1) {
        uint8_t state = (uint8_t)atoi(cmd->params[0]);
        digitalWrite(RELAY_PIN, state);
        TJC_SendCommand(state ? "t_relay.txt=\"ON\"" : "t_relay.txt=\"OFF\"");
        USART_Print(state ? ">> Relay ON\r\n" : ">> Relay OFF\r\n");
    }
    else if (strcmp(cmd->command, "pwm") == 0 && cmd->param_count >= 1) {
        current_pwm_duty = (uint16_t)atoi(cmd->params[0]);
        if (current_pwm_duty > 100) current_pwm_duty = 100;
        PWM_SetDutyCycle(BUZZER_PWM, (uint8_t)current_pwm_duty);

        char buf[32];
        snprintf(buf, sizeof(buf), "n_pwm.val=%d", current_pwm_duty);
        TJC_SendCommand(buf);
        USART_Print(">> PWM: ");
        USART_PrintNum(current_pwm_duty);
        USART_Print("%\r\n");
    }
    else if (strcmp(cmd->command, "goto") == 0 && cmd->param_count >= 1) {
        char buf[32];
        snprintf(buf, sizeof(buf), "page %s", cmd->params[0]);
        TJC_SendCommand(buf);
        USART_Print(">> Goto page: ");
        USART_Print(cmd->params[0]);
        USART_Print("\r\n");
    }
    else if (strcmp(cmd->command, "status") == 0) {
        char buf[64];
        snprintf(buf, sizeof(buf), "t_status.txt=\"%s\"",
                 digitalRead(LED_PIN) ? "LED=ON" : "LED=OFF");
        TJC_SendCommand(buf);

        snprintf(buf, sizeof(buf), "n_pwm.val=%d", current_pwm_duty);
        TJC_SendCommand(buf);

        USART_Print(">> Status sent\r\n");
    }
    else {
        USART_Print(">> Unknown command\r\n");
        TJC_SendCommand("t_err.txt=\"Unknown CMD\"");
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    pinMode(LED_PIN, PIN_MODE_OUTPUT);
    pinMode(RELAY_PIN, PIN_MODE_OUTPUT);
    PWM_Init(BUZZER_PWM, 1000);
    PWM_Start(BUZZER_PWM);

    TJC_Init(BAUD_115200, USART_PINS_DEFAULT);
    TJC_RegisterCommandCallback(OnTJCCommand);

    Delay_Ms(100);
    USART_Print("=== TJC Custom Command ===\r\n");

    TJC_SendCommand("bkcmd=1");
    TJC_SendCommand("page 0");

    while (1) {
        TJC_ProcessResponse();
    }
}
