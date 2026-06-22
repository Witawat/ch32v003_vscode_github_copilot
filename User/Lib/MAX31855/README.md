# MAX31855 / MAX6675 Thermocouple Library

> **Library สำหรับอ่านอุณหภูมิจาก Thermocouple ผ่าน MAX31855 หรือ MAX6675 ผ่าน SPI สำหรับ CH32V003**

## 📋 สารบัญ

1. [ภาพรวม](#ภาพรวม)
2. [คุณสมบัติ](#คุณสมบัติ)
3. [Hardware Setup](#hardware-setup)
4. [การใช้งานพื้นฐาน](#การใช้งานพื้นฐาน)
5. [API Reference](#api-reference)

---

## ภาพรวม

MAX31855 และ MAX6675 เป็น IC แปลงสัญญาณจาก Thermocouple เป็นดิจิทัลผ่าน SPI ใช้วัดอุณหภูมิสูงที่ thermocouple ทั่วไปไม่สามารถวัดได้

**MAX31855:** รองรับ K/J/N/T/S/E/B type thermocouple
- Resolution: 0.25°C (thermocouple), 0.0625°C (internal)
- Range: -270°C ถึง +1800°C (ขึ้นกับ Thermocouple Type)

**MAX6675:** รองรับ K-type thermocouple
- Resolution: 0.25°C
- Range: 0°C ถึง +1024°C

---

## คุณสมบัติ

- ✅ รองรับทั้ง MAX31855 และ MAX6675
- ✅ อ่านอุณหภูมิ thermocouple (°C)
- ✅ อ่าน internal temperature (cold-junction compensation)
- ✅ ตรวจสอบ fault (open circuit, short to GND, short to VCC)

---

## Hardware Setup

| MAX31855 Pin | CH32V003 Pin | หมายเหตุ |
|-------------|--------------|----------|
| VCC         | 3.3V         | แรงดันไฟเลี้ยง |
| GND         | GND          | กราวด์ร่วม |
| SCK         | PC5          | SPI Clock |
| SO          | PC7 (MISO)   | Slave Out (data จาก MAX31855) |
| CS          | GPIO_Pin     | Chip Select (เลือก pin ใดก็ได้) |

> ใช้ SPI Mode 0 (CPOL=0, CPHA=0) ด้วย `SPI_SimpleInit(SPI_MODE0, SPI_2MHZ, SPI_PINS_DEFAULT)`

---

## การใช้งานพื้นฐาน

```c
#include "main.h"
#include "Lib/MAX31855/MAX31855.h"

MAX31855_Instance therm;

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    SPI_SimpleInit(SPI_MODE0, SPI_2MHZ, SPI_PINS_DEFAULT);

    MAX31855_Init(&therm, PC0);  // CS = PC0

    while (1) {
        float temp     = MAX31855_ReadTemp(&therm);
        float internal = MAX31855_ReadInternalTemp(&therm);

        uint8_t fault;
        MAX31855_GetFault(&therm, &fault);

        printf("TC: %.2fC  Int: %.2fC  Fault: 0x%02X\r\n",
               temp, internal, fault);
        Delay_Ms(500);
    }
}
```

### การใช้ MAX6675

```c
float temp = MAX6675_ReadTemp(&therm);
printf("Temp: %.2fC\r\n", temp);
```

---

## API Reference

### MAX31855

- `MAX31855_Init(therm, cs_pin)` : เริ่มต้น (ใช้ CS pin ที่กำหนด)
- `MAX31855_ReadTemp(therm)` : อ่านอุณหภูมิ thermocouple (°C)
- `MAX31855_ReadInternalTemp(therm)` : อ่าน internal temperature (°C)
- `MAX31855_GetFault(therm, *fault)` : อ่านสถานะ fault
- `MAX31855_IsThermocoupleOpen(therm)` : ตรวจสอบ thermocouple ขาด
- `MAX31855_IsShortedToGND(therm)` : ตรวจสอบ short to GND
- `MAX31855_IsShortedToVCC(therm)` : ตรวจสอบ short to VCC

### MAX6675

- `MAX6675_ReadTemp(therm)` : อ่านอุณหภูมิ (°C) สำหรับ MAX6675

---
**พัฒนาโดย:** CH32V003 Library Team
**รองรับบอร์ด:** CH32V003 Development Board
