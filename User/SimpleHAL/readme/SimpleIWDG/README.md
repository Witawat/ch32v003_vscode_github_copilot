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

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| MCU reset ตลอด | ลืม Feed / task ช้าเกิน | เพิ่ม `IWDG_Feed()` ในทุก loop iteration ที่สำคัญ |
| Timeout ไม่แม่นยำ | LSI ±25% variation | เผื่อ margin 2x จาก timeout จริงที่ต้องการ |
| Feed ใน ISR | ควรหลีกเลี่ยง | Feed ใน main loop ดีกว่า เพราะ ISR อาจทำงานแม้ main ค้าง |
| เริ่ม IWDG แล้วหยุดไม่ได้ | IWDG เป็น hardware วงจรอิสระ | ทดสอบให้ครบก่อน enable IWDG |
