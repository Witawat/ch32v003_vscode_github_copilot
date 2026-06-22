# PID Controller Library

> **Library สำหรับ PID (Proportional-Integral-Derivative) Controller สำหรับ CH32V003**

## 📋 สารบัญ

1. [ภาพรวม](#ภาพรวม)
2. [คุณสมบัติ](#คุณสมบัติ)
3. [การปรับค่า PID](#การปรับค่า-pid)
4. [การใช้งานพื้นฐาน](#การใช้งานพื้นฐาน)
5. [API Reference](#api-reference)

---

## ภาพรวม

PID Controller เป็นอัลกอริทึมควบคุมป้อนกลับ (Feedback Control) ที่ใช้กันอย่างแพร่หลายในระบบ embedded เช่น ควบคุมอุณหภูมิ, ความเร็วรอบมอเตอร์, ตำแหน่ง, แรงดัน ฯลฯ

---

## คุณสมบัติ

- ✅ รองรับ P, PI, PD, PID (ตั้ง Ki/Kd = 0 เพื่อปิด)
- ✅ Output limiting พร้อม anti-windup
- ✅ Direct / Reverse action
- ✅ Manual / Auto mode
- ✅ Pure software ไม่ใช้ทรัพยากรฮาร์ดแวร์

---

## การปรับค่า PID

| พารามิเตอร์ | Effect | Rise Time | Overshoot | Settling Time | Steady-State Error |
|-------------|--------|-----------|-----------|---------------|-------------------|
| Kp เพิ่มขึ้น | เพิ่ม | ลดลง | เพิ่มขึ้น | เปลี่ยนแปลงเล็กน้อย | ลดลง |
| Ki เพิ่มขึ้น | เพิ่ม | ลดลง | เพิ่มขึ้น | เพิ่มขึ้น | กำจัด |
| Kd เพิ่มขึ้น | เพิ่ม | เปลี่ยนแปลงเล็กน้อย | ลดลง | ลดลง | เปลี่ยนแปลงเล็กน้อย |

---

## การใช้งานพื้นฐาน

```c
#include "main.h"
#include "Lib/PID/PID.h"

PID_Controller pid;

int main(void) {
    PID_Init(&pid, 2.0f, 0.5f, 0.1f, 0.01f);  // Kp=2, Ki=0.5, Kd=0.1, dt=10ms
    PID_SetSetpoint(&pid, 100.0f);              // ค่าเป้าหมาย
    PID_SetLimits(&pid, 0.0f, 255.0f);          // จำกัด output
    PID_SetMode(&pid, PID_MODE_AUTO);

    while (1) {
        float input = read_sensor();
        float output = PID_Compute(&pid, input);
        apply_output(output);
        Delay_Ms(10);
    }
}
```

---

## API Reference

- `PID_Init(pid, kp, ki, kd, dt)` : เริ่มต้น PID controller
- `PID_SetTunings(pid, kp, ki, kd)` : เปลี่ยนค่า Kp, Ki, Kd
- `PID_SetLimits(pid, min, max)` : จำกัดช่วง output
- `PID_SetSetpoint(pid, setpoint)` : ตั้งค่าจุดหมาย
- `PID_SetMode(pid, mode)` : เปลี่ยนโหมด MANUAL / AUTO
- `PID_SetDirection(pid, direction)` : DIRECT / REVERSE action
- `PID_Reset(pid)` : รีเซ็ตค่า integral และ output
- `PID_Compute(pid, input)` : คำนวณค่า output
- `PID_GetOutput(pid)` : อ่านค่า output ล่าสุด

---
**พัฒนาโดย:** CH32V003 Library Team
**รองรับบอร์ด:** CH32V003 Development Board
