# CH32V003 Project — Knowledge Base

> **Generated:** 2026-05-25
> **Purpose:** เอกสารสรุปโครงสร้าง project, แนวทางการเขียน code, patterns, และ API ทั้งหมด สำหรับให้ AI/Copilot ใช้อ้างอิงในการพัฒนา

---

## 1. Project Overview

### 1.1 Microcontroller: CH32V003

| Property | Value |
|----------|-------|
| **Core** | RISC-V V2 (rv32ecxw) |
| **ABI** | ilp32e |
| **Max Clock** | 48 MHz (HSI, PLL) |
| **Flash** | 16 KB (256 pages × 64 bytes) |
| **RAM** | 2 KB |
| **GPIO Ports** | GPIOA (PA1-PA2), GPIOC (PC0-PC7), GPIOD (PD2-PD7) |
| **ADC** | 10-bit, 8 external + 2 internal channels |
| **Timers** | TIM1 (advanced), TIM2 (general purpose) |
| **USART** | 1× USART1 |
| **I2C** | 1× I2C1 |
| **SPI** | 1× SPI1 |
| **DMA** | 7 channels |
| **Interrupts** | Nested Vectored Interrupt Controller (PFIC/NVIC) |

### 1.2 Directory Structure

```
CH32V003/
├── build.bat              # Build script (Windows batch)
├── clean.bat              # Clean script
├── upload.bat             # Flash via OpenOCD + WCH-Link
├── rebuild.bat            # Clean + Build
├── Ld/Link.ld             # Linker script (Flash=16K, RAM=2K)
├── Startup/startup_ch32v00x.S  # Vector table + reset handler
├── Core/
│   ├── core_riscv.h       # RISC-V core peripherals (PFIC, SysTick, inline asm)
│   └── core_riscv.c       # RISC-V CSR access functions
├── Debug/
│   ├── debug.h            # UART printf / SDI printf config
│   └── debug.c            # USART_Printf_Init, SDI_Printf_Enable
├── Peripheral/
│   ├── inc/               # WCH Standard Peripheral Library headers
│   │   ├── ch32v00x.h     # Master header: IRQn, peripheral structs, base addresses
│   │   ├── ch32v00x_gpio.h, ch32v00x_rcc.h, ch32v00x_usart.h, ...
│   └── src/               # WCH Standard Peripheral Library sources
│       ├── ch32v00x_gpio.c, ch32v00x_rcc.c, ch32v00x_usart.c, ...
├── User/
│   ├── main.c / main.h    # Application entry point
│   ├── system_ch32v00x.c/h  # System clock init (default: 48MHz HSI)
│   ├── ch32v00x_it.c/h    # Default interrupt handlers (NMI, HardFault)
│   ├── ch32v00x_conf.h    # Peripheral includes + utility macros
│   ├── SimpleHAL/         # Arduino-style HAL (★CORE API for all Libs★)
│   │   ├── SimpleHAL.h/c  # Master include
│   │   ├── SimpleGPIO.h/c, SimpleDelay.h/c, SimpleUSART.h/c,
│   │   ├── SimpleI2C.h/c, SimpleSPI.h/c, SimpleADC.h/c,
│   │   ├── SimpleTIM.h/c, SimpleTIM_Ext.h/c, SimplePWM.h/c,
│   │   ├── SimpleDMA.h/c, SimpleFlash.h/c,
│   │   ├── Simple1Wire.h/c, SimplePWR.h/c, SimpleOPAMP.h/c,
│   │   ├── SimpleIWDG.h/c, SimpleWWDG.h/c
│   └── Lib/               # Device driver libraries (~50 libraries)
│       ├── Servo/, DHT/, HCSR04/, Button/  (Phase 1 - complete)
│       ├── I2CScan/, LCDMenu/, LCD1602_I2C/, OLED/, ...
│       └── P10/, WS2812Matrix/, MAX7219/, ...
```

---

## 2. Build System

### 2.1 Toolchain

| Component | Path |
|-----------|------|
| **Compiler (recommended)** | `riscv-wch-elf-gcc` (GCC 12.2.0) from MounRiver Studio 2 |
| **Fallback compiler** | `riscv-none-embed-gcc` (GCC 8.2.0) |
| **OpenOCD** | `MounRiver_Studio2/.../OpenOCD/bin/openocd.exe` |
| **Config** | `wch-riscv.cfg` |

### 2.2 Compile Flags

```
ARCH    = -march=rv32ecxw -mabi=ilp32e
CFLAGS  = -msmall-data-limit=0 -msave-restore -Os -fmessage-length=0
          -fsigned-char -ffunction-sections -fdata-sections -fno-common
          -Wunused -Wuninitialized -g
```

### 2.3 Include Paths (compile order)

```
-I Debug/
-I Core/
-I User/
-I Peripheral/inc/
-I User/SimpleHAL/
```

### 2.4 Build Process (build.bat)

1. Compile `Core/core_riscv.c`
2. Compile `Debug/debug.c`
3. Compile ALL `Peripheral/src/*.c`
4. Compile `User/main.c`, `system_ch32v00x.c`, `ch32v00x_it.c`
5. Compile ALL `User/SimpleHAL/*.c`
6. Compile ALL `User/Lib/**/*.c` (recursive, prefix `lib_`)
7. Assemble `Startup/startup_ch32v00x.S`
8. Link → `output/CH32V003.elf`
9. Create `output/CH32V003.hex`

### 2.5 Memory Layout (Link.ld)

```
FLASH: 0x00000000, 16K
RAM:   0x20000000, 2K
```

Special sections:
- `.init` — vector table (Flash)
- `.highcode` — functions copied to RAM for execution (RAM AT>FLASH)
- `.text`, `.rodata` — code/const in Flash
- `.data`, `.bss` — in RAM

---

## 3. Coding Conventions & Patterns

### 3.1 File Structure Conventions

| Item | Convention |
|------|-----------|
| **Author (main.c, TJC)** | `@author MAKER WITAWAT (https://www.makerwitawat.com)` |
| **Author (Libs)** | `@author CH32V003 Library Team` |
| **Lib path** | `User/Lib/<Name>/<Name>.h` + `<Name>.c` + `README.md` |
| **Include from Lib** | `#include "../../SimpleHAL/SimpleHAL.h"` |
| **Header guard** | `#ifndef __<NAME>_H` |
| **Copyright** | MIT License |
| **Comments** | ภาษาไทย (Thai) |

### 3.2 Struct / Instance Pattern (MANDATORY)

Every device driver library follows this pattern:

```c
typedef struct {
    // config fields...
    uint8_t pin;
    uint32_t some_config;
    // state fields...
    float last_value;
    uint32_t last_time;
    uint8_t initialized;    // ★ REQUIRED FLAG
} Device_Instance;

// All public functions:
// 1. Check null pointer
// 2. Check initialized flag
// 3. Return error code or value
```

**RULES:**
- Every struct **MUST** have `uint8_t initialized;` field
- Every public function **MUST** check `instance != NULL` and `instance->initialized`
- Internal/helper functions **MUST** be `static`
- Struct variables must be **global or static** — never local (stack is only 2KB)

### 3.3 Init Pattern

```c
void Device_Init(Device_Instance* dev, ...) {
    if (dev == NULL) return;
    memset(dev, 0, sizeof(Device_Instance));  // zero all fields
    dev->pin = pin;
    // set config...
    dev->initialized = 1;  // LAST
}
```

### 3.4 ISR / Interrupt Handling

```c
// Disable interrupts for critical sections
__disable_irq();
// ... critical code ...
__enable_irq();

// Interrupt handler attribute
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
```

### 3.5 Naming Conventions

| Type | Convention | Example |
|------|-----------|---------|
| **Enums** | `UPPER_SNAKE` or `Pascal_Snake` | `SPI_MODE0`, `I2C_Status` |
| **Functions** | `PascalCase` per module | `DHT_Read()`, `Servo_Write()` |
| **Static functions** | `_lowerCase` prefix `_` | `_read_reg()`, `_calc_crc()` |
| **Macros** | `UPPER_SNAKE` | `SERVO_PWM_FREQUENCY` |
| **Struct fields** | `snake_case` | `pin_trig`, `last_distance_cm` |

### 3.6 Utility Macros (from ch32v00x_conf.h)

```c
#define MAP(x, in_min, in_max, out_min, out_max) ...
#define CONSTRAIN(x, low, high) ...
#define ARRAY_SIZE(a)       (sizeof(a)/sizeof((a)[0]))
#define ABS(x)              ((x) < 0 ? -(x) : (x))
#define MAX(a, b) / MIN(a, b)
#define LOW_BYTE(w) / HIGH_BYTE(w) / MAKE_WORD(high, low)
#define _BV(bit)            (1UL << (bit))
#define BIT_READ / BIT_SET / BIT_CLEAR / BIT_TOGGLE / BIT_WRITE
```

---

## 4. SimpleHAL API Reference

> **ALL Lib drivers MUST use SimpleHAL — never call WCH peripheral lib directly.**

### 4.1 Include Chain

```c
#include <main.h>            // → includes SimpleHAL/SimpleHAL.h
// OR
#include "../../SimpleHAL/SimpleHAL.h"  // from Lib files
```

`SimpleHAL.h` includes ALL sub-modules and `<ch32v00x.h>` automatically.

### 4.2 SimpleDelay (Timer & Delay)

```c
// Auto-initialized via __attribute__((constructor)) — no need to call Timer_Init()

void Delay_Ms(uint32_t n);          // Blocking ms delay
void Delay_Us(uint32_t n);          // Blocking µs delay
uint32_t Get_CurrentMs(void);       // Millisecond counter (overflow-safe 49.7 days)
uint32_t Get_CurrentUs(void);       // Microsecond counter (overflow-safe 71.6 min)

// Non-blocking timers
void Start_Timer(Timer_t* timer, uint32_t ms, uint8_t repeat);
uint8_t Is_Timer_Expired(Timer_t* timer);
void Stop_Timer(Timer_t* timer);

// Helper macros
#define ELAPSED_TIME(start, current)  ((uint32_t)((current) - (start)))
#define IS_TIMEOUT(start, timeout)    (ELAPSED_TIME(start, Get_CurrentMs()) >= (timeout))
```

### 4.3 SimpleGPIO

```c
// Pin enum: PA1=0, PA2=1, PC0=10, PC1=11, ..., PC7=17, PD2=20, ..., PD7=25
// Mode enum: PIN_MODE_INPUT, PIN_MODE_OUTPUT, PIN_MODE_INPUT_PULLUP,
//            PIN_MODE_INPUT_PULLDOWN, PIN_MODE_OUTPUT_OD

void pinMode(uint8_t pin, GPIO_PinMode mode);
void digitalWrite(uint8_t pin, uint8_t value);  // HIGH/LOW
uint8_t digitalRead(uint8_t pin);
void digitalToggle(uint8_t pin);

// Multiple pins at once
#define pinModeMultiple(pins_array, mode)  // auto-calc count

// Interrupts
typedef enum { RISING, FALLING, CHANGE } GPIO_InterruptMode;
void attachInterrupt(uint8_t pin, GPIO_InterruptMode mode, void (*callback)(void));
void detachInterrupt(uint8_t pin);

// Analog
uint16_t analogRead(uint8_t pin);     // PD2-PD7 pins, returns 0-1023
void analogWrite(uint8_t pin, uint16_t value);  // PWM on capable pins
```

### 4.4 SimpleUSART

```c
// Baud: BAUD_9600 .. BAUD_460800
// Pins: USART_PINS_DEFAULT (TX=PD5,RX=PD6),
//        USART_PINS_REMAP1 (TX=PD0,RX=PD1),
//        USART_PINS_REMAP2 (TX=PD6,RX=PD5)

void USART_SimpleInit(USART_BaudRate baud, USART_PinConfig pin_config);
void USART_Print(const char* str);
void USART_PrintNum(int32_t num);
void USART_PrintHex(uint32_t num, uint8_t uppercase);
void USART_WriteByte(uint8_t data);
uint8_t USART_Available(void);
uint8_t USART_Read(void);          // blocking
uint16_t USART_ReadBytes(uint8_t* buffer, uint16_t length);
void USART_Flush(void);
```

### 4.5 SimpleI2C

```c
// Speed: I2C_100KHZ, I2C_400KHZ
// Pins: I2C_PINS_DEFAULT (SCL=PC2,SDA=PC1), I2C_PINS_REMAP (SCL=PD0,SDA=PD1)
// Status: I2C_OK, I2C_ERROR_TIMEOUT, I2C_ERROR_NACK, I2C_ERROR_BUS_BUSY

void I2C_SimpleInit(I2C_Speed speed, I2C_PinConfig pin_config);
I2C_Status I2C_Write(uint8_t addr, uint8_t* data, uint16_t len);
I2C_Status I2C_Read(uint8_t addr, uint8_t* data, uint16_t len);
I2C_Status I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t data);
uint8_t I2C_ReadReg(uint8_t addr, uint8_t reg);             // returns value directly!
I2C_Status I2C_WriteRegMulti(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t len);
I2C_Status I2C_ReadRegMulti(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t len);
uint8_t I2C_Scan(uint8_t* found_devices, uint8_t max_devices);
uint8_t I2C_IsDeviceReady(uint8_t addr);
```

**⚠️ CRITICAL:** `I2C_ReadReg()` returns the read value directly (NOT a status). To get both status and value, use `I2C_ReadRegMulti(addr, reg, &val, 1)`.

### 4.6 SimpleSPI

```c
// Mode: SPI_MODE0..SPI_MODE3
// Speed: SPI_125KHZ .. SPI_12MHZ
// Pins: SPI_PINS_DEFAULT (SCK=PC5,MISO=PC7,MOSI=PC6,NSS=PC4),
//        SPI_PINS_REMAP (SCK=PC6,MISO=PC8,MOSI=PC7,NSS=PC5)

void SPI_SimpleInit(SPI_Mode mode, SPI_Speed speed, SPI_PinConfig pin_config);
uint8_t SPI_Transfer(uint8_t data);
void SPI_TransferBuffer(uint8_t* tx, uint8_t* rx, uint16_t len);
void SPI_Write(uint8_t* data, uint16_t len);
void SPI_Read(uint8_t* data, uint16_t len, uint8_t dummy_byte);
void SPI_SetCSPin(GPIO_TypeDef* port, uint16_t pin);  // เปลี่ยน CS pin สำหรับ multi-device
void SPI_SetCS(uint8_t state);
void SPI_SetBitOrder(SPI_BitOrder order);
void SPI_SetSpeed(SPI_Speed speed);
```

### 4.7 SimpleADC

```c
// Channels: ADC_CH_0..ADC_CH_7 (also ADC_CH_PA2, ADC_CH_PD2, etc.)
// Internal: ADC_CH_VREFINT (≈1.2V), ADC_CH_VCALINT

void ADC_SimpleInit(void);
void ADC_SimpleInitChannels(ADC_Channel* channels, uint8_t count);
uint16_t ADC_Read(ADC_Channel channel);
void ADC_ReadMultiple(ADC_Channel* channels, uint16_t* values, uint8_t count);
float ADC_ToVoltage(uint16_t adc_value, float vref);
float ADC_ReadVoltage(ADC_Channel channel, float vref);
```

### 4.8 SimpleTIM

```c
// Timer: TIM_1 (advanced), TIM_2 (general purpose)

void TIM_SimpleInit(TIM_Instance timer, uint32_t frequency_hz);
void TIM_Start(TIM_Instance timer);
void TIM_Stop(TIM_Instance timer);
void TIM_SetFrequency(TIM_Instance timer, uint32_t frequency_hz);
void TIM_AttachInterrupt(TIM_Instance timer, void (*callback)(void));
void TIM_DetachInterrupt(TIM_Instance timer);
uint16_t Simple_TIM_GetCounter(TIM_Instance timer);
void Simple_TIM_SetCounter(TIM_Instance timer, uint16_t value);
uint16_t TIM_GetPeriod(TIM_Instance timer);

// Advanced: manual prescaler + period
void TIM_AdvancedInit(TIM_Instance timer, uint16_t prescaler, uint16_t period, TIM_Mode mode);
```

**⚠️ Rule:** Do NOT use SimpleTIM and SimplePWM on the same timer instance.

**🔧 Bug fix (2026-05-25):** `enableTimerClock()` เคยใช้ `RCC_APB2PeriphClockCmd` สำหรับ TIM2 — แก้เป็น `RCC_APB1PeriphClockCmd` แล้ว (TIM2 อยู่บน APB1 bus)

### 4.9 SimplePWM

```c
// Channels: PWM1_CH1(PD2), PWM1_CH2(PA1), PWM1_CH3(PC3), PWM1_CH4(PC4),
//            PWM2_CH1(PD4), PWM2_CH2(PD3), PWM2_CH3(PC0), PWM2_CH4(PD7)

void PWM_Init(PWM_Channel channel, uint32_t frequency_hz);
void PWM_InitRemap(PWM_Channel channel, uint32_t frequency_hz, PWM_Remap remap);
void PWM_Start(PWM_Channel channel);
void PWM_Stop(PWM_Channel channel);
void PWM_SetDutyCycle(PWM_Channel channel, uint8_t duty_percent);   // 0-100
void PWM_SetDutyCycleRaw(PWM_Channel channel, uint16_t duty_value); // raw
void PWM_SetFrequency(PWM_Channel channel, uint32_t frequency_hz);
uint16_t PWM_GetPeriod(PWM_Channel channel);
uint16_t PWM_GetDutyCycleRaw(PWM_Channel channel);
```

### 4.10 SimpleDMA

```c
// 7 channels: DMA_CH1..DMA_CH7
// Direction: DMA_DIR_PERIPH_TO_MEM, DMA_DIR_MEM_TO_PERIPH, DMA_DIR_MEM_TO_MEM
// Mode: DMA_MODE_NORMAL, DMA_MODE_CIRCULAR

// --- Core ---
void DMA_Init(DMA_Channel channel, void* periph_addr, void* mem_addr,
              uint16_t size, DMA_Direction dir, DMA_Mode mode);
void DMA_Start(DMA_Channel channel);
void DMA_Stop(DMA_Channel channel);
void DMA_SetCallback(DMA_Channel channel, void (*callback)(void));
void DMA_SetHalfTransferCallback(DMA_Channel channel, void (*callback)(void));
void DMA_WaitComplete(DMA_Channel channel, uint32_t timeout_ms);
void DMA_MemCopy(void* dst, void* src, uint32_t size);  // blocking copy via DMA

// --- USART ---
void DMA_USART_Send(uint8_t* data, uint16_t len);  // one-shot USART TX via DMA_CH2

// --- I2C ---
void DMA_I2C_InitTx(uint8_t* data, uint16_t len);
void DMA_I2C_InitRx(uint8_t* data, uint16_t len);
void DMA_I2C_Transfer(uint8_t* tx_data, uint8_t* rx_data, uint16_t len);

// --- TIM ---
void DMA_TIM_InitCapture(TIM_Instance timer, DMA_Channel ch);  // TIM -> DMA on CC event
void DMA_TIM_UpdatePWM(TIM_Instance timer, DMA_Channel ch, uint16_t* duty_buf);
uint16_t* DMA_TIM_GetCCRAddress(TIM_Instance timer, PWM_Ch pwm_ch);

// --- ADC (existing) ---
void DMA_SetAnalogReadChannel(ADC_Channel channel);  // set ADC ch for DMA-triggered conversion
```

### 4.11 SimpleFlash

```c
// Storage: last 2 pages (254-255), 64 bytes each
// Address: 0x0800_3F80 - 0x0800_3FFF

FlashStatus Flash_Init(void);
FlashStatus Flash_WriteByte(uint32_t addr, uint8_t data);
FlashStatus Flash_ReadByte(uint32_t addr, uint8_t* data);
FlashStatus Flash_ErasePage(uint8_t page);
FlashStatus Flash_EraseAll(void);
FlashStatus Flash_SaveConfig(void* data, uint16_t size);   // with CRC16
FlashStatus Flash_LoadConfig(void* data, uint16_t size);   // with CRC16 validation
```

### 4.12 Simple1Wire

```c
OneWire_Bus* OneWire_Init(uint8_t pin);      // max 4 buses
bool OneWire_Reset(OneWire_Bus* bus);
void OneWire_WriteByte(OneWire_Bus* bus, uint8_t data);
uint8_t OneWire_ReadByte(OneWire_Bus* bus);
void OneWire_SkipROM(OneWire_Bus* bus);
void OneWire_MatchROM(OneWire_Bus* bus, uint8_t rom[8]);
void OneWire_ReadROM(OneWire_Bus* bus, uint8_t rom[8]);
uint8_t OneWire_Search(OneWire_Bus* bus, uint8_t* rom);
uint8_t OneWire_CRC8(uint8_t* data, uint8_t len);
```

### 4.13 SimplePWR (Power Management)

```c
void PWR_Sleep(void);                              // Sleep mode (CPU off, peripherals on)
void PWR_Standby(uint8_t entry);                   // Standby mode (deep sleep)
void PWR_AWU_Init(uint32_t prescaler, uint8_t window);  // Auto wake-up
void PWR_PVD_Init(uint32_t level);                 // Power voltage detector
uint8_t PWR_GetWakeUpSource(void);
```

### 4.14 SimpleIWDG / SimpleWWDG (Watchdogs)

```c
void IWDG_SimpleInit(uint32_t timeout_ms);    // Independent watchdog
void IWDG_Feed(void);                          // Reset watchdog counter
void WWDG_SimpleInit(uint8_t timeout_ms);      // Window watchdog
```

---

## 5. Application Entry Point (main.c)

```c
#include <main.h>
#include "debug.h"

#define ENABLE_PRINTF  0   // 1 = enable, 0 = disable (saves flash)

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();  // sets SystemCoreClock = 48MHz (default)

    // printf is auto-disabled via macro if ENABLE_PRINTF=0
    // Timer_Init is auto-called via constructor (SimpleDelay)

    Delay_Ms(100);
    printf("SystemClk:%d\r\n", SystemCoreClock);

    // --- Application init here ---

    while (1) {
        // --- Main loop ---
    }
}
```

**Key points:**
- `SystemCoreClockUpdate()` required for all peripheral timing
- `Timer_Init()` is auto-called (SimpleDelay constructor)
- `printf()` disabled in production via `ENABLE_PRINTF 0` (saves ~2KB flash)
- `SDI_Printf_Enable()` for debug output via SDI (1-wire debug interface)

---

## 6. System Initialization Flow

```
Power On / Reset
  → _start (startup_ch32v00x.S)
    → handle_reset
      → SystemInit()              // HSI on, PLL, clock tree
        → SetSysClock()           // 48MHz HSI by default
      → __libc_init_array()       // C runtime init
        → SimpleDelay_AutoInit()  // constructor: SysTick @ 1ms
      → main()
        → NVIC_PriorityGroupConfig()
        → SystemCoreClockUpdate()
        → ... user code ...
```

---

## 7. Lib Driver Pattern (Template)

### 7.1 Header File Template

```c
/**
 * @file DeviceName.h
 * @brief Description
 * @version 1.0
 * @date YYYY-MM-DD
 */
#ifndef __DEVICE_NAME_H
#define __DEVICE_NAME_H
#ifdef __cplusplus
extern "C" {
#endif

#include "../../SimpleHAL/SimpleHAL.h"
#include <stdint.h>
#include <stdbool.h>

/* ========== Configuration ========== */
#define DEVICE_MAX_INSTANCES    4
#define DEVICE_CONFIG_VALUE     100

/* ========== Type Definitions ========== */
typedef enum { ... } Device_Status;

typedef struct {
    uint8_t     config_field;
    float       last_value;
    uint8_t     initialized;    // ★ REQUIRED
} Device_Instance;

/* ========== Function Prototypes ========== */
void Device_Init(Device_Instance* dev, ...);
uint8_t Device_Read(Device_Instance* dev);
// ...

#ifdef __cplusplus
}
#endif
#endif /* __DEVICE_NAME_H */
```

### 7.2 Source File Template

```c
#include "DeviceName.h"

static void _internal_helper(Device_Instance* dev) {
    // static = not visible outside this file
}

void Device_Init(Device_Instance* dev, uint8_t pin) {
    if (dev == NULL) return;
    memset(dev, 0, sizeof(Device_Instance));
    dev->config_field = pin;
    pinMode(pin, PIN_MODE_OUTPUT);
    dev->initialized = 1;
}

uint8_t Device_Read(Device_Instance* dev) {
    if (dev == NULL || !dev->initialized) return 0;
    // ... read logic ...
    return value;
}
```

---

## 8. Common Pitfalls & Error Patterns

### 8.1 I2C_ReadReg Signature

**WRONG:**
```c
I2C_Status status = I2C_ReadReg(addr, reg, &val);  // ❌ too many args
```

**CORRECT:**
```c
uint8_t val = I2C_ReadReg(addr, reg);              // ✅ returns value directly
// OR for status:
I2C_Status s = I2C_ReadRegMulti(addr, reg, &val, 1);
```

### 8.2 GPIO Pin Mode (NO Arduino-style macros)

**WRONG:**
```c
pinMode(pin, OUTPUT);         // ❌ undefined
pinMode(pin, INPUT_PULLUP);   // ❌ undefined
```

**CORRECT:**
```c
pinMode(pin, PIN_MODE_OUTPUT);
pinMode(pin, PIN_MODE_INPUT_PULLUP);
```

### 8.3 USART API Names

```c
USART_SendByte(data);   // ❌ OLD name
USART_WriteByte(data);  // ✅ NEW name

USART_ReadByte();       // ❌ OLD name
USART_Read();           // ✅ NEW name

USART_SimpleInit(baud);               // ❌ missing pin_config
USART_SimpleInit(baud, USART_PINS_DEFAULT); // ✅
```

### 8.4 Name Conflicts with SimpleHAL

If a Lib function name conflicts with SimpleHAL (e.g., `I2C_Scan`), rename the Lib function using a library-specific prefix:
```c
// In I2CScan lib:
void I2CScan_Run(void);  // instead of I2C_Scan
```

### 8.5 Static Functions that are Never Called

Remove dead static functions — GCC will warn with `-Wunused-function`. Do NOT suppress with `__attribute__((unused))`.

### 8.6 The `__disable_irq()` / `__enable_irq()` Pattern

Always use for critical ISR-protected sections:
```c
__disable_irq();
// read/write shared volatile data
__enable_irq();
```

### 8.7 TIM2 Clock Enable (CRITICAL)

TIM2 อยู่บน **APB1** bus, **ไม่ใช่** APB2. ห้ามใช้ `RCC_APB2PeriphClockCmd()` สำหรับ TIM2:
```c
// ❌ ผิด:
RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM2, ENABLE);

// ✅ ถูก:
RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
```

### 8.8 `volatile` for ISR-Shared Data (CRITICAL)

ตัวแปร callback pointers ที่ shared ระหว่าง main code และ ISR **ต้อง** ประกาศเป็น `volatile`:
```c
static void (*volatile dma_callbacks[DMA_CHANNEL_COUNT])(void);
```
ถ้าไม่ใส่ `volatile` compiler อาจ optimize อ่านจาก register แทน memory ทำให้ callback ล่าสุดไม่ถูกเรียก

### 8.9 NVIC Priority Initialization (WARNING)

ต้องเรียก `NVIC_SetPriority()` สำหรับทุก IRQ ที่เปิดใช้งาน ก่อนเข้า main loop:
```c
NVIC_SetPriority(DMA1_Channel1_IRQn, 1);   // DMA: priority 1
NVIC_SetPriority(SysTick_IRQn, 3);          // SysTick: lowest priority
```

### 8.10 Callback Setter — Interrupt Tearing (WARNING)

Setter functions ที่เขียน shared volatile data ต้องป้องกันด้วย `__disable_irq()`/`__enable_irq()`:
```c
void DMA_SetCallback(DMA_Channel channel, void (*callback)(void)) {
    __disable_irq();
    dma_callbacks[channel] = callback;
    __enable_irq();
}
```

### 8.11 DMA_WaitComplete — Timeout (WARNING)

`DMA_WaitComplete()` ควรมี timeout เพื่อป้องกัน infinite loop ถ้า DMA ไม่ complete:
```c
void DMA_WaitComplete(DMA_Channel channel, uint32_t timeout_ms) {
    uint32_t start = Get_CurrentMs();
    while (DMA_GetFlagStatus(channel, DMA_FLAG_TC) == RESET) {
        if (Get_CurrentMs() - start >= timeout_ms) break;
    }
    DMA_ClearFlag(channel, DMA_FLAG_TC);
}
```

---

## 9. Interrupt Vector Table

| IRQn | Handler | Peripheral |
|------|---------|-----------|
| 16 | WWDG_IRQHandler | Window Watchdog |
| 17 | PVD_IRQHandler | Power Voltage Detector |
| 18 | FLASH_IRQHandler | Flash |
| 19 | RCC_IRQHandler | Clock |
| 20 | EXTI7_0_IRQHandler | External Interrupt 7..0 |
| 21 | AWU_IRQHandler | Auto Wake-Up |
| 22-28 | DMA1_Channel1..7_IRQHandler | DMA Channels |
| 29 | ADC1_IRQHandler | ADC |
| 30 | I2C1_EV_IRQHandler | I2C1 Event |
| 31 | I2C1_ER_IRQHandler | I2C1 Error |
| 32 | USART1_IRQHandler | USART1 |
| 33 | SPI1_IRQHandler | SPI1 |
| 34 | TIM1_BRK_IRQHandler | TIM1 Break |
| 35 | TIM1_UP_IRQHandler | TIM1 Update |
| 36 | TIM1_TRG_COM_IRQHandler | TIM1 Trigger/Commutation |
| 37 | TIM1_CC_IRQHandler | TIM1 Capture Compare |
| 38 | TIM2_IRQHandler | TIM2 |

All handlers are declared `.weak` in startup — override by defining the function with `__attribute__((interrupt("WCH-Interrupt-fast")))`.

---

## 10. Available Libraries (User/Lib/)

| Library | Description | Interface |
|---------|------------|-----------|
| **ADS1115** | 16-bit ADC (I2C) | I2C |
| **AT24Cxx** | EEPROM (I2C) | I2C |
| **BH1750** | Light Sensor | I2C |
| **BMP280** | Pressure/Temperature | I2C |
| **Button** | Push Button with debounce | GPIO |
| **Buzzer** | Buzzer control | GPIO |
| **DHT** | DHT11/DHT22 Temp/Humidity | GPIO (1-Wire) |
| **DRV8825** | Stepper Driver | GPIO |
| **DS18B20** | 1-Wire Temp Sensor | 1-Wire |
| **DS3231** | RTC Module | I2C |
| **ESC** | ESC Motor Control | PWM |
| **ESP01** | ESP8266 WiFi Module | USART |
| **FlameSensor_KY026** | Flame Sensor | GPIO/ADC |
| **GPS_NEO6M** | GPS Module | USART |
| **HC05** | Bluetooth Module | USART |
| **HCSR04** | Ultrasonic Distance | GPIO |
| **HX711** | Load Cell ADC | GPIO |
| **I2CScan** | I2C Bus Scanner | I2C |
| **INA219** | Current/Power Monitor | I2C |
| **IR** | IR Remote Receiver | GPIO |
| **KeyMatrix** | Keypad Matrix | GPIO |
| **L298N** | Motor Driver | GPIO |
| **LCD1602_I2C** | 16x2 LCD (I2C) | I2C |
| **LCDMenu** | Menu System for LCD1602/2004 (4-btn) | I2C + GPIO |
| **MAX7219** | LED Matrix 8x8 | SPI |
| **MCP4725** | 12-bit DAC | I2C |
| **MPU6050** | IMU Gyro/Accel | I2C |
| **MQGas** | MQ Gas Sensors | ADC |
| **NeoPixel** | WS2812 LED Strip | GPIO (bit-bang) |
| **nRF24L01** | 2.4GHz Radio | SPI |
| **NTC10K** | Thermistor | ADC |
| **OH49E** | Hall Effect Sensor | GPIO |
| **OLED** | SSD1306 OLED 128x64 | I2C |
| **P10** | P10 LED Matrix Display | GPIO (timer scan) |
| **PCA9685** | 16-ch PWM Driver | I2C |
| **PCF8574** | I/O Expander | I2C |
| **PIR** | Motion Sensor | GPIO |
| **PMS5003** | PM2.5 Sensor | USART |
| **PZEM004T** | AC Energy Meter | USART |
| **PZEM004Tv3** | AC Energy Meter v3 | USART |
| **RainSensor_YL83** | Rain Sensor | GPIO/ADC |
| **RC522** | RFID Reader | SPI |
| **RCWL0516** | Microwave Radar | GPIO |
| **Relay** | Relay Control | GPIO |
| **RotaryEncoder** | Rotary Encoder | GPIO |
| **Servo** | RC Servo Motor | PWM |
| **ServoCluster** | Multi-Servo Manager | PWM |
| **ServoTester** | Servo Test Utility | PWM |
| **ShiftReg595** | 74HC595 Shift Register | GPIO |
| **SHT3x** | Temp/Humidity Sensor | I2C |
| **SoilMoisture_YL69** | Soil Moisture | ADC |
| **SoundSensor_KY038** | Sound Sensor | ADC |
| **StepperMotor** | Stepper Motor | GPIO |
| **TJC** | TJC HMI Display | USART |
| **TM1637** | 4-digit 7-seg Display | GPIO |
| **TM1650** | 4-digit 7-seg Display (I2C) | I2C |
| **TMC220x** | Stepper Driver (UART) | USART |
| **TMC5160** | Stepper Driver (SPI) | SPI |
| **VL53L0X** | ToF Distance Sensor | I2C |
| **W25Qxx** | SPI Flash Memory | SPI |
| **WaterFlow_YFS201** | Water Flow Sensor | GPIO |
| **WS2812Matrix** | WS2812 8x8 LED Matrix | GPIO |
| **WS2815Matrix** | WS2815 LED Matrix | GPIO |

---

## 11. Printf Control

```c
// In main.c:
#define ENABLE_PRINTF  1   // Development - printf enabled
#define ENABLE_PRINTF  0   // Production - printf compiled out (saves flash)

#if ENABLE_PRINTF
    SDI_Printf_Enable();   // Enable SDI printf (debug interface)
#endif

// Normal UART printf (from debug.c):
USART_Printf_Init(115200);  // Init UART for printf
```

---

## 12. Quick Start for Adding a New Lib

1. Create folder: `User/Lib/<Name>/`
2. Create files: `<Name>.h`, `<Name>.c`, `README.md`
3. In `.h`: header guard, struct with `initialized`, function prototypes
4. In `.c`: `#include "<Name>.h"`, static helpers, public functions
5. From `.c` use API from: `#include "../../SimpleHAL/SimpleHAL.h"`
6. All public functions: check `NULL` + `initialized`
7. Build test: run `build.bat`
8. `build.bat` auto-discovers all `*.c` under `User/Lib/` — no need to register

---

## 13. Version History

| Date | Note |
|------|------|
| 2026-05-25 | DMA examples doc + full code audit: fixes for TIM2 clock, volatile, IRQ safety, NVIC priority, DMA timeout |
| 2026-05-25 | AGENT.MD created: quick reference for AI/Copilot |
| 2026-05-04 | LCDMenu v1.0: menu system on LCD with 4-button navigation |
| 2026-05-03 | Initial Knowledge Base created |
| 2025-12-22 | SimpleHAL v1.9: added SimpleDMA |
| 2025-12-21 | SimpleHAL: SimpleTIM_Ext, SimpleFlash added |
| 2025-12-12 | SimpleHAL v1.0: Arduino-style API launched |
