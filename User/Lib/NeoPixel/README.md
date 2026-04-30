# SimpleNeoPixel Library - คู่มือการใช้งานฉบับสมบูรณ์

> **WS2812/WS2812B RGB LED Library สำหรับ CH32V003**  
> เวอร์ชัน 2.0 - อัพเดท 2025-12-21

---

## 📑 สารบัญ

1. [บทนำ](#1-บทนำ)
2. [Hardware Setup](#2-hardware-setup)
3. [Quick Start](#3-quick-start)
4. [API Reference](#4-api-reference)
5. [Tutorials](#5-tutorials)
6. [Techniques](#6-techniques)
7. [Troubleshooting](#7-troubleshooting)
8. [Project Examples](#8-project-examples)

---

## 1. บทนำ

### 1.1 WS2812 คืออะไร?

WS2812 (หรือที่รู้จักในชื่อ NeoPixel) เป็น RGB LED แบบ addressable ที่สามารถควบคุมสีและความสว่างของแต่ละดวงได้อย่างอิสระ โดยใช้สัญญาณข้อมูลเพียงสายเดียว

**คุณสมบัติ:**
- ควบคุมด้วยสัญญาณเดียว (Single-wire protocol)
- แต่ละ LED มี IC ควบคุมในตัว
- สามารถต่อเรียงกันได้หลายดวง (Daisy-chain)
- สี RGB 24-bit (16.7 ล้านสี)
- ความสว่างปรับได้

### 1.2 หลักการทำงาน

WS2812 ใช้การส่งข้อมูลแบบ bit-banging โดยมี timing ที่เฉพาะเจาะจง:

```
Bit 0: HIGH 0.4µs, LOW 0.85µs
Bit 1: HIGH 0.8µs, LOW 0.45µs
Reset: LOW > 50µs
```

ลำดับข้อมูล: **GRB** (Green-Red-Blue) ไม่ใช่ RGB!

### 1.3 ข้อดีข้อเสีย

**ข้อดี:**
- ✅ ควบคุมง่าย ใช้ pin เดียว
- ✅ สีสันสดใส ปรับได้หลากหลาย
- ✅ ราคาถูก
- ✅ ต่อเรียงได้หลายดวง

**ข้อเสีย:**
- ❌ ต้องการ timing ที่แม่นยำ
- ❌ ใช้ไฟมาก (60mA/LED ที่สว่างเต็ม)
- ❌ ต้องปิด interrupt ขณะส่งข้อมูล
- ❌ ไวต่อแรงดันไฟ (ต้องใช้ 5V)

---

## 2. Hardware Setup

### 2.1 การต่อวงจร

#### การต่อพื้นฐาน

```
CH32V003          WS2812 Strip
---------         ------------
PC4 (DATA) -----> DIN
GND ------------> GND
5V -------------> VCC
```

#### ข้อควรระวัง

1. **แรงดันไฟ:**
   - WS2812 ต้องการ 5V
   - CH32V003 ทำงานที่ 3.3V
   - ควรใช้ Level Shifter (แนะนำ 74HCT245)

2. **กระแสไฟ:**
   - แต่ละ LED ใช้สูงสุด ~60mA (สีขาวเต็ม)
   - 8 LEDs = 480mA สูงสุด
   - ใช้ Power Supply แยก สำหรับ LED strip ที่ยาว

3. **Capacitor:**
   - ใส่ 1000µF capacitor ที่ขา VCC-GND ของ strip
   - ป้องกัน voltage spike

4. **Resistor:**
   - ใส่ 330Ω resistor ที่สาย DATA
   - ป้องกันสัญญาณรบกวน

#### วงจรแบบสมบูรณ์

```
                    330Ω
CH32V003  ----/\/\/\-----> Level    -----> WS2812
  PC4                      Shifter          DIN
                          (74HCT245)
  
  GND  ----------------------+-----------> GND
                             |
  5V   ---[1000µF]----------+-----------> VCC
           Cap
```

---

## 3. Quick Start

### 3.1 การติดตั้ง Library

1. Copy ไฟล์ทั้งหมดใน `/User/Lib/NeoPixel/` ไปยังโปรเจค
2. Include header file:

```c
#include "NeoPixel.h"
```

### 3.2 ตัวอย่างแรก

```c
#include "main.h"
#include "Lib/NeoPixel/NeoPixel.h"

#define NUM_LEDS 8
#define DATA_PIN GPIO_Pin_4

int main(void) {
    // เริ่มต้นระบบ
    SystemCoreClockUpdate();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    Timer_Init();
    
    // เริ่มต้น NeoPixel
    NeoPixel_Init(GPIOC, GPIO_Pin_4, NUM_LEDS);
    
    // ตั้งค่าสี LED ดวงแรกเป็นสีแดง
    NeoPixel_SetPixelColor(0, 255, 0, 0);
    
    // แสดงผล
    NeoPixel_Show();
    
    while(1) {
        Delay_Ms(1000);
    }
}
```

---

## 4. API Reference

### 4.1 Initialization Functions

#### `NeoPixel_Init()`
```c
void NeoPixel_Init(GPIO_TypeDef* port, uint16_t pin, uint16_t num_leds);
```
เริ่มต้นการใช้งาน NeoPixel

### 4.2 Basic Functions

- `NeoPixel_SetPixelColor(pixel, r, g, b)` : ตั้งค่าสีของ LED ดวงที่ระบุ
- `NeoPixel_SetPixelColor32(pixel, color)` : ตั้งค่าสีด้วย 32-bit color
- `NeoPixel_Show()` : อัพเดทการแสดงผล (ต้องเรียกหลัง Set color)
- `NeoPixel_Clear()` : ดับ LEDs ทั้งหมด (ต้องเรียก Show() ด้วย)
- `NeoPixel_Fill(r, g, b)` : ตั้งค่าสีเดียวกันให้ทุก LEDs
- `NeoPixel_SetBrightness(brightness)` : ตั้งค่าความสว่างทั้งหมด (0-255)

---
**พัฒนาโดย:** CH32V003 Library Team
**รองรับบอร์ด:** CH32V003 Development Board
