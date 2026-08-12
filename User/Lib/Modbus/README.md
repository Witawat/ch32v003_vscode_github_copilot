# Modbus RTU Library (Master) — USART + DMA

ไลบรารี Modbus RTU แบบ Master สำหรับ CH32V003 — ใช้คุยกับอุปกรณ์ slave
เช่น PZEM-004T v3, VFD, เซนเซอร์อุตสาหกรรม, PLC ฯลฯ ผ่าน USART1

## สารบัญ

1. [ภาพรวม](#ภาพรวม)
2. [คุณสมบัติ](#คุณสมบัติ)
3. [Hardware Setup](#hardware-setup)
4. [หลักการทำงาน](#หลักการทำงาน)
5. [การใช้งานพื้นฐาน (โหมด USART)](#การใช้งานพื้นฐานโหมด-usart)
6. [การใช้งานขั้นสูง (โหมด DMA)](#การใช้งานขั้นสูงโหมด-dma)
7. [Troubleshooting](#troubleshooting)
8. [API Reference](#api-reference)
9. [ข้อจำกัด](#ข้อจำกัด)

---

## ภาพรวม

- **โปรโตคอล:** Modbus RTU (Master) — 9600 baud, 8N1
- **Function Code:** 01, 02, 03, 04, 05, 06, 0F, 10 (อ่าน/เขียน Coil + Register)
- **2 โหมดขนส่ง:** USART (ring buffer) และ DMA (circular buffer + IDLE interrupt)
- **ตรวจสอบ:** CRC-16 Modbus, timeout, exception response

---

## คุณสมบัติ

- อ่าน Holding/Input Registers, Coils, Discrete Inputs
- เขียน Single/Multiple Register และ Coil
- ตรวจจับและรายงาน Modbus exception (รหัส 01-08)
- โหมด DMA — รับข้อมูลโดย CPU ไม่ถูก interrupt ทุก byte (ใช้ DMA_CH2 TX + DMA_CH3 RX)
- API instance-based เดียวกับไลบรารีอื่นในโปรเจกต์

---

## Hardware Setup

### แบบจุดต่อจุด (direct)

| CH32V003 (Master) | Slave (Modbus RTU) |
|---|---|
| PD5 (TX) | RX |
| PD6 (RX) | TX |
| GND | GND |

### แบบ RS-485 (ระยะไกล / หลายอุปกรณ์)

| CH32V003 | MAX485 | Bus |
|---|---|---|
| PD5 (TX) | DI | — |
| PD6 (RX) | RO | — |
| GPIO (เช่น PC0) | DE + RE (ต่อรวม) | — |
| — | A | A (485+) |
| — | B | B (485-) |

> ตั้ง DE=1 ก่อนส่ง, DE=0 หลังส่งเสร็จ — ตัวอย่างใน `Examples/ex04_rs485_de_pin.c`
> RS-485 ต้องใช้ 2 สาย (A/B) + ตัวต้านทานจบสาย 120Ω ที่ปลายทั้งสอง

---

## หลักการทำงาน

### เฟรม Modbus RTU

```
Request:  [SlaveAddr][FC][Data...][CRC_LO][CRC_HI]
Response: [SlaveAddr][FC][Data...][CRC_LO][CRC_HI]
```

- ระยะห่างระหว่างเฟรมต้องเงียบ ≥ 3.5 character time (~4ms ที่ 9600 baud)
- CRC-16 (poly 0xA001) คำนวณครอบทุก byte ยกเว้น CRC เอง
- Exception response: FC + 0x80 + รหัสเหตุผล

### โหมดขนส่ง 2 แบบ

| | USART | DMA |
|---|---|---|
| RX | RXNE interrupt → ring buffer ของ SimpleUSART | DMA_CH3 circular buffer + IDLE interrupt |
| TX | USART_WriteByte (polling) | DMA_CH2 (DMA_USART_Send) |
| จับจบเฟรม | อ่านตามโครงสร้าง frame | USART IDLE line (เส้นเงียบ > 1 char time) |
| CPU โหลด | interrupt ทุก byte | interrupt เฉพาะจบเฟรม |
| ใช้กับ | งานง่าย ไม่ยุ่งกับ DMA | งานที่ CPU ต้องว่าง / โปรโตคอลจับเวลา |

---

## การใช้งานพื้นฐาน (โหมด USART)

### Quick start

```c
#include "SimpleHAL.h"
#include "Modbus.h"

Modbus mb;
uint16_t regs[10];

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();

    MODBUS_Init(&mb, 1, MODBUS_TRANSPORT_USART, USART_PINS_DEFAULT);

    while (1) {
        if (MODBUS_ReadHoldingRegisters(&mb, 0x0000, 10, regs) == MODBUS_OK) {
            // regs[0..9] — ค่าจาก slave (Big-endian แปลงให้แล้ว)
        } else {
            const char* err = MODBUS_StatusStr(MODBUS_GetLastError(&mb));
            // USART_Print(err);
        }
        Delay_Ms(1000);
    }
}
```

### เขียน register

```c
MODBUS_WriteSingleRegister(&mb, 0x0000, 1000);   // reg 0 = 1000
MODBUS_WriteSingleCoil(&mb, 3, 1);               // coil 3 = ON
```

### อ่าน coils (bit-packed)

```c
uint8_t coils[4];  // รองรับสูงสุด 32 coils
if (MODBUS_ReadCoils(&mb, 0, 20, coils) == MODBUS_OK) {
    uint8_t c0 = coils[0] & 0x01;        // coil 0
    uint8_t c5 = (coils[0] >> 5) & 0x01; // coil 5
    uint8_t c16 = (coils[2] >> 0) & 0x01; // coil 16
}
```

---

## การใช้งานขั้นสูง (โหมด DMA)

### เปิดโหมด DMA

```c
Modbus mb;
MODBUS_Init(&mb, 1, MODBUS_TRANSPORT_DMA, USART_PINS_DEFAULT);
```

โหมด DMA เปลี่ยนเพียงบรรทัดเดียว — API อื่นเหมือนกันทุกอย่าง

### สิ่งที่โหมด DMA ทำอัตโนมัติ

1. `USART_SimpleInit` เปิด USART 9600 8N1
2. `DMA_USART_InitRx(DMA_CH3, buf, 256, circular)` + เปิด channel
3. `DMA_USART_InitTx(DMA_CH2)` + ใช้ `DMA_USART_Send`
4. ปิด RXNE interrupt (กัน ring buffer แย่งข้อมูลกับ DMA) + เปิด IDLE interrupt
5. override `USART_IdleHook()` — คัดลอกเฟรมออกจาก circular buffer ทันทีใน ISR

> ⚠️ `USART_IdleHook()` เป็น global — ถ้าโปรเจกต์มีไลบรารีอื่นที่ override hook นี้ด้วย
> จะชนกัน (link error) — ปัจจุบันมีแค่ Modbus เท่านั้นที่ใช้

### ข้อควรระวัง DMA

- DMA_CH2/CH3 ถูก Modbus ยึด — ห้ามใช้กับ `DMA_analogReadStart` หรือ DMA อื่น
- RAM เพิ่มเติม ~1.3KB (DMA buffer 256B + capture buffer 258B + protocol buffers)
- CH32V003 stack มีแค่ 256 bytes — ไลบรารีใช้ static buffer ทั้งหมดแล้ว
- ⚠️ **ห้ามใช้ `USART_Print`/`USART_Println` ระหว่างที่กำลังรอ response** — ระหว่าง
  DMA TX กำลังส่งเฟรมคำขอ การพิมพ์จะแทรก byte เข้าเฟรม Modbus (ข้อมูลเสียหาย) —
  ให้พิมพ์หลังคำขอเสร็จ หรือใช้ USART2/SoftUART สำหรับ debug
- ไลบรารีรอ `USART_FLAG_TC` หลังส่งเสร็จแล้ว (กัน DE/RE ของ RS-485 สลับเร็วเกิน)

### ตัวอย่างโหมด DMA

```c
#include "SimpleHAL.h"
#include "Modbus.h"

Modbus mb;
uint16_t regs[10];

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();

    MODBUS_Init(&mb, 1, MODBUS_TRANSPORT_DMA, USART_PINS_DEFAULT);

    while (1) {
        if (MODBUS_ReadInputRegisters(&mb, 0x0000, 10, regs) == MODBUS_OK) {
            // CPU ทำงานอื่นระหว่างรอ response ได้ — รับเฟรมจบด้วย IDLE interrupt
        }
        Delay_Ms(1000);
    }
}
```

### จัดการ Modbus exception

```c
Modbus_Status st = MODBUS_ReadHoldingRegisters(&mb, 0x0000, 10, regs);
if (st == MODBUS_ERROR_EXCEPT) {
    uint8_t ex = MODBUS_GetLastException(&mb);
    // ex = MODBUS_EX_ILLEGAL_DATA_ADDR (0x02) = register เกินช่วง ฯลฯ
}
```

---

## Troubleshooting

### Timeout ตลอด

| สาเหตุ | วิธีแก้ |
|---|---|
| สาย TX/RX กลับด้าน | ต่อไขว้ TX→RX, RX→TX |
| baud ไม่ตรง | slave ต้องเป็น 9600 8N1 (บางรุ่น 4800/19200) |
| slave address ผิด | เช็ค DIP switch / config ของ slave |
| ไม่ต่อ GND ร่วม | ต่อ GND ให้ครบ |
| slave ตอบช้าเกิน 500ms | เพิ่ม `#define MODBUS_TIMEOUT_MS 1000` ก่อน include |

> 💡 timeout วัดรวมทั้งคำขอ (ตั้งแต่ส่งคำขอจนรับ response ครบ) — อ่าน 125 registers
> ที่ 9600 baud ใช้เวลา ~265ms ยังพอดีในค่า default 500ms — ถ้า baud ต่ำกว่าหรือ
> slave ตอบช้า ให้เพิ่ม MODBUS_TIMEOUT_MS

### CRC Error

| สาเหตุ | วิธีแก้ |
|---|---|
| สายยาว/มี noise | ใช้ RS-485 (MAX485) + สาย twisted pair |
| ส่งข้อมูลซ้อนเฟรม | เว้นระยะ ≥ 3.5 char time ระหว่าง poll |

### Exception 02 (Illegal Data Address)

| สาเหตุ | วิธีแก้ |
|---|---|
| register เกินช่วง | เปิดคู่มือ slave ดู register map จริง |
| อ่านเกิน 125 regs/ครั้ง | แบ่งอ่านหลายครั้ง (ครั้งละ ≤ 125) |

### โหมด DMA ใช้ไม่ได้ แต่ USART ปกติ

| สาเหตุ | วิธีแก้ |
|---|---|
| DMA_CH2/3 ถูกใช้ที่อื่น | ปลดการใช้ DMA ของ ADC ฯลฯ |
| RAM ไม่พอ | ลด buffer อื่น (OLED 1KB) หรือใช้โหมด USART |
| IDLE hook ชนกับไลบรารีอื่น | มี override ได้ตัวเดียวต่อโปรเจกต์ |

---

## API Reference

### Constants

- MODBUS_TIMEOUT_MS = 500
- MODBUS_BAUD = BAUD_9600
- MODBUS_DMA_RX_SIZE = 256
- MODBUS_MAX_RESP_DATA = 250
- MODBUS_ADDR_BROADCAST = 0x00

### Types

- Modbus_Transport
  - MODBUS_TRANSPORT_USART
  - MODBUS_TRANSPORT_DMA
- Modbus_Status
  - MODBUS_OK, MODBUS_ERROR_PARAM, MODBUS_ERROR_TIMEOUT
  - MODBUS_ERROR_CRC, MODBUS_ERROR_RESP, MODBUS_ERROR_EXCEPT, MODBUS_ERROR_BUSY
- Modbus (instance)

### Functions

- Modbus_Status MODBUS_Init(Modbus* mb, uint8_t slave_addr, Modbus_Transport transport, uint8_t pin_config)
- Modbus_Status MODBUS_ReadHoldingRegisters(Modbus* mb, uint16_t reg, uint16_t count, uint16_t* data)
- Modbus_Status MODBUS_ReadInputRegisters(Modbus* mb, uint16_t reg, uint16_t count, uint16_t* data)
- Modbus_Status MODBUS_ReadCoils(Modbus* mb, uint16_t coil, uint16_t count, uint8_t* data)
- Modbus_Status MODBUS_ReadDiscreteInputs(Modbus* mb, uint16_t input, uint16_t count, uint8_t* data)
- Modbus_Status MODBUS_WriteSingleCoil(Modbus* mb, uint16_t coil, uint8_t value)
- Modbus_Status MODBUS_WriteSingleRegister(Modbus* mb, uint16_t reg, uint16_t value)
- Modbus_Status MODBUS_WriteMultipleRegisters(Modbus* mb, uint16_t reg, uint16_t count, const uint16_t* data)
- Modbus_Status MODBUS_WriteMultipleCoils(Modbus* mb, uint16_t coil, uint16_t count, const uint8_t* data)
- Modbus_Status MODBUS_GetLastError(Modbus* mb)
- uint8_t MODBUS_GetLastException(Modbus* mb)
- const char* MODBUS_StatusStr(Modbus_Status status)
- uint16_t MODBUS_CRC16(const uint8_t* buf, uint16_t len)

### สัมพันธ์กับไลบรารีอื่น

| ไลบรารี | ความสัมพันธ์ |
|---|---|
| SimpleUSART | ใช้เป็นฐาน — โหมด DMA ต้องปิด RXNE IT (ทำอัตโนมัติ) |
| SimpleDMA | ใช้ DMA_CH2 (TX) + DMA_CH3 (RX) |
| PZEM004Tv3 | มี Modbus ในตัวเฉพาะอุปกรณ์ — ไลบรารีนี้เป็นแบบทั่วไป |

---

## ข้อจำกัด

- **Master อย่างเดียว** — ไม่รองรับโหมด Slave (CH32V003 เป็นผู้ถามอย่างเดียว)
- ใช้ USART1 เพียงตัวเดียว → มี Modbus master ได้ตัวเดียวต่อโปรเจกต์
- โหมด DMA ยึด DMA_CH2 + DMA_CH3
- RAM: โหมด USART ~1.0KB / โหมด DMA ~1.3KB (จาก RAM ทั้งหมด 2KB)
- ไม่ควบคุม DE/RE ของ MAX485 อัตโนมัติ — ตัวอย่างใน ex04
- ห้าม `USART_Print` ขณะคำขอ DMA กำลังทำงาน (byte แทรกเฟรม — ดู ข้อควรระวัง DMA)
