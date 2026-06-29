# 01_GPIO — ตัวอย่าง GPIO

## ไฟล์ในโฟลเดอร์

| ไฟล์ | คำอธิบาย |
|------|----------|
| `ex01_LED_Blink.c` | LED กระพริบพื้นฐาน (pinMode, digitalWrite, Delay_Ms) |
| `ex02_Button_Input.c` | อ่านปุ่มกด + USART + digitalRead |
| `ex03_External_Interrupt.c` | External Interrupt (attachInterrupt, FALLING) |
| `ex04_GPIO_Multiple.c` | ควบคุมหลาย pin พร้อมกัน (pinModeMultiple, digitalWriteMultiple) |
| `ex05_Port_Operations.c` | Port-level I/O (portWrite, portRead) |
| `ex06_Shift_Register.c` | Shift Register 74HC595 + Knight Rider (shiftOut) |
| `ex07_PulseIn_Measurement.c` | วัด pulse ด้วย HC-SR04 Ultrasonic (pulseIn) |
| `ex08_GPIO_Toggle.c` | สลับสถานะ pin (digitalToggle) |

## การเลือกแพ็กเกจ

```c
#define CH32V003_PACKAGE  PACKAGE_TSSOP20   // หรือ SOP8, SOP16, QFN20
#include "SimpleHAL.h"
```

SOP-8 มี 6 pins: PD1(SWIO), PD4, PD5, PD6, PC1, PC2
SOP-16 มี ~14 pins (ไม่มี PD0)
TSSOP-20/QFN-20 มีครบทุก pin

## Build

```bat
cd User\Examples
build_all.bat
```
