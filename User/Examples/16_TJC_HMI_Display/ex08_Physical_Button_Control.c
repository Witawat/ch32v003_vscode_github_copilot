/**
 * ============================================================
 * TJC Example 8: Physical Button Control (No Touch Screen)
 * ============================================================
 *
 * ตัวอย่างการใช้ปุ่มกดภายนอกควบคุม TJC display
 * เหมาะสำหรับโปรเจกต์ที่ไม่ใช้จอสัมผัส หรือต้องการ backup control
 *
 * แผนผังวงจร (Circuit Diagram):
 *   TJC HMI Display (UART):
 *     TX  -> PD6 (MCU RX)
 *     RX  -> PD5 (MCU TX)
 *     VCC -> 5V หรือ 3.3V
 *     GND -> GND
 *
 *   ปุ่มกด 4 ปุ่ม (Active LOW — ต่อลง GND):
 *     BTN_UP    -> PC0 (pull-up ภายใน)
 *     BTN_DOWN  -> PC1 (pull-up ภายใน)
 *     BTN_LEFT  -> PC2 (pull-up ภายใน)
 *     BTN_RIGHT -> PC3 (pull-up ภายใน)
 *
 *   LED (optional สำหรับ feedback):
 *     LED -> PD3 (ผ่าน resistor 220Ω)
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - กด BTN_UP    → เพิ่มค่า counter, แสดงบนจอ n0
 *   - กด BTN_DOWN  → ลดค่า counter, แสดงบนจอ n0
 *   - กด BTN_LEFT  → ไปหน้าก่อนหน้า
 *   - กด BTN_RIGHT → ไปหน้าถัดไป
 *   - กด BTN_UP + BTN_DOWN พร้อมกัน → toggle LED
 *   - กดค้าง > 500ms → repeat ทุก 150ms
 * ============================================================
 * TJC Editor Setup:
 *   Page 0 (Main Menu):
 *     - t_title: ข้อความ "Main Menu"
 *     - n0: numeric แสดงค่า counter
 *     - t_info: ข้อความสถานะ
 *
 *   Page 1 (Settings):
 *     - t_title: "Settings"
 *     - n1: ค่า brightness
 *     - t_info: สถานะ
 *
 *   Page 2 (About):
 *     - t_title: "About"
 *     - t_info: "TJC Button Control v1.0"
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>
#include <stdio.h>
#include "TJC.h"

/* ========== Pin Configuration ========== */
#define BTN_UP     PC0
#define BTN_DOWN   PC1
#define BTN_LEFT   PC2
#define BTN_RIGHT  PC3
#define LED_PIN    PD3

/* ========== Button Debounce/Repeat ========== */
#define DEBOUNCE_MS    50
#define REPEAT_START   500
#define REPEAT_INTERVAL 150

/* ========== State ========== */
typedef struct {
    uint8_t pin;
    uint8_t prev_state;
    uint8_t curr_state;
    uint32_t last_change_ms;
    uint32_t press_start_ms;
    uint8_t repeat_triggered;
} Button_t;

static Button_t btn_up, btn_down, btn_left, btn_right;
static int16_t counter = 0;
static uint8_t current_page = 0;
static uint8_t max_page = 2;
static uint8_t led_state = 0;

/* ========== Forward Declarations ========== */
static void Button_Init(Button_t *btn, uint8_t pin);
static uint8_t Button_Update(Button_t *btn);
static uint8_t Button_IsPressed(Button_t *btn);
static uint8_t Button_JustPressed(Button_t *btn);
static uint8_t Button_Repeat(Button_t *btn);
static void UpdateDisplay(void);
static void GoToPage(uint8_t page);

/* ========== Button Implementation ========== */
static void Button_Init(Button_t *btn, uint8_t pin) {
    btn->pin = pin;
    btn->prev_state = 1;
    btn->curr_state = 1;
    btn->last_change_ms = 0;
    btn->press_start_ms = 0;
    btn->repeat_triggered = 0;
    pinMode(pin, PIN_MODE_INPUT_PULLUP);
}

static uint8_t Button_Update(Button_t *btn) {
    uint8_t raw = digitalRead(btn->pin);
    uint32_t now = Get_CurrentMs();

    btn->prev_state = btn->curr_state;

    if (raw != btn->curr_state) {
        if (now - btn->last_change_ms >= DEBOUNCE_MS) {
            btn->curr_state = raw;
            btn->last_change_ms = now;

            if (raw == 0) {
                btn->press_start_ms = now;
                btn->repeat_triggered = 0;
            }
        }
    }

    return btn->curr_state;
}

static uint8_t Button_IsPressed(Button_t *btn) {
    return btn->curr_state == 0;
}

static uint8_t Button_JustPressed(Button_t *btn) {
    return (btn->prev_state == 1 && btn->curr_state == 0);
}

static uint8_t Button_Repeat(Button_t *btn) {
    if (!Button_IsPressed(btn)) return 0;

    uint32_t now = Get_CurrentMs();
    uint32_t held = now - btn->press_start_ms;

    if (!btn->repeat_triggered && held >= REPEAT_START) {
        btn->repeat_triggered = 1;
        btn->press_start_ms = now;
        return 1;
    }

    if (btn->repeat_triggered && held >= REPEAT_INTERVAL) {
        btn->press_start_ms = now;
        return 1;
    }

    return 0;
}

/* ========== Display Update ========== */
static void UpdateDisplay(void) {
    char buf[48];

    snprintf(buf, sizeof(buf), "n%d.val=%d", current_page, counter);
    TJC_SendCommand(buf);

    snprintf(buf, sizeof(buf), "t_info.txt=\"Page %d/%d | LED:%s\"",
             current_page + 1, max_page + 1, led_state ? "ON" : "OFF");
    TJC_SendCommand(buf);
}

static void GoToPage(uint8_t page) {
    if (page > max_page) page = 0;
    current_page = page;

    char buf[16];
    snprintf(buf, sizeof(buf), "page %d", current_page);
    TJC_SendCommand(buf);

    counter = 0;
    UpdateDisplay();

    USART_Print("Page: ");
    USART_PrintNum(current_page);
    USART_Print("\r\n");
}

/* ========== TJC Callbacks ========== */
void OnSystemEvent(uint8_t event_type) {
    if (event_type == TJC_RET_STARTUP) {
        USART_Print(">> TJC Ready\r\n");
        TJC_SendCommand("bkcmd=1");
        GoToPage(0);
    }
}

void OnError(uint8_t error_code) {
    if (error_code == TJC_ERR_SUCCESS) return;
    USART_Print("TJC ERR: ");
    USART_Print(TJC_GetErrorString(error_code));
    USART_Print("\r\n");
}

/* ========== Main ========== */
int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    pinMode(LED_PIN, PIN_MODE_OUTPUT);

    Button_Init(&btn_up, BTN_UP);
    Button_Init(&btn_down, BTN_DOWN);
    Button_Init(&btn_left, BTN_LEFT);
    Button_Init(&btn_right, BTN_RIGHT);

    TJC_Init(BAUD_115200, USART_PINS_DEFAULT);
    TJC_RegisterSystemEventCallback(OnSystemEvent);
    TJC_RegisterErrorCallback(OnError);

    Delay_Ms(100);
    USART_Print("=== TJC Physical Button Control ===\r\n");
    USART_Print("UP=PC0  DOWN=PC1  LEFT=PC2  RIGHT=PC3\r\n");

    TJC_SendCommand("bkcmd=1");
    GoToPage(0);

    while (1) {
        TJC_ProcessResponse();

        Button_Update(&btn_up);
        Button_Update(&btn_down);
        Button_Update(&btn_left);
        Button_Update(&btn_right);

        if (Button_IsPressed(&btn_up) && Button_IsPressed(&btn_down)) {
            led_state = !led_state;
            digitalWrite(LED_PIN, led_state);
            UpdateDisplay();
            USART_Print(led_state ? "LED ON\r\n" : "LED OFF\r\n");
            while (Button_IsPressed(&btn_up) || Button_IsPressed(&btn_down)) {
                Button_Update(&btn_up);
                Button_Update(&btn_down);
                Delay_Ms(10);
            }
            continue;
        }

        if (Button_JustPressed(&btn_up) || Button_Repeat(&btn_up)) {
            counter++;
            if (counter > 100) counter = 100;
            UpdateDisplay();
            USART_Print("UP: ");
            USART_PrintNum(counter);
            USART_Print("\r\n");
        }

        if (Button_JustPressed(&btn_down) || Button_Repeat(&btn_down)) {
            counter--;
            if (counter < -100) counter = -100;
            UpdateDisplay();
            USART_Print("DOWN: ");
            USART_PrintNum(counter);
            USART_Print("\r\n");
        }

        if (Button_JustPressed(&btn_left)) {
            if (current_page > 0) {
                GoToPage(current_page - 1);
            } else {
                GoToPage(max_page);
            }
        }

        if (Button_JustPressed(&btn_right)) {
            GoToPage(current_page + 1);
        }

        Delay_Ms(10);
    }
}
