/**
 * @file WS2812Matrix.c
 * @brief WS2812 LED Matrix Library Implementation
 * @version 1.0
 * @date 2026-05-03
 */

#include "WS2812Matrix.h"

/* ========== Private Variables ========== */

static WS2812M_Instance* _active_instance = NULL;

/* ========== Private Function Prototypes ========== */

/**
 * @brief แปลง SimpleGPIO pin number เป็น GPIO_TypeDef*
 * @param pin SimpleGPIO pin (PC4=14, PD2=20, ...)
 * @param[out] out_port ตัวชี้ไปยัง GPIO port ที่จะรับค่า
 * @param[out] out_pin_mask ตัวชี้ไปยัง pin mask ที่จะรับค่า
 * @return 1 = สำเร็จ, 0 = pin ไม่ถูกต้อง
 */
static uint8_t resolve_pin(uint8_t pin, GPIO_TypeDef** out_port,
                           uint16_t* out_pin_mask);

/* ========== Private Functions ========== */

static uint8_t resolve_pin(uint8_t pin, GPIO_TypeDef** out_port,
                           uint16_t* out_pin_mask) {
    // PA1–PA2
    if (pin == 0)  { *out_port = GPIOA; *out_pin_mask = GPIO_Pin_1; return 1; }
    if (pin == 1)  { *out_port = GPIOA; *out_pin_mask = GPIO_Pin_2; return 1; }
    // PC0–PC7
    if (pin >= 10 && pin <= 17) {
        *out_port = GPIOC;
        *out_pin_mask = (uint16_t)(1 << (pin - 10));
        return 1;
    }
    // PD2–PD7
    if (pin >= 20 && pin <= 25) {
        *out_port = GPIOD;
        *out_pin_mask = (uint16_t)(1 << (pin - 18));
        return 1;
    }
    return 0;  // invalid pin
}

/* ========== Public Functions ========== */

uint8_t WS2812M_Init(WS2812M_Instance* inst, uint8_t data_pin,
                     uint8_t width, uint8_t height, WS2812M_Wiring wiring) {
    GPIO_TypeDef* port = NULL;
    uint16_t pin_mask = 0;

    // === Null check ===
    if (inst == NULL) return 0;

    // === Validate params ===
    if (width  == 0 || width  > WS2812M_MAX_WIDTH)  return 0;
    if (height == 0 || height > WS2812M_MAX_HEIGHT) return 0;
    if (!resolve_pin(data_pin, &port, &pin_mask))   return 0;

    // === Store config ===
    inst->data_pin   = data_pin;
    inst->width      = width;
    inst->height     = height;
    inst->num_pixels = (uint16_t)width * height;
    inst->wiring     = wiring;

    // === Init NeoPixel (low-level driver) ===
    NeoPixel_Init(port, pin_mask, inst->num_pixels);
    NeoPixel_Clear();
    NeoPixel_Show();

    // === Register active instance ===
    _active_instance = inst;

    // === Mark initialized ===
    inst->initialized = 1;

    return 1;
}

void WS2812M_SetPixel(WS2812M_Instance* inst, uint8_t x, uint8_t y,
                      uint8_t r, uint8_t g, uint8_t b) {
    uint16_t index;

    if (inst == NULL) return;
    if (!inst->initialized) return;
    if (x >= inst->width || y >= inst->height) return;

    index = WS2812M_XYtoIndex(x, y, inst->width, inst->wiring);
    NeoPixel_SetPixelColor(index, r, g, b);
}

void WS2812M_SetPixelColor(WS2812M_Instance* inst, uint8_t x, uint8_t y,
                           uint32_t color) {
    uint16_t index;

    if (inst == NULL) return;
    if (!inst->initialized) return;
    if (x >= inst->width || y >= inst->height) return;

    index = WS2812M_XYtoIndex(x, y, inst->width, inst->wiring);
    NeoPixel_SetPixelColor32(index, color);
}

uint32_t WS2812M_GetPixel(WS2812M_Instance* inst, uint8_t x, uint8_t y) {
    uint16_t index;

    if (inst == NULL) return 0;
    if (!inst->initialized) return 0;
    if (x >= inst->width || y >= inst->height) return 0;

    index = WS2812M_XYtoIndex(x, y, inst->width, inst->wiring);
    return NeoPixel_GetPixelColor(index);
}

void WS2812M_Clear(WS2812M_Instance* inst) {
    if (inst == NULL) return;
    if (!inst->initialized) return;

    NeoPixel_Clear();
}

void WS2812M_Fill(WS2812M_Instance* inst, uint8_t r, uint8_t g, uint8_t b) {
    if (inst == NULL) return;
    if (!inst->initialized) return;

    NeoPixel_Fill(r, g, b);
}

void WS2812M_Show(WS2812M_Instance* inst) {
    if (inst == NULL) return;
    if (!inst->initialized) return;

    NeoPixel_Show();
}

void WS2812M_SetBrightness(WS2812M_Instance* inst, uint8_t brightness) {
    if (inst == NULL) return;
    if (!inst->initialized) return;

    NeoPixel_SetBrightness(brightness);
}

void WS2812M_Deinit(WS2812M_Instance* inst) {
    if (inst == NULL) return;
    if (!inst->initialized) return;

    _active_instance = NULL;
    inst->initialized = 0;
}

/* ========== XY-to-Index Mapping ========== */

uint16_t WS2812M_XYtoIndex(uint8_t x, uint8_t y, uint8_t width,
                           WS2812M_Wiring wiring) {
    if (wiring == WIRING_ZIGZAG) {
        // Zigzag: even rows → left-to-right, odd rows → right-to-left
        if (y & 1) {
            return (uint16_t)(y * width) + (uint16_t)(width - 1 - x);
        } else {
            return (uint16_t)(y * width) + x;
        }
    } else {
        // Snake: all rows left-to-right
        return (uint16_t)(y * width) + x;
    }
}

/* ========== Drawing Primitives ========== */

void WS2812M_DrawLine(WS2812M_Instance* inst, int16_t x0, int16_t y0,
                      int16_t x1, int16_t y1, uint8_t r, uint8_t g, uint8_t b) {
    int16_t dx, dy, sx, sy, err, e2;

    if (inst == NULL) return;
    if (!inst->initialized) return;

    // Bresenham's line algorithm
    dx = abs(x1 - x0);
    dy = abs(y1 - y0);
    sx = (x0 < x1) ? 1 : -1;
    sy = (y0 < y1) ? 1 : -1;
    err = dx - dy;

    while (1) {
        if (x0 >= 0 && y0 >= 0 &&
            x0 < (int16_t)inst->width && y0 < (int16_t)inst->height) {
            WS2812M_SetPixel(inst, (uint8_t)x0, (uint8_t)y0, r, g, b);
        }

        if (x0 == x1 && y0 == y1) break;

        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

void WS2812M_DrawLineColor(WS2812M_Instance* inst, int16_t x0, int16_t y0,
                           int16_t x1, int16_t y1, uint32_t color) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    WS2812M_DrawLine(inst, x0, y0, x1, y1, r, g, b);
}

void WS2812M_DrawRect(WS2812M_Instance* inst, int16_t x, int16_t y,
                      uint8_t w, uint8_t h, uint8_t r, uint8_t g, uint8_t b) {
    if (inst == NULL) return;
    if (!inst->initialized) return;
    if (w == 0 || h == 0) return;

    WS2812M_DrawLine(inst, x, y, x + w - 1, y, r, g, b);                  // top
    WS2812M_DrawLine(inst, x, y + h - 1, x + w - 1, y + h - 1, r, g, b); // bottom
    WS2812M_DrawLine(inst, x, y, x, y + h - 1, r, g, b);                  // left
    WS2812M_DrawLine(inst, x + w - 1, y, x + w - 1, y + h - 1, r, g, b); // right
}

void WS2812M_FillRect(WS2812M_Instance* inst, int16_t x, int16_t y,
                      uint8_t w, uint8_t h, uint8_t r, uint8_t g, uint8_t b) {
    int16_t i, j;

    if (inst == NULL) return;
    if (!inst->initialized) return;
    if (w == 0 || h == 0) return;

    for (j = y; j < y + (int16_t)h; j++) {
        for (i = x; i < x + (int16_t)w; i++) {
            if (i >= 0 && j >= 0 &&
                i < (int16_t)inst->width && j < (int16_t)inst->height) {
                WS2812M_SetPixel(inst, (uint8_t)i, (uint8_t)j, r, g, b);
            }
        }
    }
}

void WS2812M_DrawCircle(WS2812M_Instance* inst, int16_t x0, int16_t y0,
                        uint8_t radius, uint8_t r, uint8_t g, uint8_t b) {
    int16_t x = radius;
    int16_t y = 0;
    int16_t err = 0;

    if (inst == NULL) return;
    if (!inst->initialized) return;

    while (x >= y) {
        WS2812M_SetPixel(inst, x0 + x, y0 + y, r, g, b);
        WS2812M_SetPixel(inst, x0 + y, y0 + x, r, g, b);
        WS2812M_SetPixel(inst, x0 - y, y0 + x, r, g, b);
        WS2812M_SetPixel(inst, x0 - x, y0 + y, r, g, b);
        WS2812M_SetPixel(inst, x0 - x, y0 - y, r, g, b);
        WS2812M_SetPixel(inst, x0 - y, y0 - x, r, g, b);
        WS2812M_SetPixel(inst, x0 + y, y0 - x, r, g, b);
        WS2812M_SetPixel(inst, x0 + x, y0 - y, r, g, b);

        if (err <= 0) { y += 1; err += 2 * y + 1; }
        if (err > 0)  { x -= 1; err -= 2 * x + 1; }
    }
}

void WS2812M_FillCircle(WS2812M_Instance* inst, int16_t x0, int16_t y0,
                        uint8_t radius, uint8_t r, uint8_t g, uint8_t b) {
    int16_t x = radius;
    int16_t y = 0;
    int16_t err = 0;

    if (inst == NULL) return;
    if (!inst->initialized) return;

    while (x >= y) {
        WS2812M_DrawLine(inst, x0 - x, y0 + y, x0 + x, y0 + y, r, g, b);
        WS2812M_DrawLine(inst, x0 - y, y0 + x, x0 + y, y0 + x, r, g, b);
        WS2812M_DrawLine(inst, x0 - x, y0 - y, x0 + x, y0 - y, r, g, b);
        WS2812M_DrawLine(inst, x0 - y, y0 - x, x0 + y, y0 - x, r, g, b);

        if (err <= 0) { y += 1; err += 2 * y + 1; }
        if (err > 0)  { x -= 1; err -= 2 * x + 1; }
    }
}
