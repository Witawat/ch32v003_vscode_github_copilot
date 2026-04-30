/**
 * @file Button.c
 * @brief Push Button Library Implementation
 * @version 1.0
 * @date 2026-04-29
 */

#include "Button.h"

/* ========== Public Functions ========== */

/**
 * @brief เริ่มต้น Button instance
 */
void Button_Init(Button_Instance* btn, uint8_t pin, Button_ActiveLevel active_level) {
    if (btn == NULL) return;

    btn->pin              = pin;
    btn->active_level     = active_level;
    btn->debounce_ms      = BUTTON_DEFAULT_DEBOUNCE_MS;
    btn->long_press_ms    = BUTTON_DEFAULT_LONG_PRESS_MS;
    btn->double_click_ms  = BUTTON_DEFAULT_DOUBLE_CLICK_MS;

    /* State initialization */
    btn->raw_state        = 0;
    btn->stable_state     = 0;
    btn->is_pressed       = 0;
    btn->long_press_fired = 0;
    btn->last_change_time = 0;
    btn->press_time       = 0;
    btn->last_release_time= 0;
    btn->click_count      = 0;
    btn->last_event       = BUTTON_EVENT_NONE;

    /* Callbacks */
    btn->on_press        = NULL;
    btn->on_release      = NULL;
    btn->on_long_press   = NULL;
    btn->on_double_click = NULL;

    btn->initialized = 1;

    /* ตั้งค่า GPIO ตาม active level */
    if (active_level == BUTTON_ACTIVE_LOW) {
        /* กด = LOW: ใช้ internal pull-up */
        pinMode(pin, PIN_MODE_INPUT_PULLUP);
    } else {
        /* กด = HIGH: ใช้ pull-down */
        pinMode(pin, PIN_MODE_INPUT_PULLDOWN);
    }
}

/**
 * @brief อัพเดตสถานะปุ่ม — ต้องเรียกใน main loop ทุก iteration
 *
 * State Machine:
 *   1. อ่านค่า raw pin
 *   2. แปลงเป็น "pressed" logic (คำนึง active level)
 *   3. Debounce: รอให้ stable นาน debounce_ms
 *   4. ตรวจจับ event: press, release, long press, double click
 *   5. เรียก callback ที่ลงทะเบียนไว้
 */
void Button_Update(Button_Instance* btn) {
    if (btn == NULL || !btn->initialized) return;

    uint32_t now = Get_CurrentMs();

    /* อ่านสถานะ pin และแปลงเป็น "pressed" logic */
    uint8_t pin_val = digitalRead(btn->pin);
    uint8_t is_raw_pressed = (btn->active_level == BUTTON_ACTIVE_LOW)
                             ? (pin_val == LOW)
                             : (pin_val == HIGH);

    /* ===== Debounce ===== */
    if (is_raw_pressed != btn->raw_state) {
        /* pin เปลี่ยนสถานะ → เริ่มนับ debounce */
        btn->raw_state       = is_raw_pressed;
        btn->last_change_time = now;
    }

    /* ตรวจสอบว่า pin stable แล้วหรือยัง */
    if ((now - btn->last_change_time) < btn->debounce_ms) {
        /* ยังอยู่ใน debounce window → ไม่ทำอะไร */
        goto check_long_press;
    }

    /* pin stable แล้ว ตรวจสอบการเปลี่ยนสถานะ */
    if (is_raw_pressed != btn->stable_state) {
        btn->stable_state = is_raw_pressed;

        if (is_raw_pressed) {
            /* ===== กดปุ่ม (Press) ===== */
            btn->is_pressed       = 1;
            btn->long_press_fired = 0;
            btn->press_time       = now;

            /* ตรวจสอบ Double Click:
             * ถ้ากดครั้งที่ 2 ภายใน double_click_ms หลังจากปล่อยครั้งแรก */
            if (btn->click_count == 1 &&
                (now - btn->last_release_time) < btn->double_click_ms) {
                btn->click_count = 2;  /* รอตรวจสอบหลังปล่อยมือ */
            } else {
                btn->click_count = 1;
            }

            btn->last_event = BUTTON_EVENT_PRESS;
            if (btn->on_press) btn->on_press();

        } else {
            /* ===== ปล่อยปุ่ม (Release) ===== */
            btn->is_pressed       = 0;
            btn->last_release_time = now;

            /* Fire Double Click ถ้าเป็นการกดครั้งที่ 2 */
            if (btn->click_count == 2 && !btn->long_press_fired) {
                btn->click_count = 0;
                btn->last_event  = BUTTON_EVENT_DOUBLE_CLICK;
                if (btn->on_double_click) btn->on_double_click();
            }

            btn->last_event = BUTTON_EVENT_RELEASE;
            if (btn->on_release) btn->on_release();
        }
    }

check_long_press:
    /* ===== Long Press ===== */
    if (btn->is_pressed && !btn->long_press_fired) {
        if ((now - btn->press_time) >= btn->long_press_ms) {
            btn->long_press_fired = 1;
            btn->click_count      = 0;  /* ยกเลิก click count เพราะเป็น long press */
            btn->last_event       = BUTTON_EVENT_LONG_PRESS;
            if (btn->on_long_press) btn->on_long_press();
        }
    }

    /* ===== Double Click Timeout ===== */
    /* ถ้า click_count=1 แต่เกิน double_click_ms แล้วยังไม่มีครั้งที่ 2 → reset */
    if (btn->click_count == 1 && !btn->is_pressed) {
        if ((now - btn->last_release_time) >= btn->double_click_ms) {
            btn->click_count = 0;  /* หมดเวลา double click window */
        }
    }
}

/**
 * @brief ตั้งค่า Callbacks
 */
void Button_SetCallback(Button_Instance* btn,
                        void (*on_press)(void),
                        void (*on_release)(void),
                        void (*on_long_press)(void),
                        void (*on_double_click)(void)) {
    if (btn == NULL) return;
    btn->on_press        = on_press;
    btn->on_release      = on_release;
    btn->on_long_press   = on_long_press;
    btn->on_double_click = on_double_click;
}

/**
 * @brief ตรวจสอบว่าปุ่มถูกกดอยู่หรือไม่
 */
bool Button_IsPressed(Button_Instance* btn) {
    if (btn == NULL || !btn->initialized) return false;
    return btn->is_pressed == 1;
}

/**
 * @brief ดึง event ล่าสุดแล้วล้าง (polling mode)
 */
Button_Event Button_GetEvent(Button_Instance* btn) {
    if (btn == NULL || !btn->initialized) return BUTTON_EVENT_NONE;
    Button_Event ev = btn->last_event;
    btn->last_event = BUTTON_EVENT_NONE;
    return ev;
}

/**
 * @brief ตั้งค่า debounce time
 */
void Button_SetDebounce(Button_Instance* btn, uint16_t ms) {
    if (btn == NULL) return;
    btn->debounce_ms = ms;
}

/**
 * @brief ตั้งค่า long press threshold
 */
void Button_SetLongPressTime(Button_Instance* btn, uint32_t ms) {
    if (btn == NULL) return;
    btn->long_press_ms = ms;
}

/**
 * @brief ตั้งค่า double click window
 */
void Button_SetDoubleClickTime(Button_Instance* btn, uint16_t ms) {
    if (btn == NULL) return;
    btn->double_click_ms = ms;
}

/**
 * @brief Reset state ของปุ่ม
 */
void Button_Reset(Button_Instance* btn) {
    if (btn == NULL) return;
    btn->raw_state         = 0;
    btn->stable_state      = 0;
    btn->is_pressed        = 0;
    btn->long_press_fired  = 0;
    btn->last_change_time  = 0;
    btn->press_time        = 0;
    btn->last_release_time = 0;
    btn->click_count       = 0;
    btn->last_event        = BUTTON_EVENT_NONE;
}

/**
 * @brief แปลง Button_Event เป็น string
 */
const char* Button_EventStr(Button_Event event) {
    switch (event) {
        case BUTTON_EVENT_NONE:         return "NONE";
        case BUTTON_EVENT_PRESS:        return "PRESS";
        case BUTTON_EVENT_RELEASE:      return "RELEASE";
        case BUTTON_EVENT_LONG_PRESS:   return "LONG_PRESS";
        case BUTTON_EVENT_DOUBLE_CLICK: return "DOUBLE_CLICK";
        default:                        return "UNKNOWN";
    }
}
