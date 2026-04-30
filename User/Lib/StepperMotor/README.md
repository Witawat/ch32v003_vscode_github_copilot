# Stepper Motor Library

> **Library สำหรับควบคุม Stepper Motor บน CH32V003**  
> รองรับ ULN2003 (28BYJ-48) และ A4988/DRV8825 (NEMA17/NEMA23)

## 📋 สารบัญ

1. [ภาพรวม](#ภาพรวม)
2. [คุณสมบัติ](#คุณสมบัติ)
3. [Hardware Setup](#hardware-setup)
4. [หลักการทำงาน](#หลักการทำงาน)
5. [การใช้งานพื้นฐาน](#การใช้งานพื้นฐาน)
6. [การใช้งานขั้นสูง](#การใช้งานขั้นสูง)
7. [Troubleshooting](#troubleshooting)
8. [API Reference](#api-reference)

---

## ภาพรวม

Stepper Motor คือมอเตอร์ที่หมุนเป็นขั้น (step) ทำให้ควบคุมมุมและตำแหน่งได้แม่นยำ โดยไม่ต้องใช้ encoder

Library นี้รองรับ 2 ประเภท driver:

| Driver | Motor ที่ใช้ | จำนวน Pins | ราคา | แรงบิด |
|--------|-------------|-----------|------|--------|
| **ULN2003** | 28BYJ-48 | 4 pins | ถูก | ปานกลาง |
| **A4988/DRV8825** | NEMA17/23 | 2-3 pins | แพงกว่า | สูง |

---

## คุณสมบัติ

- ✅ รองรับ ULN2003 + 28BYJ-48 (4-wire unipolar)
- ✅ รองรับ A4988 / DRV8825 / TB6600 (STEP+DIR+EN)
- ✅ Full Step และ Half Step สำหรับ ULN2003
- ✅ ควบคุมความเร็วด้วย RPM
- ✅ หมุนด้วย steps, องศา (degrees), หรือรอบ (revolutions)
- ✅ ติดตามตำแหน่ง (position tracking)
- ✅ Auto power-off หลังหมุนเสร็จ (ลดความร้อน)
- ✅ เอกสารภาษาไทยครบถ้วน

---

## Hardware Setup

### วงจร ULN2003 + 28BYJ-48

```
CH32V003           ULN2003 Board       28BYJ-48
                   +-----------+
PC0 (IN1) -------> | IN1   OUT1| -----> BLUE   (สาย 1)
PC1 (IN2) -------> | IN2   OUT2| -----> PINK   (สาย 2)
PC2 (IN3) -------> | IN3   OUT3| -----> YELLOW (สาย 3)
PC3 (IN4) -------> | IN4   OUT4| -----> ORANGE (สาย 4)
GND       -------> | GND       |        RED = VCC 5V
5V        -------> | VCC       |
                   +-----------+
```

> ⚠️ **สำคัญ**: 28BYJ-48 ต้องใช้ไฟ **5V** เท่านั้น ห้ามใช้ 3.3V จาก CH32V003  
> ใช้ ULN2003 board เป็น level-shifter (รับ logic 3.3V ได้)

### วงจร A4988 + NEMA17

```
CH32V003           A4988 Module        NEMA17
                   +----------+
PC0 (STEP) ------> | STEP     |
PC1 (DIR)  ------> | DIR      |        2A ---+-- 1A
PC2 (EN)   ------> | EN       |        2B ---+-- 1B
GND        ------> | GND      |
3.3V       ------> | VDD      |
12V PSU    ------> | VMOT     |
GND        ------> | PGND     |
                   +----------+

ต่อ VREF: ตั้ง current limit ด้วย potentiometer บน A4988
```

> ⚠️ **สำคัญ**: A4988 ต้องการไฟ VMOT **8-35V** (NEMA17 ทั่วไปใช้ 12V)  
> ต่อ capacitor 100µF ระหว่าง VMOT และ GND เพื่อป้องกัน voltage spike

### GPIO Pins ที่ใช้ได้

ใช้ได้กับทุก GPIO pin ของ CH32V003:
- **GPIOA**: PA1, PA2
- **GPIOC**: PC0 – PC7
- **GPIOD**: PD2 – PD7

---

## หลักการทำงาน

### Full Step vs Half Step (ULN2003)

```
Full Step (4 phases):
Phase 0: IN1=1 IN2=0 IN3=0 IN4=0
Phase 1: IN1=0 IN2=1 IN3=0 IN4=0
Phase 2: IN1=0 IN2=0 IN3=1 IN4=0
Phase 3: IN1=0 IN2=0 IN3=0 IN4=1

Half Step (8 phases — ละเอียดขึ้น 2×):
Phase 0: IN1=1 IN2=0 IN3=0 IN4=0
Phase 1: IN1=1 IN2=1 IN3=0 IN4=0
Phase 2: IN1=0 IN2=1 IN3=0 IN4=0
Phase 3: IN1=0 IN2=1 IN3=1 IN4=0
...ฯลฯ
```

### 28BYJ-48 Specifications

| รายการ | ค่า |
|--------|-----|
| Steps ต่อรอบ (Full Step) | 2048 ÷ 2 = 1024 steps |
| Steps ต่อรอบ (Half Step) | 2048 steps |
| มุมต่อ step (Half Step) | 360° / 2048 ≈ 0.176° |
| ความเร็วสูงสุดแนะนำ | 15 RPM |
| แรงดันไฟ | 5V DC |

### A4988 Step Pulse

```
STEP pin:

   ___     ___     ___
__|   |___|   |___|   |___

↑ HIGH อย่างน้อย 1µs per pulse
```

---

## การใช้งานพื้นฐาน

### ULN2003 + 28BYJ-48

#### ขั้นตอนที่ 1: Include และประกาศตัวแปร

```c
#include "SimpleHAL.h"
#include "StepperMotor.h"

StepperMotor_Instance motor;
```

#### ขั้นตอนที่ 2: Init ใน main()

```c
int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    // IN1=PC0, IN2=PC1, IN3=PC2, IN4=PC3, Half Step
    StepperMotor_InitULN2003(&motor, PC0, PC1, PC2, PC3, STEPPER_HALF_STEP);
    StepperMotor_SetSpeed(&motor, 10);  // 10 RPM
```

#### ขั้นตอนที่ 3: ใช้งาน

```c
    // หมุน 1 รอบตามเข็มนาฬิกา (CW)
    StepperMotor_MoveRevolutions(&motor, 1);
    Delay_Ms(500);

    // หมุน 90° ทวนเข็มนาฬิกา (CCW)
    StepperMotor_MoveDegrees(&motor, -90);
    Delay_Ms(500);

    // หมุน 512 steps (CW)
    StepperMotor_Move(&motor, 512);

    while (1) {}
}
```

### A4988 + NEMA17

```c
#include "SimpleHAL.h"
#include "StepperMotor.h"

StepperMotor_Instance motor;

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    // STEP=PC0, DIR=PC1, EN=PC2, 200 steps/rev (NEMA17 1.8°)
    StepperMotor_InitA4988(&motor, PC0, PC1, PC2, 200);
    StepperMotor_SetSpeed(&motor, 60);   // 60 RPM

    StepperMotor_Enable(&motor);         // เปิด coil ก่อน move

    StepperMotor_MoveRevolutions(&motor, 5);  // หมุน 5 รอบ

    StepperMotor_Disable(&motor);        // ปิด coil ลดความร้อน

    while (1) {}
}
```

---

## การใช้งานขั้นสูง

### ควบคุม 2 มอเตอร์พร้อมกัน

```c
StepperMotor_Instance motor_x;
StepperMotor_Instance motor_y;

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    StepperMotor_InitA4988(&motor_x, PC0, PC1, PC2, 200);
    StepperMotor_InitA4988(&motor_y, PC3, PC4, PC5, 200);

    StepperMotor_SetSpeed(&motor_x, 60);
    StepperMotor_SetSpeed(&motor_y, 60);

    StepperMotor_Enable(&motor_x);
    StepperMotor_Enable(&motor_y);

    // ย้ายทีละแกน
    StepperMotor_MoveDegrees(&motor_x, 90);
    StepperMotor_MoveDegrees(&motor_y, 180);

    StepperMotor_Disable(&motor_x);
    StepperMotor_Disable(&motor_y);

    while (1) {}
}
```

### ติดตามตำแหน่งและ Return to Home

```c
StepperMotor_Instance motor;

void go_home(void) {
    int32_t pos = StepperMotor_GetPosition(&motor);
    StepperMotor_Move(&motor, -pos);  // ย้อนกลับตำแหน่ง 0
    StepperMotor_ResetPosition(&motor);
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    StepperMotor_InitULN2003(&motor, PC0, PC1, PC2, PC3, STEPPER_HALF_STEP);
    StepperMotor_SetSpeed(&motor, 10);

    StepperMotor_MoveDegrees(&motor, 180);   // ไปที่ 180°
    printf("Position: %ld steps\r\n", StepperMotor_GetPosition(&motor));

    Delay_Ms(1000);

    go_home();   // กลับ 0°
    printf("Back home: %ld steps\r\n", StepperMotor_GetPosition(&motor));

    while (1) {}
}
```

### เปลี่ยนความเร็วระหว่างทาง

```c
StepperMotor_SetSpeed(&motor, 5);    // เริ่มช้า
StepperMotor_Move(&motor, 500);

StepperMotor_SetSpeed(&motor, 15);   // เพิ่มความเร็ว
StepperMotor_Move(&motor, 1000);

StepperMotor_SetSpeed(&motor, 5);    // ชะลอก่อนหยุด
StepperMotor_Move(&motor, 500);
```

### การเลือก Full Step vs Half Step

```c
// Full Step: แรงบิดสูงสุด เหมาะกับ load หนัก
StepperMotor_InitULN2003(&motor, PC0, PC1, PC2, PC3, STEPPER_FULL_STEP);
// 1024 steps/rev, มุม 0.352°/step

// Half Step: ความละเอียดสูง การหมุนนุ่มนวลกว่า (แนะนำ)
StepperMotor_InitULN2003(&motor, PC0, PC1, PC2, PC3, STEPPER_HALF_STEP);
// 2048 steps/rev, มุม 0.176°/step
```

---

## Troubleshooting

### ปัญหา: มอเตอร์ไม่หมุน (ULN2003)

| สาเหตุ | วิธีแก้ |
|--------|---------|
| ไฟ 5V ไม่พอ | ใช้แหล่งจ่าย 5V แยก (ไม่ใช่จาก CH32V003) |
| สายต่อผิด | ตรวจ IN1-IN4 ตรงกับ pin ที่ Init |
| Delay_Ms ใน main loop ทำให้ step delay ผิด | ไม่ต้องมี Delay_Ms ระหว่างการหมุน |

### ปัญหา: มอเตอร์สั่น แต่ไม่หมุน

| สาเหตุ | วิธีแก้ |
|--------|---------|
| ความเร็วสูงเกินไป | ลด RPM ลง (28BYJ-48 max ~15 RPM) |
| Sequence ผิดลำดับ | ตรวจสอบลำดับ IN1-IN4 กับสายของมอเตอร์ |
| ไฟไม่พอ | ตรวจแหล่งจ่ายกระแส |

### ปัญหา: A4988 ร้อนมาก

| สาเหตุ | วิธีแก้ |
|--------|---------|
| Current limit สูงเกิน | ปรับ VREF ลง |
| ไม่ได้ Disable หลังใช้ | เรียก `StepperMotor_Disable()` เมื่อหยุด |
| ไม่มี heatsink | ติด heatsink บน A4988 |

### ปัญหา: ตำแหน่งคลาดเคลื่อน

| สาเหตุ | วิธีแก้ |
|--------|---------|
| Motor ขาดขั้นตอน (skipped steps) | ลดความเร็วหรือลด load |
| steps_per_rev ผิด | ตรวจสอบ datasheet ของมอเตอร์ |

---

## API Reference

### `StepperMotor_InitULN2003(motor, in1, in2, in3, in4, mode)`
เริ่มต้น motor แบบ ULN2003 (4-wire)

| Parameter | Type | คำอธิบาย |
|-----------|------|----------|
| `motor` | `StepperMotor_Instance*` | ตัวแปร instance |
| `in1`–`in4` | `uint8_t` | GPIO pins สำหรับ IN1–IN4 |
| `mode` | `StepperMotor_StepMode` | `STEPPER_FULL_STEP` หรือ `STEPPER_HALF_STEP` |

---

### `StepperMotor_InitA4988(motor, step_pin, dir_pin, en_pin, steps_per_rev)`
เริ่มต้น motor แบบ A4988/DRV8825

| Parameter | Type | คำอธิบาย |
|-----------|------|----------|
| `motor` | `StepperMotor_Instance*` | ตัวแปร instance |
| `step_pin` | `uint8_t` | GPIO pin สำหรับ STEP |
| `dir_pin` | `uint8_t` | GPIO pin สำหรับ DIR |
| `en_pin` | `uint8_t` | GPIO pin สำหรับ EN (ใส่ `0` ถ้าไม่มี) |
| `steps_per_rev` | `uint32_t` | Steps ต่อ 1 รอบ (NEMA17 = 200) |

---

### `StepperMotor_SetSpeed(motor, rpm)`
ตั้งความเร็ว — ULN2003 แนะนำ 1-15 RPM, A4988 แนะนำ 1-300 RPM

---

### `StepperMotor_Move(motor, steps)`
หมุนตาม steps — บวก=CW, ลบ=CCW (blocking)

---

### `StepperMotor_MoveDegrees(motor, degrees)`
หมุนตามองศา — บวก=CW, ลบ=CCW (blocking)

---

### `StepperMotor_MoveRevolutions(motor, revolutions)`
หมุนตามรอบ — บวก=CW, ลบ=CCW (blocking)

---

### `StepperMotor_Enable(motor)` / `StepperMotor_Disable(motor)`
เปิด/ปิด coil — เรียก Enable ก่อน Move (A4988), เรียก Disable หลังหยุดเพื่อลดความร้อน

---

### `StepperMotor_Stop(motor)`
ปิด coils ทั้งหมดทันที (ULN2003: ตัด current ทั้ง 4 pins)

---

### `StepperMotor_GetPosition(motor)` → `int32_t`
ดูตำแหน่งปัจจุบัน (steps จาก home, บวก=CW, ลบ=CCW)

---

### `StepperMotor_ResetPosition(motor)`
ตั้งตำแหน่งปัจจุบันเป็น 0 (home)

---

### StepperMotor_StepMode Values

| ค่า | ความหมาย |
|-----|----------|
| `STEPPER_FULL_STEP` | Full Step — 4 phases, แรงบิดสูงสุด |
| `STEPPER_HALF_STEP` | Half Step — 8 phases, ละเอียดขึ้น 2× |

---

### StepperMotor Constants

| Constant | ค่า | ความหมาย |
|----------|-----|----------|
| `STEPPER_28BYJ48_STEPS_PER_REV` | 2048 | 28BYJ-48 half-step steps/rev |
| `STEPPER_NEMA17_STEPS_PER_REV` | 200 | NEMA17 1.8°/step |
| `STEPPER_ULN2003_SPEED_MAX_RPM` | 15 | ความเร็วสูงสุด ULN2003 |
| `STEPPER_A4988_SPEED_MAX_RPM` | 300 | ความเร็วสูงสุด A4988 |
