# MAX7219 LED Matrix Driver Library

**เวอร์ชัน:** 1.0  
**วันที่:** 2025-12-21  
**License:** MIT

## ภาพรวม

Library สำหรับควบคุม MAX7219 LED Matrix (8x8) บน CH32V003 พร้อมฟีเจอร์ครบถ้วนตั้งแต่ขั้นพื้นฐานถึงขั้นสูง

## คุณสมบัติหลัก

- ✅ **ใช้งานง่าย** - Arduino-style API
- ✅ **Graphics Primitives** - Line, Rectangle, Circle, Triangle
- ✅ **Text Rendering** - ASCII + Thai fonts (5x7, 8x8)
- ✅ **Scrolling Text** - Horizontal scrolling (non-blocking)
- ✅ **Animation System** - Frame-based animation
- ✅ **Sprite Support** - Transparency mask
- ✅ **Cascaded Displays** - รองรับ 1-8 matrices
- ✅ **Effects** - Fade in/out, brightness control
- ✅ **14 ตัวอย่าง** - จากพื้นฐานถึงขั้นสูง
- ✅ **เอกสารภาษาไทย** - ครบถ้วน 1000+ บรรทัด

## Quick Start

```c
#include "SimpleMAX7219.h"

int main(void) {
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เริ่มต้น MAX7219
    MAX7219_Handle* display = MAX7219_Init(PC5, PC6, PC4, 1);
    MAX7219_SetIntensity(display, 8);
    
    // แสดงข้อความ
    MAX7219_DrawString(display, 0, 0, "HI");
    MAX7219_Update(display);
    
    while(1) {}
}
```

## การต่อวงจร

```
CH32V003          MAX7219
--------          -------
PC5 (CLK)      -> CLK
PC6 (MOSI)     -> DIN
PC4 (CS)       -> CS
GND            -> GND
5V             -> VCC
```

## ตัวอย่างโค้ด

### 1. Basic Display (01_basic_display.c)
- เริ่มต้นใช้งาน
- แสดงรูปแบบต่างๆ
- ควบคุมความสว่าง

### 2. Text Display (02_text_display.c)
- แสดงตัวอักษร ASCII
- แสดงข้อความ
- นับเลข

### 3. Graphics Primitives (03_graphics_primitives.c)
- วาดเส้น, สี่เหลี่ยม, วงกลม, สามเหลี่ยม
- Animation

### 4. Brightness Control (04_brightness_control.c)
- Fade in/out
- Pulse effect
- Breathing effect

### 5. Scrolling Text (05_scrolling_text.c)
- เลื่อนข้อความแนวนอน
- Non-blocking

### 6. Thai Text (06_thai_text.c)
- แสดงตัวอักษรไทย
- Font ภาษาไทย

### 7. Animation (07_animation.c)
- Frame-based animation
- Bouncing ball
- Rotating patterns

### 8. Sprite Display (08_sprite_display.c)
- แสดง sprite
- Transparency
- Movement animation

### 9. Multiple Matrix (09_multiple_matrix.c)
- Cascaded displays (4 matrices)
- ข้อความยาว
- Wave animation

### 10. Snake Game (10_game_snake.c)
- เกมงูกินอาหาร
- ควบคุมด้วยปุ่ม
- Collision detection

### 11. Clock Display (11_clock_display.c)
- นาฬิกาดิจิตอล
- แสดงเวลา HH:MM
- Scrolling messages

### 12. Spectrum Analyzer (12_spectrum_analyzer.c)
- แสดงระดับเสียง
- Bar graph
- ADC input

### 13. Weather Station (13_weather_station.c)
- แสดงไอคอนสภาพอากาศ
- อุณหภูมิ
- Scrolling text

### 14. Custom Animation (14_custom_animation.c)
- Plasma effect
- Ripple effect
- Matrix rain
- Starfield

## API Reference

### Initialization
- `MAX7219_Init()` - เริ่มต้น display
- `MAX7219_SetIntensity()` - ตั้งความสว่าง (0-15)
- `MAX7219_DisplayControl()` - เปิด/ปิด display
- `MAX7219_Clear()` - ล้างหน้าจอ
- `MAX7219_Update()` - อัพเดท hardware

### Graphics
- `MAX7219_SetPixel()` - ตั้งค่า pixel
- `MAX7219_DrawLine()` - วาดเส้น
- `MAX7219_DrawRect()` - วาดสี่เหลี่ยม
- `MAX7219_DrawCircle()` - วาดวงกลม
- `MAX7219_DrawTriangle()` - วาดสามเหลี่ยม
- `MAX7219_DrawBitmap()` - แสดง bitmap

### Text
- `MAX7219_SetFont()` - เลือก font
- `MAX7219_DrawChar()` - แสดงตัวอักษร
- `MAX7219_DrawString()` - แสดงข้อความ
- `MAX7219_GetStringWidth()` - คำนวณความกว้าง

### Scrolling
- `MAX7219_StartScrollText()` - เริ่มเลื่อนข้อความ
- `MAX7219_UpdateScroll()` - อัพเดทการเลื่อน
- `MAX7219_StopScroll()` - หยุดการเลื่อน

### Animation
- `MAX7219_StartAnimation()` - เริ่ม animation
- `MAX7219_UpdateAnimation()` - อัพเดท animation
- `MAX7219_StopAnimation()` - หยุด animation

### Sprite
- `MAX7219_DrawSprite()` - วาด sprite พร้อม transparency

### Effects
- `MAX7219_FadeIn()` - Fade in effect
- `MAX7219_FadeOut()` - Fade out effect
- `MAX7219_Invert()` - สลับ on/off ทุก pixel

## Fonts

Library มี 3 fonts:
- **font_5x7** - ASCII 5x7 (ประหยัดพื้นที่)
- **font_8x8** - ASCII 8x8 (ใช้พื้นที่เต็ม)
- **font_thai_5x7** - ตัวอักษรไทย 5x7

## เอกสารเพิ่มเติม

📖 **[MAX7219_Documentation_TH.md](MAX7219_Documentation_TH.md)** - เอกสารภาษาไทยฉบับสมบูรณ์ (1000+ บรรทัด)

เนื้อหาครอบคลุม:
- บทนำและฮาร์ดแวร์
- การติดตั้งและเริ่มต้นใช้งาน
- การใช้งานขั้นพื้นฐาน
- การแสดงข้อความ
- กราฟิกขั้นสูง
- Animation และ Sprite
- เทคนิคขั้นสูง
- API Reference ครบถ้วน
- การแก้ปัญหา
- ตัวอย่างโปรเจกต์

## การติดตั้ง

1. คัดลอกโฟลเดอร์ `MAX7219` ไปยัง `/User/Lib/`
2. Include ใน project:
```c
#include "SimpleMAX7219.h"
```

## ข้อกำหนดระบบ

- **MCU:** CH32V003
- **Dependencies:** SimpleSPI, SimpleGPIO, Timer
- **RAM:** ~1KB (สำหรับ 1 matrix)
- **Flash:** ~8KB

## License

MIT License - ใช้งานได้อย่างอิสระ

## ผู้พัฒนา

CH32V003 SimpleHAL Team

## การสนับสนุน

หากพบปัญหาหรือมีคำถาม:
1. ดูเอกสาร [MAX7219_Documentation_TH.md](MAX7219_Documentation_TH.md)
2. ดูตัวอย่างใน `Examples/`
3. ตรวจสอบ [การแก้ปัญหา](MAX7219_Documentation_TH.md#การแก้ปัญหา)

---

**Happy Coding! 🚀**
