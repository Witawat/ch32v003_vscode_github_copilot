# SimpleDelay — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `SimpleDelay.h / SimpleDelay.c`

---

## ภาพรวม

SimpleDelay ให้ฟังก์ชัน delay แบบ blocking และ non-blocking timer พร้อม API สำหรับอ่านเวลา (millis/micros) ทั้งหมดใช้ SysTick เป็น time base ที่ความละเอียด 1ms

---

## API Reference

### Initialization

#### `void Timer_Init(void)`

เริ่มต้น SysTick timer — **ต้องเรียกก่อนใช้งานฟังก์ชันอื่นทุกตัว**

```c
int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();  // <-- บรรทัดที่สองเสมอ
}
```

---

### Blocking Delay

#### `void Delay_Ms(uint32_t n)`

หน่วงเวลา n milliseconds — **หยุดรอ (บล็อก)**

```c
Delay_Ms(1000);   // 1 วินาที
Delay_Ms(500);    // 500ms
Delay_Ms(1);      // 1ms (ต่ำสุดที่มีความหมาย)
```

#### `void Delay_Us(uint32_t n)`

หน่วงเวลา n microseconds — ความละเอียดสูง ~1µs

```c
Delay_Us(100);    // 100 microseconds
Delay_Us(10);     // 10 microseconds
Delay_Us(1);      // ~1 microsecond
```

> ใช้สำหรับ bit-bang protocol เช่น 1-Wire, DHT, bit timing

---

### Time Reading (millis / micros)

#### `uint32_t Get_CurrentMs(void)`

อ่านเวลาปัจจุบันในหน่วย ms นับจากเปิดเครื่อง — overflow หลัง ~49.7 วัน

```c
uint32_t now = Get_CurrentMs();  // เช่น 12345
```

#### `uint32_t Get_CurrentUs(void)`

อ่านเวลาในหน่วย microseconds — overflow หลัง ~71.6 นาที

```c
uint32_t us = Get_CurrentUs();
```

#### `uint32_t Get_TickMicros(void)`

อ่าน sub-millisecond offset (0–999 µs ใน tick ปัจจุบัน)  
ใช้คู่กับ `Get_CurrentMs()` เพื่อความละเอียดสูงสุด

```c
uint32_t ms  = Get_CurrentMs();
uint32_t sub = Get_TickMicros();  // 0-999 µs
```

---

### Non-blocking Timer (`Timer_t`)

ใช้แทน `Delay_Ms` เพื่อให้โปรแกรมไม่หยุดรอ — สามารถทำงานหลายอย่างพร้อมกันได้

#### โครงสร้าง

```c
typedef struct {
    uint32_t start_time;  // เวลาที่เริ่ม (ms)
    uint32_t duration;    // ระยะเวลา (ms)
    uint8_t  active;      // 1 = กำลังทำงาน
    uint8_t  repeat;      // 1 = วนซ้ำอัตโนมัติ
} Timer_t;
```

#### `void Start_Timer(Timer_t *timer, uint32_t ms, uint8_t repeat)`

```c
Timer_t t1;
Start_Timer(&t1, 500, 1);   // ทำงานซ้ำทุก 500ms
Start_Timer(&t1, 1000, 0);  // ทำงานครั้งเดียวหลัง 1s
```

#### `uint8_t Is_Timer_Expired(Timer_t *timer)`

คืน `1` ถ้าหมดเวลา  
- ถ้า `repeat=1` → reset อัตโนมัติ  
- ถ้า `repeat=0` → ปิด `active` หลัง expire

```c
if (Is_Timer_Expired(&t1)) {
    // ทำงานที่นี่
}
```

#### `void Reset_Timer(Timer_t *timer, uint8_t repeat)`

รีเซ็ตให้นับใหม่จากปัจจุบัน

```c
Reset_Timer(&t1, 1);
```

#### `void Stop_Timer(Timer_t *timer)`

หยุด timer

```c
Stop_Timer(&t1);
```

---

## ตัวอย่างการใช้งาน

### ขั้นต้น — Blocking Blink

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    pinMode(PC0, PIN_MODE_OUTPUT);

    while (1) {
        digitalWrite(PC0, HIGH);
        Delay_Ms(500);
        digitalWrite(PC0, LOW);
        Delay_Ms(500);
    }
}
```

### ขั้นกลาง — Non-blocking หลาย Task พร้อมกัน

```c
#include "SimpleHAL.h"

Timer_t led_timer;
Timer_t sensor_timer;
Timer_t heartbeat_timer;

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    pinMode(PC0, PIN_MODE_OUTPUT);
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    Start_Timer(&led_timer,       250,  1);  // LED toggle ทุก 250ms
    Start_Timer(&sensor_timer,    100,  1);  // อ่าน sensor ทุก 100ms
    Start_Timer(&heartbeat_timer, 1000, 1);  // heartbeat ทุก 1s

    while (1) {
        if (Is_Timer_Expired(&led_timer)) {
            digitalToggle(PC0);
        }

        if (Is_Timer_Expired(&sensor_timer)) {
            uint16_t adc = ADC_Read(ADC_CH_PA2);
            // ประมวลผล...
        }

        if (Is_Timer_Expired(&heartbeat_timer)) {
            USART_Print("OK\r\n");
        }
    }
}
```

### ขั้นสูง — Timeout Pattern (non-blocking wait)

```c
#include "SimpleHAL.h"

// รอจนกว่า pin จะเป็น LOW หรือหมด timeout
// return 1 = สำเร็จ, 0 = timeout
uint8_t wait_for_low(uint8_t pin, uint32_t timeout_ms) {
    uint32_t start = Get_CurrentMs();
    while (digitalRead(pin) == HIGH) {
        if ((Get_CurrentMs() - start) >= timeout_ms) {
            return 0;
        }
    }
    return 1;
}

// วัดความกว้างของ pulse (microseconds)
uint32_t pulse_width_us(uint8_t pin, uint8_t level, uint32_t timeout_us) {
    // รอ edge
    uint32_t start_wait = Get_CurrentUs();
    while (digitalRead(pin) != level) {
        if ((Get_CurrentUs() - start_wait) >= timeout_us) return 0;
    }
    // วัด pulse
    uint32_t t_start = Get_CurrentUs();
    while (digitalRead(pin) == level) {
        if ((Get_CurrentUs() - t_start) >= timeout_us) return 0;
    }
    return Get_CurrentUs() - t_start;
}
```

### ขั้นสูง — Profiling / วัดเวลา execution

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    uint32_t t0 = Get_CurrentUs();

    // โค้ดที่ต้องการวัดเวลา
    for (volatile uint32_t i = 0; i < 10000; i++);

    uint32_t elapsed = Get_CurrentUs() - t0;
    USART_Print("elapsed=");
    USART_PrintNum((int32_t)elapsed);
    USART_Print(" us\r\n");
}
```

---

## ข้อควรระวัง

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| `Delay_Ms` ไม่ทำงาน | ลืม `Timer_Init()` | เรียก `Timer_Init()` ต้น main |
| overflow ของ `Get_CurrentMs()` | ใช้ `>=` เปรียบเทียบตรงๆ | ใช้ `(now - start) >= duration` เสมอ |
| `Timer_t` ทำงานผิด | ประกาศ local ใน loop | ประกาศ `static` หรือ global |
| ห้ามใช้ `Delay_Ms` ใน ISR | บล็อก interrupt handler | ใช้ flag แล้วจัดการใน main loop |
| `Delay_Us` ไม่แม่นที่ n=1-2 | compiler optimization | ใช้ `volatile` loop หรือยอมรับ margin เล็กน้อย |
