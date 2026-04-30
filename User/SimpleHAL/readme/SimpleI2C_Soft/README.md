# SimpleI2C_Soft — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `SimpleI2C_Soft.h / SimpleI2C_Soft.c`

---

## ภาพรวม

SimpleI2C_Soft ใช้เทคนิค **Bit-bang** สร้างสัญญาณ I2C ผ่าน GPIO ทั่วไปใดก็ได้ ไม่ต้องใช้ hardware I2C peripheral เหมาะสำหรับ:

- ต้องการ I2C bus ที่ 2 (CH32V003 มี hardware I2C แค่ 1 ตัว)
- Debug / ทดสอบกับ Logic Analyzer (รองรับ ignore ACK)
- ใช้ pin คู่ใดก็ได้เป็น SCL/SDA

---

## API Reference

### Initialization

#### `void I2C_Soft_Init(uint8_t scl_pin, uint8_t sda_pin, I2C_Soft_Speed speed)`

```c
I2C_Soft_Init(PC3, PC4, I2C_SOFT_100KHZ);  // ใช้ PC3=SCL, PC4=SDA
I2C_Soft_Init(PD5, PD6, I2C_SOFT_400KHZ);  // pin ใดก็ได้
```

> ต้องต่อ pull-up 4.7kΩ ที่ SCL และ SDA

---

### High-level Functions

#### `I2C_Soft_Status I2C_Soft_Write(uint8_t addr, uint8_t* data, uint16_t len, uint8_t ignore_ack)`

```c
uint8_t buf[] = {0x00, 0xAB};

// ปกติ — หยุดถ้าเจอ NACK
I2C_Soft_Write(0x50, buf, 2, 0);

// Debug mode — ส่งต่อแม้ไม่ได้ ACK (สำหรับ Logic Analyzer)
I2C_Soft_Write(0x50, buf, 2, 1);
```

| `ignore_ack` | พฤติกรรม |
|:---:|---------|
| `0` | หยุดส่งทันทีถ้าเจอ NACK |
| `1` | ส่งต่อไม่หยุด (ใช้สำหรับ debug/sniff) |

#### `I2C_Soft_Status I2C_Soft_Read(uint8_t addr, uint8_t* data, uint16_t len)`

```c
uint8_t buf[2];
I2C_Soft_Read(0x50, buf, 2);
```

---

### Low-level Functions

สำหรับ implement protocol เองแบบละเอียด

```c
I2C_Soft_Start();           // ส่ง START condition
I2C_Soft_Stop();            // ส่ง STOP condition
I2C_Soft_WriteByte(0x55);   // ส่ง 1 byte, คืน ACK (0=ACK, 1=NACK)
uint8_t b = I2C_Soft_ReadByte(1);   // อ่าน 1 byte, 1=ACK, 0=NACK
```

---

## Status Codes

```c
typedef enum {
    I2C_SOFT_OK        = 0,
    I2C_SOFT_ERROR_NACK = 1,
    I2C_SOFT_ERROR_BUSY = 2
} I2C_Soft_Status;
```

---

## ตัวอย่างการใช้งาน

### ขั้นต้น — I2C Bus ที่ 2 (คู่กับ Hardware I2C)

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    // Hardware I2C1 สำหรับ sensor หลัก
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);   // PC2=SCL, PC1=SDA

    // Software I2C สำหรับ device ที่ 2
    I2C_Soft_Init(PC3, PC4, I2C_SOFT_100KHZ);       // PC3=SCL, PC4=SDA

    // ใช้งานทั้งสอง bus พร้อมกัน
    I2C_WriteReg(0x68, 0x6B, 0x00);           // MPU6050 บน HW I2C
    uint8_t cmd = 0x01;
    I2C_Soft_Write(0x3C, &cmd, 1, 0);         // OLED บน SW I2C
}
```

### ขั้นกลาง — Debug ด้วย Logic Analyzer

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    // ต่อ Logic Analyzer ที่ PC3 (SCL) และ PC4 (SDA)
    I2C_Soft_Init(PC3, PC4, I2C_SOFT_100KHZ);

    uint8_t data[] = {0x00, 0xAE};  // OLED: display off

    while (1) {
        // ignore_ack=1 → ส่งได้แม้ไม่มี device (debug only)
        I2C_Soft_Write(0x3C, data, 2, 1);
        Delay_Ms(1000);
    }
}
```

### ขั้นสูง — Custom I2C Protocol ด้วย Low-level API

```c
#include "SimpleHAL.h"

// ตัวอย่าง: อ่าน SHT31 (ส่ง command แล้วรับ 6 bytes)
uint8_t read_sht31(uint16_t* temp_raw, uint16_t* hum_raw) {
    uint8_t buf[6];

    // ส่ง measurement command
    I2C_Soft_Start();
    if (I2C_Soft_WriteByte(0x44 << 1)) return 0;  // address + write, NACK=fail
    if (I2C_Soft_WriteByte(0x2C)) return 0;        // command MSB
    if (I2C_Soft_WriteByte(0x06)) return 0;        // command LSB
    I2C_Soft_Stop();

    Delay_Ms(15);  // measurement time

    // อ่านผลลัพธ์
    I2C_Soft_Start();
    if (I2C_Soft_WriteByte((0x44 << 1) | 1)) return 0;  // address + read
    for (uint8_t i = 0; i < 5; i++) {
        buf[i] = I2C_Soft_ReadByte(1);   // ACK
    }
    buf[5] = I2C_Soft_ReadByte(0);       // NACK (last byte)
    I2C_Soft_Stop();

    *temp_raw = (buf[0] << 8) | buf[1];
    *hum_raw  = (buf[3] << 8) | buf[4];
    return 1;
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    I2C_Soft_Init(PD3, PD4, I2C_SOFT_100KHZ);

    while (1) {
        uint16_t t, h;
        if (read_sht31(&t, &h)) {
            // แปลงค่า: T(°C) = -45 + 175 * t/65535
            USART_Print("T_raw="); USART_PrintNum(t);
            USART_Print(" H_raw="); USART_PrintNum(h);
            USART_Print("\r\n");
        }
        Delay_Ms(1000);
    }
}
```

---

## เปรียบเทียบ SimpleI2C vs SimpleI2C_Soft

| คุณสมบัติ | SimpleI2C (HW) | SimpleI2C_Soft (SW) |
|-----------|:---:|:---:|
| ความเร็ว | สูง (DMA ได้) | ต่ำกว่า (bit-bang) |
| Pin ที่ใช้ | PC2/PC1 หรือ PD0/PD1 | GPIO ใดก็ได้ |
| จำนวน bus | 1 ตัว | ไม่จำกัด (สร้างใหม่ได้) |
| ใช้ peripheral | ใช่ (I2C1) | ไม่ (GPIO เท่านั้น) |
| ignore ACK | ไม่รองรับ | รองรับ (debug mode) |
| เหมาะกับ | งานหลักที่ต้องความเร็ว | debug, bus ที่ 2, pin อิสระ |

---

## ข้อควรระวัง

- Software I2C ช้ากว่า hardware I2C ประมาณ 10-50x สำหรับ 400kHz จริงอาจได้แค่ 100-200kHz
- ห้ามใช้ pin เดียวกับ hardware I2C (PC1/PC2) เพราะ peripheral จะ interfere
- `ignore_ack=1` ใช้สำหรับ debug เท่านั้น อย่าใช้ใน production
- ไม่มี timeout protection — ถ้า bus stuck จะค้างไปเลย
