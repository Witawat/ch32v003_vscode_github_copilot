# SimplePWM — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `SimplePWM.h / SimplePWM.c`

---

## ภาพรวม

SimplePWM ให้ใช้งาน PWM output บน CH32V003 ได้ง่าย โดยรองรับ **8 channels** จาก TIM1 และ TIM2 ปรับ frequency และ duty cycle ได้อิสระ พร้อม API แบบ Arduino `analogWrite`

---

## Channel Map

| Channel | Enum | Pin | Timer |
|:-------:|------|:---:|:-----:|
| 0 | `PWM1_CH1` | PD2 | TIM1 CH1 |
| 1 | `PWM1_CH2` | PA1 | TIM1 CH2 |
| 2 | `PWM1_CH3` | PC3 | TIM1 CH3 |
| 3 | `PWM1_CH4` | PC4 | TIM1 CH4 |
| 4 | `PWM2_CH1` | PD4 | TIM2 CH1 |
| 5 | `PWM2_CH2` | PD3 | TIM2 CH2 |
| 6 | `PWM2_CH3` | PC0 | TIM2 CH3 |
| 7 | `PWM2_CH4` | PD7 | TIM2 CH4 |

> TIM1 และ TIM2 แต่ละตัวใช้ frequency เดียวกันร่วมกัน — ถ้าตั้ง freq ของ channel ใด channel หนึ่งใน TIM1 จะเปลี่ยนทุก channel ของ TIM1

---

## API Reference

### Initialization

#### `void PWM_Init(PWM_Channel channel, uint32_t frequency_hz)`

```c
PWM_Init(PWM1_CH1, 1000);   // PD2, 1kHz (default pin)
PWM_Init(PWM2_CH3, 50);     // PC0, 50Hz (servo)
```

#### `void PWM_InitRemap(PWM_Channel channel, uint32_t frequency_hz, PWM_Remap remap)`

เริ่มต้น PWM พร้อม remap — เลือก pin ใหม่ให้ PWM output

```c
PWM_InitRemap(PWM1_CH1, 1000, PWM_REMAP_PARTIAL1);
PWM_Start(PWM1_CH1);
PWM_SetDutyCycle(PWM1_CH1, 50);
```

#### `void PWM_SetRemap(PWM_Channel channel, PWM_Remap remap)` 🆕

ตั้งค่ารีแมปล่วงหน้า — `PWM_Write()` / `analogWrite()` ใช้ตอน auto-init

```c
// วิธีง่าย — ตั้ง remap ล่วงหน้า แล้ว analogWrite จัดการเอง
PWM_SetRemap(PWM1_CH1, PWM_REMAP_PARTIAL1);
analogWrite(PD2, 128);  // auto-init @1kHz + PARTIAL1 remap

PWM_SetRemap(PWM2_CH1, PWM_REMAP_PARTIAL2);
analogWrite(PD4, 255);  // เลือกใช้ remap คนละแบบ
```

#### PWM_Remap Options

| Option | ใช้กับ | หมายเหตุ |
|--------|:---:|----------|
| `PWM_REMAP_NONE` | TIM1, TIM2 | Default pins (ค่าเริ่มต้น) |
| `PWM_REMAP_PARTIAL1` | TIM1, TIM2 | Partial remap 1 |
| `PWM_REMAP_PARTIAL2` | TIM1, TIM2 | Partial remap 2 |

> ⚠️ `PWM_REMAP_FULL` ถูกลบ — ใช้พอร์ท PE/PB ที่ไม่มีใน CH32V003

#### `IS_PWM_VALID_PACKAGE(ch)` 🆕

ตรวจสอบว่า PWM channel ใช้ได้กับแพ็กเกจปัจจุบัน — **SOP-8 ใช้ได้แค่ PWM1_CH1 + PWM2_CH1**

```c
#if CH32V003_IS_SOP8
  if (IS_PWM_VALID_PACKAGE(PWM1_CH1))  // true
      PWM_Init(PWM1_CH1, 1000);
  if (IS_PWM_VALID_PACKAGE(PWM1_CH2))  // false — PA1 ไม่มีบน SOP-8
      PWM_Init(PWM1_CH2, 1000);
#endif
```

---

### Duty Cycle

#### `void PWM_SetDutyCycle(PWM_Channel channel, uint8_t percent)`

ตั้ง duty cycle เป็น **เปอร์เซ็นต์ 0–100**

```c
PWM_SetDutyCycle(PWM1_CH1, 50);   // 50%
PWM_SetDutyCycle(PWM1_CH1, 0);    // ปิด
PWM_SetDutyCycle(PWM1_CH1, 100);  // เต็ม
```

#### `void PWM_SetDutyCycleRaw(PWM_Channel channel, uint16_t value)`

ตั้ง duty cycle เป็น raw CCR register (0 ถึง ARR value)

```c
uint16_t period = PWM_GetPeriod(PWM1_CH1);
PWM_SetDutyCycleRaw(PWM1_CH1, period / 2);  // 50%
```

#### `void PWM_Write(PWM_Channel channel, uint8_t value)`

Arduino-compatible: `value` 0–255 (เหมือน `analogWrite`)

```c
PWM_Write(PWM1_CH1, 128);   // 50% (128/255)
PWM_Write(PWM1_CH1, 0);     // 0%
PWM_Write(PWM1_CH1, 255);   // 100%
```

---

### Frequency

#### `void PWM_SetFrequency(PWM_Channel channel, uint32_t frequency_hz)`

เปลี่ยน frequency — **duty cycle จะรีเซ็ตเป็น 0%**

```c
PWM_SetFrequency(PWM1_CH1, 2000);   // เปลี่ยนเป็น 2kHz
PWM_SetFrequency(PWM2_CH1, 20000);  // เปลี่ยนเป็น 20kHz
```

#### `uint16_t PWM_GetPeriod(PWM_Channel channel)`

คืนค่า ARR register (ใช้กับ SetDutyCycleRaw)

```c
uint16_t arr = PWM_GetPeriod(PWM1_CH1);
```

---

### Start / Stop

#### `void PWM_Start(PWM_Channel channel)`
#### `void PWM_Stop(PWM_Channel channel)`

```c
PWM_Start(PWM1_CH1);
// ...
PWM_Stop(PWM1_CH1);
```

### Advanced

#### `uint8_t PWM_GetDutyCycle(PWM_Channel channel)`
อ่าน duty cycle ปัจจุบันเป็นเปอร์เซ็นต์ (0-100)

```c
uint8_t pct = PWM_GetDutyCycle(PWM1_CH1);
```

#### `uint16_t PWM_GetDutyCycleRaw(PWM_Channel channel)`
อ่านค่า CCR register (raw value)

```c
uint16_t raw = PWM_GetDutyCycleRaw(PWM1_CH1);
```

#### `void PWM_AdvancedInit(channel, prescaler, period, duty_value)`
ตั้งค่าแบบละเอียด — กำหนด prescaler และ period เอง

```c
// ตัวอย่าง: 1kHz @48MHz → prescaler=47, period=999
PWM_AdvancedInit(PWM1_CH1, 47, 999, 500);  // 50% duty
```

#### `void PWM_SetPolarity(PWM_Channel channel, uint8_t inverted)`
กลับขั้ว output — 0=normal, 1=inverted

```c
PWM_SetPolarity(PWM1_CH1, 1);  // Active low
```

### Helper Macros

| Macro | ใช้ทำอะไร |
|-------|----------|
| `PWM_PERCENT_TO_RAW(pct, period)` | แปลง % → raw CCR |
| `PWM_RAW_TO_PERCENT(raw, period)` | แปลง raw CCR → % |
| `PWM_ARDUINO_TO_PERCENT(val)` | arduino 0-255 → % |

---

## ตัวอย่างการใช้งาน

### ขั้นต้น — Fade LED

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    // PD2, 1kHz
    PWM_Init(PWM1_CH1, 1000);
    PWM_Start(PWM1_CH1);

    while (1) {
        // Fade up
        for (uint8_t i = 0; i <= 100; i++) {
            PWM_SetDutyCycle(PWM1_CH1, i);
            Delay_Ms(10);
        }
        // Fade down
        for (uint8_t i = 100; i > 0; i--) {
            PWM_SetDutyCycle(PWM1_CH1, i);
            Delay_Ms(10);
        }
    }
}
```

### ขั้นกลาง — ควบคุม Servo Motor

```c
#include "SimpleHAL.h"

// Servo: 50Hz (20ms period)
// Pulse: 1ms = 0°, 1.5ms = 90°, 2ms = 180°
// duty (us) = 1000 + (angle/180) * 1000

void servo_set_angle(PWM_Channel channel, uint8_t angle) {
    if (angle > 180) angle = 180;
    uint16_t period = PWM_GetPeriod(channel);     // ARR at 50Hz
    // period = 48MHz / (prescaler+1) / 50 - 1
    // us_per_tick = 1e6 / 50 / (period+1)
    uint32_t us_total = 20000;  // 20ms period
    uint32_t pulse_us = 1000 + ((uint32_t)angle * 1000 / 180);
    uint16_t raw = (uint16_t)((uint32_t)(period + 1) * pulse_us / us_total);
    PWM_SetDutyCycleRaw(channel, raw);
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    PWM_Init(PWM2_CH3, 50);  // PC0, 50Hz
    PWM_Start(PWM2_CH3);

    while (1) {
        servo_set_angle(PWM2_CH3, 0);
        Delay_Ms(1000);
        servo_set_angle(PWM2_CH3, 90);
        Delay_Ms(1000);
        servo_set_angle(PWM2_CH3, 180);
        Delay_Ms(1000);
    }
}
```

### ขั้นสูง — Motor Speed Control (DC Motor + L298N)

```c
#include "SimpleHAL.h"

// L298N: IN1=PC0_PWM, IN2=PC3, ENA=PC1_PWM
// การควบคุม: ตั้ง IN1/IN2 สำหรับทิศ แล้วปรับ PWM สำหรับความเร็ว

void motor_init(void) {
    PWM_Init(PWM2_CH3, 10000);   // PC0, 10kHz
    PWM_Init(PWM1_CH3, 10000);   // PC3, 10kHz (TIM1)
    PWM_Start(PWM2_CH3);
    PWM_Start(PWM1_CH3);
}

// speed: -100 (ถอย) ถึง +100 (ไปข้างหน้า)
void motor_set(int8_t speed) {
    if (speed >= 0) {
        // ไปข้างหน้า: IN1=PWM, IN2=0
        PWM_SetDutyCycle(PWM2_CH3, (uint8_t)speed);
        PWM_SetDutyCycle(PWM1_CH3, 0);
    } else {
        // ถอยหลัง: IN1=0, IN2=PWM
        PWM_SetDutyCycle(PWM2_CH3, 0);
        PWM_SetDutyCycle(PWM1_CH3, (uint8_t)(-speed));
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    motor_init();

    // เร่งไปข้างหน้า
    for (int8_t s = 0; s <= 100; s += 5) {
        motor_set(s);
        Delay_Ms(50);
    }
    Delay_Ms(1000);
    // หยุด
    motor_set(0);
}
```

---

## ข้อควรระวัง

> **⚠️ Frequency Sharing:** ทุกช่องบน timer เดียวกัน (PWM1_CH1-CH4 บน TIM1, PWM2_CH1-CH4 บน TIM2) **แชร์ความถี่ร่วมกัน** — ความถี่จากช่องแรกที่ init จะเป็นตัวกำหนด ช่องหลังที่ init ด้วยความถี่ต่างกันจะถูกเพิกเฉย

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| TIM1 channel ทั้งหมด freq เปลี่ยน | ใช้ prescaler/ARR ร่วมกัน | ตั้ง freq ก่อน init ทุก channel |
| Servo สั่น / ออกนอกพิกัด | คำนวณ raw pulse ผิด | ตรวจสอบ period ด้วย `PWM_GetPeriod()` |
| `PWM_Write` ไม่ตรงกับ percent | 255 ≠ 100 ใน SetDutyCycle | เลือกใช้อย่างใดอย่างหนึ่ง อย่าผสม |
| PC4 ใช้ ADC ด้วยไม่ได้ | PC4 เป็นทั้ง ADC_CH_2 และ PWM1_CH4 | เลือกใช้อย่างใดอย่างหนึ่ง |
