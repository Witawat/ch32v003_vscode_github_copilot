# SimpleSPI — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `SimpleSPI.h / SimpleSPI.c`

---

## ภาพรวม

SimpleSPI ห่อหุ้ม Hardware SPI1 ให้ใช้งานง่าย รองรับ 4 โหมด (Mode 0-3), ความเร็ว 6 ระดับ (125kHz – 12MHz) และ pin remapping CS ใช้ GPIO digital write ควบคุมเอง

---

## Pin Configuration

| Config | SCK | MISO | MOSI | NSS (CS) |
|--------|-----|------|------|---------|
| `SPI_PINS_DEFAULT` | PC5 | PC7 | PC6 | PC4 |
| `SPI_PINS_REMAP`   | PC5 | PC7 | PC6 | PA2 |

> NSS ไม่ได้ถูกใช้โดย hardware — จัดการด้วย `digitalWrite` เอง

---

## SPI Modes

| Mode | CPOL | CPHA | หมายเหตุ |
|:----:|:----:|:----:|---------|
| `SPI_MODE0` | 0 | 0 | ใช้บ่อยที่สุด (W25Qxx, MAX7219, SD) |
| `SPI_MODE1` | 0 | 1 | |
| `SPI_MODE2` | 1 | 0 | |
| `SPI_MODE3` | 1 | 1 | (ADS1115, TFT บางรุ่น) |

---

## Speed Options

| Enum | Clock | F = 48MHz / Divider |
|------|-------|---------------------|
| `SPI_125KHZ`  | 125kHz  | /256 (PCLK) |
| `SPI_375KHZ`  | 375kHz  | /128 |
| `SPI_750KHZ`  | 750kHz  | /64 |
| `SPI_1_5MHZ`  | 1.5MHz  | /32 |
| `SPI_3MHZ`    | 3MHz    | /16 |
| `SPI_6MHZ`    | 6MHz    | /8 |
| `SPI_12MHZ`   | 12MHz   | /4 |

> PCLK1 = HCLK/2 = 24MHz ดังนั้น `SPI_12MHZ` = 12MHz จริง

---

## API Reference

### Initialization

#### `void SPI_SimpleInit(SPI_Mode mode, SPI_Speed speed, SPI_PinConfig pin_config)`

```c
SPI_SimpleInit(SPI_MODE0, SPI_6MHZ,  SPI_PINS_DEFAULT);  // Fast, Mode0
SPI_SimpleInit(SPI_MODE3, SPI_1_5MHZ, SPI_PINS_DEFAULT); // Mode3 ช้ากว่า
```

---

### Transfer

#### `uint8_t SPI_Transfer(uint8_t data)`

ส่งและรับ 1 byte พร้อมกัน (full-duplex)

```c
uint8_t received = SPI_Transfer(0x55);
uint8_t dummy    = SPI_Transfer(0x00);  // รับข้อมูล (ส่ง dummy byte)
```

#### `void SPI_TransferBuffer(uint8_t* tx, uint8_t* rx, uint16_t len)`

ส่ง/รับ buffer ขนาด len bytes

```c
uint8_t tx[] = {0x01, 0x02, 0x03};
uint8_t rx[3];
SPI_TransferBuffer(tx, rx, 3);

// TX only (rx = NULL)
SPI_TransferBuffer(tx, NULL, 3);

// RX only (tx = NULL → ส่ง 0x00 dummy)
SPI_TransferBuffer(NULL, rx, 3);
```

#### `void SPI_Write(uint8_t* data, uint16_t len)`

ส่ง buffer (TX only, ไม่สนใจ RX)

```c
uint8_t cmd[] = {0xAB, 0xCD};
SPI_Write(cmd, 2);
```

---

## ตัวอย่างการใช้งาน

### ขั้นต้น — ส่งข้อมูลไปยัง MAX7219 (7-segment driver)

```c
#include "SimpleHAL.h"

#define CS_PIN  PC4

void max7219_write(uint8_t reg, uint8_t data) {
    digitalWrite(CS_PIN, LOW);
    SPI_Transfer(reg);
    SPI_Transfer(data);
    digitalWrite(CS_PIN, HIGH);
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    SPI_SimpleInit(SPI_MODE0, SPI_1_5MHZ, SPI_PINS_DEFAULT);
    pinMode(CS_PIN, PIN_MODE_OUTPUT);
    digitalWrite(CS_PIN, HIGH);

    // Init MAX7219
    max7219_write(0x09, 0xFF);   // decode mode: BCD for all digits
    max7219_write(0x0A, 0x07);   // intensity
    max7219_write(0x0B, 0x07);   // scan limit: 8 digits
    max7219_write(0x0C, 0x01);   // shutdown = normal

    // แสดงเลข 1-8
    for (uint8_t i = 1; i <= 8; i++) {
        max7219_write(i, i);
    }
}
```

### ขั้นกลาง — อ่าน/เขียน W25Q Flash Memory

```c
#include "SimpleHAL.h"

#define FLASH_CS  PC4

uint8_t flash_read_id(void) {
    uint8_t id;
    digitalWrite(FLASH_CS, LOW);
    SPI_Transfer(0x9F);         // Read JEDEC ID
    SPI_Transfer(0x00);         // manufacturer
    id = SPI_Transfer(0x00);    // memory type
    SPI_Transfer(0x00);         // capacity
    digitalWrite(FLASH_CS, HIGH);
    return id;
}

void flash_write_enable(void) {
    digitalWrite(FLASH_CS, LOW);
    SPI_Transfer(0x06);         // WREN
    digitalWrite(FLASH_CS, HIGH);
}

void flash_read(uint32_t addr, uint8_t* buf, uint16_t len) {
    digitalWrite(FLASH_CS, LOW);
    SPI_Transfer(0x03);                         // READ command
    SPI_Transfer((addr >> 16) & 0xFF);
    SPI_Transfer((addr >>  8) & 0xFF);
    SPI_Transfer( addr        & 0xFF);
    SPI_TransferBuffer(NULL, buf, len);         // RX only
    digitalWrite(FLASH_CS, HIGH);
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    SPI_SimpleInit(SPI_MODE0, SPI_6MHZ, SPI_PINS_DEFAULT);
    pinMode(FLASH_CS, PIN_MODE_OUTPUT);
    digitalWrite(FLASH_CS, HIGH);

    uint8_t id = flash_read_id();
    USART_Print("Flash ID: 0x");
    USART_PrintHex(id, 1);
    USART_Print("\r\n");

    uint8_t data[4];
    flash_read(0x000000, data, 4);
    for (uint8_t i = 0; i < 4; i++) {
        USART_PrintHex(data[i], 1);
        USART_Print(" ");
    }
    USART_Print("\r\n");
}
```

### ขั้นสูง — SPI + DMA สำหรับ bulk transfer

```c
#include "SimpleHAL.h"

#define DISP_CS   PC4
#define DISP_DC   PD2
#define BUF_SIZE  128

uint8_t screen_buf[BUF_SIZE];

// ส่ง framebuffer ผ่าน SPI_Write (blocking แต่ใช้ hardware shift register)
void flush_display(void) {
    digitalWrite(DISP_DC, HIGH);  // data mode
    digitalWrite(DISP_CS, LOW);
    SPI_Write(screen_buf, BUF_SIZE);
    digitalWrite(DISP_CS, HIGH);
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    SPI_SimpleInit(SPI_MODE0, SPI_12MHZ, SPI_PINS_DEFAULT);
    pinMode(DISP_CS, PIN_MODE_OUTPUT);
    pinMode(DISP_DC, PIN_MODE_OUTPUT);
    digitalWrite(DISP_CS, HIGH);

    // เติมข้อมูล
    for (uint16_t i = 0; i < BUF_SIZE; i++) {
        screen_buf[i] = (uint8_t)i;
    }

    while (1) {
        flush_display();
        Delay_Ms(16);  // ~60fps
    }
}
```

---

## ข้อควรระวัง

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| ข้อมูลผิด | SPI Mode ไม่ตรงกับ datasheet device | ดู CPOL/CPHA ใน datasheet แล้วตั้ง mode ให้ถูก |
| Device ไม่ตอบสนอง | ลืม `digitalWrite(CS, LOW)` | ต้อง pull CS ต่ำก่อนส่งทุกครั้ง |
| สัญญาณเสียง (ที่ 12MHz) | สาย/breadboard มี capacitance สูง | ใช้ PCB จริงหรือลดความเร็ว |
| MISO อ่านค่าสุ่ม | ไม่มี pull-up/pull-down บน MISO | ต่อ pull-up 10kΩ บน MISO |
| ใช้ SPI พร้อมกัน 2 device | CS ไม่ถูกบังคับ HIGH | ยกเว้น CS ของทุก device ก่อน init |
