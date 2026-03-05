/**
 * @file oled_fonts.c
 * @brief OLED Font Library Implementation
 * @version 1.0
 * @date 2025-12-21
 */

#include "oled_fonts.h"
#include "oled_graphics.h"
#include <stdio.h>

/* ========== Font Data (6x8) ========== */

// Font 6x8 - Basic ASCII (32-126)
static const uint8_t font_6x8_data[] = {
    // Space (32)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ! (33)
    0x00, 0x00, 0x5F, 0x00, 0x00, 0x00,
    // " (34)
    0x00, 0x07, 0x00, 0x07, 0x00, 0x00,
    // # (35)
    0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00,
    // $ (36)
    0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00,
    // % (37)
    0x23, 0x13, 0x08, 0x64, 0x62, 0x00,
    // & (38)
    0x36, 0x49, 0x55, 0x22, 0x50, 0x00,
    // ' (39)
    0x00, 0x05, 0x03, 0x00, 0x00, 0x00,
    // ( (40)
    0x00, 0x1C, 0x22, 0x41, 0x00, 0x00,
    // ) (41)
    0x00, 0x41, 0x22, 0x1C, 0x00, 0x00,
    // * (42)
    0x08, 0x2A, 0x1C, 0x2A, 0x08, 0x00,
    // + (43)
    0x08, 0x08, 0x3E, 0x08, 0x08, 0x00,
    // , (44)
    0x00, 0x50, 0x30, 0x00, 0x00, 0x00,
    // - (45)
    0x08, 0x08, 0x08, 0x08, 0x08, 0x00,
    // . (46)
    0x00, 0x60, 0x60, 0x00, 0x00, 0x00,
    // / (47)
    0x20, 0x10, 0x08, 0x04, 0x02, 0x00,
    // 0-9 (48-57)
    0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00,  // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, 0x00,  // 1
    0x42, 0x61, 0x51, 0x49, 0x46, 0x00,  // 2
    0x21, 0x41, 0x45, 0x4B, 0x31, 0x00,  // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, 0x00,  // 4
    0x27, 0x45, 0x45, 0x45, 0x39, 0x00,  // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00,  // 6
    0x01, 0x71, 0x09, 0x05, 0x03, 0x00,  // 7
    0x36, 0x49, 0x49, 0x49, 0x36, 0x00,  // 8
    0x06, 0x49, 0x49, 0x29, 0x1E, 0x00,  // 9
    // : ; < = > ? @ (58-64)
    0x00, 0x36, 0x36, 0x00, 0x00, 0x00,
    0x00, 0x56, 0x36, 0x00, 0x00, 0x00,
    0x00, 0x08, 0x14, 0x22, 0x41, 0x00,
    0x14, 0x14, 0x14, 0x14, 0x14, 0x00,
    0x41, 0x22, 0x14, 0x08, 0x00, 0x00,
    0x02, 0x01, 0x51, 0x09, 0x06, 0x00,
    0x32, 0x49, 0x79, 0x41, 0x3E, 0x00,
    // A-Z (65-90)
    0x7E, 0x11, 0x11, 0x11, 0x7E, 0x00,  // A
    0x7F, 0x49, 0x49, 0x49, 0x36, 0x00,  // B
    0x3E, 0x41, 0x41, 0x41, 0x22, 0x00,  // C
    0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00,  // D
    0x7F, 0x49, 0x49, 0x49, 0x41, 0x00,  // E
    0x7F, 0x09, 0x09, 0x01, 0x01, 0x00,  // F
    0x3E, 0x41, 0x41, 0x51, 0x32, 0x00,  // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00,  // H
    0x00, 0x41, 0x7F, 0x41, 0x00, 0x00,  // I
    0x20, 0x40, 0x41, 0x3F, 0x01, 0x00,  // J
    0x7F, 0x08, 0x14, 0x22, 0x41, 0x00,  // K
    0x7F, 0x40, 0x40, 0x40, 0x40, 0x00,  // L
    0x7F, 0x02, 0x04, 0x02, 0x7F, 0x00,  // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00,  // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00,  // O
    0x7F, 0x09, 0x09, 0x09, 0x06, 0x00,  // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00,  // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, 0x00,  // R
    0x46, 0x49, 0x49, 0x49, 0x31, 0x00,  // S
    0x01, 0x01, 0x7F, 0x01, 0x01, 0x00,  // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00,  // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00,  // V
    0x7F, 0x20, 0x18, 0x20, 0x7F, 0x00,  // W
    0x63, 0x14, 0x08, 0x14, 0x63, 0x00,  // X
    0x03, 0x04, 0x78, 0x04, 0x03, 0x00,  // Y
    0x61, 0x51, 0x49, 0x45, 0x43, 0x00,  // Z
    // [ \ ] ^ _ ` (91-96)
    0x00, 0x00, 0x7F, 0x41, 0x41, 0x00,
    0x02, 0x04, 0x08, 0x10, 0x20, 0x00,
    0x41, 0x41, 0x7F, 0x00, 0x00, 0x00,
    0x04, 0x02, 0x01, 0x02, 0x04, 0x00,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x00,
    0x00, 0x01, 0x02, 0x04, 0x00, 0x00,
    // a-z (97-122)
    0x20, 0x54, 0x54, 0x54, 0x78, 0x00,  // a
    0x7F, 0x48, 0x44, 0x44, 0x38, 0x00,  // b
    0x38, 0x44, 0x44, 0x44, 0x20, 0x00,  // c
    0x38, 0x44, 0x44, 0x48, 0x7F, 0x00,  // d
    0x38, 0x54, 0x54, 0x54, 0x18, 0x00,  // e
    0x08, 0x7E, 0x09, 0x01, 0x02, 0x00,  // f
    0x0C, 0x52, 0x52, 0x52, 0x3E, 0x00,  // g
    0x7F, 0x08, 0x04, 0x04, 0x78, 0x00,  // h
    0x00, 0x44, 0x7D, 0x40, 0x00, 0x00,  // i
    0x20, 0x40, 0x44, 0x3D, 0x00, 0x00,  // j
    0x00, 0x7F, 0x10, 0x28, 0x44, 0x00,  // k
    0x00, 0x41, 0x7F, 0x40, 0x00, 0x00,  // l
    0x7C, 0x04, 0x18, 0x04, 0x78, 0x00,  // m
    0x7C, 0x08, 0x04, 0x04, 0x78, 0x00,  // n
    0x38, 0x44, 0x44, 0x44, 0x38, 0x00,  // o
    0x7C, 0x14, 0x14, 0x14, 0x08, 0x00,  // p
    0x08, 0x14, 0x14, 0x18, 0x7C, 0x00,  // q
    0x7C, 0x08, 0x04, 0x04, 0x08, 0x00,  // r
    0x48, 0x54, 0x54, 0x54, 0x20, 0x00,  // s
    0x04, 0x3F, 0x44, 0x40, 0x20, 0x00,  // t
    0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00,  // u
    0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00,  // v
    0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00,  // w
    0x44, 0x28, 0x10, 0x28, 0x44, 0x00,  // x
    0x0C, 0x50, 0x50, 0x50, 0x3C, 0x00,  // y
    0x44, 0x64, 0x54, 0x4C, 0x44, 0x00,  // z
    // { | } ~ (123-126)
    0x00, 0x08, 0x36, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x7F, 0x00, 0x00, 0x00,
    0x00, 0x41, 0x36, 0x08, 0x00, 0x00,
    0x08, 0x04, 0x08, 0x10, 0x08, 0x00,
};

const OLED_Font Font_6x8 = {
    .width = 6,
    .height = 8,
    .first_char = 32,
    .last_char = 126,
    .data = font_6x8_data
};

/* Note: Font_8x16 and Font_12x16 would be defined similarly but with larger bitmaps */
/* For brevity, we'll create placeholder fonts that reference the 6x8 data */

const OLED_Font Font_8x16 = {
    .width = 8,
    .height = 16,
    .first_char = 32,
    .last_char = 126,
    .data = font_6x8_data  // Placeholder - would be actual 8x16 data
};

const OLED_Font Font_12x16 = {
    .width = 12,
    .height = 16,
    .first_char = 32,
    .last_char = 126,
    .data = font_6x8_data  // Placeholder - would be actual 12x16 data
};

/* ========== Global Variables ========== */

static const OLED_Font* current_font = &Font_6x8;

/* ========== Font Selection ========== */

void OLED_SetFont(OLED_Handle* oled, const OLED_Font* font) {
    (void)oled;  // Unused
    current_font = font;
}

const OLED_Font* OLED_GetFont(OLED_Handle* oled) {
    (void)oled;  // Unused
    return current_font;
}

/* ========== Character Drawing ========== */

uint8_t OLED_DrawChar(OLED_Handle* oled, uint8_t x, uint8_t y, char c, OLED_Color color) {
    if(c < current_font->first_char || c > current_font->last_char) {
        return 0;
    }
    
    uint8_t char_index = c - current_font->first_char;
    uint16_t offset = char_index * current_font->width;
    
    for(uint8_t i = 0; i < current_font->width; i++) {
        uint8_t line = current_font->data[offset + i];
        
        for(uint8_t j = 0; j < current_font->height; j++) {
            if(line & (1 << j)) {
                OLED_SetPixel(oled, x + i, y + j, color);
            } else if(color != OLED_COLOR_INVERT) {
                OLED_SetPixel(oled, x + i, y + j, OLED_COLOR_BLACK);
            }
        }
    }
    
    return current_font->width;
}

uint8_t OLED_DrawCharInverse(OLED_Handle* oled, uint8_t x, uint8_t y, char c) {
    // Draw background
    OLED_FillRect(oled, x, y, current_font->width, current_font->height, OLED_COLOR_WHITE);
    
    // Draw character in black
    return OLED_DrawChar(oled, x, y, c, OLED_COLOR_BLACK);
}

/* ========== String Drawing ========== */

uint16_t OLED_DrawString(OLED_Handle* oled, uint8_t x, uint8_t y, const char* str, OLED_Color color) {
    uint16_t total_width = 0;
    
    while(*str) {
        uint8_t char_width = OLED_DrawChar(oled, x + total_width, y, *str, color);
        total_width += char_width;
        str++;
    }
    
    return total_width;
}

void OLED_DrawStringAlign(OLED_Handle* oled, uint8_t x, uint8_t y, const char* str, OLED_Color color, OLED_TextAlign align) {
    uint16_t width = OLED_GetStringWidth(oled, str);
    uint8_t start_x = x;
    
    switch(align) {
        case OLED_ALIGN_CENTER:
            start_x = x - (width / 2);
            break;
        case OLED_ALIGN_RIGHT:
            start_x = x - width;
            break;
        case OLED_ALIGN_LEFT:
        default:
            start_x = x;
            break;
    }
    
    OLED_DrawString(oled, start_x, y, str, color);
}

uint16_t OLED_DrawStringInverse(OLED_Handle* oled, uint8_t x, uint8_t y, const char* str) {
    uint16_t total_width = 0;
    
    while(*str) {
        uint8_t char_width = OLED_DrawCharInverse(oled, x + total_width, y, *str);
        total_width += char_width;
        str++;
    }
    
    return total_width;
}

/* ========== Number Drawing ========== */

uint16_t OLED_DrawInt(OLED_Handle* oled, uint8_t x, uint8_t y, int32_t num, OLED_Color color) {
    char buffer[12];
    snprintf(buffer, sizeof(buffer), "%ld", (long)num);
    return OLED_DrawString(oled, x, y, buffer, color);
}

uint16_t OLED_DrawFloat(OLED_Handle* oled, uint8_t x, uint8_t y, float num, uint8_t decimals, OLED_Color color) {
    char buffer[16];
    char format[8];
    snprintf(format, sizeof(format), "%%.%df", decimals);
    snprintf(buffer, sizeof(buffer), format, num);
    return OLED_DrawString(oled, x, y, buffer, color);
}

/* ========== Text Measurement ========== */

uint16_t OLED_GetStringWidth(OLED_Handle* oled, const char* str) {
    (void)oled;  // Unused
    return strlen(str) * current_font->width;
}

uint8_t OLED_GetFontHeight(OLED_Handle* oled) {
    (void)oled;  // Unused
    return current_font->height;
}

/* ========== Advanced Text Features ========== */

void OLED_DrawMultiLine(OLED_Handle* oled, uint8_t x, uint8_t y, const char* str, OLED_Color color, uint8_t line_spacing) {
    uint8_t current_y = y;
    const char* line_start = str;
    
    while(*str) {
        if(*str == '\n' || *(str + 1) == '\0') {
            // Draw line
            char line[128];
            uint8_t len = str - line_start;
            if(*(str + 1) == '\0' && *str != '\n') len++;
            
            if(len > 0 && len < sizeof(line)) {
                memcpy(line, line_start, len);
                line[len] = '\0';
                OLED_DrawString(oled, x, current_y, line, color);
            }
            
            current_y += current_font->height + line_spacing;
            line_start = str + 1;
        }
        str++;
    }
}

void OLED_DrawScrollText(OLED_Handle* oled, uint8_t x, uint8_t y, uint8_t w, const char* str, OLED_Color color, uint16_t offset) {
    uint16_t text_width = OLED_GetStringWidth(oled, str);
    
    // Calculate scroll position
    int16_t scroll_x = x - (offset % (text_width + w));
    
    // Draw text at scrolled position
    OLED_DrawString(oled, scroll_x, y, str, color);
    
    // Draw again for seamless loop
    if(scroll_x + text_width < x + w) {
        OLED_DrawString(oled, scroll_x + text_width + w, y, str, color);
    }
}

void OLED_DrawTextBox(OLED_Handle* oled, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char* str, OLED_Color color, OLED_TextAlign align) {
    // Draw box
    OLED_DrawRect(oled, x, y, w, h, color);
    
    // Calculate text position
    uint8_t text_y = y + (h - current_font->height) / 2;
    uint8_t text_x = x + w / 2;
    
    if(align == OLED_ALIGN_LEFT) {
        text_x = x + 2;
    } else if(align == OLED_ALIGN_RIGHT) {
        text_x = x + w - 2;
    }
    
    OLED_DrawStringAlign(oled, text_x, text_y, str, color, align);
}

/* ========== UTF-8 Helper Functions ========== */

uint8_t OLED_UTF8ToUnicode(const char* utf8, uint16_t* unicode) {
    uint8_t c = (uint8_t)*utf8;
    
    if(c < 0x80) {
        // 1-byte character (ASCII)
        *unicode = c;
        return 1;
    } else if((c & 0xE0) == 0xC0) {
        // 2-byte character
        *unicode = ((c & 0x1F) << 6) | (utf8[1] & 0x3F);
        return 2;
    } else if((c & 0xF0) == 0xE0) {
        // 3-byte character
        *unicode = ((c & 0x0F) << 12) | ((utf8[1] & 0x3F) << 6) | (utf8[2] & 0x3F);
        return 3;
    } else if((c & 0xF8) == 0xF0) {
        // 4-byte character
        *unicode = ((c & 0x07) << 18) | ((utf8[1] & 0x3F) << 12) | ((utf8[2] & 0x3F) << 6) | (utf8[3] & 0x3F);
        return 4;
    }
    
    return 0;
}

uint8_t OLED_IsThaiChar(uint16_t unicode) {
    // Thai Unicode range: 0x0E00 - 0x0E7F
    return (unicode >= 0x0E00 && unicode <= 0x0E7F);
}

/* ========== Thai Text Support ========== */

// Note: Full Thai font implementation would require ~100 characters x 32 bytes = 3.2KB
// This is a placeholder implementation

uint16_t OLED_DrawStringThai(OLED_Handle* oled, uint8_t x, uint8_t y, const char* str, OLED_Color color) {
    // Placeholder - would implement full Thai rendering
    // For now, just draw a box to indicate Thai text
    uint16_t width = strlen(str) * 16;  // Assume 16px per Thai char
    OLED_DrawRect(oled, x, y, width, 16, color);
    return width;
}

uint16_t OLED_DrawStringMixed(OLED_Handle* oled, uint8_t x, uint8_t y, const char* str, OLED_Color color) {
    uint16_t total_width = 0;
    const char* p = str;
    
    while(*p) {
        uint16_t unicode;
        uint8_t bytes = OLED_UTF8ToUnicode(p, &unicode);
        
        if(OLED_IsThaiChar(unicode)) {
            // Draw Thai character (placeholder)
            OLED_DrawRect(oled, x + total_width, y, 16, 16, color);
            total_width += 16;
        } else {
            // Draw ASCII character
            total_width += OLED_DrawChar(oled, x + total_width, y, *p, color);
        }
        
        p += bytes;
    }
    
    return total_width;
}
