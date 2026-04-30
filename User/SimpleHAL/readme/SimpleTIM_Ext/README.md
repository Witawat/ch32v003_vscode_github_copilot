# SimpleTIM_Ext — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `SimpleTIM_Ext.h / SimpleTIM_Ext.c`

---

## ภาพรวม

SimpleTIM_Ext ให้ฟังก์ชัน **Stopwatch** และ **Countdown** สำเร็จรูป โดยใช้ TIM2 ที่ 1000Hz (ความละเอียด 1ms) พร้อมฟังก์ชัน format เวลาเป็น string สำหรับแสดงผล

> **หมายเหตุ:** SimpleTIM_Ext ใช้ TIM2 — ห้ามใช้ SimpleTIM `TIM_2` พร้อมกัน

---

## โครงสร้างข้อมูล

### `Time_t`

```c
typedef struct {
    uint16_t hours;    // ชั่วโมง (ไม่จำกัด)
    uint8_t  minutes;  // นาที 0-59
    uint8_t  seconds;  // วินาที 0-59
} Time_t;
```

---

## Buffer Sizes

```c
#define TIME_BUFFER_SIZE_SS    12   // "HH:MM:SS" + null
#define TIME_BUFFER_SIZE_MMSS  16   // "MMMM:SS" + null
#define TIME_BUFFER_SIZE_HHMMSS 20  // "HHHH:MM:SS" + null
```

---

## Time Formats

| Enum | ตัวอย่าง output |
|------|----------------|
| `TIME_FORMAT_SS`    | `"65"` (seconds only) |
| `TIME_FORMAT_MMSS`  | `"1:05"` |
| `TIME_FORMAT_HHMMSS`| `"0:01:05"` |

## Display Modes

| Enum | พฤติกรรม |
|------|---------|
| `TIME_DISPLAY_NORMALIZED` | 60s = 1min (ปกติ) |
| `TIME_DISPLAY_RAW`        | แสดงวินาทีแบบ raw ไม่แปลง |

---

## API Reference — Stopwatch

#### `void Stopwatch_Init(void)`

เริ่มต้น (ต้องเรียกก่อนใช้งาน)

#### `void Stopwatch_Start(void)`
#### `void Stopwatch_Stop(void)`
#### `void Stopwatch_Reset(void)`

```c
Stopwatch_Init();
Stopwatch_Start();
// ... ทำงาน
Stopwatch_Stop();
Stopwatch_Reset();
```

#### `uint8_t Stopwatch_IsRunning(void)`

คืน `1` ถ้ากำลังนับอยู่

#### `void Stopwatch_GetTime(Time_t* t)`

อ่านเวลาปัจจุบันลงใน struct

```c
Time_t t;
Stopwatch_GetTime(&t);
// t.hours, t.minutes, t.seconds
```

#### `uint32_t Stopwatch_GetTotalSeconds(void)`
#### `uint32_t Stopwatch_GetTotalMilliseconds(void)`

```c
uint32_t ms = Stopwatch_GetTotalMilliseconds();
uint32_t s  = Stopwatch_GetTotalSeconds();
```

#### `void Stopwatch_GetTimeString(char* buf, TIME_FORMAT fmt, TIME_DISPLAY mode)`

สร้าง string เวลาสำหรับแสดงผล

```c
char buf[TIME_BUFFER_SIZE_HHMMSS];
Stopwatch_GetTimeString(buf, TIME_FORMAT_HHMMSS, TIME_DISPLAY_NORMALIZED);
// buf = "0:01:23"
```

---

## API Reference — Countdown

ฟังก์ชัน mirror ของ Stopwatch สำหรับนับถอยหลัง

```c
void Countdown_Init(void);
void Countdown_Start(void);
void Countdown_Stop(void);
void Countdown_Reset(void);
void Countdown_SetTime(uint32_t seconds);      // ตั้งเวลา
uint8_t Countdown_IsRunning(void);
uint8_t Countdown_IsFinished(void);            // คืน 1 ถ้าหมดเวลา
void Countdown_GetTime(Time_t* t);
void Countdown_GetTimeString(char* buf, TIME_FORMAT fmt, TIME_DISPLAY mode);
uint32_t Countdown_GetRemainingSeconds(void);
uint32_t Countdown_GetRemainingMilliseconds(void);
```

---

## ตัวอย่างการใช้งาน

### ขั้นต้น — Stopwatch บน USART

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(PD2, PIN_MODE_INPUT_PULLUP);   // ปุ่ม Start/Stop

    Stopwatch_Init();

    uint8_t last_btn = HIGH;

    while (1) {
        uint8_t btn = digitalRead(PD2);
        if (btn == LOW && last_btn == HIGH) {  // falling edge = กดปุ่ม
            if (Stopwatch_IsRunning()) {
                Stopwatch_Stop();
            } else {
                Stopwatch_Start();
            }
            Delay_Ms(50);  // debounce
        }
        last_btn = btn;

        char buf[TIME_BUFFER_SIZE_HHMMSS];
        Stopwatch_GetTimeString(buf, TIME_FORMAT_HHMMSS, TIME_DISPLAY_NORMALIZED);
        USART_Print(buf);
        USART_Print("\r");
        Delay_Ms(100);
    }
}
```

### ขั้นกลาง — Countdown Timer พร้อม Buzzer

```c
#include "SimpleHAL.h"

#define BUZZER_PIN  PC3
#define COUNTDOWN_S 60   // 60 วินาที

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(BUZZER_PIN, PIN_MODE_OUTPUT);

    Countdown_Init();
    Countdown_SetTime(COUNTDOWN_S);
    Countdown_Start();

    USART_Print("Countdown started!\r\n");

    while (1) {
        if (Countdown_IsFinished()) {
            // เตือน 3 ครั้ง
            for (uint8_t i = 0; i < 3; i++) {
                digitalWrite(BUZZER_PIN, HIGH);
                Delay_Ms(200);
                digitalWrite(BUZZER_PIN, LOW);
                Delay_Ms(200);
            }
            USART_Print("TIME'S UP!\r\n");
            // รีเซ็ตและเริ่มใหม่
            Countdown_Reset();
            Countdown_SetTime(COUNTDOWN_S);
            Countdown_Start();
        }

        char buf[TIME_BUFFER_SIZE_MMSS];
        Countdown_GetTimeString(buf, TIME_FORMAT_MMSS, TIME_DISPLAY_NORMALIZED);
        USART_Print(buf);
        USART_Print("\r");
        Delay_Ms(200);
    }
}
```

### ขั้นสูง — Lap Timer (Stopwatch + split times)

```c
#include "SimpleHAL.h"

#define LAP_BTN   PD2
#define RESET_BTN PD3
#define MAX_LAPS  5

uint32_t lap_times[MAX_LAPS];
uint8_t lap_count = 0;
uint32_t last_lap_ms = 0;

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(LAP_BTN,   PIN_MODE_INPUT_PULLUP);
    pinMode(RESET_BTN, PIN_MODE_INPUT_PULLUP);

    Stopwatch_Init();
    Stopwatch_Start();

    uint8_t last_lap   = HIGH;
    uint8_t last_reset = HIGH;

    while (1) {
        uint8_t btn_lap   = digitalRead(LAP_BTN);
        uint8_t btn_reset = digitalRead(RESET_BTN);

        // กด Lap button
        if (btn_lap == LOW && last_lap == HIGH) {
            if (lap_count < MAX_LAPS) {
                uint32_t now = Stopwatch_GetTotalMilliseconds();
                lap_times[lap_count++] = now - last_lap_ms;
                last_lap_ms = now;

                USART_Print("LAP ");
                USART_PrintNum(lap_count);
                USART_Print(": ");
                USART_PrintNum((int32_t)(now - last_lap_ms + lap_times[lap_count-1]));
                USART_Print(" ms\r\n");
            }
            Delay_Ms(50);
        }

        // กด Reset
        if (btn_reset == LOW && last_reset == HIGH) {
            Stopwatch_Reset();
            Stopwatch_Start();
            lap_count  = 0;
            last_lap_ms = 0;
            USART_Print("RESET\r\n");
            Delay_Ms(50);
        }

        last_lap   = btn_lap;
        last_reset = btn_reset;

        char buf[TIME_BUFFER_SIZE_HHMMSS];
        Stopwatch_GetTimeString(buf, TIME_FORMAT_HHMMSS, TIME_DISPLAY_NORMALIZED);
        USART_Print(buf); USART_Print("\r");
        Delay_Ms(50);
    }
}
```

---

## ข้อควรระวัง

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| ใช้ `SimpleTIM TIM_2` พร้อมกันไม่ได้ | ทั้งคู่ใช้ TIM2 hardware | เลือกอย่างใดอย่างหนึ่ง |
| Stopwatch หยุดระหว่าง sleep | TIM2 ถูกปิดใน standby | ไม่รองรับ deep sleep ขณะนับ |
| `GetTimeString` buffer เล็กเกิน | buffer overflow | ใช้ขนาด buffer จาก define เสมอ |
| Countdown ไม่เตือน | ลืมเช็ค `IsFinished()` | ต้อง poll ใน main loop ทุก iteration |
