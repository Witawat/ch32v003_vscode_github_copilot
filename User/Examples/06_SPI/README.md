# 06_SPI — ตัวอย่าง SPI

## ไฟล์ในโฟลเดอร์

| ไฟล์ | คำอธิบาย |
|------|----------|
| `ex01_SPI_Basic_Transfer.c` | SPI พื้นฐาน + W25Qxx (SPI_SimpleInit, SPI_Transfer) |
| `ex02_SPI_Buffer_Transfer.c` | Transfer buffer (SPI_Write, SPI_Read, SPI_TransferBuffer) |
| `ex03_SPI_Configuration.c` | ปรับแต่ง SPI (SPI_SetSpeed, SPI_SetBitOrder, MSB/LSB) |
| **`ex04_SPI_Remap.c`** | 🆕 SPI Pin Remap (DEFAULT PC5-7 / REMAP PD0-3) |

## Pin Configurations

| Config | SCK | MISO | MOSI | NSS | SOP-8 |
|--------|-----|------|------|-----|:---:|
| `SPI_PINS_DEFAULT` | PC5 | PC7 | PC6 | PC4 | ❌ |
| `SPI_PINS_REMAP` | PD1 | PD2 | PD3 | PD0 | ❌ |

⚠️ **SOP-8:** Hardware SPI ใช้ไม่ได้เลย! ใช้ `shiftOut()`/`shiftIn()` (software SPI) แทน  
⚠️ **SOP-16:** `SPI_PINS_REMAP` ใช้ไม่ได้ (ต้องใช้ PD0) — ใช้ `SPI_PINS_DEFAULT`  
✅ **TSSOP-20/QFN-20:** ใช้ได้ทั้ง 2 แบบ

## SOP-8 Alternative

```c
// Software SPI ผ่าน SimpleGPIO (ใช้กับ SOP-8 ได้)
#define DATA PD4
#define CLK  PD5
shiftOut(DATA, CLK, MSBFIRST, 0x55);
uint8_t rx = shiftIn(DATA, CLK, MSBFIRST);
```

## Remap Usage

```c
#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include "SimpleHAL.h"

#if CH32V003_HAS_PD0
  SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_REMAP);
#else
  SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);
#endif
```
