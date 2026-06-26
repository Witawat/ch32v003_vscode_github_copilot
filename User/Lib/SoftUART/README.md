# SoftUART Library

> **Software Bit-Bang UART สำหรับสร้าง Serial Port เพิ่มเติมบน CH32V003 (มี HW USART แค่ 1 ตัว)**

## 📋 สารบัญ

1. [ภาพรวม](#ภาพรวม)
2. [คุณสมบัติ](#คุณสมบัติ)
3. [ข้อจำกัด](#ข้อจำกัด)
4. [Hardware Setup](#hardware-setup)
5. [การใช้งานพื้นฐาน](#การใช้งานพื้นฐาน)
6. [API Reference](#api-reference)

---

## ภาพรวม

CH32V003 มี USART ฮาร์ดแวร์เพียง 1 ตัว SoftUART ช่วยสร้าง serial port เพิ่มโดยใช้ GPIO bit-bang ทำให้สามารถเชื่อมต่ออุปกรณ์ serial ได้หลายตัวพร้อมกัน

---

## คุณสมบัติ

- ✅ TX: bit-bang ด้วย Delay_Us (blocking)
- ✅ RX: polling mode พร้อม timeout
- ✅ Internal RX buffer (64 bytes)
- ✅ รองรับ baud rate 9600 - 115200
- ✅ ใช้ GPIO pin ใดก็ได้

---

## ข้อจำกัด

- TX/RX เป็นแบบ blocking (ปิด interrupt ชั่วคราวระหว่าง bit-bang)
- RX polling อาจพลาดข้อมูลถ้าเรียกไม่ทันความเร็ว baud
- แนะนำ 9600-38400 baud เพื่อความเสถียร

---

## Hardware Setup

| SoftUART Pin | CH32V003 Pin | หมายเหตุ |
|--------------|--------------|----------|
| TX           | PA2          | ต่อกับ RX ของอุปกรณ์ปลายทาง |
| RX           | PA1          | ต่อกับ TX ของอุปกรณ์ปลายทาง |
| GND          | GND          | กราวด์ร่วม |

> เลือก pin ใดก็ได้ ใช้แค่ `pinMode()` และ `digitalWrite/Read`

---

## การใช้งานพื้นฐาน

```c
#include "main.h"
#include "Lib/SoftUART/SoftUART.h"

SoftUART_Instance uart;

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    SoftUART_Init(&uart, PA2, PA1, 9600);

    SoftUART_Printf(&uart, "Hello from SoftUART!\r\n");

    while (1) {
        uint8_t data;
        if (SoftUART_ReadByte(&uart, &data, 100) == SOFTUART_OK) {
            SoftUART_WriteByte(&uart, data);
        }
    }
}
```

---

## API Reference

- `SoftUART_Init(uart, tx_pin, rx_pin, baud)` : เริ่มต้น SoftUART
- `SoftUART_SetBaud(uart, baud)` : เปลี่ยน baud rate
- `SoftUART_WriteByte(uart, data)` : ส่ง 1 byte
- `SoftUART_ReadByte(uart, *data, timeout_ms)` : อ่าน 1 byte (มี timeout)
- `SoftUART_Available(uart)` : จำนวน byte ใน RX buffer
- `SoftUART_Flush(uart)` : ล้าง RX buffer
- `SoftUART_Write(uart, data, len)` : ส่งข้อมูลหลาย byte
- `SoftUART_WriteString(uart, str)` : ส่ง string
- `SoftUART_Printf(uart, format, ...)` : ส่งแบบ formatted print

---

## ข้อจำกัด

| ข้อจำกัด | รายละเอียด |
|----------|-----------|
| **Baud ≤ 38400** | Baud สูงกว่า timing error สะสม ~8% ต่อ 10 bits |
| **Baud=0 guard** | Return error — ป้องกัน DIV/0 |
| **RX buffer** | Ring buffer ไม่รับข้อมูลแบบ async — ใช้ `SoftUART_ReadByte` พร้อม timeout |
| **`Timer_Init()`** | ต้องเรียก `Timer_Init()` หลัง `SystemCoreClockUpdate()` |

ดูข้อจำกัดทั้งหมด: [`LIMITATIONS.md`](../LIMITATIONS.md)

---
**พัฒนาโดย:** CH32V003 Library Team
**รองรับบอร์ด:** CH32V003 Development Board
