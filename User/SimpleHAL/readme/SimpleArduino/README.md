# SimpleArduino — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `SimpleArduino.h / SimpleArduino.c`

---

## ภาพรวม

SimpleArduino รวมฟังก์ชันและ macro ที่มีชื่อตรงกับ Arduino standard API เพื่อให้ผู้ที่เคยใช้ Arduino สามารถย้ายมาใช้ CH32V003 ได้สะดวก ไม่ต้องจำชื่อฟังก์ชันใหม่

---

## Arduino Time Aliases

| Arduino | SimpleHAL จริง | คำอธิบาย |
|---------|---------------|----------|
| `millis()` | `Get_CurrentMs()` | เวลาปัจจุบัน (ms) |
| `micros()` | `Get_CurrentUs()` | เวลาปัจจุบัน (µs) |
| `delay(ms)` | `Delay_Ms(ms)` | หน่วงเวลาแบบ blocking (ms) |
| `delayMicroseconds(us)` | `Delay_Us(us)` | หน่วงเวลาแบบ blocking (µs) |

```c
#include "SimpleHAL.h"

uint32_t start = millis();
delay(1000);
uint32_t elapsed = millis() - start;  // ~1000
```

## Interrupt Aliases

| Arduino | SimpleHAL จริง |
|---------|---------------|
| `interrupts()` | `__enable_irq()` |
| `noInterrupts()` | `__disable_irq()` |

## Pin Interrupt Mapping

#### `uint8_t digitalPinToInterrupt(uint8_t pin)`

แปลง GPIO pin → EXTI line number (0-7) สำหรับใช้กับ `attachInterrupt()`

```c
uint8_t line = digitalPinToInterrupt(PC1);  // → 1
attachInterrupt(PC1, my_isr, FALLING);
```

CH32V003 มี EXTI 8 lines (0-7) แชร์กันทุก pins

## Random Number Generator

#### `void randomSeed(uint32_t seed)`

ตั้งค่า seed สำหรับ PRNG (LCG algorithm)

```c
randomSeed(analogRead(PD2));  // ใช้ ADC noise เป็น seed
```

#### `long _randomMax(long max)`

สุ่มตัวเลขในช่วง `[0, max-1]`

```c
uint8_t dice = _randomMax(6) + 1;  // 1-6
```

#### `long _randomRange(long min, long max)`

สุ่มตัวเลขในช่วง `[min, max-1]`

```c
long val = _randomRange(100, 200);  // 100-199
```

> ⚠️ ใช้ชื่อ `_randomMax` / `_randomRange` เพราะ `random` ชนกับ `stdlib.h`
> ⚠️ ต้องเรียก `randomSeed()` ก่อน `_randomMax`/`_randomRange` ไม่งั้นจะได้ค่าเดิมทุกครั้งที่รีเซ็ต

## yield() — Cooperative Multitasking

#### `void yield(void)`

ให้ CPU ทำงานพื้นหลัง — ถ้า IWDG ถูกเปิดใช้งาน จะ feed watchdog อัตโนมัติ

```c
while (1) {
    if (Is_Timer_Expired(&my_timer)) {
        digitalToggle(PC0);
    }
    yield();  // feed IWDG + future background tasks
}
```

> การ feed IWDG จะทำงานเฉพาะเมื่อเรียก `IWDG_Init()` หรือ `IWDG_SimpleInit()` ก่อน — `yield()` ปลอดภัยที่จะเรียกแม้ไม่ได้เปิด IWDG

## dtostrf() — Float to String (Lightweight)

#### `char* dtostrf(double val, int width, unsigned int precision, char* buf)`

แปลง float/double เป็น string โดยใช้ integer arithmetic ล้วน — ไม่ใช้ FPU, ไม่พึ่ง `sprintf` (ประหยัด Flash ~2KB)

```c
char buf[16];
dtostrf(3.14159, 6, 2, buf);  // "  3.14"
USART_Print(buf);
```

## USART Print Extensions (Optional)

เปิด/ปิดด้วย define ก่อน include:

```c
#define ENABLE_USART_PRINTLN   1  // เปิด USART_Println
#define ENABLE_USART_PRINTFLOAT 1  // เปิด USART_PrintFloat
#include "SimpleHAL.h"
```

| ฟังก์ชัน | คำอธิบาย |
|----------|----------|
| `USART_Println(str)` | ส่ง string + `\r\n` |
| `USART_PrintlnNum(num)` | ส่งตัวเลข + `\r\n` |
| `USART_PrintlnHex(num, uppercase)` | ส่ง hex + `\r\n` |
| `USART_PrintFloat(val, decimal_places)` | ส่ง float |
| `USART_PrintlnFloat(val, decimal_places)` | ส่ง float + `\r\n` |

> ⚠️ `USART_PrintFloat` ใช้ `dtostrf()` → ต้องเปิดทั้งสอง macro

## ข้อควรระวัง

| ปัญหา | วิธีแก้ |
|-------|--------|
| `random()` ชนกับ stdlib.h | ใช้ `_randomMax()` / `_randomRange()` |
| `yield()` ไม่ feed IWDG | ต้องเรียก `IWDG_Init()` ก่อน — `arduino_SetIWDGActive()` จะถูกเรียกอัตโนมัติ |
| `dtostrf` buffer เล็กเกิน | แนะนำ `width + 2` bytes |
| `delay()` ใน ISR | ห้ามใช้ — ISR ควรสั้นที่สุด ใช้ flag + main loop |
