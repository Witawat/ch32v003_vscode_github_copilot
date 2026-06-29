# 05_I2C — ตัวอย่าง I2C

## ไฟล์ในโฟลเดอร์

| ไฟล์ | คำอธิบาย |
|------|----------|
| `ex01_I2C_Scan_Bus.c` | สแกนหา I2C devices (I2C_Scan, I2C_IsDeviceReady) |
| `ex02_I2C_EEPROM.c` | AT24Cxx EEPROM (I2C_WriteRegMulti, I2C_ReadRegMulti) |
| `ex03_I2C_MultiByte_Transfer.c` | BH1750 Light Sensor (I2C_Write, I2C_Read) |
| `ex04_Software_I2C.c` | Software I2C (I2C_Soft_Init — ใช้ GPIO ไหนก็ได้) |
| **`ex05_I2C_Remap.c`** | 🆕 I2C Remap (DEFAULT/PARTIAL/FULL pin configs) |

## Pin Configurations

| Config | SCL | SDA | SOP-8 | หมายเหตุ |
|--------|-----|-----|:---:|----------|
| `I2C_PINS_DEFAULT` | PC2 | PC1 | ⚠️ | ใช้ SimpleI2C_Soft บน SOP-8 |
| `I2C_PINS_PARTIAL_REMAP` 🆕 | PC2 | PC1 | ⚠️ | Pin mapping ต้องตรวจ datasheet |
| `I2C_PINS_REMAP` | PD0 | PD1 | ❌ (#error) | ต้องใช้ PD0 (เฉพาะ TSSOP-20/QFN-20) |

⚠️ ต้องต่อ **pull-up 4.7kΩ** ที่ SCL และ SDA ทุกครั้ง

## วิธีใช้ Remap

```c
#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include "SimpleHAL.h"

#if CH32V003_HAS_PD0
  I2C_SimpleInit(I2C_100KHZ, I2C_PINS_REMAP);      // PD0=SCL, PD1=SDA
#else
  I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);     // PC2=SCL, PC1=SDA
#endif
```
