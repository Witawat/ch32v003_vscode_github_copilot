# CH32V003 Project — Programming Guidelines

> เขียนสำหรับใช้เป็น reference ก่อนเริ่มพัฒนา feature ใหม่

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
