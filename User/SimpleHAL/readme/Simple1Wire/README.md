# Simple1Wire — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `Simple1Wire.h / Simple1Wire.c`

---

## ภาพรวม

Simple1Wire ให้ใช้งาน **Dallas/Maxim 1-Wire protocol** บน GPIO ใดก็ได้ รองรับการค้นหา device หลายตัวบน bus เดียวกัน (multi-drop), ROM command, และ CRC8 validation ใช้งานหลักกับ DS18B20, DS2431, และ iButton devices

---

## Hardware Requirements

- GPIO 1 pin (ใดก็ได้)
- **Pull-up resistor 4.7kΩ** ระหว่าง data line และ VDD (จำเป็น!)
- สาย 1-Wire ยาวไม่เกิน ~100m (ต้องลด pull-up เป็น 2.2kΩ)

---

## โครงสร้างข้อมูล

### `OneWire_Bus`

```c
typedef struct {
    uint8_t pin;                      // GPIO pin ที่ใช้
    uint8_t rom[8];                   // ROM address ของ device ที่เลือก
    uint8_t last_discrepancy;         // สำหรับ search algorithm
    uint8_t last_family_discrepancy;
    uint8_t last_device_flag;         // 1 = device สุดท้ายแล้ว
    uint8_t initialized;
} OneWire_Bus;
```

---

## ROM Commands

> ⚠️ `SearchFirst/SearchNext` ถูกเปลี่ยนเป็น `ResetSearch() + Search()` ตั้งแต่ v2.0

| ฟังก์ชัน | Command | ใช้เมื่อ |
|---------|---------|---------|
| `OneWire_SkipROM(bus)` | 0xCC | device เดียว / broadcast |
| `OneWire_ReadROM(bus, rom)` | 0x33 | device เดียว — อ่าน ROM 8 bytes |
| `OneWire_MatchROM(bus, rom)` | 0x55 | เลือก device เฉพาะ |
| `OneWire_ResetSearch(bus)` | — | รีเซ็ต search state |
| `OneWire_Search(bus)` | 0xF0 | ค้นหา device ถัดไป (คืน `1` = เจอ) |
| `OneWire_AlarmSearch(bus)` | 0xEC | ค้นหา devices ที่มี alarm |
| `OneWire_Select(bus, rom)` | — | Reset + MatchROM ในขั้นตอนเดียว |

---

## Timing Constants

| Constant | ค่า (µs) | หมายเหตุ |
|---------|:-------:|---------|
| RESET_PULSE | 480 | ยาวที่สุด |
| PRESENCE_WAIT | 70 | รอ device ตอบ |
| WRITE_0_LOW | 60 | ส่ง bit 0 |
| WRITE_1_LOW | 10 | ส่ง bit 1 |
| READ_RELEASE | 3 | ปล่อย bus ก่อนอ่าน |
| READ_SAMPLE | 12 | อ่านค่า bit |

---

## API Reference

### Init

#### `OneWire_Bus* OneWire_Init(uint8_t pin)`

คืน pointer ไปยัง bus struct — ใช้ pointer นี้ทุกครั้ง

```c
OneWire_Bus* bus = OneWire_Init(PD4);
```

---

### Reset

#### `uint8_t OneWire_Reset(OneWire_Bus* bus)`

ส่ง reset pulse และรอ presence pulse  
คืน `1` ถ้ามี device ตอบ

```c
if (!OneWire_Reset(bus)) {
    USART_Print("No 1-Wire device!\r\n");
}
```

---

### Read / Write

#### `void OneWire_WriteByte(OneWire_Bus* bus, uint8_t byte)`
#### `uint8_t OneWire_ReadByte(OneWire_Bus* bus)`

```c
OneWire_WriteByte(bus, 0x44);           // DS18B20: Convert T
uint8_t data = OneWire_ReadByte(bus);
```

---

### ROM Search (ใหม่ — ResetSearch + Search)

#### `void OneWire_ResetSearch(OneWire_Bus* bus)`

รีเซ็ต search state — ต้องเรียกก่อน search ครั้งแรก

#### `uint8_t OneWire_Search(OneWire_Bus* bus)`

ค้นหา device ถัดไป — **คืน `1` ถ้าพบ**, ROM ถูกเก็บใน `bus->rom[8]`

```c
uint8_t roms[8][8], count = 0;
OneWire_ResetSearch(bus);
while (OneWire_Search(bus) && count < 8) {
    memcpy(roms[count++], bus->rom, 8);
}
```

> ⚠️ **API เก่าถูกเปลี่ยนแล้ว** — `SearchFirst/SearchNext` ไม่มีใน SimpleHAL v2.0

#### `uint8_t OneWire_AlarmSearch(OneWire_Bus* bus)`

ค้นหาเฉพาะ devices ที่มี alarm flag — ใช้กับ DS18B20 temperature alarm

#### `void OneWire_GetAddress(OneWire_Bus* bus, uint8_t rom[8])`

ดึง ROM address จาก bus หลัง search

#### `void OneWire_Select(OneWire_Bus* bus, const uint8_t rom[8])`

`Reset + MatchROM` ในขั้นตอนเดียว — สะดวกกว่าเรียกแยก

```c
OneWire_Select(bus, sensor1_rom);  // เลือก sensor 1
```

### Low-Level Bit/Byte

#### `void OneWire_WriteBit(OneWire_Bus* bus, uint8_t bit)`

ส่ง 1 bit

#### `uint8_t OneWire_ReadBit(OneWire_Bus* bus)`

รับ 1 bit — **คืน `0` หรือ `1`**

#### `void OneWire_WriteBytes(OneWire_Bus* bus, const uint8_t* data, uint16_t len)`

ส่งหลาย bytes — เร็วกว่าเรียก `WriteByte` ต่อเนื่อง

#### `void OneWire_ReadBytes(OneWire_Bus* bus, uint8_t* buf, uint16_t len)`

รับหลาย bytes

### Utility

#### `OneWire_Bus* OneWire_GetBusByPin(uint8_t pin)`

หา bus instance จาก GPIO pin — **คืน `NULL` ถ้าไม่พบ**

#### `void OneWire_Depower(OneWire_Bus* bus)`

ปิด strong pull-up (ใช้ใน parasite power mode)

#### `uint8_t OneWire_VerifyCRC(OneWire_Bus* bus, uint8_t* data, uint8_t len)`

ตรวจสอบ CRC — **คืน `1` ถ้าถูกต้อง**

### CRC

#### `uint8_t OneWire_CRC8(uint8_t* data, uint8_t len)`

คำนวณ CRC8 Dallas/Maxim สำหรับ verify ROM address หรือ scratchpad

```c
uint8_t crc = OneWire_CRC8(bus->rom, 7);
if (crc != bus->rom[7]) {
    USART_Print("ROM CRC error!\r\n");
}
```

---

## ตัวอย่างการใช้งาน

### ขั้นต้น — อ่าน DS18B20 (temperature sensor)

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    OneWire_Bus* bus = OneWire_Init(PD4);  // ต่อ 4.7k pull-up ที่ PD4

    while (1) {
        // Step 1: Reset + SkipROM (device เดียว)
        if (!OneWire_Reset(bus)) {
            USART_Print("DS18B20 not found!\r\n");
            Delay_Ms(1000);
            continue;
        }
        OneWire_SkipROM(bus);
        OneWire_WriteByte(bus, 0x44);  // Convert T command

        // Step 2: รอ conversion (12-bit = 750ms)
        Delay_Ms(800);

        // Step 3: อ่าน scratchpad
        OneWire_Reset(bus);
        OneWire_SkipROM(bus);
        OneWire_WriteByte(bus, 0xBE);  // Read scratchpad

        uint8_t scratchpad[9];
        for (uint8_t i = 0; i < 9; i++) {
            scratchpad[i] = OneWire_ReadByte(bus);
        }

        // Verify CRC
        if (OneWire_CRC8(scratchpad, 8) != scratchpad[8]) {
            USART_Print("CRC Error!\r\n");
            continue;
        }

        // แปลงอุณหภูมิ
        int16_t raw = (scratchpad[1] << 8) | scratchpad[0];
        // DS18B20: raw / 16 = °C (12-bit resolution)
        int16_t temp_int  = raw / 16;
        uint16_t temp_frac = (raw & 0x0F) * 100 / 16;

        USART_Print("Temp=");
        USART_PrintNum(temp_int);
        USART_Print(".");
        if (temp_frac < 10) USART_Print("0");
        USART_PrintNum(temp_frac);
        USART_Print(" C\r\n");

        Delay_Ms(2000);
    }
}
```

### ขั้นกลาง — Multi-device (หลาย DS18B20)

```c
#include "SimpleHAL.h"
#include <string.h>

#define MAX_SENSORS  4

uint8_t sensor_roms[MAX_SENSORS][8];
uint8_t sensor_count = 0;

void scan_sensors(OneWire_Bus* bus) {
    sensor_count = 0;
    if (OneWire_SearchFirst(bus)) {
        do {
            if (sensor_count < MAX_SENSORS) {
                memcpy(sensor_roms[sensor_count++], bus->rom, 8);
            }
        } while (OneWire_SearchNext(bus));
    }
    USART_Print("Found "); USART_PrintNum(sensor_count); USART_Print(" sensors\r\n");
}

float read_temperature(OneWire_Bus* bus, uint8_t* rom) {
    // Start conversion for specific sensor
    OneWire_Reset(bus);
    OneWire_MatchROM(bus, rom);
    OneWire_WriteByte(bus, 0x44);
    Delay_Ms(800);

    // Read scratchpad
    OneWire_Reset(bus);
    OneWire_MatchROM(bus, rom);
    OneWire_WriteByte(bus, 0xBE);

    uint8_t sp[9];
    for (uint8_t i = 0; i < 9; i++) sp[i] = OneWire_ReadByte(bus);

    if (OneWire_CRC8(sp, 8) != sp[8]) return -999.0f;

    int16_t raw = (sp[1] << 8) | sp[0];
    return (float)raw / 16.0f;
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    OneWire_Bus* bus = OneWire_Init(PD4);
    scan_sensors(bus);

    while (1) {
        for (uint8_t i = 0; i < sensor_count; i++) {
            float t = read_temperature(bus, sensor_roms[i]);
            USART_Print("Sensor "); USART_PrintNum(i);
            USART_Print(": ");
            USART_PrintNum((int32_t)t);
            USART_Print(" C\r\n");
        }
        Delay_Ms(2000);
    }
}
```

### ขั้นสูง — Simultaneous Conversion (ทุก sensor พร้อมกัน)

```c
// แทนที่จะอ่านทีละตัว ให้ Convert T พร้อมกันทุกตัวแล้วค่อยอ่าน
// ประหยัดเวลา: N × 800ms → 800ms (fixed)

void start_all_conversions(OneWire_Bus* bus) {
    OneWire_Reset(bus);
    OneWire_SkipROM(bus);        // broadcast ทุก device
    OneWire_WriteByte(bus, 0x44); // Convert T ทุกตัวพร้อมกัน
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    OneWire_Bus* bus = OneWire_Init(PD4);
    scan_sensors(bus);

    while (1) {
        start_all_conversions(bus);  // สั่งทุกตัวพร้อมกัน
        Delay_Ms(800);               // รอ 1 ครั้ง

        for (uint8_t i = 0; i < sensor_count; i++) {
            // อ่านทีละตัวหลัง conversion เสร็จแล้ว
            OneWire_Reset(bus);
            OneWire_MatchROM(bus, sensor_roms[i]);
            OneWire_WriteByte(bus, 0xBE);

            uint8_t sp[9];
            for (uint8_t j = 0; j < 9; j++) sp[j] = OneWire_ReadByte(bus);

            int16_t raw = (sp[1] << 8) | sp[0];
            USART_Print("S"); USART_PrintNum(i);
            USART_Print("="); USART_PrintNum(raw / 16);
            USART_Print("C ");
        }
        USART_Print("\r\n");
        Delay_Ms(1200);
    }
}
```

---

## ข้อควรระวัง

> **⚡ v2.0:** IRQ disable/enable ใช้ depth counter — ป้องกัน nested enable/disable  
> 1-Wire Reset ปิด global interrupt ~790µs — อาจสูญเสีย UART data ที่ 115200 bps

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| Reset ไม่พบ device | ไม่มี pull-up 4.7kΩ | ต่อ resistor pull-up ทุกครั้ง |
| อ่าน CRC ผิดตลอด | สายยาวเกิน / noise | ลดสายหรือเพิ่ม capacitor bypass |
| อ่านได้แค่ 1 device จาก 4 | ไม่ใช้ MatchROM | ใช้ `OneWire_MatchROM()` กับ ROM address |
| อุณหภูมิผิด (-0.5°C error) | resolution 9-bit default | กำหนด resolution 12-bit ผ่าน config register |
| Search หา device ไม่ครบ | bus กำลัง busy | Reset ก่อน SearchFirst เสมอ |
