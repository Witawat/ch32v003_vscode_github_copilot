# AHT10/AHT20 Library

> **Library สำหรับอ่านอุณหภูมิและความชื้นจาก AHT10/AHT20 ผ่าน I2C สำหรับ CH32V003**

## 📋 สารบัญ

1. [ภาพรวม](#ภาพรวม)
2. [คุณสมบัติ](#คุณสมบัติ)
3. [Hardware Setup](#hardware-setup)
4. [การใช้งานพื้นฐาน](#การใช้งานพื้นฐาน)
5. [API Reference](#api-reference)

---

## ภาพรวม

AHT10 และ AHT20 เป็นเซนเซอร์วัดอุณหภูมิและความชื้นความแม่นยำสูง ใช้การสื่อสารแบบ I2C ได้รับความนิยมสูงในปัจจุบัน เนื่องจากมีความแม่นยำกว่า DHT11/DHT22 และไม่ต้องใช้ timing ที่ยุ่งยาก

---

## คุณสมบัติ

- ✅ อุณหภูมิ: -40 ถึง +85°C, ±0.3°C accuracy
- ✅ ความชื้น: 0 ถึง 100% RH, ±2% (AHT20) / ±3% (AHT10)
- ✅ ใช้ I2C interface (address 0x38)
- ✅ รองรับทั้ง AHT10 และ AHT20 (protocol เดียวกัน)

---

## Hardware Setup

| AHT10 Pin | CH32V003 Pin | หมายเหตุ |
|-----------|--------------|----------|
| VCC       | 3.3V         | แรงดันไฟเลี้ยง |
| GND       | GND          | กราวด์ร่วม |
| SCL       | PC2          | I2C Clock (ต้องมี Pull-up 4.7kΩ) |
| SDA       | PC1          | I2C Data (ต้องมี Pull-up 4.7kΩ) |

---

## การใช้งานพื้นฐาน

```c
#include "main.h"
#include "Lib/AHT10/AHT10.h"

AHT10_Instance sensor;

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);

    if (AHT10_Init(&sensor) == AHT10_OK) {
        while (1) {
            float temp, hum;
            if (AHT10_Read(&sensor, &temp, &hum) == AHT10_OK) {
                printf("T=%.1fC H=%.1f%%\r\n", temp, hum);
            }
            Delay_Ms(1000);
        }
    }
}
```

---

## API Reference

- `AHT10_Init(sensor)` : เริ่มต้นเซนเซอร์
- `AHT10_Read(sensor, *temp, *humidity)` : อ่านอุณหภูมิ (°C) และความชื้น (%)
- `AHT10_SoftReset(sensor)` : รีเซ็ตเซนเซอร์
- `AHT10_GetStatus(sensor, *status)` : อ่าน status register
- `AHT10_IsCalibrated(sensor)` : ตรวจสอบว่า calibrated หรือยัง
- `AHT10_IsBusy(sensor)` : ตรวจสอบว่ากำลังวัดค่าอยู่หรือไม่

---
**พัฒนาโดย:** CH32V003 Library Team
**รองรับบอร์ด:** CH32V003 Development Board
