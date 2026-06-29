# เริ่มต้นใช้งาน SimpleHAL สำหรับ CH32V003

> **MCU:** CH32V003 (RISC-V 48MHz, 16KB Flash, 2KB RAM)
> **SimpleHAL Version:** 2.0 | **ภาษา:** C (gnu99)

---

## 1. Quick Start — โค้ดขั้นต่ำ

```c
#define CH32V003_PACKAGE  PACKAGE_TSSOP20   // เลือกแพ็กเกจของคุณ
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();   // ① ต้องเป็นบรรทัดแรกสุด
    Timer_Init();              // ② เริ่มต้น SysTick timer

    pinMode(PC0, PIN_MODE_OUTPUT);

    while (1) {
        digitalToggle(PC0);
        Delay_Ms(500);
    }
}
```

3 สิ่งที่ต้องทำเสมอ:
1. `#define CH32V003_PACKAGE` — เลือกแพ็กเกจ
2. `SystemCoreClockUpdate()` — อัปเดตความถี่ระบบ (บรรทัดแรก)
3. `Timer_Init()` — เริ่มต้น SysTick (บรรทัดที่สอง)

---

## 2. เลือกแพ็กเกจ (CH32V003_PACKAGE)

CH32V003 มี 4 แพ็กเกจที่มีจำนวนพินต่างกัน:

| แพ็กเกจ | กำหนดด้วย | จำนวนพิน |
|---------|----------|:--:|
| SOP-8 (J4M6) | `PACKAGE_SOP8` | 6 |
| SOP-16 (A4M6) | `PACKAGE_SOP16` | ~14 |
| TSSOP-20 (F4P6) | `PACKAGE_TSSOP20` | 18 |
| QFN-20 (F4U6) | `PACKAGE_QFN20` | 18 |

```c
// เลือกก่อน #include — ถ้าไม่เลือก default เป็น TSSOP20
#define CH32V003_PACKAGE  PACKAGE_SOP8
#include "SimpleHAL.h"
```

ถ้าไม่กำหนด → ได้ `#warning` + default `PACKAGE_TSSOP20`

ผลของการเลือกแพ็กเกจ:
- พินที่ไม่มีในแพ็กเกจ → `IS_VALID_PIN()` คืน false, `pinMode()` เงียบไม่ทำงาน
- ฟีเจอร์ที่ใช้พินที่ขาด → ไม่ถูกคอมไพล์ (SPI HW บน SOP-8)
- `ADC_SimpleInit()` กรอง channels อัตโนมัติ

---

## 3. SystemCoreClockUpdate() — ทำไมต้องเป็นบรรทัดแรก

`SystemCoreClockUpdate()` อ่านค่า system clock จริงจาก hardware registers ใส่ใน `SystemCoreClock` (ตัวแปร global uint32_t)

**ทุกอย่างที่ SimpleHAL ทำขึ้นกับค่านี้:**
- USART baud rate → ผิด → อ่านข้อมูลไม่ออก
- PWM frequency → ผิด → servo หมุนเกิน, LED ไม่ fade
- Delay timing → ผิด → Delay_Ms(1000) อาจไม่ได้ 1 วิ
- Timer period → ผิด → interrupt ไม่ตรงเวลา

```c
// ❌ ผิด — USART baud rate จะผิด!
USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
SystemCoreClockUpdate();  // เรียกทีหลัง → ไม่มีผล

// ✅ ถูก
SystemCoreClockUpdate();  // เรียกก่อนทุกอย่าง
USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
```

---

## 4. Timer_Init() — SysTick คือหัวใจของ SimpleDelay

`Timer_Init()` ตั้งค่า SysTick hardware timer ให้ทำงานที่ 1ms resolution

**สิ่งที่ใช้ SysTick:**
- `Delay_Ms()`, `Delay_Us()` — blocking delay
- `Get_CurrentMs()`, `Get_CurrentUs()` — อ่านเวลาปัจจุบัน (ใช้ใน `millis()`, `micros()`)
- `Timer_t` + `Start_Timer()`/`Is_Timer_Expired()` — non-blocking timer
- `yield()` — cooperative multitasking

**ถ้าไม่เรียก Timer_Init():**
- `Delay_Ms()` / `Delay_Us()` — คืน 0 ทันที (runtime guard ป้องกัน crash)
- `Get_CurrentMs()` / `Get_CurrentUs()` — คืน 0
- `Timer_t` — ไม่ทำงาน

```c
SystemCoreClockUpdate();  // ก่อน
Timer_Init();             // หลัง — เพราะ SysTick ใช้ SystemCoreClock
```

---

## 5. 3 วิธี Debug Output

### 5.1 SDI_Printf — printf ผ่าน WCH-Link

ส่งข้อความผ่าน SWIO (PD1) → WCH-Link programmer → MounRiver Studio console

```c
// ใน main.c
#define ENABLE_PRINTF  1   // 1 = เปิด, 0 = ปิด (ประหยัด Flash)

int main(void) {
    SystemCoreClockUpdate();

#if ENABLE_PRINTF
    SDI_Printf_Enable();   // ★ ต้องเรียกก่อน printf ครั้งแรก
#endif

    printf("Hello from CH32V003!\r\n");
    printf("ADC: %d\r\n", analogRead(PD2));
}
```

**หลักการทำงาน:**
```
printf("hello") → newlib → _write() (override ใน debug.c) →
  → เขียน register 0xE00000F4/0xE00000F8 →
  → WCH-Link อ่านผ่าน SWIO → console output
```

**ข้อดี:**
- ดูผลได้ใน MounRiver Studio โดยตรง
- ไม่ใช้ขา USART (TX/RX ว่างสำหรับงานอื่น)

**ข้อเสีย:**
- ต้องต่อ WCH-Link programmer (USB)
- ดูผลได้เฉพาะใน MounRiver Studio

**Flash overhead:** ~1.5KB

### 5.2 USART_Print — ผ่าน USB-to-Serial

```c
#include "SimpleHAL.h"

USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);  // PD5=TX, PD6=RX
USART_Print("Hello World!\r\n");
USART_PrintNum(12345);
USART_PrintHex(0xFF, 1);     // "0xFF"
USART_WriteByte(0x55);       // ไบนารี 1 byte
```

**ข้อดี:**
- USB-to-Serial converter ถูก หาง่าย
- ใช้ terminal ไหนก็ได้ (PuTTY, Arduino Serial Monitor, screen, minicom)
- ไม่ต้องต่อ programmer

**ข้อเสีย:**
- ใช้ 2 ขา (PD5, PD6)

**Flash overhead:** ~0.5KB

### 5.3 USART_Println / PrintFloat — Arduino-Style

```c
#define ENABLE_USART_PRINTLN   1    // เปิด USART_Println*
#define ENABLE_USART_PRINTFLOAT 1    // เปิด USART_PrintFloat*
#include "SimpleHAL.h"

USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

USART_Println("Hello");              // "Hello\r\n"
USART_PrintlnNum(42);                // "42\r\n"
USART_PrintlnHex(0xFF, 1);           // "0xFF\r\n"
USART_PrintFloat(3.14f, 2);         // "3.14"
USART_PrintlnFloat(2.718f, 3);      // "2.718\r\n"
```

**หมายเหตุ:**
- `ENABLE_USART_PRINTLN` และ `ENABLE_USART_PRINTFLOAT` default = 0 (ปิด)
- `USART_PrintFloat` ใช้ `dtostrf()` — integer-only conversion ไม่ใช้ FPU
- `USART_PrintlnFloat` ต้องเปิดทั้ง 2 macro

**Flash overhead:** PRINTLN ~0.3KB, PRINTFLOAT ~1.2KB

---

## 6. เปิด/ปิด Print — Compile-Time Macros

| Macro | Default | เปิดที่บรรทัด | เปิดแล้วได้อะไร |
|-------|:---:|------|------|
| `ENABLE_PRINTF` | 0 | `main.c` | `printf()` ผ่าน SDI/WCH-Link |
| `ENABLE_USART_PRINTLN` | 0 | ก่อน `#include` | `USART_Println*()` |
| `ENABLE_USART_PRINTFLOAT` | 0 | ก่อน `#include` | `USART_PrintFloat()` |

**วิธีปิดทั้งหมด (Release mode):**
```c
#define ENABLE_PRINTF           0
#define ENABLE_USART_PRINTLN    0
#define ENABLE_USART_PRINTFLOAT 0
```

> `ENABLE_PRINTF=0` → `printf()` ถูก macro เปลี่ยนเป็น `((void)0)` — **ไม่ใช้ Flash เลย**

---

## 7. Debug Pattern — เปิดตอนพัฒนา ปิดตอน Release

```c
#define DEBUG_MODE  1   // 1 = พัฒนา, 0 = release

#if DEBUG_MODE
    #define ENABLE_PRINTF          1
    #define ENABLE_USART_PRINTLN   1
    #define DBG_PRINT(x)           USART_Println(x)
    #define DBG_PRINT_VAL(name, val) \
        do { USART_Print(name); USART_PrintNum(val); USART_Print("\r\n"); } while(0)
#else
    #define ENABLE_PRINTF          0
    #define ENABLE_USART_PRINTLN   0
    #define DBG_PRINT(x)           ((void)0)
    #define DBG_PRINT_VAL(name, val) ((void)0)
#endif

#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    DBG_PRINT("System started");
    DBG_PRINT_VAL("  ADC: ", analogRead(PD2));
}
```

เปลี่ยน `DEBUG_MODE` เป็น 0 → ทุก `DBG_PRINT*` หายไป — ไม่ใช้ Flash และ RAM เลย

---

## 8. เลือกวิธี Debug แบบไหนดี

| สถานการณ์ | ใช้วิธี | Flash |
|-----------|--------|:---:|
| MounRiver Studio + WCH-Link | `SDI_Printf` (`printf`) | +1.5KB |
| USB-to-Serial + terminal | `USART_Print` | +0.5KB |
| Arduino comfort | `USART_Println` + `PrintFloat` | +1.5KB |
| Release / Flash ใกล้เต็ม | ปิดทุก print | 0KB |
| Debug เฉพาะจุด | `DBG_PRINT` macro | 0KB ใน release |
| แสดงค่า float | `USART_PrintFloat` (dtostrf) | +1.2KB |
| ไบนารี 1 byte | `USART_WriteByte` | +0KB |

---

## 9. ข้อผิดพลาดที่พบบ่อย

| อาการ | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| USART อ่านข้อมูลไม่ออก / ตัวอักษรเพี้ยน | ลืม `SystemCoreClockUpdate()` | เรียกเป็นบรรทัดแรกของ `main()` |
| `Delay_Ms` ไม่ทำงาน (กลับ 0 ทันที) | ลืม `Timer_Init()` | เรียกหลัง `SystemCoreClockUpdate()` |
| `#warning "CH32V003_PACKAGE not defined"` | ไม่ได้เลือกแพ็กเกจ | `#define CH32V003_PACKAGE PACKAGE_xxx` |
| `printf` ไม่มี output | ลืม `SDI_Printf_Enable()` | เรียกก่อน `printf` ครั้งแรก |
| `printf` ไม่มี output (release) | `ENABLE_PRINTF=0` | ต้อง `ENABLE_PRINTF=1` |
| USART pins ไม่ทำงาน | ลืม `pinMode()` | `USART_SimpleInit` จัดการให้เอง — ไม่ต้อง `pinMode` เอง |
| ใช้ SPI_REMAP บน SOP-8/SOP-16 | `#error` หยุดคอมไพล์ | ใช้ `SPI_PINS_DEFAULT` หรือ `shiftOut` |
| I2C ไม่ทำงานบน SOP-8 | PC1/PC2 อาจไม่มี | ใช้ `SimpleI2C_Soft` |
| ใช้ pin ที่ไม่มีจริง | `pinMode` เงียบไม่ทำงาน | ใช้ `IS_VALID_PIN()` ตรวจสอบก่อน |
| TIM1/TIM2 ชนกัน | ใช้ timer เดียวกันกับ PWM | ใช้ `SysTick` (`Timer_t`) แทน hardware timer |

---

## ดูเพิ่มเติม

- [SimpleHAL README](../User/SimpleHAL/readme/README.md) — ภาพรวมทุกโมดูล
- [Examples](../User/Examples/) — 88 ตัวอย่างการใช้งาน
- [CHANGELOG](CHANGELOG.md) — ประวัติการเปลี่ยนแปลง
- [KNOWLEDGE_BASE](KNOWLEDGE_BASE.md) — ความรู้ทางเทคนิค
