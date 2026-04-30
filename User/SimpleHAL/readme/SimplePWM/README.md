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

#### `void PWM_Init(uint8_t channel, uint32_t freq_hz)`

```c
PWM_Init(PWM1_CH1, 1000);   // PD2, 1kHz
PWM_Init(PWM2_CH3, 50);     // PC0, 50Hz (servo)
```

#### `void PWM_InitRemap(uint8_t channel, uint32_t freq_hz, uint8_t remap)`

ใช้ pin remapping (ดู datasheet CH32V003 สำหรับ remap options)

```c
PWM_InitRemap(PWM1_CH1, 1000, 1);
```

---

### Duty Cycle

#### `void PWM_SetDutyCycle(uint8_t channel, uint8_t percent)`

ตั้ง duty cycle เป็น **เปอร์เซ็นต์ 0–100**

```c
PWM_SetDutyCycle(PWM1_CH1, 50);   // 50%
PWM_SetDutyCycle(PWM1_CH1, 0);    // ปิด
PWM_SetDutyCycle(PWM1_CH1, 100);  // เต็ม
```

#### `void PWM_SetDutyCycleRaw(uint8_t channel, uint16_t value)`

ตั้ง duty cycle เป็น raw CCR register (0 ถึง ARR value)

```c
uint16_t period = PWM_GetPeriod(PWM1_CH1);
PWM_SetDutyCycleRaw(PWM1_CH1, period / 2);  // 50%
```

#### `void PWM_Write(uint8_t channel, uint8_t value)`

Arduino-compatible: `value` 0–255 (เหมือน `analogWrite`)

```c
PWM_Write(PWM1_CH1, 128);   // 50% (128/255)
PWM_Write(PWM1_CH1, 0);     // 0%
PWM_Write(PWM1_CH1, 255);   // 100%
```

---

### Frequency

#### `void PWM_SetFrequency(uint8_t channel, uint32_t freq_hz)`

เปลี่ยน frequency — **duty cycle จะรีเซ็ตเป็น 0%**

```c
PWM_SetFrequency(PWM1_CH1, 2000);   // เปลี่ยนเป็น 2kHz
PWM_SetFrequency(PWM2_CH1, 20000);  // เปลี่ยนเป็น 20kHz
```

#### `uint16_t PWM_GetPeriod(uint8_t channel)`

คืนค่า ARR register (ใช้กับ SetDutyCycleRaw)

```c
uint16_t arr = PWM_GetPeriod(PWM1_CH1);
```

---

### Start / Stop

#### `void PWM_Start(uint8_t channel)`
#### `void PWM_Stop(uint8_t channel)`

```c
PWM_Start(PWM1_CH1);
// ...
PWM_Stop(PWM1_CH1);
```

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

void servo_set_angle(uint8_t channel, uint8_t angle) {
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

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| TIM1 channel ทั้งหมด freq เปลี่ยน | ใช้ prescaler/ARR ร่วมกัน | ตั้ง freq ก่อน init ทุก channel |
| Servo สั่น / ออกนอกพิกัด | คำนวณ raw pulse ผิด | ตรวจสอบ period ด้วย `PWM_GetPeriod()` |
| `PWM_Write` ไม่ตรงกับ percent | 255 ≠ 100 ใน SetDutyCycle | เลือกใช้อย่างใดอย่างหนึ่ง อย่าผสม |
| PC4 ใช้ ADC ด้วยไม่ได้ | PC4 เป็นทั้ง ADC_CH_2 และ PWM1_CH4 | เลือกใช้อย่างใดอย่างหนึ่ง |
