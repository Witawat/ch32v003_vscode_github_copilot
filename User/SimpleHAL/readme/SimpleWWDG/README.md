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

---

### Refresh

#### `void WWDG_Refresh(uint8_t counter)`

ต้อง refresh ด้วยค่า counter ใหม่ **เมื่ออยู่ใน window เท่านั้น**

```c
WWDG_Refresh(0x7F);  // reload counter กลับ max
```

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

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| Reset ทันทีเมื่อเปิด | Refresh ก่อน window เปิด | ตรวจสอบ counter/window ให้ถูกต้อง |
| Timeout สั้นเกินไป | Prescaler เล็ก + counter ต่ำ | ใช้ `WWDG_PRESCALER_8` และ `counter=0x7F` |
| ต่างจาก IWDG | ต้อง refresh ในช่วงเวลาเฉพาะ | คำนวณ window time ให้ตรงกับ loop period |
| ใช้ WWDG กับ sleep | PCLK หยุดใน sleep mode | ใช้ IWDG แทนถ้าต้องการ sleep |
