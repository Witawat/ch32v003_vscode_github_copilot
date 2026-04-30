# SimpleTIM — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `SimpleTIM.h / SimpleTIM.c`

---

## ภาพรวม

SimpleTIM ให้ใช้งาน TIM1 และ TIM2 เป็น **periodic interrupt timer** ปรับ frequency ได้ตั้งแต่ Hz จนถึง MHz พร้อม callback function และ manual register access ผ่าน `TIM_AdvancedInit`

> ถ้าต้องการเพียงแค่ `Delay_Ms` หรือ non-blocking timer ใช้ **SimpleDelay** แทน  
> SimpleTIM เหมาะสำหรับ interrupt-driven periodic tasks

---

## Timer Instances

| Enum | Hardware | หมายเหตุ |
|:----:|:--------:|---------|
| `TIM_1` | TIM1 | Advanced timer |
| `TIM_2` | TIM2 | General purpose timer |

---

## API Reference

### Initialization

#### `void TIM_SimpleInit(TIM_Instance timer, uint32_t freq_hz)`

ตั้ง timer ให้ interrupt ที่ความถี่ที่กำหนด

```c
TIM_SimpleInit(TIM_1, 1000);    // 1kHz = interrupt ทุก 1ms
TIM_SimpleInit(TIM_2, 10);      // 10Hz = interrupt ทุก 100ms
TIM_SimpleInit(TIM_1, 1000000); // 1MHz = interrupt ทุก 1µs
```

---

### Start / Stop

#### `void TIM_Start(TIM_Instance timer)`
#### `void TIM_Stop(TIM_Instance timer)`

```c
TIM_Start(TIM_1);
// ...
TIM_Stop(TIM_1);
```

---

### Frequency

#### `void TIM_SetFrequency(TIM_Instance timer, uint32_t freq_hz)`

เปลี่ยน frequency ขณะ timer กำลังทำงาน

```c
TIM_SetFrequency(TIM_1, 500);  // เปลี่ยนจาก 1kHz เป็น 500Hz
```

---

### Interrupt Callback

#### `void TIM_AttachInterrupt(TIM_Instance timer, void (*callback)(void))`
#### `void TIM_DetachInterrupt(TIM_Instance timer)`

```c
void my_task(void) {
    digitalToggle(PC0);
}

TIM_AttachInterrupt(TIM_1, my_task);   // เรียก my_task ทุก interrupt
TIM_DetachInterrupt(TIM_1);             // ยกเลิก callback
```

---

### Counter Access

#### `uint16_t Simple_TIM_GetCounter(TIM_Instance timer)`
#### `void Simple_TIM_SetCounter(TIM_Instance timer, uint16_t val)`
#### `uint16_t TIM_GetPeriod(TIM_Instance timer)`

```c
uint16_t cnt = Simple_TIM_GetCounter(TIM_1);
Simple_TIM_SetCounter(TIM_1, 0);
uint16_t arr = TIM_GetPeriod(TIM_1);
```

---

### Advanced Init

#### `void TIM_AdvancedInit(TIM_Instance timer, uint16_t prescaler, uint16_t period, TIM_Mode mode)`

ตั้งค่า prescaler และ period เองโดยตรง

```c
// Freq = 48MHz / (prescaler+1) / (period+1)
// ตัวอย่าง: 48MHz / 48 / 1000 = 1kHz
TIM_AdvancedInit(TIM_1, 47, 999, TIM_MODE_UP);
```

| Mode | พฤติกรรม |
|:----:|---------|
| `TIM_MODE_UP` | นับขึ้น 0 → ARR |
| `TIM_MODE_DOWN` | นับลง ARR → 0 |

---

## ตัวอย่างการใช้งาน

### ขั้นต้น — LED Blink ด้วย Timer Interrupt

```c
#include "SimpleHAL.h"

void blink_callback(void) {
    digitalToggle(PC0);
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    pinMode(PC0, PIN_MODE_OUTPUT);

    TIM_SimpleInit(TIM_1, 2);       // 2Hz = toggle ทุก 500ms = blink 1Hz
    TIM_AttachInterrupt(TIM_1, blink_callback);
    TIM_Start(TIM_1);

    while (1) {
        // main loop ทำงานอื่นได้เต็มที่
    }
}
```

### ขั้นกลาง — Scheduler ง่ายๆ (Multi-rate Tasks)

```c
#include "SimpleHAL.h"

volatile uint32_t tick_1ms = 0;

void tick_callback(void) {
    tick_1ms++;
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    ADC_SimpleInit();

    TIM_SimpleInit(TIM_1, 1000);     // 1kHz = 1ms tick
    TIM_AttachInterrupt(TIM_1, tick_callback);
    TIM_Start(TIM_1);

    uint32_t last_100ms = 0;
    uint32_t last_1s    = 0;

    while (1) {
        uint32_t now = tick_1ms;

        if (now - last_100ms >= 100) {
            last_100ms = now;
            // Task 100ms: อ่าน ADC
            uint16_t raw = ADC_Read(ADC_CH_PA2);
            USART_Print("ADC="); USART_PrintNum(raw); USART_Print("\r\n");
        }

        if (now - last_1s >= 1000) {
            last_1s = now;
            // Task 1s: heartbeat
            digitalToggle(PC0);
        }
    }
}
```

### ขั้นสูง — Input Capture (วัด Pulse Width)

```c
#include "SimpleHAL.h"

// วัด pulse width โดยใช้ TIM2 counter + GPIO interrupt
volatile uint32_t pulse_start = 0;
volatile uint32_t pulse_width = 0;
volatile uint8_t  measuring = 0;

void edge_isr(void) {
    if (!measuring) {
        Simple_TIM_SetCounter(TIM_2, 0);
        measuring = 1;
    } else {
        pulse_width = Simple_TIM_GetCounter(TIM_2);
        measuring = 0;
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    // TIM2 เป็น free-running counter @ 1MHz (1µs per tick)
    TIM_AdvancedInit(TIM_2, 47, 0xFFFF, TIM_MODE_UP);
    TIM_Start(TIM_2);

    // GPIO interrupt บน PD2 (rising + falling)
    attachInterrupt(PD2, edge_isr, INTERRUPT_CHANGE);

    while (1) {
        if (!measuring && pulse_width > 0) {
            USART_Print("Pulse=");
            USART_PrintNum((int32_t)pulse_width);
            USART_Print(" us\r\n");
            pulse_width = 0;
        }
        Delay_Ms(10);
    }
}
```

---

## ข้อควรระวัง

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| SimplePWM กับ SimpleTIM ชน | PWM ใช้ TIM1/TIM2 | ถ้าใช้ PWM ใน TIM1 อย่าใช้ SimpleTIM TIM_1 |
| Callback ทำงานนานเกินไป | บล็อก interrupt | ใช้ flag แล้วประมวลผลใน main loop |
| Frequency ต่ำมาก (<1Hz) | overflow prescaler 16-bit | ใช้ software counter ใน callback แทน |
| `tick_1ms` overflow | uint32_t ล้นหลัง ~49 วัน | ใช้ subtraction `(now - last) >= interval` เสมอ |
