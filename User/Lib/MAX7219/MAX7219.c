/**
 * @file SimpleMAX7219.c
 * @brief MAX7219 LED Matrix Driver Library Implementation
 * @version 1.0
 * @date 2025-12-21
 */

#include "MAX7219.h"
#include "max7219_fonts.h"
#include <stdlib.h>

/* ========== Private Variables ========== */

static MAX7219_Handle g_max7219_handle;  // Global handle instance

/* ========== Private Helper Functions ========== */

/**
 * @brief ส่งข้อมูลไปยัง MAX7219 ผ่าน SPI
 */
static void MAX7219_WriteRegister(MAX7219_Handle* handle, uint8_t reg, uint8_t data) {
    if (!handle) return;
    
    // Pull CS low
    digitalWrite(handle->cs_pin, LOW);
    Delay_Us(1);
    
    // Send register address and data for each device in cascade
    for (int i = 0; i < handle->num_devices; i++) {
        SPI_Transfer(reg);
        SPI_Transfer(data);
    }
    
    // Pull CS high to latch data
    digitalWrite(handle->cs_pin, HIGH);
    Delay_Us(1);
}

/**
 * @brief ส่งข้อมูลไปยัง device เฉพาะใน cascade
 * @note Currently unused but kept for future multi-device support
 */
__attribute__((unused))
static void MAX7219_WriteRegisterToDevice(MAX7219_Handle* handle, uint8_t device_idx, uint8_t reg, uint8_t data) {
    if (!handle || device_idx >= handle->num_devices) return;
    
    // Pull CS low
    digitalWrite(handle->cs_pin, LOW);
    Delay_Us(1);
    
    // Send data to specific device, NO-OP to others
    for (int i = handle->num_devices - 1; i >= 0; i--) {
        if (i == device_idx) {
            SPI_Transfer(reg);
            SPI_Transfer(data);
        } else {
            SPI_Transfer(MAX7219_REG_NOOP);
            SPI_Transfer(0x00);
        }
    }
    
    // Pull CS high to latch data
    digitalWrite(handle->cs_pin, HIGH);
    Delay_Us(1);
}

/**
 * @brief Swap function for sorting
 */
static void swap_int16(int16_t* a, int16_t* b) {
    int16_t temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * @brief Absolute value
 */
static int16_t abs_int16(int16_t x) {
    return (x < 0) ? -x : x;
}

/* ========== Initialization Functions ========== */

MAX7219_Handle* MAX7219_Init(uint8_t clk_pin, uint8_t mosi_pin, uint8_t cs_pin, uint8_t num_devices) {
    if (num_devices == 0 || num_devices > MAX7219_MAX_DEVICES) {
        return NULL;
    }
    
    MAX7219_Handle* handle = &g_max7219_handle;
    
    // Store configuration
    handle->clk_pin = clk_pin;
    handle->mosi_pin = mosi_pin;
    handle->cs_pin = cs_pin;
    handle->num_devices = num_devices;
    handle->intensity = 8;
    handle->display_on = true;
    handle->font = &font_5x7;  // Default font
    
    // Initialize animation state
    handle->animation.active = false;
    handle->animation.frames = NULL;
    handle->animation.num_frames = 0;
    handle->animation.current_frame = 0;
    handle->animation.frame_delay = 0;
    handle->animation.last_update = 0;
    handle->animation.loop = false;
    
    // Initialize scrolling state
    handle->scroll.active = false;
    handle->scroll.text = NULL;
    handle->scroll.offset = 0;
    handle->scroll.scroll_delay = 0;
    handle->scroll.last_update = 0;
    handle->scroll.vertical = false;
    handle->scroll.font = &font_5x7;
    
    // Clear display buffer
    memset(handle->buffer, 0, sizeof(handle->buffer));
    
    // Initialize SPI
    SPI_SimpleInit(SPI_MODE0, SPI_2MHZ, SPI_PINS_DEFAULT);
    
    // Initialize CS pin
    pinMode(cs_pin, PIN_MODE_OUTPUT);
    digitalWrite(cs_pin, HIGH);
    
    // Initialize MAX7219 registers for all devices
    MAX7219_WriteRegister(handle, MAX7219_REG_SCANLIMIT, 0x07);    // Scan all 8 digits
    MAX7219_WriteRegister(handle, MAX7219_REG_DECODE, 0x00);       // No decode mode
    MAX7219_WriteRegister(handle, MAX7219_REG_DISPLAYTEST, 0x00);  // Normal operation
    MAX7219_WriteRegister(handle, MAX7219_REG_INTENSITY, handle->intensity);
    MAX7219_WriteRegister(handle, MAX7219_REG_SHUTDOWN, 0x01);     // Normal operation (not shutdown)
    
    // Clear display
    MAX7219_Clear(handle, true);
    
    return handle;
}

void MAX7219_SetIntensity(MAX7219_Handle* handle, uint8_t intensity) {
    if (!handle) return;
    
    if (intensity > MAX7219_INTENSITY_MAX) {
        intensity = MAX7219_INTENSITY_MAX;
    }
    
    handle->intensity = intensity;
    MAX7219_WriteRegister(handle, MAX7219_REG_INTENSITY, intensity);
}

void MAX7219_DisplayControl(MAX7219_Handle* handle, bool on) {
    if (!handle) return;
    
    handle->display_on = on;
    MAX7219_WriteRegister(handle, MAX7219_REG_SHUTDOWN, on ? 0x01 : 0x00);
}

void MAX7219_Clear(MAX7219_Handle* handle, bool update) {
    if (!handle) return;
    
    // Clear buffer
    memset(handle->buffer, 0, sizeof(handle->buffer));
    
    // Update hardware if requested
    if (update) {
        MAX7219_Update(handle);
    }
}

void MAX7219_Update(MAX7219_Handle* handle) {
    if (!handle) return;
    
    // Send each row to all devices
    for (uint8_t row = 0; row < MAX7219_MATRIX_SIZE; row++) {
        digitalWrite(handle->cs_pin, LOW);
        Delay_Us(1);
        
        // Send row data for each device (reverse order for cascading)
        for (int dev = handle->num_devices - 1; dev >= 0; dev--) {
            SPI_Transfer(MAX7219_REG_DIGIT0 + row);
            SPI_Transfer(handle->buffer[dev][row]);
        }
        
        digitalWrite(handle->cs_pin, HIGH);
        Delay_Us(1);
    }
}

void MAX7219_DisplayTest(MAX7219_Handle* handle, bool test) {
    if (!handle) return;
    
    MAX7219_WriteRegister(handle, MAX7219_REG_DISPLAYTEST, test ? 0x01 : 0x00);
}

/* ========== Graphics Primitives ========== */

void MAX7219_SetPixel(MAX7219_Handle* handle, int16_t x, int16_t y, bool on) {
    if (!handle) return;
    
    // Calculate which device and local x coordinate
    uint8_t device_idx = x / MAX7219_MATRIX_SIZE;
    uint8_t local_x = x % MAX7219_MATRIX_SIZE;
    
    // Check bounds
    if (device_idx >= handle->num_devices || y < 0 || y >= MAX7219_MATRIX_SIZE) {
        return;
    }
    
    // Set or clear pixel in buffer
    if (on) {
        handle->buffer[device_idx][y] |= (1 << (7 - local_x));
    } else {
        handle->buffer[device_idx][y] &= ~(1 << (7 - local_x));
    }
}

bool MAX7219_GetPixel(MAX7219_Handle* handle, int16_t x, int16_t y) {
    if (!handle) return false;
    
    // Calculate which device and local x coordinate
    uint8_t device_idx = x / MAX7219_MATRIX_SIZE;
    uint8_t local_x = x % MAX7219_MATRIX_SIZE;
    
    // Check bounds
    if (device_idx >= handle->num_devices || y < 0 || y >= MAX7219_MATRIX_SIZE) {
        return false;
    }
    
    return (handle->buffer[device_idx][y] & (1 << (7 - local_x))) != 0;
}

void MAX7219_DrawLine(MAX7219_Handle* handle, int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool on) {
    if (!handle) return;
    
    // Bresenham's line algorithm
    int16_t dx = abs_int16(x1 - x0);
    int16_t dy = abs_int16(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;
    
    while (1) {
        MAX7219_SetPixel(handle, x0, y0, on);
        
        if (x0 == x1 && y0 == y1) break;
        
        int16_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void MAX7219_DrawRect(MAX7219_Handle* handle, int16_t x, int16_t y, int16_t w, int16_t h, bool filled, bool on) {
    if (!handle) return;
    
    if (filled) {
        // Draw filled rectangle
        for (int16_t i = 0; i < h; i++) {
            for (int16_t j = 0; j < w; j++) {
                MAX7219_SetPixel(handle, x + j, y + i, on);
            }
        }
    } else {
        // Draw rectangle outline
        MAX7219_DrawLine(handle, x, y, x + w - 1, y, on);           // Top
        MAX7219_DrawLine(handle, x, y + h - 1, x + w - 1, y + h - 1, on);  // Bottom
        MAX7219_DrawLine(handle, x, y, x, y + h - 1, on);           // Left
        MAX7219_DrawLine(handle, x + w - 1, y, x + w - 1, y + h - 1, on);  // Right
    }
}

void MAX7219_DrawCircle(MAX7219_Handle* handle, int16_t x0, int16_t y0, int16_t r, bool filled, bool on) {
    if (!handle) return;
    
    // Bresenham's circle algorithm
    int16_t x = 0;
    int16_t y = r;
    int16_t d = 3 - 2 * r;
    
    while (x <= y) {
        if (filled) {
            // Draw horizontal lines to fill circle
            MAX7219_DrawLine(handle, x0 - x, y0 + y, x0 + x, y0 + y, on);
            MAX7219_DrawLine(handle, x0 - x, y0 - y, x0 + x, y0 - y, on);
            MAX7219_DrawLine(handle, x0 - y, y0 + x, x0 + y, y0 + x, on);
            MAX7219_DrawLine(handle, x0 - y, y0 - x, x0 + y, y0 - x, on);
        } else {
            // Draw circle outline (8 octants)
            MAX7219_SetPixel(handle, x0 + x, y0 + y, on);
            MAX7219_SetPixel(handle, x0 - x, y0 + y, on);
            MAX7219_SetPixel(handle, x0 + x, y0 - y, on);
            MAX7219_SetPixel(handle, x0 - x, y0 - y, on);
            MAX7219_SetPixel(handle, x0 + y, y0 + x, on);
            MAX7219_SetPixel(handle, x0 - y, y0 + x, on);
            MAX7219_SetPixel(handle, x0 + y, y0 - x, on);
            MAX7219_SetPixel(handle, x0 - y, y0 - x, on);
        }
        
        x++;
        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            y--;
            d = d + 4 * (x - y) + 10;
        }
    }
}

void MAX7219_DrawTriangle(MAX7219_Handle* handle, int16_t x0, int16_t y0, int16_t x1, int16_t y1, 
                          int16_t x2, int16_t y2, bool filled, bool on) {
    if (!handle) return;
    
    if (filled) {
        // Sort vertices by y coordinate
        if (y0 > y1) { swap_int16(&y0, &y1); swap_int16(&x0, &x1); }
        if (y1 > y2) { swap_int16(&y1, &y2); swap_int16(&x1, &x2); }
        if (y0 > y1) { swap_int16(&y0, &y1); swap_int16(&x0, &x1); }
        
        // Draw filled triangle using horizontal lines
        int16_t total_height = y2 - y0;
        for (int16_t i = 0; i < total_height; i++) {
            bool second_half = i > y1 - y0 || y1 == y0;
            int16_t segment_height = second_half ? y2 - y1 : y1 - y0;
            
            if (segment_height == 0) continue;
            
            float alpha = (float)i / total_height;
            float beta = (float)(i - (second_half ? y1 - y0 : 0)) / segment_height;
            
            int16_t ax = x0 + (x2 - x0) * alpha;
            int16_t bx = second_half ? x1 + (x2 - x1) * beta : x0 + (x1 - x0) * beta;
            
            if (ax > bx) swap_int16(&ax, &bx);
            
            for (int16_t j = ax; j <= bx; j++) {
                MAX7219_SetPixel(handle, j, y0 + i, on);
            }
        }
    } else {
        // Draw triangle outline
        MAX7219_DrawLine(handle, x0, y0, x1, y1, on);
        MAX7219_DrawLine(handle, x1, y1, x2, y2, on);
        MAX7219_DrawLine(handle, x2, y2, x0, y0, on);
    }
}

void MAX7219_DrawBitmap(MAX7219_Handle* handle, int16_t x, int16_t y, const uint8_t* bitmap, 
                        uint8_t width, uint8_t height) {
    if (!handle || !bitmap) return;
    
    for (uint8_t row = 0; row < height; row++) {
        uint8_t byte_data = bitmap[row];
        for (uint8_t col = 0; col < width; col++) {
            bool pixel_on = (byte_data & (1 << (7 - col))) != 0;
            MAX7219_SetPixel(handle, x + col, y + row, pixel_on);
        }
    }
}

/* ========== Text Functions ========== */

void MAX7219_SetFont(MAX7219_Handle* handle, const MAX7219_Font* font) {
    if (!handle || !font) return;
    handle->font = font;
}

uint8_t MAX7219_DrawChar(MAX7219_Handle* handle, int16_t x, int16_t y, char ch) {
    if (!handle || !handle->font) return 0;
    
    const MAX7219_Font* font = handle->font;
    
    // Check if character is in font range
    if (ch < font->first_char || ch > font->last_char) {
        return 0;
    }
    
    // Calculate character index
    uint16_t char_idx = ch - font->first_char;
    uint16_t data_offset = char_idx * font->width;
    
    // Draw character
    for (uint8_t col = 0; col < font->width; col++) {
        uint8_t col_data = font->data[data_offset + col];
        for (uint8_t row = 0; row < font->height; row++) {
            bool pixel_on = (col_data & (1 << row)) != 0;
            MAX7219_SetPixel(handle, x + col, y + row, pixel_on);
        }
    }
    
    return font->width + 1;  // Return character width + spacing
}

uint16_t MAX7219_DrawString(MAX7219_Handle* handle, int16_t x, int16_t y, const char* text) {
    if (!handle || !text) return 0;
    
    uint16_t cursor_x = x;
    
    while (*text) {
        uint8_t char_width = MAX7219_DrawChar(handle, cursor_x, y, *text);
        cursor_x += char_width;
        text++;
    }
    
    return cursor_x - x;  // Return total width
}

uint16_t MAX7219_GetStringWidth(MAX7219_Handle* handle, const char* text) {
    if (!handle || !text || !handle->font) return 0;
    
    uint16_t width = 0;
    const MAX7219_Font* font = handle->font;
    
    while (*text) {
        if (*text >= font->first_char && *text <= font->last_char) {
            width += font->width + 1;  // Character width + spacing
        }
        text++;
    }
    
    return width > 0 ? width - 1 : 0;  // Remove last spacing
}

/* ========== Scrolling Functions ========== */

void MAX7219_StartScrollText(MAX7219_Handle* handle, const char* text, uint16_t scroll_delay) {
    if (!handle || !text) return;
    
    handle->scroll.text = text;
    handle->scroll.offset = handle->num_devices * MAX7219_MATRIX_SIZE;  // Start from right
    handle->scroll.scroll_delay = scroll_delay;
    handle->scroll.last_update = Get_CurrentMs();
    handle->scroll.active = true;
    handle->scroll.vertical = false;
    handle->scroll.font = handle->font;
}

bool MAX7219_UpdateScroll(MAX7219_Handle* handle) {
    if (!handle || !handle->scroll.active) return false;
    
    uint32_t current_time = Get_CurrentMs();
    
    // Check if it's time to update
    if (current_time - handle->scroll.last_update < handle->scroll.scroll_delay) {
        return true;  // Still scrolling, but not time to update yet
    }
    
    handle->scroll.last_update = current_time;
    
    // Clear display
    MAX7219_Clear(handle, false);
    
    // Draw text at current offset
    MAX7219_DrawString(handle, handle->scroll.offset, 0, handle->scroll.text);
    
    // Update display
    MAX7219_Update(handle);
    
    // Move offset left
    handle->scroll.offset--;
    
    // Check if scrolling is complete
    int16_t text_width = MAX7219_GetStringWidth(handle, handle->scroll.text);
    if (handle->scroll.offset < -text_width) {
        handle->scroll.active = false;
        return false;  // Scrolling complete
    }
    
    return true;  // Still scrolling
}

void MAX7219_StopScroll(MAX7219_Handle* handle) {
    if (!handle) return;
    handle->scroll.active = false;
}

/* ========== Animation Functions ========== */

void MAX7219_StartAnimation(MAX7219_Handle* handle, const uint8_t** frames, uint8_t num_frames, 
                           uint16_t frame_delay, bool loop) {
    if (!handle || !frames || num_frames == 0) return;
    
    handle->animation.frames = frames;
    handle->animation.num_frames = num_frames;
    handle->animation.current_frame = 0;
    handle->animation.frame_delay = frame_delay;
    handle->animation.last_update = Get_CurrentMs();
    handle->animation.loop = loop;
    handle->animation.active = true;
}

bool MAX7219_UpdateAnimation(MAX7219_Handle* handle) {
    if (!handle || !handle->animation.active) return false;
    
    uint32_t current_time = Get_CurrentMs();
    
    // Check if it's time to update
    if (current_time - handle->animation.last_update < handle->animation.frame_delay) {
        return true;  // Still animating, but not time to update yet
    }
    
    handle->animation.last_update = current_time;
    
    // Display current frame
    const uint8_t* frame = handle->animation.frames[handle->animation.current_frame];
    MAX7219_DrawBitmap(handle, 0, 0, frame, 8, 8);
    MAX7219_Update(handle);
    
    // Move to next frame
    handle->animation.current_frame++;
    
    // Check if animation is complete
    if (handle->animation.current_frame >= handle->animation.num_frames) {
        if (handle->animation.loop) {
            handle->animation.current_frame = 0;  // Loop back to first frame
        } else {
            handle->animation.active = false;
            return false;  // Animation complete
        }
    }
    
    return true;  // Still animating
}

void MAX7219_StopAnimation(MAX7219_Handle* handle) {
    if (!handle) return;
    handle->animation.active = false;
}

/* ========== Sprite Functions ========== */

void MAX7219_DrawSprite(MAX7219_Handle* handle, int16_t x, int16_t y, const uint8_t* sprite, 
                       const uint8_t* mask, uint8_t width, uint8_t height) {
    if (!handle || !sprite) return;
    
    for (uint8_t row = 0; row < height; row++) {
        uint8_t sprite_data = sprite[row];
        uint8_t mask_data = mask ? mask[row] : 0xFF;  // Default: all pixels visible
        
        for (uint8_t col = 0; col < width; col++) {
            bool sprite_pixel = (sprite_data & (1 << (7 - col))) != 0;
            bool mask_pixel = (mask_data & (1 << (7 - col))) != 0;
            
            // Only draw if mask allows
            if (mask_pixel) {
                MAX7219_SetPixel(handle, x + col, y + row, sprite_pixel);
            }
        }
    }
}

/* ========== Utility Functions ========== */

void MAX7219_FadeIn(MAX7219_Handle* handle, uint16_t duration) {
    if (!handle) return;
    
    uint16_t steps = MAX7219_INTENSITY_MAX + 1;
    uint16_t delay_per_step = duration / steps;
    
    for (uint8_t i = 0; i <= MAX7219_INTENSITY_MAX; i++) {
        MAX7219_SetIntensity(handle, i);
        Delay_Ms(delay_per_step);
    }
}

void MAX7219_FadeOut(MAX7219_Handle* handle, uint16_t duration) {
    if (!handle) return;
    
    uint16_t steps = MAX7219_INTENSITY_MAX + 1;
    uint16_t delay_per_step = duration / steps;
    
    for (int8_t i = MAX7219_INTENSITY_MAX; i >= 0; i--) {
        MAX7219_SetIntensity(handle, i);
        Delay_Ms(delay_per_step);
    }
}

void MAX7219_Invert(MAX7219_Handle* handle) {
    if (!handle) return;
    
    // Invert all pixels in buffer
    for (uint8_t dev = 0; dev < handle->num_devices; dev++) {
        for (uint8_t row = 0; row < MAX7219_MATRIX_SIZE; row++) {
            handle->buffer[dev][row] = ~handle->buffer[dev][row];
        }
    }
}
