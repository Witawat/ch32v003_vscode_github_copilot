# TCS34725 Color Sensor Library

> **Library สำหรับอ่านค่าสี RGBC จาก TCS34725 ผ่าน I2C สำหรับ CH32V003**

## 📋 สารบัญ

1. [ภาพรวม](#ภาพรวม)
2. [คุณสมบัติ](#คุณสมบัติ)
3. [Hardware Setup](#hardware-setup)
4. [การใช้งานพื้นฐาน](#การใช้งานพื้นฐาน)
5. [API Reference](#api-reference)

---

## ภาพรวม

TCS34725 เป็นเซนเซอร์วัดสีแบบดิจิทัล ให้ค่า RGBC (Red, Green, Blue, Clear) สามารถคำนวณ Lux (ความสว่าง) และ Color Temperature (อุณหภูมิสี) ได้

---

## คุณสมบัติ

- ✅ อ่านค่า RGBC raw data (16-bit ต่อช่อง)
- ✅ คำนวณ Lux
- ✅ คำนวณ Color Temperature (Kelvin)
- ✅ ปรับ Gain (1x, 4x, 16x, 60x) และ Integration Time ได้
- ✅ มี LED ในตัวสำหรับให้แสง

---

## Hardware Setup

| TCS34725 Pin | CH32V003 Pin | หมายเหตุ |
|--------------|--------------|----------|
| VCC          | 3.3V         | แรงดันไฟเลี้ยง |
| GND          | GND          | กราวด์ร่วม |
| SCL          | PC2          | I2C Clock (Pull-up 4.7kΩ) |
| SDA          | PC1          | I2C Data (Pull-up 4.7kΩ) |
| ADDR         | GND / VCC    | GND = 0x29, VCC = 0x49 |

---

## การใช้งานพื้นฐาน

```c
#include "main.h"
#include "Lib/TCS34725/TCS34725.h"

TCS34725_Instance color;

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);

    TCS34725_Init(&color, TCS34725_GAIN_4X, TCS34725_INTEG_50MS);

    while (1) {
        uint16_t r, g, b, c;
        TCS34725_ReadRGBC(&color, &r, &g, &b, &c);

        float lux       = TCS34725_GetLux(&color);
        uint16_t cct    = TCS34725_GetColorTemp(&color);

        printf("RGB(%u,%u,%u) Lux=%.0f CCT=%uK\r\n", r, g, b, lux, cct);
        Delay_Ms(500);
    }
}
```

---

## API Reference

- `TCS34725_Init(sensor, gain, time)` : เริ่มต้นเซนเซอร์
- `TCS34725_SetGain(sensor, gain)` : ตั้งค่า Gain
- `TCS34725_SetIntegrationTime(sensor, time)` : ตั้งค่า Integration Time
- `TCS34725_Enable(sensor)` / `TCS34725_Disable(sensor)` : เปิด/ปิดเซนเซอร์
- `TCS34725_ReadRGBC(sensor, *r, *g, *b, *c)` : อ่านค่า RGBC
- `TCS34725_GetLux(sensor)` : คำนวณค่าความสว่าง
- `TCS34725_GetColorTemp(sensor)` : คำนวณอุณหภูมิสี (Kelvin)

---
**พัฒนาโดย:** CH32V003 Library Team
**รองรับบอร์ด:** CH32V003 Development Board
