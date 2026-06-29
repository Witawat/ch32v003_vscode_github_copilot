# SimpleFlash — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `SimpleFlash.h / SimpleFlash.c`

---

## ภาพรวม

SimpleFlash ให้บันทึกและโหลดข้อมูลถาวรลงใน Flash memory โดยใช้ **2 หน้าสุดท้าย** ของ Flash (page 254-255 = 128 bytes) ข้อมูลที่บันทึกจะยังคงอยู่แม้ไฟดับหรือ MCU reset พร้อม **CRC validation** เพื่อตรวจสอบความถูกต้อง

---

## Flash Layout

```
CH32V003 Flash: 16KB = 256 pages × 64 bytes
┌─────────────────────────────────────────┐
│  Page 0–253  │  Code + Read-only data   │  (ห้ามเขียน!)
├──────────────┼──────────────────────────┤
│  Page 254    │  Config Storage          │  @ 0x0803F80
│  Page 255    │  Data Storage            │  @ 0x0803FC0
└──────────────┴──────────────────────────┘
```

- **Page size:** 64 bytes
- **Max save size:** ~60 bytes (เว้น overhead สำหรับ CRC)
- **`FLASH_MAX_STRING_LENGTH`:** 60 characters

---

## Status Codes

```c
typedef enum {
    FLASH_OK = 0,
    FLASH_ERROR,
    FLASH_ERROR_BUSY,
    FLASH_ERROR_TIMEOUT,
    FLASH_ERROR_WRITE,
    FLASH_ERROR_ERASE,
    FLASH_ERROR_VERIFY,
    FLASH_ERROR_ALIGN,
    FLASH_ERROR_RANGE,
    FLASH_ERROR_CRC,      // ข้อมูล corrupt
    FLASH_ERROR_INVALID   // ไม่มีข้อมูลใน Flash (blank)
} Flash_Status;
```

---

## API Reference

### Initialization

#### `void Flash_Init(void)`

ต้องเรียกก่อนใช้งาน

```c
Flash_Init();
```

---

### Save / Load

#### `Flash_Status Flash_SaveConfig(void* data, uint16_t size)`

บันทึกข้อมูลลง Page 254 พร้อม CRC

```c
typedef struct {
    uint8_t  mode;
    uint16_t threshold;
    float    calibration;
} Config_t;

Config_t cfg = {1, 500, 1.25f};
Flash_Status st = Flash_SaveConfig(&cfg, sizeof(cfg));
if (st == FLASH_OK) {
    USART_Print("Saved!\r\n");
}
```

#### `Flash_Status Flash_LoadConfig(void* data, uint16_t size)`

โหลดข้อมูลจาก Page 254 พร้อม CRC check

```c
Config_t cfg;
Flash_Status st = Flash_LoadConfig(&cfg, sizeof(cfg));
if (st == FLASH_OK) {
    // ใช้งาน cfg.mode, cfg.threshold, ...
} else if (st == FLASH_ERROR_INVALID) {
    // ยังไม่เคยบันทึก → ใช้ default values
    cfg.mode = 0;
    cfg.threshold = 100;
}
```

---

### Erase

#### `Flash_Status Flash_EraseAll(void)`

ลบ page 254 และ 255 ทั้งหมด (factory reset)

#### `Flash_Status Flash_ErasePage(uint8_t page_num)`

ลบเฉพาะหน้า — `254` = config, `255` = data

| ⚠️ | ต้อง erase page ก่อนเขียนข้อมูลใหม่ทุกครั้ง |

---

### Basic Read/Write

#### `uint8_t Flash_ReadByte(uint32_t addr)`

| **คืนค่า** | 8-bit |

#### `uint16_t Flash_ReadHalfWord(uint32_t addr)`

| **คืนค่า** | 16-bit |
| ⚠️ | addr ต้อง align 2 bytes (เลขคู่) |

#### `uint32_t Flash_ReadWord(uint32_t addr)`

| **คืนค่า** | 32-bit |
| ⚠️ | addr ต้อง align 4 bytes (หาร 4 ลงตัว) |

#### `Flash_Status Flash_WriteByte(uint32_t addr, uint8_t data)`

| ⚠️ | ต้อง erase page ก่อน |

#### `Flash_Status Flash_WriteHalfWord(uint32_t addr, uint16_t data)`

| ⚠️ | ต้อง erase page ก่อน + align 2 bytes |

#### `Flash_Status Flash_WriteWord(uint32_t addr, uint32_t data)`

| ⚠️ | ต้อง erase page ก่อน + align 4 bytes |

```c
Flash_ErasePage(FLASH_CONFIG_PAGE);
Flash_WriteHalfWord(FLASH_CONFIG_ADDR, 0x1234);
uint16_t val = Flash_ReadHalfWord(FLASH_CONFIG_ADDR);
```

### Auto-Erase Write

#### `Flash_Status Flash_WriteByteWithErase(uint32_t addr, uint8_t data)` / `Flash_WriteHalfWordWithErase(...)` / `Flash_WriteWordWithErase(...)`

เขียนโดย erase page ให้อัตโนมัติ — สะดวกแต่**ช้า** (modify-erase-write)

| ⚠️ | ใช้ RAM 64 bytes เป็น buffer | ไม่เหมาะเขียนบ่อย — ใช้ manual erase + write แทน |

```c
Flash_WriteByteWithErase(FLASH_CONFIG_ADDR + 5, 0xAB);  // 1 ขั้นตอน
```

### String

#### `uint16_t Flash_ReadString(uint32_t addr, char* buffer, uint16_t max_len)`

| **คืนค่า** | จำนวน byte ที่อ่านได้ (ไม่รวม null), `0` = error |

#### `Flash_Status Flash_WriteString(uint32_t addr, const char* str)`

| ⚠️ | ต้อง erase page ก่อน | string ≤ 60 chars |

### Struct

#### `Flash_Status Flash_ReadStruct(uint32_t addr, void* ptr, uint16_t size)` / `Flash_WriteStruct(uint32_t addr, const void* ptr, uint16_t size)`

| ⚠️ | WriteStruct: ต้อง erase ก่อน | size ≤ 64 bytes |

### Utility

#### `bool Flash_IsConfigValid(void)`

| **คืนค่า** | `true` = มี valid config ใน Flash |

#### `uint16_t Flash_CalculateCRC16(const uint8_t* data, uint16_t len)`

คำนวณ CRC16-CCITT checksum

#### `bool Flash_IsAddressValid(uint32_t addr)`

ตรวจสอบว่า addr อยู่ใน storage area หรือไม่

#### `uint32_t Flash_GetPageAddress(uint8_t page_num)`

แปลง page → address — คืน `0` ถ้าไม่ถูกต้อง

### Simplified Macros

```c
FLASH_SAVE_CONFIG(&config)       // บันทึก — คำนวณ size + CRC อัตโนมัติ
FLASH_LOAD_CONFIG(&config)       // โหลด — ตรวจสอบ CRC
FLASH_WRITE_AUTO(addr, val)      // เขียน + auto-erase (auto-detect type)
FLASH_READ(addr, &var)           // อ่าน (auto-detect type)
```

```c
Flash_EraseAll();
```

> **ข้อควรระวัง:** ข้อมูลที่บันทึกไว้จะหายถาวร

---

## ตัวอย่างการใช้งาน

### ขั้นต้น — บันทึกและโหลด Config

```c
#include "SimpleHAL.h"

typedef struct {
    uint8_t  led_brightness;    // 0-100
    uint16_t blink_interval_ms;
    uint8_t  uart_enabled;
} AppConfig_t;

const AppConfig_t default_cfg = {50, 500, 1};

AppConfig_t cfg;

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    Flash_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    // โหลด config
    Flash_Status st = Flash_LoadConfig(&cfg, sizeof(cfg));
    if (st != FLASH_OK) {
        // ใช้ default ถ้าโหลดไม่ได้
        cfg = default_cfg;
        USART_Print("Using default config\r\n");
    } else {
        USART_Print("Config loaded OK\r\n");
    }

    USART_Print("brightness="); USART_PrintNum(cfg.led_brightness);
    USART_Print("\r\n");

    // ปรับ config
    cfg.led_brightness = 75;
    cfg.blink_interval_ms = 250;

    // บันทึก
    if (Flash_SaveConfig(&cfg, sizeof(cfg)) == FLASH_OK) {
        USART_Print("Config saved!\r\n");
    }
}
```

### ขั้นกลาง — Config Manager พร้อม Version Check

```c
#include "SimpleHAL.h"

#define CONFIG_VERSION  0x02

typedef struct {
    uint8_t  version;
    uint8_t  mode;
    uint16_t threshold;
    uint8_t  crc_dummy;   // placeholder
} Config_t;

Config_t cfg;

void config_apply_defaults(void) {
    cfg.version   = CONFIG_VERSION;
    cfg.mode      = 0;
    cfg.threshold = 1000;
}

void config_load(void) {
    Flash_Status st = Flash_LoadConfig(&cfg, sizeof(cfg));
    if (st != FLASH_OK || cfg.version != CONFIG_VERSION) {
        // version เปลี่ยน หรือ ไม่มีข้อมูล → reset
        config_apply_defaults();
        Flash_SaveConfig(&cfg, sizeof(cfg));
        USART_Print("Config reset to defaults\r\n");
    }
}

void config_save(void) {
    if (Flash_SaveConfig(&cfg, sizeof(cfg)) == FLASH_OK) {
        USART_Print("Saved\r\n");
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    Flash_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    config_load();

    USART_Print("mode="); USART_PrintNum(cfg.mode); USART_Print("\r\n");

    // ปรับค่าตาม input
    cfg.mode = 1;
    cfg.threshold = 500;
    config_save();
}
```

### ขั้นสูง — Wear Leveling อย่างง่าย (นับจำนวนครั้ง write)

```c
// Flash มีอายุ erase cycle จำกัด (~10,000 ครั้ง)
// หลีกเลี่ยงการบันทึกบ่อยเกินไป

#include "SimpleHAL.h"

typedef struct {
    uint32_t uptime_minutes;  // สะสม uptime
    uint8_t  save_count;      // นับ write cycle
} PersistData_t;

PersistData_t pdata;
uint32_t last_save_min = 0;

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    Flash_Init();

    Flash_Status st = Flash_LoadConfig(&pdata, sizeof(pdata));
    if (st != FLASH_OK) {
        pdata.uptime_minutes = 0;
        pdata.save_count = 0;
    }

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    USART_Print("Uptime: "); USART_PrintNum((int32_t)pdata.uptime_minutes);
    USART_Print(" min (saved "); USART_PrintNum(pdata.save_count); USART_Print(" times)\r\n");

    Timer_t save_timer;
    Start_Timer(&save_timer, 60000, 1);   // บันทึกทุก 60 วินาที

    while (1) {
        if (Is_Timer_Expired(&save_timer)) {
            pdata.uptime_minutes++;
            pdata.save_count++;
            Flash_SaveConfig(&pdata, sizeof(pdata));
        }
    }
}
```

---

## ข้อควรระวัง

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| `FLASH_ERROR_CRC` | ข้อมูล corrupt หรือ partial write | ใช้ default แล้ว save ใหม่ |
| `FLASH_ERROR_INVALID` | ยังไม่เคย save | ใช้ default values |
| Flash อายุสั้น | save บ่อยมาก (~10,000 cycle) | save เฉพาะเมื่อข้อมูลเปลี่ยน |
| ข้อมูลใหญ่เกิน 60 bytes | เกิน page limit | แยก struct ให้เล็กลง |
| เขียนช้า 4× erase | ใช้ `WriteWordWithErase` v1.x | v2.0 แก้เป็น 1 erase ต่อ operation |
| บันทึกระหว่าง interrupt | Flash write ต้องการ disable IRQ | ทำใน main loop เท่านั้น |

> ⚠️ Flash endurance ~10,000-80,000 cycles ต่อหน้า — สำหรับข้อมูลที่เปลี่ยนบ่อยมาก ใช้ external EEPROM
