# SimpleWWDG — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `SimpleWWDG.h / SimpleWWDG.c`

---

## ภาพรวม

SimpleWWDG ควบคุม Window Watchdog (WWDG) ซึ่งต่างจาก IWDG ตรงที่มี **Window** — ต้อง refresh ภายใน **ช่วงเวลาที่กำหนด** (ไม่เร็วเกิน ไม่ช้าเกิน) ถ้า refresh ก่อนเวลาหรือหลังเวลา MCU จะ reset ทำให้ตรวจจับ timing error ของ firmware ได้

---

## หลักการทำงาน

```
Counter: 0x7F → ลดลงทุก clock → 0x40 → 0x3F → RESET!
              ↑                   ↑
           เริ่มต้น           Window boundary (early-warning interrupt)

การ Refresh ที่ถูกต้อง: ต้องทำเมื่อ window < counter < 0x40
                         (counter ยังอยู่ใน window)
```

- Counter range: **0x40 – 0x7F** (64–127)
- Window range: **0x40 – counter_initial**
- PCLK1 = 24MHz

---

## Prescaler Options

| Enum | Divider | หมายเหตุ |
|------|:-------:|---------|
| `WWDG_PRESCALER_1` | /1 | เร็วที่สุด |
| `WWDG_PRESCALER_2` | /2 | |
| `WWDG_PRESCALER_4` | /4 | |
| `WWDG_PRESCALER_8` | /8 | ช้าที่สุด |

---

## สูตรคำนวณ Timeout

```
T_wwdg (ms) = (counter - 0x3F) × prescaler × 4096 / PCLK1
             = (counter - 0x3F) × prescaler × 4096 / 24,000,000 × 1000
```

ตัวอย่าง: `counter=0x7F, prescaler=8`  
T = (0x7F - 0x3F) × 8 × 4096 / 24,000,000 × 1000 = 55ms

---

## API Reference

### Simple Init

#### `void WWDG_SimpleInit(uint8_t counter, uint8_t window)`

ใช้ `WWDG_PRESCALER_8` โดยค่าเริ่มต้น

```c
WWDG_SimpleInit(0x7F, 0x5F);
// counter=0x7F (max), window=0x5F
// Refresh ได้เมื่อ 0x5F < counter < 0x40
```

> **v2.1:** ถ้าตั้งค่า `window > counter` (config ผิดที่ทำให้ refresh ไม่มีทาง valid ได้เลย
> → MCU reset วนลูปถาวร) ตอนนี้ library จะ clamp `window` ให้ไม่เกิน `counter` อัตโนมัติแทน
> ปล่อยให้ config พังแบบเงียบๆ

---

### Refresh

#### `void WWDG_Refresh(uint8_t counter)`

ต้อง refresh ด้วยค่า counter ใหม่ **เมื่ออยู่ใน window เท่านั้น**

```c
WWDG_Refresh(0x7F);  // reload counter กลับ max
```

---

### Timeout Calculation

#### `uint32_t WWDG_CalcTimeout(uint32_t prescaler, uint8_t counter)` — คำนวณ timeout เป็น ms

| พารามิเตอร์ | ชนิด | คำอธิบาย |
|------------|------|----------|
| `prescaler` | `uint32_t` | Prescaler จาก enum `WWDG_PRESCALER_*` |
| `counter` | `uint8_t` | ค่า counter (0x40–0x7F) |

**คืนค่า:** `uint32_t` — timeout ในหน่วย ms

```c
uint32_t ms = WWDG_CalcTimeout(WWDG_PRESCALER_8, 0x7F);
// ms ≈ 55ms ที่ PCLK1=24MHz
```

---

### Interrupt Flag Management

#### `uint8_t WWDG_GetInterruptFlag(void)` — ตรวจสอบ interrupt flag (early warning)

**คืนค่า:** `uint8_t` — `1` ถ้า counter ถึง 0x40 และเกิด early warning interrupt, `0` ถ้ายังไม่เกิด

```c
if (WWDG_GetInterruptFlag()) {
    // counter ใกล้ถึง 0x3F แล้ว!
}
```

#### `void WWDG_ClearInterruptFlag(void)` — ล้าง interrupt flag

```c
if (WWDG_GetInterruptFlag()) {
    WWDG_ClearInterruptFlag();
}
```

---

### Control

#### `void WWDG_Disable(void)` — รีเซ็ต WWDG config registers (**ไม่ใช่การปิดจริง**)

```c
WWDG_Disable();
```

> ⚠️ **สำคัญ:** ตาม CH32V003 Reference Manual เมื่อ WDGA bit ถูกตั้งแล้ว (คือหลังเรียก
> `WWDG_Init()`/`WWDG_SimpleInit()`) **WWDG ปิดไม่ได้ด้วยซอฟต์แวร์อีกต่อไป** — เป็นข้อจำกัด
> ของฮาร์ดแวร์ ไม่ใช่บั๊กของ library ฟังก์ชันนี้แค่รีเซ็ต prescaler/window/counter registers
> แต่ counter จะยังคงนับถอยหลังต่อและ reset MCU ถ้าไม่ refresh ทัน (`SimpleIWDG` ก็มีข้อจำกัด
> เดียวกัน — watchdog ทั้งสองแบบออกแบบมาให้ enable แล้วปิดไม่ได้ ตามเจตนาด้าน safety) —
> ถ้ายังไม่มั่นใจเรื่อง timing ของโปรแกรม ให้ **เลื่อนการเรียก `WWDG_Init()`/`WWDG_SimpleInit()`
> ออกไปจนกว่าจะพร้อม refresh ตรงเวลาจริง** แทนที่จะพึ่ง `WWDG_Disable()`

#### `void WWDG_InitWithInterrupt(uint8_t counter, uint8_t window, uint8_t prescaler)` — เริ่ม WWDG พร้อมเปิด early warning interrupt

| พารามิเตอร์ | ชนิด | คำอธิบาย |
|------------|------|----------|
| `counter` | `uint8_t` | ค่าเริ่มต้น counter (0x40–0x7F) |
| `window` | `uint8_t` | ค่า window (0x40–0x7F) |
| `prescaler` | `uint8_t` | Prescaler จาก enum `WWDG_PRESCALER_*` |

```c
void wwdg_warning(void) {
    // ตั้ง flag ให้ main loop จัดการ
}

WWDG_SetCallback(wwdg_warning);                     // ตั้ง callback ก่อน
WWDG_InitWithInterrupt(0x7F, 0x50, WWDG_PRESCALER_8); // แล้วจึง init
```

> ⚠️ ข้อควรระวัง: ต้องเรียก `WWDG_SetCallback()` **ก่อน** `WWDG_InitWithInterrupt()` มิฉะนั้น callback จะไม่ถูกเรียกเมื่อเกิด early warning

---

### Manual Init

#### `void WWDG_Init(uint8_t counter, uint8_t window, uint8_t prescaler)`

```c
WWDG_Init(0x7F, 0x50, WWDG_PRESCALER_8);
```

---

### Interrupt (Early Warning)

#### `void WWDG_InitWithInterrupt(void)`
#### `void WWDG_SetCallback(void (*callback)(void))`

เรียก callback เมื่อ counter ถึง **0x40** (เตือนก่อน reset)

```c
void wwdg_warning(void) {
    // เวลาน้อยแล้ว! ทำงานฉุกเฉิน
    USART_Print("WWDG near reset!\r\n");
    WWDG_Refresh(0x7F);  // อาจ refresh ที่นี่ถ้าจำเป็น
}

WWDG_InitWithInterrupt();
WWDG_SetCallback(wwdg_warning);
```

---

## ตัวอย่างการใช้งาน

### ขั้นต้น — WWDG พื้นฐาน

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    // counter=0x7F, window=0x5F, prescaler=8
    // timeout ≈ 55ms
    // refresh ต้องทำก่อน 55ms และหลัง window crossing
    WWDG_SimpleInit(0x7F, 0x5F);

    USART_Print("WWDG started\r\n");

    while (1) {
        // ทำงาน ~20ms
        Delay_Ms(20);

        // Refresh (อยู่ใน window)
        WWDG_Refresh(0x7F);
    }
}
```

### ขั้นกลาง — WWDG พร้อม Early Warning Interrupt

```c
#include "SimpleHAL.h"

volatile uint8_t wwdg_alert = 0;

void wwdg_callback(void) {
    wwdg_alert = 1;
    // อย่าทำงานหนักใน callback นี้
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    WWDG_InitWithInterrupt();
    WWDG_SetCallback(wwdg_callback);
    WWDG_Init(0x7F, 0x50, WWDG_PRESCALER_8);

    while (1) {
        if (wwdg_alert) {
            wwdg_alert = 0;
            USART_Print("WWDG WARNING: refreshing\r\n");
            WWDG_Refresh(0x7F);
        }

        // ทำงานหลัก
        Delay_Ms(10);
    }
}
```

### ขั้นสูง — ใช้ WWDG ตรวจจับ Timing Violation

```c
// WWDG เหมาะสำหรับ real-time system ที่ต้องทำงานตามเวลาพอดี
// ถ้า task ช้าเกินไป (ไม่ refresh ใน window) → reset แสดงว่ามี timing bug

#include "SimpleHAL.h"

// Task ต้องทำงานครบภายใน 10-40ms
// WWDG window: ห้าม refresh ก่อน 10ms, ต้อง refresh ก่อน 55ms

Timer_t task_timer;

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    WWDG_Init(0x7F, 0x5F, WWDG_PRESCALER_8);  // 55ms max

    Start_Timer(&task_timer, 20, 1);  // ทำงานทุก 20ms

    while (1) {
        if (Is_Timer_Expired(&task_timer)) {
            // ทำงานหลัก ต้องไม่เกิน ~35ms
            uint16_t adc = ADC_Read(ADC_CH_PA2);
            USART_PrintNum(adc);
            USART_Print("\r\n");

            // Refresh หลังจากทำงาน (อยู่ใน window ≈ 20ms หลัง start)
            WWDG_Refresh(0x7F);
        }
    }
}
```

---

## เปรียบเทียบ IWDG vs WWDG

| คุณสมบัติ | IWDG | WWDG |
|-----------|:----:|:----:|
| Clock source | LSI (~40kHz) | PCLK1 (24MHz) |
| ความแม่นยำ | ต่ำ (±25%) | สูง |
| Window constraint | ไม่มี | มี (ห้าม refresh เร็วเกิน) |
| Timeout range | 1ms–32.7s | ~1-55ms |
| ใช้งานหลัก | ป้องกัน firmware hang | ป้องกัน timing violation |
| Early warning | ไม่มี | มี (ที่ counter=0x40) |

---

## ข้อควรระวัง

> **⚡ v2.0:** PCLK1 ใช้ `SystemCoreClock` runtime แทน hardcoded 24MHz — คำนวณ timeout ถูกต้องแม้เปลี่ยน system clock

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| Reset ทันทีเมื่อเปิด | Refresh ก่อน window เปิด | ตรวจสอบ counter/window ให้ถูกต้อง |
| Timeout สั้นเกินไป | Prescaler เล็ก + counter ต่ำ | ใช้ `WWDG_PRESCALER_8` และ `counter=0x7F` |
| ต่างจาก IWDG | ต้อง refresh ในช่วงเวลาเฉพาะ | คำนวณ window time ให้ตรงกับ loop period |
| ใช้ WWDG กับ sleep | PCLK หยุดใน sleep mode | ใช้ IWDG แทนถ้าต้องการ sleep |
