# CircularBuffer Library

> **Library สำหรับจัดการข้อมูลแบบ FIFO (First-In-First-Out) โดยใช้ Circular Buffer สำหรับ CH32V003**

## 📋 สารบัญ

1. [ภาพรวม](#ภาพรวม)
2. [คุณสมบัติ](#คุณสมบัติ)
3. [การใช้งานพื้นฐาน](#การใช้งานพื้นฐาน)
4. [API Reference](#api-reference)

---

## ภาพรวม

Circular Buffer คือโครงสร้างข้อมูลที่ใช้หน่วยความจำแบบคงที่ สำหรับเก็บข้อมูลแบบคิว (Queue) เหมาะสำหรับใช้เป็น receive buffer ของ USART, SPI, I2C หรือการเก็บข้อมูลชั่วคราวทั่วไป

---

## คุณสมบัติ

- ✅ ขนาด buffer กำหนดเองได้ (user-provided buffer)
- ✅ Push/Pop แบบรายการเดี่ยวและหลายรายการ
- ✅ Peek (ดูค่าโดยไม่ดึงออกจากคิว)
- ✅ ไม่ใช้ทรัพยากรฮาร์ดแวร์ใดๆ

---

## การใช้งานพื้นฐาน

```c
#include "main.h"
#include "Lib/CircularBuffer/CircularBuffer.h"

uint8_t storage[64];
CircularBuffer cb;

int main(void) {
    CircularBuffer_Init(&cb, storage, sizeof(storage));

    CircularBuffer_Push(&cb, 0x55);
    CircularBuffer_Push(&cb, 0xAA);

    uint8_t data;
    while (CircularBuffer_Pop(&cb, &data) == CIRCULAR_BUFFER_OK) {
        printf("0x%02X ", data);
    }
}
```

---

## API Reference

- `CircularBuffer_Init(cb, buf, size)` : เริ่มต้น Circular Buffer
- `CircularBuffer_Push(cb, data)` : เพิ่มข้อมูลต่อท้ายคิว
- `CircularBuffer_Pop(cb, *data)` : นำข้อมูลหน้าคิวออก
- `CircularBuffer_Peek(cb, index, *data)` : ดูข้อมูลโดยไม่ดึงออก
- `CircularBuffer_Available(cb)` : จำนวนข้อมูลที่รออ่าน
- `CircularBuffer_Remaining(cb)` : พื้นที่ว่างที่เหลือ
- `CircularBuffer_IsEmpty(cb)` : ตรวจสอบว่าคิวว่าง
- `CircularBuffer_IsFull(cb)` : ตรวจสอบว่าคิวเต็ม
- `CircularBuffer_Flush(cb)` : ล้างข้อมูลทั้งหมด
- `CircularBuffer_PushMulti(cb, data, len)` : เพิ่มข้อมูลหลาย byte
- `CircularBuffer_PopMulti(cb, data, len)` : นำข้อมูลหลาย byte ออก

---
**พัฒนาโดย:** CH32V003 Library Team
**รองรับบอร์ด:** CH32V003 Development Board
