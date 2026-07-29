# SimpleIWDG — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `SimpleIWDG.h / SimpleIWDG.c`

---

## ภาพรวม

SimpleIWDG ให้ใช้งาน Independent Watchdog (IWDG) ซึ่งเป็น hardware timer ที่ใช้ LSI clock อิสระ (ไม่ขึ้นกับ system clock) ถ้าไม่ได้เรียก `IWDG_Feed()` ก่อนหมดเวลา MCU จะ reset อัตโนมัติ ใช้สำหรับ **ตรวจจับ firmware hang / deadlock**

---

## คุณสมบัติ

- LSI Clock: ~40kHz (ค่าจริงอาจอยู่ระหว่าง 30–60kHz)
- Timeout range: **1ms ถึง 32,768ms** (~32.7 วินาที)
- เมื่อเริ่มแล้ว **หยุดไม่ได้** — ต้อง feed ตลอด
- Reset จาก IWDG จะเซ็ต flag `RCC->RSTSCKR` bit `IWDGRSTF`

---

## Prescaler Options

| Enum | Divider | ช่วง Timeout (approx.) |
|------|:-------:|------------------------|
| `IWDG_PRESCALER_4`   | /4   | 0.4ms – 409ms |
| `IWDG_PRESCALER_8`   | /8   | 0.8ms – 819ms |
| `IWDG_PRESCALER_16`  | /16  | 1.6ms – 1.6s |
| `IWDG_PRESCALER_32`  | /32  | 3.2ms – 3.2s |
| `IWDG_PRESCALER_64`  | /64  | 6.4ms – 6.5s |
| `IWDG_PRESCALER_128` | /128 | 12.8ms – 13s |
| `IWDG_PRESCALER_256` | /256 | 25.6ms – 26.2s |

---

## Helper Macros

```c
// คำนวณ timeout จาก prescaler + reload (approximate)
// LSI = 40kHz, reload range: 0-4095
IWDG_TIMEOUT_MS(prescaler, reload)

// คำนวณ reload value จาก prescaler + target ms
IWDG_CALC_RELOAD(prescaler, ms)
```

---

## API Reference

### Simple Init

#### `void IWDG_SimpleInit(uint32_t timeout_ms)`

ตั้งค่า IWDG โดยอัตโนมัติ — เลือก prescaler ที่เหมาะสมเอง

```c
IWDG_SimpleInit(1000);   // reset ถ้าไม่ feed ใน 1 วินาที
IWDG_SimpleInit(5000);   // 5 วินาที
IWDG_SimpleInit(100);    // 100ms (เข้มงวดมาก)
```

---

### Feed (Kick)

#### `void IWDG_Feed(void)`

รีเซ็ต watchdog counter — ต้องเรียกก่อนหมดเวลา

```c
IWDG_Feed();  // "เรายังทำงานอยู่"
```

---

### Status & Reset Cause

#### `uint8_t IWDG_IsBusy(void)` — ตรวจสอบว่า IWDG พร้อมรับคำสั่ง config หรือไม่

**คืนค่า:** `uint8_t` — `1` ถ้ากำลัง busy (PVU หรือ RVU flag ยังไม่เคลียร์), `0` ถ้าพร้อม

```c
while (IWDG_IsBusy());  // รอจนกว่าจะพร้อม
IWDG_Init(IWDG_PRESCALER_256, 4095);
```

> ⚠️ ข้อควรระวัง: ต้องรอ `IWDG_IsBusy() == 0` ก่อนเปลี่ยน prescaler หรือ reload ทุกครั้ง

#### `void IWDG_Init(uint8_t prescaler, uint16_t reload)` — ตั้งค่า IWDG แบบ manual (advanced init)

| พารามิเตอร์ | ชนิด | คำอธิบาย |
|------------|------|----------|
| `prescaler` | `uint8_t` | Prescaler จาก enum `IWDG_PRESCALER_*` |
| `reload` | `uint16_t` | ค่า reload (0–4095) |

```c
// timeout ≈ (reload + 1) × prescaler / LSI_freq
// ตัวอย่าง: ~1 วินาที
IWDG_Init(IWDG_PRESCALER_256, 156);
```

#### `uint32_t IWDG_GetTimeout(uint8_t prescaler, uint16_t reload)` — คำนวณ timeout เป็นมิลลิวินาที

| พารามิเตอร์ | ชนิด | คำอธิบาย |
|------------|------|----------|
| `prescaler` | `uint8_t` | Prescaler จาก enum `IWDG_PRESCALER_*` |
| `reload` | `uint16_t` | ค่า reload |

**คืนค่า:** `uint32_t` — timeout โดยประมาณในหน่วย ms

```c
uint32_t ms = IWDG_GetTimeout(IWDG_PRESCALER_128, 4095);
USART_Print("Timeout: "); USART_PrintNum((int32_t)ms); USART_Print(" ms\r\n");
```

#### `uint8_t IWDG_WasResetCause(void)` — ตรวจสอบว่า MCU ถูกรีเซ็ตโดย IWDG หรือไม่

**คืนค่า:** `uint8_t` — `1` ถ้าการรีเซ็ตล่าสุดเกิดจาก IWDG timeout, `0` ถ้าไม่ใช่

```c
if (IWDG_WasResetCause()) {
    USART_Print("Previous reset: Watchdog timeout!\r\n");
}
```

> ⚠️ ข้อควรระวัง: ต้องเรียก `IWDG_WasResetCause()` **ก่อน** `IWDG_Init()` หรือ `IWDG_SimpleInit()` เพราะ flag จะถูกล้างระหว่าง init

#### `void IWDG_ClearResetFlag(void)` — ล้าง reset flag ทุกตัว (ไม่ใช่แค่ IWDG)

```c
if (IWDG_WasResetCause()) {
    IWDG_ClearResetFlag();
}
```

> ⚠️ **ข้อจำกัดฮาร์ดแวร์:** WCH SDK ไม่มี selective clear — ฟังก์ชันนี้เรียก `RCC_ClearFlag()`
> ซึ่งเคลียร์ reset flag **ทุกตัวพร้อมกัน** (POR, PIN, Software, IWDG, WWDG, LowPower) ไม่ใช่
> แค่ `IWDGRSTF` เท่านั้น ถ้าต้องตรวจสอบสาเหตุ reset อื่นด้วย ให้เรียก `RCC_GetFlagStatus()`
> ของ flag อื่นๆ **ก่อน** เรียกฟังก์ชันนี้ (ดูตัวอย่าง "ขั้นสูง — ตรวจสอบสาเหตุ Reset" ด้านล่าง)

---

### Manual Init

#### `void IWDG_Init(uint8_t prescaler, uint16_t reload)`

ตั้งค่าเอง

```c
// timeout ≈ (reload + 1) × prescaler / LSI_freq
// ตัวอย่าง: 1000ms = 4095 × 8 / 40000 ≈ 819ms
IWDG_Init(IWDG_PRESCALER_8, 4095);
```

---

## ตัวอย่างการใช้งาน

### ขั้นต้น — Watchdog พื้นฐาน

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    USART_Print("Starting...\r\n");

    IWDG_SimpleInit(2000);  // reset ถ้าไม่ feed ใน 2 วินาที

    USART_Print("IWDG started. Feed every <2s\r\n");

    while (1) {
        // ทำงานหลัก
        USART_Print("Working...\r\n");

        IWDG_Feed();       // ป้องกัน reset

        Delay_Ms(1000);    // delay 1s < timeout 2s → OK
    }
}
```

### ขั้นกลาง — Watchdog กับ Task ที่อาจค้าง

```c
#include "SimpleHAL.h"

// ฟังก์ชันที่อาจค้าง (เช่น รอรับข้อมูล USART)
uint8_t receive_data_timeout(uint8_t* buf, uint8_t len, uint32_t timeout_ms) {
    uint32_t start = Get_CurrentMs();
    uint8_t i = 0;
    while (i < len) {
        if ((Get_CurrentMs() - start) > timeout_ms) return 0;  // timeout
        if (USART_Available()) {
            buf[i++] = USART_Read();
        }
    }
    return 1;
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    IWDG_SimpleInit(5000);   // 5 วินาที timeout

    uint8_t buf[8];
    while (1) {
        // ถ้า receive_data ค้างนานกว่า 5s → IWDG reset MCU
        if (receive_data_timeout(buf, 8, 3000)) {
            // ประมวลผล buf
            USART_Print("Got data\r\n");
        } else {
            USART_Print("RX timeout\r\n");
        }

        IWDG_Feed();  // ยืนยันว่ายังทำงานอยู่
    }
}
```

### ขั้นสูง — ตรวจสอบสาเหตุ Reset

```c
#include "SimpleHAL.h"
#include "ch32v00x.h"

void check_reset_cause(void) {
    uint32_t csr = RCC->RSTSCKR;

    if (csr & RCC_IWDGRSTF) {
        USART_Print("RESET: Watchdog timeout!\r\n");
    } else if (csr & RCC_PINRSTF) {
        USART_Print("RESET: NRST pin\r\n");
    } else if (csr & RCC_PORRSTF) {
        USART_Print("RESET: Power-On Reset\r\n");
    } else if (csr & RCC_SFTRSTF) {
        USART_Print("RESET: Software Reset\r\n");
    }

    // เคลียร์ flag
    RCC->RSTSCKR |= RCC_RMVF;
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    check_reset_cause();

    IWDG_SimpleInit(3000);

    while (1) {
        // simulate task
        Delay_Ms(2500);
        IWDG_Feed();
    }
}
```

---

## ข้อควรระวัง

> **⚡ v2.0:** reload ถูก clamp ≥ 1 — ป้องกัน reload=0 (ตีความเป็นค่าสูงสุด)  
> LSI tolerance ±25% — timeout เป็นค่าประมาณ ควรเลือก timeout ที่มี margin

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| MCU reset ตลอด | ลืม Feed / task ช้าเกิน | เพิ่ม `IWDG_Feed()` ในทุก loop iteration ที่สำคัญ |
| Timeout ไม่แม่นยำ | LSI ±25% variation | เผื่อ margin 2x จาก timeout จริงที่ต้องการ |
| Feed ใน ISR | ควรหลีกเลี่ยง | Feed ใน main loop ดีกว่า เพราะ ISR อาจทำงานแม้ main ค้าง |
| เริ่ม IWDG แล้วหยุดไม่ได้ | IWDG เป็น hardware วงจรอิสระ | ทดสอบให้ครบก่อน enable IWDG |
