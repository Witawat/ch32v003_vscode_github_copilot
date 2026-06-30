/**
 * ============================================================
 * ตัวอย่างที่ 2: LED Controller via USART
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *   LED1 -> PWM1_CH1(PD2) -> 220Ω -> GND
 *   LED2 -> PC0 -> 220Ω -> GND
 *   Button -> PC1 -> 10kΩ pull-up -> 3.3V
 *   USB-Serial: TX=PD5, RX=PD6
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   Serial input "LED1=50" -> LED1 PWM duty 50% -> "LED1 => 50%"
 *   "LED2=1" -> LED2 ON -> "LED2 => ON"
 *   "LED2=0" -> LED2 OFF -> "LED2 => OFF"
 *   "BLINK=500" -> LED2 กระพริบทุก 500ms -> "Blink interval: 500ms"
 *   "STATUS" -> แสดงสถานะทั้งหมด
 * ============================================================
 * คำเตือน (WARNINGS):
 *   - การ parse string ใน embedded ต้องทำแบบ char-by-char
 *   - ไม่ใช้ standard library string functions
 *   - Timer ต้องไม่ blocking การรับ USART
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["PWM_Init + pinMode + USART_Init"]
 *     C --> D["while(1)"]
 *     D --> E{"USART_Available()"}
 *     E -->|"Yes"| F["Read char"]
 *     F --> G{"CR or LF?"}
 *     G -->|"No"| H["Buffer char"]
 *     H --> E
 *     G -->|"Yes"| I["ProcessCommand()"]
 *     I --> J{"Command?"}
 *     J -->|"LED1=x"| K["PWM_Write duty"]
 *     J -->|"LED2=x"| L["digitalWrite ON/OFF"]
 *     J -->|"BLINK=x"| M["Set interval"]
 *     J -->|"STATUS"| N["Print status"]
 *     D --> O{"blinkInterval > 0?"}
 *     O -->|"Yes"| P{"Timer expired?"}
 *     P -->|"Yes"| Q["Toggle LED2"]
 *     D --> R["Button debounce state machine"]
 *     R --> D
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

#define LED1_PIN PD2
#define LED2_PIN PC0
#define BTN_PIN  PC1
#define CMD_BUF  32

uint8_t cmdBuf[CMD_BUF];
uint8_t cmdIndex = 0;
uint8_t cmdReady = 0;

uint8_t led1Duty = 0;
uint8_t led2State = 0;
uint16_t blinkInterval = 0;
uint32_t blinkStart = 0;

uint8_t StrCompare(uint8_t *a, uint8_t *b) {
    uint8_t i = 0;
    while (a[i] != 0 && b[i] != 0) {
        if (a[i] != b[i]) return 0;
        i++;
    }
    return (a[i] == 0 && b[i] == 0) ? 1 : 0;
}

uint16_t StrToNum(uint8_t *str) {
    uint16_t num = 0;
    uint8_t i = 0;
    while (str[i] >= '0' && str[i] <= '9') {
        num = num * 10 + (str[i] - '0');
        i++;
    }
    return num;
}

uint8_t FindEquals(uint8_t *buf) {
    uint8_t i = 0;
    while (buf[i] != 0) {
        if (buf[i] == '=') return i;
        i++;
    }
    return 255;
}

void ProcessCommand(uint8_t *buf) {
    uint8_t eqPos = FindEquals(buf);

    if (eqPos == 255) {
        if (StrCompare(buf, (uint8_t*)"STATUS")) {
            USART_Print("Status: LED1=");
            USART_PrintNum((int32_t)led1Duty);
            USART_Print("%, LED2=");
            USART_Print(led2State ? "ON" : "OFF");
            USART_Print(", BLINK=");
            USART_PrintNum((int32_t)blinkInterval);
            USART_Print("ms\r\n");
        }
        return;
    }

    uint8_t key[16];
    uint8_t val[16];
    uint8_t i, k = 0, v = 0;

    // Bounds check — ป้องกัน buffer overflow
    for (i = 0; i < eqPos && k < 15; i++) key[k++] = buf[i];
    key[k] = 0;

    i = eqPos + 1;
    while (buf[i] != 0 && v < 15) val[v++] = buf[i++];
    val[v] = 0;

    if (StrCompare(key, (uint8_t*)"LED1")) {
        led1Duty = (uint8_t)StrToNum(val);
        if (led1Duty > 100) led1Duty = 100;
        PWM_Write(PWM1_CH1, (led1Duty * 255) / 100);
        USART_Print("LED1 => ");
        USART_PrintNum((int32_t)led1Duty);
        USART_Print("%\r\n");
    } else if (StrCompare(key, (uint8_t*)"LED2")) {
        if (val[0] == '1' || StrCompare(val, (uint8_t*)"ON")) {
            led2State = 1;
            digitalWrite(LED2_PIN, HIGH);
            USART_Print("LED2 => ON\r\n");
        } else {
            led2State = 0;
            digitalWrite(LED2_PIN, LOW);
            USART_Print("LED2 => OFF\r\n");
        }
    } else if (StrCompare(key, (uint8_t*)"BLINK")) {
        blinkInterval = StrToNum(val);
        blinkStart = 0;
        USART_Print("Blink interval: ");
        USART_PrintNum((int32_t)blinkInterval);
        USART_Print("ms\r\n");
    }
}

int main(void) {
    SystemCoreClockUpdate();

    Timer_Init();
    PWM_Init(PWM1_CH1, 1000);
    PWM_Start(PWM1_CH1);
    PWM_Write(PWM1_CH1, 0);

    pinMode(LED2_PIN, PIN_MODE_OUTPUT);
    pinMode(BTN_PIN, PIN_MODE_INPUT_PULLUP);

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    digitalWrite(LED2_PIN, LOW);

    USART_Print("LED Controller Ready\r\n");

    while (1) {
        while (USART_Available()) {
            uint8_t c = USART_Read();
            if (c == '\r' || c == '\n') {
                cmdBuf[cmdIndex] = 0;
                cmdReady = 1;
                cmdIndex = 0;
            } else if (cmdIndex < CMD_BUF - 1) {
                cmdBuf[cmdIndex++] = c;
            }
        }

        if (cmdReady) {
            ProcessCommand(cmdBuf);
            cmdReady = 0;
        }

        if (blinkInterval > 0) {
            uint32_t now = Stopwatch_GetTotalSeconds() * 1000;
            if (blinkStart == 0) {
                blinkStart = now;
            }
            if ((now - blinkStart) >= blinkInterval) {
                led2State = !led2State;
                digitalWrite(LED2_PIN, led2State ? HIGH : LOW);
                blinkStart = now;
            }
        }

        // === ปุ่มกดแบบ non-blocking (state machine) ===
        static uint8_t btn_state = 0;
        static Timer_t btn_timer;

        if (btn_state == 0 && digitalRead(BTN_PIN) == LOW) {
            btn_state = 1;
            Start_Timer(&btn_timer, 50, 0);
        }
        if (btn_state == 1 && Is_Timer_Expired(&btn_timer)) {
            if (digitalRead(BTN_PIN) == LOW) {
                btn_state = 2;
                USART_Print("Button pressed\r\n");
            } else {
                btn_state = 0;
            }
        }
        if (btn_state == 2 && digitalRead(BTN_PIN) == HIGH) {
            btn_state = 0;
        }
    }
}
