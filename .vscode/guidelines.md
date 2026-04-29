# CH32V003 Project — Programming & README Guidelines

> อ่านไฟล์นี้ก่อนทุกครั้งก่อนเริ่มพัฒนา feature ใหม่หรือเขียน Lib ใหม่  
> อัพเดตล่าสุด: 2026-04-29

---

## สารบัญ

1. [Hardware Overview](#1-hardware-overview)
2. [โครงสร้างไฟล์](#2-โครงสร้างไฟล์)
3. [SimpleHAL Library](#3-simplehal-library)
4. [Lib Directory](#4-lib-directory)
5. [โครงสร้าง main.c มาตรฐาน](#5-mainct--โครงสร้างมาตรฐาน)
6. [Interrupt Handlers](#6-interrupt-handlers--ch32v00x_itc)
7. [printf และ Debug Output](#7-printf-และ-debug-output)
8. [Build System](#8-build-system)
9. [Memory Budget](#9-memory-budget)
10. [กฎการเขียนโค้ด](#10-กฎการเขียนโค้ด)
11. [Resource Conflicts](#11-resource-conflicts)
12. [Quick Reference](#12-quick-reference)
13. **[แนวทางการเขียน Lib ใหม่](#13-แนวทางการเขียน-lib-ใหม่)**
14. **[แนวทางการเขียน README ของ Lib](#14-แนวทางการเขียน-readme-ของ-lib)**
15. **[Lib ที่มีอยู่แล้วและสถานะ](#15-lib-ที่มีอยู่แล้วและสถานะ)**

---

## 1. Hardware Overview

| Item        | Detail                          |
|-------------|---------------------------------|
| MCU         | CH32V003 (WCH)                  |
| Core        | RISC-V RV32EC                   |
| Flash       | 16 KB (`0x00000000`)            |
| RAM         | 2 KB (`0x20000000`)             |
| Clock       | 48 MHz (default after `SystemCoreClockUpdate()`) |
| GPIO Ports  | GPIOA (PA1–PA2), GPIOC (PC0–PC7), GPIOD (PD2–PD7) |

### Pin ที่รองรับ PWM
| Pin | Timer Channel |
|-----|--------------|
| PA1 | TIM1_CH2     |
| PC0 | TIM2_CH3     |
| PC3 | TIM1_CH3     |
| PC4 | TIM1_CH4     |
| PD2 | TIM1_CH1     |
| PD3 | TIM2_CH2     |
| PD4 | TIM2_CH1     |
| PD7 | TIM2_CH4     |

### Pin ที่รองรับ ADC
PD2 (Ch3), PD3 (Ch4), PD4 (Ch7), PD5 (Ch5), PD6 (Ch6), PD7 (Ch0)

---

## 2. โครงสร้างไฟล์ (Project Layout)

```
CH32V003/
├── Core/                    # RISC-V core (อย่าแก้ไข)
│   ├── core_riscv.c/.h
├── Debug/                   # WCH debug utility (อย่าแก้ไข)
│   ├── debug.c/.h
├── Peripheral/              # WCH Standard Peripheral Library (อย่าแก้ไข)
│   ├── inc/                 # ch32v00x_*.h
│   └── src/                 # ch32v00x_*.c
├── Startup/
│   └── startup_ch32v00x.S  # Assembly startup (อย่าแก้ไข)
├── Ld/
│   └── Link.ld              # Linker script (อย่าแก้ไข)
├── User/                    # <<< โค้ดที่เราเขียน >>>
│   ├── main.c / main.h      # Entry point
│   ├── ch32v00x_conf.h      # เปิด/ปิด Peripheral headers
│   ├── ch32v00x_it.c/.h     # Interrupt handlers
│   ├── system_ch32v00x.c/.h # Clock config
│   ├── SimpleHAL/           # HAL Library (ดูหัวข้อ 3)
│   └── Lib/                 # Device Libraries (ดูหัวข้อ 4)
├── obj/                     # Object files (auto-generated, อย่าแก้ไข)
├── output/                  # ผลลัพธ์ build (auto-generated)
│   ├── CH32V003.elf
│   ├── CH32V003.hex
│   └── CH32V003.map
├── build.bat                # compile และ link
├── clean.bat                # ลบ obj/ และ output/
├── rebuild.bat              # clean + build
└── upload.bat               # flash ผ่าน WCH-Link
```

**กฎสำคัญ:**
- เขียนโค้ดใหม่ทั้งหมดใน `User/` เท่านั้น
- `Core/`, `Debug/`, `Peripheral/`, `Startup/`, `Ld/` — ห้ามแก้ไขเด็ดขาด

---

## 3. SimpleHAL Library

HAL ที่เขียนขึ้นเอง ห่อหุ้ม WCH Peripheral Library ให้ใช้งานง่ายแบบ Arduino

### 3.1 การ Include

```c
// วิธีที่ 1: Include ทั้งหมดผ่าน main.h (แนะนำ)
#include <main.h>   // main.h include "SimpleHAL/SimpleHAL.h" อยู่แล้ว

// วิธีที่ 2: Include เฉพาะ module ที่ต้องการ
#include "SimpleHAL/SimpleGPIO.h"
#include "SimpleHAL/SimpleTIM.h"
```

### 3.2 Module ทั้งหมดใน SimpleHAL

| File              | ความสามารถ                                    |
|-------------------|-----------------------------------------------|
| `SimpleGPIO`      | pinMode, digitalWrite, digitalRead, analogRead, analogWrite, attachInterrupt |
| `SimpleTIM`       | Timer interrupt callbacks (TIM1, TIM2)         |
| `SimpleTIM_Ext`   | Input capture, output compare                 |
| `SimplePWM`       | PWM output ด้วย duty cycle %                  |
| `SimpleUSART`     | Serial TX/RX, printf                          |
| `SimpleI2C`       | Hardware I2C (Master mode)                    |
| `SimpleI2C_Soft`  | Software I2C (bit-bang, ใช้ pin ใดก็ได้)      |
| `SimpleSPI`       | Hardware SPI                                  |
| `SimpleADC`       | Analog to Digital Converter                   |
| `SimpleDelay`     | Delay_Ms, Delay_Us, millis(), Timer_t          |
| `SimpleFlash`     | Flash memory read/write                       |
| `SimpleIWDG`      | Independent Watchdog                          |
| `SimpleWWDG`      | Window Watchdog                               |
| `SimpleDMA`       | Direct Memory Access                          |
| `Simple1Wire`     | 1-Wire protocol (DS18B20)                     |
| `SimpleOPAMP`     | OPAMP (OPA peripheral ของ CH32V003)            |
| `SimplePWR`       | Power management, sleep modes                 |

### 3.3 GPIO — รูปแบบการใช้งาน

```c
// Pin Names (enum GPIO_Pin):
// GPIOA: PA1, PA2
// GPIOC: PC0–PC7
// GPIOD: PD2–PD7

pinMode(PC0, PIN_MODE_OUTPUT);
digitalWrite(PC0, HIGH);
digitalToggle(PC0);
uint8_t val = digitalRead(PC1);

// Interrupt
attachInterrupt(PC1, my_callback, FALLING);  // RISING, FALLING, CHANGE
detachInterrupt(PC1);

// ADC (PD2–PD7 เท่านั้น)
uint16_t adc = analogRead(PD5);   // 0–4095 (12-bit)

// PWM
analogWrite(PC0, 50);  // duty 50%
```

### 3.4 Timer — รูปแบบการใช้งาน

```c
void on_tick(void) { ... }

TIM_SimpleInit(TIM_1, 1000);        // 1000 Hz (1 kHz)
TIM_AttachInterrupt(TIM_1, on_tick);
TIM_Start(TIM_1);

// ข้อควรระวัง:
// - TIM1 ใช้ร่วมกับ PWM channel PA1, PC3, PC4, PD2 ไม่ได้
// - TIM2 ใช้ร่วมกับ PWM channel PC0, PD3, PD4, PD7 ไม่ได้
```

### 3.5 Delay / Timing

```c
// ต้องเรียกในช่วงต้นของ main() เสมอ
SystemCoreClockUpdate();
Timer_Init();   // เปิด SysTick สำหรับ millis/delay

// Blocking delay
Delay_Ms(500);
Delay_Us(100);

// Non-blocking timer (Timer_t)
Timer_t t;
TimerStart(&t, 1000, 0);   // 1000 ms, repeat=0

while(1) {
    if(TimerExpired(&t)) {
        // ทำงาน
    }
}
```

### 3.6 USART — Serial

```c
// Pin options: DEFAULT (TX=PD5, RX=PD6), REMAP1 (PD0/PD1), REMAP2 (PD6/PD5)
USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

USART_Print("Hello!\r\n");
USART_PrintNum(12345);

// มาตรฐาน printf ใช้งานได้ผ่าน debug.h (ต้อง DISABLE_PRINTF == PRINTF_ON)
printf("Value: %d\r\n", val);

// อ่านข้อมูล
if(USART_Available()) {
    uint8_t c = USART_Read();
}
```

### 3.7 I2C

```c
// Hardware I2C: DEFAULT (SCL=PC2, SDA=PC1), REMAP (PD0/PD1)
I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);

I2C_WriteReg(0x50, reg, value);
uint8_t val = I2C_ReadReg(0x50, reg);

// ต้องต่อ pull-up 4.7kΩ ที่ SDA และ SCL
```

### 3.8 SPI

```c
SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);
uint8_t rx = SPI_Transfer(0xAA);
```

---

## 4. Lib Directory — Device Libraries

ไลบรารี driver สำหรับ hardware component แต่ละตัว ใน `User/Lib/`

| Library       | ใช้กับ                        | Protocol   |
|---------------|-------------------------------|------------|
| `NeoPixel`    | WS2812 RGB LED                | Bit-bang   |
| `WS2815Matrix`| WS2815 LED Matrix             | Bit-bang   |
| `OLED`        | SSD1306 OLED 128×64           | I2C        |
| `LCD1602_I2C` | LCD 16×2 พร้อม PCF8574 I2C   | I2C        |
| `MAX7219`     | 7-segment/LED matrix driver   | SPI        |
| `TM1637`      | 4-digit 7-segment display     | Custom 2-wire |
| `DS18B20`     | Digital temperature sensor    | 1-Wire     |
| `NTC10K`      | NTC thermistor temperature    | ADC        |
| `IR`          | IR receiver                   | GPIO/Timer |
| `Buzzer`      | Piezo buzzer tone             | PWM/GPIO   |
| `PIR`         | PIR motion sensor             | GPIO       |
| `RotaryEncoder`| Rotary encoder               | GPIO/Interrupt |
| `TJC`         | TJC/Nextion HMI display       | USART      |

### รูปแบบการ Include Lib

```c
#include "Lib/OLED/oled_i2c.h"
#include "Lib/NeoPixel/NeoPixel.h"
#include "Lib/DS18B20/DS18B20.h"
```

---

## 5. main.c — โครงสร้างมาตรฐาน

```c
#include <main.h>   // includes SimpleHAL ทั้งหมด + debug.h

/* Global typedefs */

/* Global defines */

/* Global variables */

int main(void) {
    // 1. System init (ต้องมีเสมอ ลำดับนี้)
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();

#if (SDI_PRINT == SDI_PR_OPEN && DISABLE_PRINTF == PRINTF_ON)
    SDI_Printf_Enable();
#endif

    // 2. HAL init
    Timer_Init();                               // เปิด SysTick
    // USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    // 3. Debug print
    Delay_Ms(100);
    printf("SystemClk:%d\r\n", SystemCoreClock);

    // 4. Application init
    // ...

    // 5. Main loop
    while(1) {
        // ...
    }
}
```

---

## 6. Interrupt Handlers — ch32v00x_it.c

Interrupt ทั้งหมดต้องประกาศใน `User/ch32v00x_it.c`

```c
// ตัวอย่าง TIM1 interrupt
void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_UP_IRQHandler(void) {
    if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) {
        // ... โค้ดของคุณ ...
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    }
}
```

> หมายเหตุ: SimpleHAL จัดการ IRQ handlers ไว้ใน `.c` ของตัวเองแล้ว (เช่น `SimpleTIM.c`)
> ห้ามประกาศ IRQ handler ซ้ำกัน (จะเกิด linker error)

---

## 7. printf และ Debug Output

```c
// ใน ch32v00x_conf.h
#define DISABLE_PRINTF PRINTF_ON   // เปิด printf
// #define DISABLE_PRINTF PRINTF_OFF  // ปิด printf (ประหยัด Flash)

// output ไปยัง USART1 หรือ SDI (WCH-Link)
printf("Value: %d\r\n", val);
```

ถ้าใช้ `printf` มาก Flash จะเต็มเร็ว (ปัจจุบัน Flash ใช้อยู่ ~21%)  
ปิด printf เมื่อ production — แก้เป็น `PRINTF_OFF` ใน `ch32v00x_conf.h`

---

## 8. Build System

| คำสั่ง        | ทำอะไร                              |
|---------------|--------------------------------------|
| `build.bat`   | Compile ทั้งโปรเจค → `output/*.elf/.hex` |
| `clean.bat`   | ลบ `obj/` และ `output/`              |
| `rebuild.bat` | clean แล้ว build ใหม่                |
| `upload.bat`  | Flash ผ่าน WCH-Link (ต้องต่อสาย)    |

**VS Code Shortcuts:**
- `Ctrl+Shift+B` → Build
- `Terminal → Run Task` → เลือก Clean / Rebuild / Upload

**Toolchain:** RISC-V Embedded GCC12 (`riscv-wch-elf-`, v12.2.0)  
**Flags:** `-march=rv32ecxw -mabi=ilp32e -Os`

**ถ้าเพิ่มไฟล์ .c ใหม่ใน `User/`:** build.bat จะ compile อัตโนมัติ (wildcard scan)  
**ถ้าเพิ่มโฟลเดอร์ใหม่ใน `User/Lib/`:** build.bat จะ compile อัตโนมัติ (recursive scan)

---

## 9. Memory Budget

| Region | Total | ใช้อยู่ | เหลือ |
|--------|-------|---------|-------|
| Flash  | 16 KB | ~3.5 KB (~21%) | ~12.5 KB |
| RAM    | 2 KB  | ~420 B (~20%) | ~1.6 KB |

**ข้อควรระวังเมื่อ RAM ใกล้เต็ม:**
- หลีกเลี่ยง array ขนาดใหญ่บน stack
- ใช้ `static` สำหรับ buffers ที่ใช้เสมอ
- `NEOPIXEL_MAX_LEDS 64` → ใช้ RAM 192 bytes (3 bytes/LED)

**ข้อควรระวังเมื่อ Flash ใกล้เต็ม:**
- ปิด `printf` (`DISABLE_PRINTF PRINTF_OFF`)
- ใช้ `-Os` (ใช้อยู่แล้ว)
- ลบ Lib ที่ไม่ได้ใช้ออกจาก `#include`

---

## 10. กฎการเขียนโค้ด

### Naming Convention

```c
// ตัวแปร: snake_case
uint8_t sensor_value = 0;
uint32_t last_tick = 0;

// Constants / Macros: UPPER_SNAKE_CASE
#define MAX_BUFFER_SIZE  64
#define SENSOR_TIMEOUT   500

// Functions (SimpleHAL style): Module_ActionName
void GPIO_SimpleInit(...)
uint16_t ADC_ReadChannel(...)

// Struct/Enum types: PascalCase + _t suffix
typedef struct { ... } Timer_t;
typedef enum { ... } GPIO_PinMode;
```

### Header File Template

```c
#ifndef __MY_MODULE_H
#define __MY_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>
#include <stdint.h>

/* ========== Definitions ========== */

/* ========== Types ========== */

/* ========== Function Prototypes ========== */

#ifdef __cplusplus
}
#endif

#endif /* __MY_MODULE_H */
```

### Source File Template

```c
/**
 * @file MyModule.c
 * @brief [คำอธิบาย]
 * @version 1.0
 * @date YYYY-MM-DD
 */

#include "MyModule.h"

/* Private variables */
static uint8_t _initialized = 0;

/* Public functions */
void MyModule_Init(void) {
    if(_initialized) return;
    // ...
    _initialized = 1;
}
```

### กฎทั่วไป

1. **เปิด Clock ก่อนใช้ Peripheral เสมอ**  
   `RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);`

2. **ตรวจสอบ Timeout เมื่อรอ hardware**  
   อย่า busy-wait ไม่มีกำหนด ใช้ counter หรือ `millis()` จับเวลา

3. **ใช้ `volatile` กับตัวแปรที่แก้ไขใน ISR**  
   `volatile uint8_t flag = 0;`

4. **ห้ามเรียก `printf` / `Delay_Ms` ใน ISR**  
   ISR ต้องสั้น เร็ว ใช้แค่ set flag แล้วออก

5. **Guard header ทุกไฟล์** (`#ifndef __FILE_H`)

6. **ลำดับ init ใน main() ต้องถูกต้อง:**  
   `NVIC_PriorityGroupConfig` → `SystemCoreClockUpdate` → `Timer_Init` → peripherals อื่น

---

## 11. Resource Conflicts (ห้ามใช้ร่วมกัน)

| Resource | ใช้โดย              | ห้ามใช้ร่วมกับ          |
|----------|---------------------|------------------------|
| TIM1     | SimpleTIM (TIM_1)   | SimplePWM บน PA1, PC3, PC4, PD2 |
| TIM2     | SimpleTIM (TIM_2)   | SimplePWM บน PC0, PD3, PD4, PD7 |
| SysTick  | SimpleDelay         | ห้ามตั้งค่า SysTick เอง |
| USART1   | SimpleUSART         | printf (ใช้ร่วมกันได้ผ่าน retarget) |
| I2C1     | SimpleI2C           | ห้ามใช้ SimpleI2C_Soft บน pin เดียวกัน |
| DMA1     | SimpleDMA           | ตรวจสอบ channel conflict |

---

## 12. Quick Reference — ตัวอย่างโปรเจคทั่วไป

### LED Blink
```c
pinMode(PC0, PIN_MODE_OUTPUT);
while(1) { digitalToggle(PC0); Delay_Ms(500); }
```

### Button + LED
```c
pinMode(PC1, PIN_MODE_INPUT_PULLUP);
pinMode(PC0, PIN_MODE_OUTPUT);
while(1) {
    if(digitalRead(PC1) == LOW) digitalWrite(PC0, HIGH);
    else                        digitalWrite(PC0, LOW);
}
```

### OLED Hello World
```c
#include "Lib/OLED/oled_i2c.h"
I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
OLED_Init();
OLED_SetCursor(0, 0);
OLED_PrintStr("Hello CH32V003");
OLED_Update();
```

### NeoPixel
```c
#include "Lib/NeoPixel/NeoPixel.h"
NeoPixel_Init(GPIOC, GPIO_Pin_4, 8);   // 8 LEDs บน PC4
NeoPixel_SetPixelColor(0, 255, 0, 0);  // LED 0 = Red
NeoPixel_Show();
```

### DS18B20 Temperature
```c
#include "Lib/DS18B20/DS18B20.h"
DS18B20_Init(PC5);
float temp = DS18B20_ReadTemp();
printf("Temp: %.1f C\r\n", temp);
```

---

*SimpleHAL Version: 1.9.0 | Target: CH32V003 | Toolchain: GCC12 riscv-wch-elf*

---

## 13. แนวทางการเขียน Lib ใหม่

Lib ทุกตัวต้องสอดคล้องกับ pattern ที่มีอยู่แล้วใน `User/Lib/` ทุกประการ

### 13.1 โครงสร้างไฟล์

```
User/Lib/<LibName>/
├── <LibName>.h      ← Header: types, config, function prototypes
├── <LibName>.c      ← Implementation
└── README.md        ← วิธีการใช้งานภาษาไทยครบถ้วน
```

- ชื่อ folder และไฟล์ใช้ **PascalCase** (เช่น `DHT`, `HCSR04`, `Servo`, `Button`)
- ยกเว้น `lcd1602_i2c` ที่ใช้ snake_case เพราะ legacy — Lib ใหม่ให้ใช้ PascalCase เสมอ

### 13.2 รูปแบบ Header File (.h)

```c
/**
 * @file <LibName>.h
 * @brief <คำอธิบายสั้นๆ> Library สำหรับ CH32V003
 * @version 1.0
 * @date YYYY-MM-DD
 *
 * @details
 * Library นี้ ...
 *
 * **คุณสมบัติ:**
 * - ...
 *
 * **Hardware Connection:**
 * ```
 *   วาด ASCII diagram วงจร
 * ```
 *
 * @example
 * // ตัวอย่างการใช้งานพื้นฐาน 10-20 บรรทัด
 * #include "SimpleHAL.h"
 * #include "<LibName>.h"
 *
 * int main(void) { ... }
 *
 * @author CH32V003 Library Team
 * @copyright MIT License
 */

#ifndef __<LIBNAME>_H
#define __<LIBNAME>_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../SimpleHAL/SimpleHAL.h"
#include <stdint.h>
#include <stdbool.h>

/* ========== Configuration ========== */
/* ========== Type Definitions ========== */
/* ========== Function Prototypes ========== */

#ifdef __cplusplus
}
#endif

#endif /* __<LIBNAME>_H */
```

**กฎ header:**
- Include path: `"../../SimpleHAL/SimpleHAL.h"` (relative จาก `User/Lib/<Name>/`)
- หากใช้เฉพาะ module: `"../../SimpleHAL/SimpleGPIO.h"` ฯลฯ
- Guard: `#ifndef __<LIBNAME>_H` (UPPERCASE ทั้งหมด + double underscore)
- Section dividers: `/* ========== Section Name ========== */`
- ทุก function prototype ต้องมี JSDoc comment (TH) อธิบาย param, return, example

### 13.3 รูปแบบ Source File (.c)

```c
/**
 * @file <LibName>.c
 * @brief <LibName> Library Implementation
 * @version 1.0
 * @date YYYY-MM-DD
 */

#include "<LibName>.h"
#include <string.h>  // ถ้าใช้ memset/memcpy

/* ========== Private Variables ========== */

static <Type> instance_pool[MAX_INSTANCES];
static uint8_t instance_count = 0;

/* ========== Private Function Prototypes ========== */

static <RetType> LibName_PrivFunc(<params>);

/* ========== Private Functions ========== */

/* ========== Public Functions ========== */
```

**กฎ source:**
- ตัวแปร internal ทั้งหมดต้องเป็น `static` เพื่อป้องกัน namespace collision
- ใช้ `static` pool array + count สำหรับ multi-instance (ดูตัวอย่างใน `PIR.c`, `DS18B20.c`)
- ต้องมี null check: `if (instance == NULL) return;` หรือ `return <default_value>;`
- ต้องมี initialized check: `if (!instance->initialized) return;`
- Private functions ต้อง `static` เสมอ

### 13.4 รูปแบบ Struct Instance

```c
typedef struct {
    /* Pin Configuration */
    uint8_t     pin;            /**< GPIO pin */

    /* Configuration */
    uint16_t    config_value;   /**< คำอธิบาย */

    /* State (internal — ห้าม access ตรงจากภายนอก) */
    uint8_t     current_state;
    uint32_t    last_time;      /**< timestamp (ms) */

    /* Callbacks */
    void (*on_event)(void);     /**< Callback เมื่อเกิด event */

    uint8_t     initialized;    /**< flag บอกว่า Init แล้ว */
} LibName_Instance;
```

- ประกาศ comment `/**< ... */` ทุก field
- `initialized` ต้องมีทุก struct
- Callback field วางก่อน `initialized` เสมอ
- ใช้ `volatile` กับ field ที่ถูกแก้ไขจาก ISR

### 13.5 รูปแบบ Enum

```c
typedef enum {
    LIBNAME_STATE_IDLE = 0,     /**< อธิบายภาษาไทย */
    LIBNAME_STATE_ACTIVE,       /**< อธิบาย */
    LIBNAME_STATE_ERROR         /**< อธิบาย */
} LibName_State;
```

- Prefix ด้วยชื่อ Lib: `LIBNAME_`
- ค่า default/safe = 0 เสมอ
- ทุก value มี comment TH

### 13.6 Pattern การใช้งาน Timing

```c
// ใช้ Get_CurrentMs() จาก SimpleDelay สำหรับ ms timing
uint32_t now = Get_CurrentMs();
if ((now - instance->last_time) >= INTERVAL_MS) { ... }

// ใช้ Get_CurrentUs() สำหรับ µs timing (ใช้ใน DHT, HC-SR04)
uint32_t t = Get_CurrentUs();
while (cond) {
    if ((Get_CurrentUs() - t) >= TIMEOUT_US) return false;
}

// ใช้ Delay_Ms / Delay_Us สำหรับ blocking delay
Delay_Ms(20);
Delay_Us(40);
```

### 13.7 Pattern การ Disable/Enable Interrupt

ใช้สำหรับ bit-banging protocol ที่ต้องการ timing แม่นยำ (DHT, 1-Wire):
```c
__disable_irq();
// ... timing-critical section ...
__enable_irq();
```

### 13.8 กฎ Resource Conflict

| Protocol / Peripheral | SimpleHAL ที่ต้องใช้ | ข้อควรระวัง |
|-----------------------|---------------------|------------|
| GPIO only | `SimpleGPIO.h` | ไม่มี conflict |
| ADC | `SimpleADC.h` หรือ `analogRead()` | เฉพาะ pin PD2-PD7, PA1, PA2, PC4 |
| PWM 50Hz (Servo) | `SimplePWM.h` | ห้ามใช้ TIM เดียวกับ SimpleTIM |
| I2C | `SimpleI2C.h` | ต้อง pull-up 4.7kΩ บน SDA/SCL |
| SPI | `SimpleSPI.h` | CS pin ต้อง control เอง |
| USART | `SimpleUSART.h` | ห้าม printf ซ้อน |
| 1-Wire timing | `Simple1Wire.h` หรือ bit-bang | ต้อง disable IRQ |
| Timer interrupt | `SimpleTIM.h` | conflict กับ PWM บน TIM เดียวกัน |

### 13.9 Config Macros ที่ Override ได้

ทุก config ที่ผู้ใช้อาจต้องการเปลี่ยนต้องใช้ `#ifndef`:
```c
#ifndef LIBNAME_MAX_INSTANCES
#define LIBNAME_MAX_INSTANCES    4
#endif

#ifndef LIBNAME_TIMEOUT_MS
#define LIBNAME_TIMEOUT_MS       100
#endif
```

### 13.10 Error Return Values

- ฟังก์ชันที่คืน pointer: คืน `NULL` เมื่อ error
- ฟังก์ชันที่คืน float: คืน constant เช่น `LIBNAME_ERROR_VALUE` (-1.0f)
- ฟังก์ชันที่คืน status: ใช้ enum `LIBNAME_OK = 0`, error code อื่นๆ > 0
- ฟังก์ชัน void: ตรวจ null+init แล้ว return เงียบ

---

## 14. แนวทางการเขียน README ของ Lib

README ทุกอันต้องเขียนเป็น **ภาษาไทยเป็นหลัก** พร้อม code examples ภาษาอังกฤษ  
ดูตัวอย่างจาก `User/Lib/DS18B20/README.md` และ `User/Lib/Button/README.md`

### 14.1 โครงสร้าง README มาตรฐาน

```markdown
# <ชื่อ Library>

> **Library สำหรับ ... บน CH32V003**

## 📋 สารบัญ
1. [ภาพรวม](#ภาพรวม)
2. [คุณสมบัติ](#คุณสมบัติ)
3. [Hardware Setup](#hardware-setup)
4. [หลักการทำงาน](#หลักการทำงาน)   ← มีถ้า protocol ซับซ้อน
5. [การใช้งานพื้นฐาน](#การใช้งานพื้นฐาน)
6. [การใช้งานขั้นสูง](#การใช้งานขั้นสูง)
7. [Troubleshooting](#troubleshooting)
8. [API Reference](#api-reference)
```

### 14.2 ส่วน ภาพรวม

- อธิบาย hardware/component ที่ library ใช้งาน
- บอก use case หลัก
- ถ้ามี variant หลายรุ่น (เช่น DHT11/DHT22) ให้มีตาราง comparison

### 14.3 ส่วน คุณสมบัติ

ใช้ checkmark list:
```markdown
- ✅ คุณสมบัติ 1
- ✅ คุณสมบัติ 2
- ✅ เอกสารภาษาไทยครบถ้วน   ← บรรทัดสุดท้ายเสมอ
```

### 14.4 ส่วน Hardware Setup

**บังคับมี ASCII diagram วงจร:**
```markdown
```
CH32V003 Pin        Component
  TRIG (PC3) -----> PIN_A
  ECHO (PC4) <----- PIN_B
  GND        -----> GND
  3.3V       -----> VCC
```
```

- ระบุ voltage level (3.3V หรือ 5V)
- ระบุ pull-up/pull-down ที่ต้องต่อ
- ถ้าต้องมี voltage divider ให้ระบุค่า resistor
- บอก GPIO pins ที่ใช้ได้ทั้งหมดของ CH32V003

### 14.5 ส่วน หลักการทำงาน

มีเมื่อ protocol มีความซับซ้อน (DHT, 1-Wire, ultrasonic):
- ใช้ ASCII timing diagram
- อธิบายสูตรคำนวณ (เช่น `distance = echo_µs / 58.0`)
- ถ้ามีตาราง lookup แสดงตัวอย่างค่า

### 14.6 ส่วน การใช้งานพื้นฐาน

**บังคับมี 3 ส่วน:**

1. **ขั้นตอนที่ 1: Include และประกาศตัวแปร**
2. **ขั้นตอนที่ 2: Init ใน main()**
3. **ขั้นตอนที่ 3: main loop**

Code ต้องเป็น complete example ที่ compile และรันได้ทันที

### 14.7 ส่วน การใช้งานขั้นสูง

ให้มีหัวข้อย่อยตามฟีเจอร์ของ lib เช่น:
- Multi-sensor
- Callback system
- Non-blocking mode
- Integration กับ lib อื่น (เช่น ใช้ OLED แสดงผล)

แต่ละหัวข้อมี code snippet ที่ครบถ้วน ไม่ใช่แค่ function call เดียว

### 14.8 ส่วน Troubleshooting

ใช้ตาราง 2 คอลัมน์ "สาเหตุ | วิธีแก้":
```markdown
| สาเหตุ | วิธีแก้ |
|--------|---------|
| ... | ... |
```

จัดกลุ่มตาม error symptom (เช่น "ปัญหา: ได้ -1 ตลอด")

### 14.9 ส่วน API Reference

**ทุก public function ต้องมี:**
- ชื่อ function พร้อม parameters และ return type ใน heading
- ตาราง parameters ถ้ามี > 2 parameter
- ตาราง return values ถ้าเป็น enum
- ใช้ `---` คั่นระหว่างแต่ละ function

ตัวอย่าง:
```markdown
### `LibName_Init(instance, pin, config)` 
เริ่มต้น instance พร้อมตั้งค่า GPIO

| Parameter | Type | คำอธิบาย |
|-----------|------|----------|
| `instance` | `LibName_Instance*` | ตัวแปร instance |
| `pin` | `uint8_t` | GPIO pin เช่น `PC4` |
```

### 14.10 ข้อกำหนดทั่วไป

- ทุก code block ต้องระบุ language: ` ```c ` ไม่ใช่ ` ``` ` เฉยๆ
- ตัวแปรและชื่อ function ใน inline text ใช้ backtick: `` `DHT_Read()` ``
- ใช้ `>` blockquote สำหรับ warning/note สำคัญ  
  เช่น `> ⚠️ **สำคัญ**: ต้องต่อ pull-up 10kΩ เสมอ`
- หน่วยวัดใช้ทั้ง metric และ หน่วยที่เกี่ยวข้องกับ MCU (µs, ms, Hz, kHz)
- Emoji ที่ใช้บ่อย: ✅ ⚠️ 📋 สำหรับ checklist / warning / section header
- จบ README ด้วย constants table (ถ้ามี configurable values)

---

## 15. Lib ที่มีอยู่แล้วและสถานะ

### SimpleHAL (User/SimpleHAL/)

| Module | สถานะ | หมายเหตุ |
|--------|--------|---------|
| SimpleGPIO | ✅ เสร็จ | pinMode, digitalWrite, digitalRead, attachInterrupt, analogRead, analogWrite |
| SimpleTIM | ✅ เสร็จ | TIM1, TIM2 interrupt callbacks |
| SimpleTIM_Ext | ✅ เสร็จ | Input capture, output compare |
| SimplePWM | ✅ เสร็จ | PWM1_CH1-4, PWM2_CH1-4 |
| SimpleUSART | ✅ เสร็จ | USART1, 3 pin configs |
| SimpleI2C | ✅ เสร็จ | Hardware I2C, 2 pin configs |
| SimpleI2C_Soft | ✅ เสร็จ | Software I2C (bit-bang) |
| SimpleSPI | ✅ เสร็จ | Hardware SPI |
| SimpleADC | ✅ เสร็จ | 10-bit ADC, 8 channels |
| SimpleDelay | ✅ เสร็จ | Delay_Ms/Us, Get_CurrentMs/Us, Timer_t |
| SimpleFlash | ✅ เสร็จ | Flash read/write |
| SimpleIWDG | ✅ เสร็จ | Independent Watchdog |
| SimpleWWDG | ✅ เสร็จ | Window Watchdog |
| SimpleDMA | ✅ เสร็จ | DMA transfers |
| Simple1Wire | ✅ เสร็จ | 1-Wire protocol |
| SimpleOPAMP | ✅ เสร็จ | OPAMP peripheral |
| SimplePWR | ✅ เสร็จ | Sleep, Standby, PVD, AWU |

### Device Libraries (User/Lib/)

| Library | สถานะ | Protocol | หมายเหตุ |
|---------|--------|---------|---------|
| Buzzer | ✅ เสร็จ | PWM | Tone, melody, volume |
| DS18B20 | ✅ เสร็จ | 1-Wire | Multi-sensor, CRC |
| I2CScan | ✅ เสร็จ | I2C | Scan หา device address |
| IR | ✅ เสร็จ | GPIO/Timer | NEC, RC5, SIRC |
| LCD1602_I2C | ✅ เสร็จ | I2C | PCF8574, 16x2/20x4 |
| MAX7219 | ✅ เสร็จ | SPI | LED Matrix 8x8, cascaded |
| NeoPixel | ✅ เสร็จ | Bit-bang | WS2812/WS2812B |
| NTC10K | ✅ เสร็จ | ADC | Steinhart-Hart equation |
| OLED | ✅ เสร็จ | I2C | SSD1306, 128x64/32 |
| PIR | ✅ เสร็จ | GPIO | NS312, RCWL-0516 |
| RotaryEncoder | ✅ เสร็จ | GPIO/EXTI | KY-040, quadrature |
| TJC | ✅ เสร็จ | USART | TJC/Nextion HMI |
| TM1637 | ✅ เสร็จ | Custom 2-wire | 4/6-digit 7-seg |
| WS2815Matrix | ✅ เสร็จ | Bit-bang | 8x8 matrix, Thai font |
| **DHT** | ✅ เสร็จ | Bit-bang | DHT11/DHT22, CRC |
| **HCSR04** | ✅ เสร็จ | GPIO | Echo timing, average |
| **Servo** | ✅ เสร็จ | PWM 50Hz | 0-180°, SweepTo |
| **Button** | ✅ เสร็จ | GPIO | Debounce, LongPress, DoubleClick |

### Phase 2 — เสร็จแล้ว

| Library | Protocol | หมายเหตุ |
|---------|---------|---------|
| **StepperMotor** | ✅ เสร็จ | GPIO | ULN2003 (28BYJ-48), A4988 (NEMA17), Full/Half step |
| **ShiftReg595** | ✅ เสร็จ | GPIO | 74HC595, cascade 1-4 ICs, shadow buffer |
| **AT24Cxx** | ✅ เสร็จ | I2C | AT24C01-AT24C512, page write, ACK polling |
| **DS3231** | ✅ เสร็จ | I2C | RTC+TCXO, Alarm1/2, temperature |
| **HX711** | ✅ เสร็จ | GPIO | 24-bit ADC, Tare, Calibration, Gain 128/64/32 |

### Phase 3 — เสร็จแล้ว ✅

| Library | สถานะ | Protocol | รายละเอียด |
|---------|-------|---------|-----------|
| **MPU6050** | ✅ เสร็จ | I2C | Accel ±2G-16G, Gyro ±250-2000DPS, Temp, CalibrateGyro, DLPF |
| **BMP280** | ✅ เสร็จ | I2C | Pressure/Temp, Altitude, Bosch compensation formula, Forced/Normal mode |
| **KeyMatrix** | ✅ เสร็จ | GPIO | 4x4/4x3, Debounce, LongPress, Custom keymap, WaitKey |
| **MQGas** | ✅ เสร็จ | ADC | MQ2/3/4/5/6/7/9/135, Auto-calibrate Ro, PPM, Alarm threshold |
| **nRF24L01** | ✅ เสร็จ | SPI | 2.4GHz TX/RX, Auto-ACK, 126ch, 250k/1M/2Mbps, PowerDown |

### Phase 4 — เสร็จแล้ว ✅

| Library | สถานะ | Protocol | RAM | รายละเอียด |
|---------|-------|---------|-----|-----------|
| **BH1750** | ✅ เสร็จ | I2C | ~20B | Light sensor (Lux), Continuous/One-time mode, 0.5-4 lux res |
| **SHT3x** | ✅ เสร็จ | I2C | ~30B | Temp ±0.1°C + Humidity, CRC-8, High/Med/Low repeatability |
| **INA219** | ✅ เสร็จ | I2C | ~40B | Current/Voltage/Power monitor, calibration register |
| **MCP4725** | ✅ เสร็จ | I2C | ~20B | 12-bit DAC output, SetVoltage, EEPROM save |
| **ADS1115** | ✅ เสร็จ | I2C | ~40B | 16-bit ADC 4ch, PGA ±256mV-±6.144V, 8-860SPS |
| **PCA9685** | ✅ เสร็จ | I2C | ~30B | 16ch PWM expander, SetServoAngle, SetDuty, SetPulse |
| **W25Qxx** | ✅ เสร็จ | SPI | ~64B | NOR Flash 2-16MB, Read/Write/EraseSector/EraseChip, JEDEC ID |
| **VL53L0X** | ✅ เสร็จ | I2C | ~50B | ToF distance 30-2000mm, Single/Continuous, multi-sensor XSHUT |
| **HC05** | ✅ เสร็จ | USART | ~70B | Bluetooth HC-05, Data mode + AT command, ReadLine |

---

*SimpleHAL Version: 1.9.0 | Target: CH32V003/CH32V006 | Toolchain: GCC12 riscv-wch-elf*
