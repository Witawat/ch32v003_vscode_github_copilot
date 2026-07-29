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
| `SPI_PINS_REMAP`   | PD1 | PD2 | PD3 | PD0 |

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

| Enum | Prescaler | @24MHz PCLK | @48MHz PCLK |
|------|-----------|-------------|-------------|
| `SPI_12MHZ`  | PCLK/2   | 12 MHz | 24 MHz |
| `SPI_8MHZ`   | PCLK/4   | 6 MHz | 12 MHz |
| `SPI_4MHZ`   | PCLK/8   | 3 MHz | 6 MHz |
| `SPI_2MHZ`   | PCLK/16  | 1.5 MHz | 3 MHz |
| `SPI_1MHZ`   | PCLK/32  | 750 kHz | 1.5 MHz |
| `SPI_500KHZ` | PCLK/64  | 375 kHz | 750 kHz |
| `SPI_250KHZ` | PCLK/128 | 187.5 kHz | 375 kHz |
| `SPI_125KHZ` | PCLK/256 | 93.75 kHz | 187.5 kHz |

> **หมายเหตุ:** ความเร็วจริงขึ้นกับ `SystemCoreClock` — ตารางอ้างอิง PCLK2=24MHz และ 48MHz
> ⚠️ ชื่อ enum (เช่น `SPI_12MHZ`) อิงจาก PCLK2=24MHz (สมมติฐานเดิม) แต่โปรเจกต์นี้รัน
> `SystemCoreClock`=48MHz (HSI+PLL) และ PCLK2 ไม่ถูกหาร — **ให้ยึดคอลัมน์ "@48MHz PCLK"
> เป็นความเร็วจริง** ไม่ใช่ตัวเลขในชื่อ enum

## ข้อจำกัดแพ็กเกจ

| แพ็กเกจ | SPI HW Default (PC4-7) | SPI HW Remap (PD0-3) | Software SPI |
|:---:|:---:|:---:|:---:|
| SOP-8 | ❌ | ❌ | ✅ shiftOut/shiftIn |
| SOP-16 | ✅ | ❌ | ✅ |
| TSSOP-20/QFN-20 | ✅ | ✅ | ✅ |

> ⚠️ **SOP-8:** Hardware SPI ถูกปิดทั้งหมด (`#if !CH32V003_IS_SOP8`) — ซอร์สโค้ด SimpleSPI.c ไม่ถูกคอมไพล์

---

## API Reference

### Initialization

#### `void SPI_SimpleInit(SPI_Mode mode, SPI_Speed speed, SPI_PinConfig pin_config)`

```c
SPI_SimpleInit(SPI_MODE0, SPI_4MHZ,  SPI_PINS_DEFAULT);  // Fast, Mode0
SPI_SimpleInit(SPI_MODE3, SPI_1MHZ, SPI_PINS_DEFAULT); // Mode3 ช้ากว่า
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

#### `void SPI_Read(uint8_t* data, uint16_t len, uint8_t dummy_byte)`

รับข้อมูลอย่างเดียว (ส่ง dummy byte)

```c
uint8_t buf[10];
SPI_Read(buf, 10, 0xFF);  // ส่ง 0xFF ไป, รับ 10 bytes
```

### Chip Select Control

#### `void SPI_SetCS(uint8_t state)`

ควบคุม CS pin — 0=LOW, 1=HIGH

```c
SPI_SetCS(0);           // Select device
SPI_Transfer(0xAA);
SPI_SetCS(1);           // Deselect
```

#### `void SPI_SetCSPin(GPIO_TypeDef* port, uint16_t pin)`

เปลี่ยน CS pin — รองรับหลาย SPI devices

```c
SPI_SetCSPin(GPIOA, GPIO_Pin_2);  // เปลี่ยน CS เป็น PA2
```

### Configuration

#### `void SPI_SetSpeed(SPI_Speed speed)`

เปลี่ยนความเร็วตอน runtime

```c
SPI_SetSpeed(SPI_8MHZ);  // เร่งความเร็ว
```

#### `void SPI_SetBitOrder(SPI_BitOrder order)`

เปลี่ยน MSB/LSB first — `SPI_MSB_FIRST` หรือ `SPI_LSB_FIRST`

```c
SPI_SetBitOrder(SPI_LSB_FIRST);
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
    SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);
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
    SPI_SimpleInit(SPI_MODE0, SPI_4MHZ, SPI_PINS_DEFAULT);  // 6MHz จริงที่ PCLK2=48MHz
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
