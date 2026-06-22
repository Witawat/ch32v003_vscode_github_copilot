# AS5600 Magnetic Rotary Encoder Library

> **Library สำหรับอ่านตำแหน่งเชิงมุมจาก AS5600 Magnetic Encoder ผ่าน I2C สำหรับ CH32V003**

## 📋 สารบัญ

1. [ภาพรวม](#ภาพรวม)
2. [คุณสมบัติ](#คุณสมบัติ)
3. [Hardware Setup](#hardware-setup)
4. [การใช้งานพื้นฐาน](#การใช้งานพื้นฐาน)
5. [API Reference](#api-reference)

---

## ภาพรวม

AS5600 เป็นเซนเซอร์วัดตำแหน่งเชิงมุมแบบแม่เหล็ก (Magnetic Rotary Encoder) ความละเอียด 12-bit (0-4095) หรือ 0.0879 องศา ใช้แม่เหล็ก NdFeB คู่กับ IC เพื่ออ่านมุมแบบไม่สัมผัส (contactless)

---

## คุณสมบัติ

- ✅ ความละเอียด 12-bit (0-4095), 0.0879°/step
- ✅ อ่านมุมได้ทั้ง raw และองศา
- ✅ ตรวจสอบ magnet detection และความแรงแม่เหล็ก
- ✅ ตั้งค่า zero position (ZPOS) และ max position (MPOS)
- ✅ อ่านค่า AGC (Automatic Gain Control) และ Magnitude

---

## Hardware Setup

| AS5600 Pin | CH32V003 Pin | หมายเหตุ |
|------------|--------------|----------|
| VCC        | 3.3V         | แรงดันไฟเลี้ยง (3.0-3.6V) |
| GND        | GND          | กราวด์ร่วม |
| SCL        | PC2          | I2C Clock (Pull-up 4.7kΩ) |
| SDA        | PC1          | I2C Data (Pull-up 4.7kΩ) |
| ADDR       | ลอยไว้       | ลอย = 0x36, GND = 0x37 |

> ต้องใช้แม่เหล็ก NdFeB ติดบนแกนหมุน ห่างจาก IC ประมาณ 1-3mm

---

## การใช้งานพื้นฐาน

```c
#include "main.h"
#include "Lib/AS5600/AS5600.h"

AS5600_Instance encoder;

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);

    AS5600_Init(&encoder);

    while (1) {
        uint16_t angle  = AS5600_ReadAngle(&encoder);
        float    deg    = AS5600_ReadAngleDegrees(&encoder);

        AS5600_MagnetStrength mag = AS5600_GetMagnetStrength(&encoder);

        printf("Angle: %u (%.1f deg) Mag: %d\r\n", angle, deg, mag);
        Delay_Ms(100);
    }
}
```

---

## API Reference

- `AS5600_Init(enc)` : เริ่มต้นเซนเซอร์
- `AS5600_ReadAngle(enc)` : อ่านมุม 12-bit (0-4095)
- `AS5600_ReadAngleDegrees(enc)` : อ่านมุมเป็นองศา (0.0-359.9)
- `AS5600_ReadRawAngle(enc)` : อ่านมุม raw (ไม่รวม offset)
- `AS5600_ReadStatus(enc, *status)` : อ่าน status register
- `AS5600_GetMagnetStrength(enc)` : ตรวจสอบความแรงแม่เหล็ก
- `AS5600_ReadAGC(enc)` : อ่านค่า Automatic Gain Control
- `AS5600_ReadMagnitude(enc)` : อ่านค่า CORDIC magnitude
- `AS5600_SetStartPosition(enc, angle)` : ตั้งค่า zero position
- `AS5600_SetEndPosition(enc, angle)` : ตั้งค่า max position
- `AS5600_BurnAngle(enc)` : เผา ZPOS/MPOS ลง OTP (ถาวร)

---
**พัฒนาโดย:** CH32V003 Library Team
**รองรับบอร์ด:** CH32V003 Development Board
