# 04_USART — ตัวอย่าง USART

## ไฟล์ในโฟลเดอร์

| ไฟล์ | คำอธิบาย |
|------|----------|
| `ex01_Hello_World.c` | ส่งข้อความพื้นฐาน (USART_SimpleInit, USART_Print) |
| `ex02_Print_Formatted.c` | ส่งตัวเลข/hex (USART_PrintNum, USART_PrintHex) |
| `ex03_Echo_Command.c` | รับคำสั่ง (USART_Available, USART_Read, LED=ON/OFF) |
| `ex04_USART_Read_Bytes.c` | อ่านหลาย bytes (USART_ReadBytes) |
| `ex05_DMA_USART_Transmit.c` | ส่งผ่าน DMA (DMA_USART_Send) |
| **`ex06_USART_Remap.c`** | 🆕 เปลี่ยน Pin Config (DEFAULT/REMAP1/REMAP2/FULL) |
| **`ex07_USART_Package_Aware.c`** | 🆕 รองรับทุกแพ็กเกจ (เปลี่ยน CH32V003_PACKAGE แล้ว rebuild) |

## Pin Configurations

| Config | TX | RX | SOP-8 | SOP-16 |
|--------|----|----|:---:|:---:|
| `USART_PINS_DEFAULT` | PD5 | PD6 | ✅ | ✅ |
| `USART_PINS_REMAP1` | PD0 | PD1 | ❌ (#error) | ❌ (#error) |
| `USART_PINS_REMAP2` | PD6 | PD5 | ✅ | ✅ |
| `USART_PINS_FULL_REMAP` 🆕 | PD6 | PD5 | ✅ | ✅ |

> ⚠️ `USART_PINS_REMAP1` ต้องใช้ PD0 (มีเฉพาะ TSSOP-20/QFN-20)

## Quick Start

```c
#define CH32V003_PACKAGE  PACKAGE_TSSOP20  // หรือ SOP8, SOP16
#include "SimpleHAL.h"

USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
USART_Print("Hello CH32V003!\r\n");
```
