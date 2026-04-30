# SimpleHAL Library

Simple Hardware Abstraction Layer สำหรับ CH32V003 - ใช้งานง่ายแบบ Arduino

> **MCU:** CH32V003 (RISC-V, 48MHz, 16KB Flash, 2KB RAM)
> **Version:** 1.3 | **License:** MIT

---

## 📚 คู่มือแยกตาม Module

| Module | ไฟล์ | คำอธิบาย |
|--------|------|----------|
| [SimpleGPIO](SimpleGPIO/README.md) | `SimpleGPIO.h/.c` | GPIO: digital R/W, interrupt, port-level |
| [SimpleDelay](SimpleDelay/README.md) | `SimpleDelay.h/.c` | Blocking delay + non-blocking timer |
| [SimpleUSART](SimpleUSART/README.md) | `SimpleUSART.h/.c` | USART serial communication |
| [SimpleI2C](SimpleI2C/README.md) | `SimpleI2C.h/.c` | Hardware I2C (100/400kHz) |
| [SimpleI2C_Soft](SimpleI2C_Soft/README.md) | `SimpleI2C_Soft.h/.c` | Software I2C (bit-bang) |
| [SimpleSPI](SimpleSPI/README.md) | `SimpleSPI.h/.c` | Hardware SPI (mode 0-3) |
| [SimpleADC](SimpleADC/README.md) | `SimpleADC.h/.c` | ADC 10-bit, 8 channels |
| [SimplePWM](SimplePWM/README.md) | `SimplePWM.h/.c` | PWM output TIM1/TIM2 |
| [SimpleOPAMP](SimpleOPAMP/README.md) | `SimpleOPAMP.h/.c` | OPAMP: follower, amplifier, comparator |
| [SimpleTIM](SimpleTIM/README.md) | `SimpleTIM.h/.c` | Hardware timer + interrupt |
| [SimpleTIM_Ext](SimpleTIM_Ext/README.md) | `SimpleTIM_Ext.h/.c` | Stopwatch & countdown timer |
| [SimpleDMA](SimpleDMA/README.md) | `SimpleDMA.h/.c` | DMA transfers (M2M, P2M, M2P) |
| [SimpleFlash](SimpleFlash/README.md) | `SimpleFlash.h/.c` | Flash storage (non-volatile) |
| [SimpleIWDG](SimpleIWDG/README.md) | `SimpleIWDG.h/.c` | Independent Watchdog |
| [SimpleWWDG](SimpleWWDG/README.md) | `SimpleWWDG.h/.c` | Window Watchdog |
| [Simple1Wire](Simple1Wire/README.md) | `Simple1Wire.h/.c` | 1-Wire protocol (DS18B20, etc.) |
| [SimplePWR](SimplePWR/README.md) | `SimplePWR.h/.c` | Power management: Sleep, Standby |

---

## ⚡ Quick Reference Card

### GPIO Pins

```
GPIOA: PA1(0)  PA2(1)
GPIOC: PC0(10) PC1(11) PC2(12) PC3(13) PC4(14) PC5(15) PC6(16) PC7(17)
GPIOD: PD2(20) PD3(21) PD4(22) PD5(23) PD6(24) PD7(25)
```

### ADC Channels

| Channel | Pin | Alias |
|---------|-----|-------|
| 0 | PA2 | `ADC_CH_PA2` |
| 1 | PA1 | `ADC_CH_PA1` |
| 2 | PC4 | `ADC_CH_PC4` |
| 3 | PD2 | `ADC_CH_PD2` |
| 4 | PD3 | `ADC_CH_PD3` |
| 5 | PD5 | `ADC_CH_PD5` |
| 6 | PD6 | `ADC_CH_PD6` |
| 7 | PD4 | `ADC_CH_PD4` |
| 8 | — | `ADC_CH_VREFINT` (internal ~1.2V) |
| 9 | — | `ADC_CH_VCALINT` (calibration) |

### PWM Channels

| Channel | Pin | Timer |
|---------|-----|-------|
| `PWM1_CH1` | PD2 | TIM1 |
| `PWM1_CH2` | PA1 | TIM1 |
| `PWM1_CH3` | PC3 | TIM1 |
| `PWM1_CH4` | PC4 | TIM1 |
| `PWM2_CH1` | PD4 | TIM2 |
| `PWM2_CH2` | PD3 | TIM2 |
| `PWM2_CH3` | PC0 | TIM2 |
| `PWM2_CH4` | PD7 | TIM2 |

### I2C Pins

| Config | SCL | SDA |
|--------|-----|-----|
| `I2C_PINS_DEFAULT` | PC2 | PC1 |
| `I2C_PINS_REMAP` | PD0 | PD1 |

### SPI Pins

| Config | SCK | MISO | MOSI | NSS |
|--------|-----|------|------|-----|
| `SPI_PINS_DEFAULT` | PC5 | PC7 | PC6 | PC4 |
| `SPI_PINS_REMAP` | PC6 | PC8 | PC7 | PC5 |

### USART Pins

| Config | TX | RX |
|--------|----|----|
| `USART_PINS_DEFAULT` | PD5 | PD6 |
| `USART_PINS_REMAP1` | PD0 | PD1 |
| `USART_PINS_REMAP2` | PD6 | PD5 |

---

## 📁 โครงสร้างไฟล์

```
SimpleHAL/
├── SimpleGPIO.h/.c         # GPIO และ Interrupt control
├── SimpleADC.h/.c          # ADC (Analog to Digital Converter)
├── SimplePWM.h/.c          # PWM output
├── SimpleTIM.h/.c          # Timer peripherals
├── SimpleTIM_Ext.h/.c      # Stopwatch & Countdown
├── SimpleUSART.h/.c        # Serial communication
├── SimpleI2C.h/.c          # Hardware I2C communication
├── SimpleI2C_Soft.h/.c     # Software I2C (bit-bang)
├── SimpleSPI.h/.c          # SPI communication
├── SimpleDMA.h/.c          # DMA controller
├── SimpleFlash.h/.c        # Flash memory storage
├── Simple1Wire.h/.c        # 1-Wire protocol
├── SimpleIWDG.h/.c         # Independent Watchdog
├── SimpleWWDG.h/.c         # Window Watchdog
├── SimpleOPAMP.h/.c        # OPAMP peripheral
├── SimplePWR.h/.c          # Power management
├── SimpleDelay.h/.c        # Delay & timing
├── SimpleHAL.h             # Header รวมทั้งหมด (include ตัวเดียว)
└── readme/                 # คู่มือแยกตาม module
```

---

## 🚀 การใช้งาน

### วิธีที่ 1: Include รวมทุกอย่าง (แนะนำ)

```c
#include "SimpleHAL.h"  // รวมทุก module

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    // เรียกใช้งาน API ได้เลย
}
```

### วิธีที่ 2: Include เฉพาะที่ต้องการ

```c
#include "SimpleHAL/SimpleGPIO.h"
#include "SimpleHAL/SimpleDelay.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    pinMode(PC0, PIN_MODE_OUTPUT);
    digitalWrite(PC0, HIGH);
    
    while(1) {
        digitalToggle(PC0);
        Delay_Ms(500);
    }
}
```

### วิธีที่ 2: Include ทั้งหมด

```c
#include "SimpleHAL/SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // ใช้งาน GPIO
    pinMode(PC0, PIN_MODE_OUTPUT);
    
    // ใช้งาน USART
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    USART_Print("Hello World!\r\n");
    
    // ใช้งาน I2C
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
    
    while(1) {
        // Your code here
    }
}
```

## 📚 Peripherals ที่รองรับ

| Peripheral | Header | คำอธิบาย |
|-----------|--------|----------|
| **GPIO** | `SimpleGPIO.h` | Digital I/O, Interrupts |
| **ADC** | `SimpleADC.h` | อ่านค่า Analog |
| **PWM** | `SimplePWM.h` | PWM output control (8 channels) |
| **OPAMP** | `SimpleOPAMP.h` | Operational Amplifier (ขยายสัญญาณ, buffer) |
| **Flash** | `SimpleFlash.h` | Flash memory storage (config/data) |
| **TIM** | `SimpleTIM.h` | Timer interrupts |
| **TIM Ext** | `SimpleTIM_Ext.h` | Stopwatch และ Countdown timers |
| **USART** | `SimpleUSART.h` | Serial communication |
| **I2C** | `SimpleI2C.h` | I2C สำหรับ sensors, EEPROM |
| **SPI** | `SimpleSPI.h` | SPI communication |
| **IWDG** | `SimpleIWDG.h` | Independent Watchdog (ป้องกันระบบค้าง) |
| **WWDG** | `SimpleWWDG.h` | Window Watchdog (ตรวจสอบ timing) |
| **Timer** | `timer.h` | Delay และ timing functions |

## 📖 ตัวอย่างเริ่มต้น (Hello World)

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    pinMode(PC0, PIN_MODE_OUTPUT);
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    USART_Print("CH32V003 Ready\r\n");

    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
    ADC_SimpleInit();

    uint16_t counter = 0;
    while (1) {
        digitalToggle(PC0);
        USART_Print("cnt=");
        USART_PrintNum(counter++);
        USART_Print("\r\n");
        Delay_Ms(500);
    }
}
```

---

## ⚠️ สิ่งที่ต้องทำเสมอ

```c
int main(void) {
    SystemCoreClockUpdate();  // ① อัปเดต system clock (ต้องเป็นบรรทัดแรก)
    Timer_Init();              // ② เริ่ม SysTick (Delay_Ms / Get_CurrentMs)
}
```

ขาด `SystemCoreClockUpdate()` → baud rate ผิด, PWM ผิดความถี่  
ขาด `Timer_Init()` → `Delay_Ms()` และ `Get_CurrentMs()` ไม่ทำงาน

---

## 🔧 Timer Resource Map

| Resource | ใช้โดย | หมายเหตุ |
|----------|--------|---------|
| SysTick | SimpleDelay | ห้ามใช้ร่วม |
| TIM1 | SimplePWM (PWM1_CH1-4) **หรือ** SimpleTIM | เลือกอย่างใดอย่างหนึ่ง |
| TIM2 | SimplePWM (PWM2_CH1-4) **หรือ** SimpleTIM **หรือ** SimpleTIM_Ext | เลือกอย่างใดอย่างหนึ่ง |
| IWDG | SimpleIWDG | LSI clock อิสระ |
| WWDG | SimpleWWDG | PCLK1 clock |

---

## 📝 License

MIT License

*สำหรับรายละเอียด API แต่ละ module ดูในไฟล์ README ของ module นั้นๆ หรือในไฟล์ `.h` โดยตรง*

