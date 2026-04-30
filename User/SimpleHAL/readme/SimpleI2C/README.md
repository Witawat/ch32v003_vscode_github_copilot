# SimpleI2C — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `SimpleI2C.h / SimpleI2C.c`

---

## ภาพรวม

SimpleI2C ห่อหุ้ม Hardware I2C1 ให้ใช้งานง่ายแบบ Arduino Wire รองรับ Standard mode (100kHz) และ Fast mode (400kHz) พร้อม helper สำหรับ register access และ I2C bus scanning

---

## Pin Configuration

| Config | SCL | SDA | หมายเหตุ |
|--------|-----|-----|---------|
| `I2C_PINS_DEFAULT` | PC2 | PC1 | ใช้บ่อยที่สุด |
| `I2C_PINS_REMAP`   | PD0 | PD1 | สำรอง |

> ต้องต่อ **pull-up resistor 4.7kΩ** ที่ SDA และ SCL ทุกครั้ง

---

## I2C Status Codes

```c
typedef enum {
    I2C_OK = 0,           // สำเร็จ
    I2C_ERROR_TIMEOUT,    // หมดเวลา
    I2C_ERROR_NACK,       // Device ไม่ตอบ
    I2C_ERROR_BUS_BUSY    // Bus ยุ่ง
} I2C_Status;
```

---

## API Reference

### Initialization

#### `void I2C_SimpleInit(I2C_Speed speed, I2C_PinConfig pin_config)`

```c
I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);  // 100kHz, PC2=SCL, PC1=SDA
I2C_SimpleInit(I2C_400KHZ, I2C_PINS_DEFAULT);  // 400kHz fast mode
I2C_SimpleInit(I2C_100KHZ, I2C_PINS_REMAP);    // 100kHz, PD0=SCL, PD1=SDA
```

---

### Raw Read / Write

#### `I2C_Status I2C_Write(uint8_t addr, uint8_t* data, uint16_t len)`

ส่งข้อมูล len bytes ไปยัง device

```c
uint8_t buf[] = {0x01, 0x02, 0x03};
if (I2C_Write(0x50, buf, 3) == I2C_OK) {
    // สำเร็จ
}
```

#### `I2C_Status I2C_Read(uint8_t addr, uint8_t* data, uint16_t len)`

รับข้อมูล len bytes จาก device

```c
uint8_t buf[4];
if (I2C_Read(0x50, buf, 4) == I2C_OK) {
    // buf[0..3] มีข้อมูล
}
```

---

### Register Access

#### `I2C_Status I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t data)`

เขียน 1 byte ไปยัง register

```c
I2C_WriteReg(0x68, 0x6B, 0x00);  // MPU6050: PWR_MGMT_1 = 0 (wake up)
I2C_WriteReg(0x3C, 0x00, 0x8D);  // OLED: command byte
```

#### `uint8_t I2C_ReadReg(uint8_t addr, uint8_t reg)`

อ่าน 1 byte จาก register — คืน `0xFF` ถ้า error

```c
uint8_t who_am_i = I2C_ReadReg(0x68, 0x75);  // MPU6050: WHO_AM_I
```

#### `I2C_Status I2C_ReadRegMulti(uint8_t addr, uint8_t reg, uint8_t* buf, uint16_t len)`

อ่านหลาย bytes ต่อเนื่องจาก register — ใช้สำหรับ burst read

```c
uint8_t raw[6];
// MPU6050: อ่าน Accel X/Y/Z (6 bytes ตั้งแต่ register 0x3B)
if (I2C_ReadRegMulti(0x68, 0x3B, raw, 6) == I2C_OK) {
    int16_t ax = (raw[0] << 8) | raw[1];
    int16_t ay = (raw[2] << 8) | raw[3];
    int16_t az = (raw[4] << 8) | raw[5];
}
```

---

### Bus Utilities

#### `uint8_t I2C_Scan(uint8_t* found_devices, uint8_t max_count)`

สแกนหา device ที่มีบน bus — คืนจำนวนที่พบ

```c
uint8_t devices[16];
uint8_t count = I2C_Scan(devices, 16);
for (uint8_t i = 0; i < count; i++) {
    USART_Print("Found: 0x");
    USART_PrintHex(devices[i], 1);
    USART_Print("\r\n");
}
```

#### `uint8_t I2C_IsDeviceReady(uint8_t addr)`

ตรวจว่า device ตอบสนองหรือไม่ — คืน `1` ถ้าพร้อม

```c
if (!I2C_IsDeviceReady(0x68)) {
    USART_Print("MPU6050 not found!\r\n");
}
```

---

## ตัวอย่างการใช้งาน

### ขั้นต้น — อ่าน/เขียน EEPROM AT24C32

```c
#include "SimpleHAL.h"

#define EEPROM_ADDR  0x50

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);

    // เขียน byte ที่ address 0x00
    uint8_t write_buf[] = {0x00, 0x00, 0xAB};  // addr_hi, addr_lo, data
    I2C_Write(EEPROM_ADDR, write_buf, 3);
    Delay_Ms(10);  // EEPROM write cycle time

    // อ่านกลับมา
    uint8_t addr_buf[] = {0x00, 0x00};
    I2C_Write(EEPROM_ADDR, addr_buf, 2);  // set address
    uint8_t val = 0;
    I2C_Read(EEPROM_ADDR, &val, 1);

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    USART_Print("Read: 0x");
    USART_PrintHex(val, 1);
    USART_Print("\r\n");
}
```

### ขั้นกลาง — อ่าน MPU6050 (IMU)

```c
#include "SimpleHAL.h"

#define MPU_ADDR  0x68

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    I2C_SimpleInit(I2C_400KHZ, I2C_PINS_DEFAULT);

    // Wake up MPU6050
    I2C_WriteReg(MPU_ADDR, 0x6B, 0x00);
    Delay_Ms(100);

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    uint8_t raw[6];
    while (1) {
        I2C_ReadRegMulti(MPU_ADDR, 0x3B, raw, 6);
        int16_t ax = (raw[0] << 8) | raw[1];
        int16_t ay = (raw[2] << 8) | raw[3];
        int16_t az = (raw[4] << 8) | raw[5];

        USART_Print("ax="); USART_PrintNum(ax);
        USART_Print(" ay="); USART_PrintNum(ay);
        USART_Print(" az="); USART_PrintNum(az);
        USART_Print("\r\n");
        Delay_Ms(100);
    }
}
```

### ขั้นสูง — Multi-device I2C พร้อม Error Recovery

```c
#include "SimpleHAL.h"

typedef struct {
    uint8_t  addr;
    const char* name;
    uint8_t  present;
} I2C_Device;

I2C_Device devices[] = {
    {0x68, "MPU6050", 0},
    {0x76, "BMP280",  0},
    {0x3C, "OLED",    0},
};

void scan_devices(void) {
    for (uint8_t i = 0; i < 3; i++) {
        devices[i].present = I2C_IsDeviceReady(devices[i].addr);
        USART_Print(devices[i].name);
        USART_Print(devices[i].present ? ": OK\r\n" : ": NOT FOUND\r\n");
    }
}

// อ่าน register พร้อม retry
I2C_Status read_reg_retry(uint8_t addr, uint8_t reg, uint8_t* out, uint8_t retries) {
    for (uint8_t i = 0; i < retries; i++) {
        I2C_Status st = I2C_ReadRegMulti(addr, reg, out, 1);
        if (st == I2C_OK) return I2C_OK;
        Delay_Ms(10);
    }
    return I2C_ERROR_TIMEOUT;
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);

    scan_devices();

    while (1) {
        if (devices[0].present) {
            uint8_t val;
            if (read_reg_retry(0x68, 0x75, &val, 3) == I2C_OK) {
                USART_Print("WHO_AM_I=0x");
                USART_PrintHex(val, 1);
                USART_Print("\r\n");
            }
        }
        Delay_Ms(500);
    }
}
```

---

## ข้อควรระวัง

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| `I2C_ERROR_NACK` ตลอด | Address ผิด หรือไม่มี pull-up | ตรวจสอบ address (7-bit) และต่อ 4.7kΩ |
| Bus stuck (ค้างที่ SDA LOW) | MCU reset กลางกาน transaction | ต่อ clock 9 ครั้งบน SCL เพื่อ release |
| ข้อมูลผิดพลาดที่ 400kHz | สาย/PCB track ยาวเกิน | ลดเป็น 100kHz หรือสายสั้นลง |
| `I2C_ReadReg` คืน `0xFF` | Error แต่ตรวจจากค่าลำบาก | ใช้ `I2C_ReadRegMulti` แล้วตรวจ return status |
| ใช้ I2C2 ไม่ได้ | CH32V003 มีแค่ I2C1 | ใช้ `SimpleI2C_Soft` ถ้าต้องการ bus ที่ 2 |
