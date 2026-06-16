# CH32V003 Project — Agent Reference

> Quick reference สำหรับ AI/Copilot ใช้พัฒนาโปรเจกต์นี้
> **Updated:** 2026-06-16

---

## 1. Hardware Overview

| Property | Value |
|----------|-------|
| MCU | CH32V003 (RISC-V V2, rv32ecxw/ilp32e) |
| Clock | 48MHz (HSI+PLL) |
| Flash | 16KB (256 pages × 64 bytes) |
| RAM | 2KB |
| GPIO | PA1-PA2, PC0-PC7, PD2-PD7 (18 pins) |
| ADC | 10-bit, 8 external + 2 internal ch |
| Timers | TIM1 (advanced), TIM2 (general) |
| USART | 1× USART1 (3 pin configs) |
| I2C | 1× I2C1 (2 pin configs) + SW I2C |
| SPI | 1× SPI1 (2 pin configs) |
| DMA | 7 channels |
| Interrupts | PFIC/NVIC, 23 IRQ lines |

## 2. Directory Structure

```
CH32V003/
├── AGENT.MD                   # THIS FILE — AI quick reference
├── scripts/                   # build.bat, clean.bat, upload.bat, rebuild.bat
├── Ld/Link.ld                 # Flash 16K, RAM 2K, stack 256
├── Startup/startup_ch32v00x.S # Vector table + reset handler
├── Core/                      # RISC-V core (PFIC, SysTick, CSR)
├── Debug/                     # UART/SDI printf
├── Peripheral/inc/ + src/     # WCH Standard Peripheral Library (read-only)
├── User/
│   ├── main.c/h               # Entry point
│   ├── system_ch32v00x.c/h    # System clock init
│   ├── ch32v00x_conf.h        # Peripheral includes + utility macros
│   ├── ch32v00x_it.c/h        # Default interrupt handlers
│   ├── SimpleHAL/             # ★ CORE API (Arduino-style HAL)
│   └── Lib/                   # ~60 device driver libraries
```

## 3. Build Commands

| Command | Description |
|---------|-------------|
| `build.bat` | Compile + Link → `output/CH32V003.hex` |
| `clean.bat` | Remove `obj/` and `output/` |
| `rebuild.bat` | Clean + Build |
| `upload.bat` | Flash via WCH-Link (OpenOCD) |

**VS Code Tasks:** `Ctrl+Shift+B` (Build), Terminal → Run Task (Clean/Rebuild/Upload)

**Toolchain:** `riscv-wch-elf-gcc` (GCC 12.2.0) from MounRiver Studio 2

## 4. SimpleHAL API (Quick Reference)

### GPIO — Pin Numbers

| Pin | Value | Alt Functions |
|-----|-------|--------------|
| PA1 | 0 | PWM1_CH2, ADC_CH1 |
| PA2 | 1 | ADC_CH0 |
| PC0-PC7 | 10-17 | Various PWM/ADC/I2C/SPI |
| PD2-PD7 | 20-25 | USART, PWM, ADC |

### Pin Mode Enum

| Mode | Description |
|------|-------------|
| `PIN_MODE_INPUT` | Floating input |
| `PIN_MODE_OUTPUT` | Push-pull output |
| `PIN_MODE_INPUT_PULLUP` | Input with pull-up |
| `PIN_MODE_INPUT_PULLDOWN` | Input with pull-down |
| `PIN_MODE_OUTPUT_OD` | Open-drain output |

### Key API Signatures

```c
// GPIO
void pinMode(uint8_t pin, GPIO_PinMode mode);
void digitalWrite(uint8_t pin, uint8_t value);
uint8_t digitalRead(uint8_t pin);
void digitalToggle(uint8_t pin);
void attachInterrupt(uint8_t pin, GPIO_InterruptMode mode, void (*cb)(void));
void detachInterrupt(uint8_t pin);
uint16_t analogRead(uint8_t pin);    // PD2-PD7 → 0-1023
void analogWrite(uint8_t pin, uint16_t value);

// Delay (auto-initialized via __attribute__((constructor)))
void Delay_Ms(uint32_t n);
void Delay_Us(uint32_t n);
uint32_t Get_CurrentMs(void);
uint32_t Get_CurrentUs(void);
void Start_Timer(Timer_t* t, uint32_t ms, uint8_t repeat);
uint8_t Is_Timer_Expired(Timer_t* t);

// USART — USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT)
void USART_SimpleInit(USART_BaudRate baud, USART_PinConfig cfg);
void USART_Print(const char* str);
void USART_WriteByte(uint8_t data);
uint8_t USART_Read(void);           // blocking
uint8_t USART_Available(void);

// I2C — I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT)
// ⚠️ I2C_ReadReg(addr, reg) returns value directly (NOT status)!
I2C_Status I2C_Write(uint8_t addr, uint8_t* data, uint16_t len);
I2C_Status I2C_Read(uint8_t addr, uint8_t* data, uint16_t len);
uint8_t I2C_ReadReg(uint8_t addr, uint8_t reg);         // ★ returns value
I2C_Status I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t data);
I2C_Status I2C_ReadRegMulti(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t len);

// SPI — SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT)
uint8_t SPI_Transfer(uint8_t data);
void SPI_TransferBuffer(uint8_t* tx, uint8_t* rx, uint16_t len);
void SPI_SetCS(uint8_t state);

// ADC — ADC_SimpleInit(), returns 0-1023
uint16_t ADC_Read(ADC_Channel ch);
float ADC_ToVoltage(uint16_t adc_value, float vref);

// TIM — TIM_SimpleInit(TIM_1, 1000)
void TIM_SimpleInit(TIM_Instance tim, uint32_t freq_hz);
void TIM_AttachInterrupt(TIM_Instance tim, void (*cb)(void));

// PWM — PWM_Init(PWM1_CH1, 50), SetDutyCycle(50)
void PWM_Init(PWM_Channel ch, uint32_t freq_hz);
void PWM_SetDutyCycle(PWM_Channel ch, uint8_t percent);
void PWM_Start(PWM_Channel ch);

// DMA — DMA_MemCopy(dst, src, size);
void DMA_MemCopy(void* dst, void* src, uint32_t size);

// Flash — Flash_SaveConfig(&data, sizeof(data))
FlashStatus Flash_SaveConfig(void* data, uint16_t size);
FlashStatus Flash_LoadConfig(void* data, uint16_t size);
```

### Arduino Compatible API (`SimpleArduino.h`)

| Function | Maps To | Notes |
|----------|---------|-------|
| `millis()` | `Get_CurrentMs()` | macro |
| `micros()` | `Get_CurrentUs()` | macro |
| `delay(ms)` | `Delay_Ms(ms)` | macro |
| `delayMicroseconds(us)` | `Delay_Us(us)` | macro |
| `interrupts()` | `__enable_irq()` | macro |
| `noInterrupts()` | `__disable_irq()` | macro |
| `digitalPinToInterrupt(pin)` | — | map pin→EXTI line |
| `randomSeed(seed)` | — | LCG PRNG seed |
| `_randomMax(max)` | — | `[0, max-1]`, ⚠️ ใช้ `_randomMax` แทน `random` เพราะ stdlib มี `long random(void)` |
| `_randomRange(min, max)` | — | `[min, max-1]` |
| `yield()` | `IWDG_Feed()` | cooperative multitasking |
| `dtostrf(val, width, precision, buf)` | — | lightweight, no sprintf |

**Optional (define before include):**
```c
#define ENABLE_USART_PRINTLN  1   // USART_Println, PrintlnNum, PrintlnHex
#define ENABLE_USART_PRINTFLOAT 1 // USART_PrintFloat, PrintlnFloat (requires ENABLE_USART_PRINTLN)
```

### Pin Configurations

| Peripheral | Config 1 | Config 2 | Config 3 |
|-----------|----------|----------|----------|
| **USART** | TX=PD5, RX=PD6 (DEFAULT) | TX=PD0, RX=PD1 (REMAP1) | TX=PD6, RX=PD5 (REMAP2) |
| **I2C** | SCL=PC2, SDA=PC1 (DEFAULT) | SCL=PD0, SDA=PD1 (REMAP) | — |
| **SPI** | SCK=PC5, MISO=PC7, MOSI=PC6, NSS=PC4 (DEFAULT) | SCK=PC6, MISO=PC8, MOSI=PC7, NSS=PC5 (REMAP) | — |

### PWM Channel → Pin Mapping

| Channel | Pin | Timer |
|---------|-----|-------|
| PWM1_CH1 | PD2 | TIM1 |
| PWM1_CH2 | PA1 | TIM1 |
| PWM1_CH3 | PC3 | TIM1 |
| PWM1_CH4 | PC4 | TIM1 |
| PWM2_CH1 | PD4 | TIM2 |
| PWM2_CH2 | PD3 | TIM2 |
| PWM2_CH3 | PC0 | TIM2 |
| PWM2_CH4 | PD7 | TIM2 |

### ADC Channel → Pin Mapping

| Channel 0 | Channel 1 | Channel 2 | Channel 3 | Channel 4 | Channel 5 | Channel 6 | Channel 7 | Ch 8 | Ch 9 |
|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|------|------|
| PA2 | PA1 | PC4 | PD2 | PD3 | PD5 | PD6 | PD4 | Vrefint | Vcalint |

## 5. Coding Patterns (MANDATORY)

### Library Struct Pattern
```c
typedef struct {
    uint8_t pin;
    uint8_t initialized;    // ★ REQUIRED — last field in Init()
    // ... state fields ...
} Device_Instance;
```

### Init Function Pattern
```c
void Device_Init(Device_Instance* dev, ...) {
    if (dev == NULL) return;
    memset(dev, 0, sizeof(Device_Instance));
    // ... set config fields ...
    dev->initialized = 1;       // LAST
}
```

### Public Function Pattern
```c
uint8_t Device_Read(Device_Instance* dev) {
    if (dev == NULL || !dev->initialized) return 0;
    // ... implementation ...
}
```

### Library File Structure
```
User/Lib/<Name>/
├── <Name>.h       # Header guard, struct, function prototypes
├── <Name>.c       # #include "<Name>.h", static helpers, implementations
└── README.md      # Thai documentation
```

### Naming Conventions

| Item | Convention | Example |
|------|-----------|---------|
| Enums | `UPPER_SNAKE` | `SPI_MODE0` |
| Public funcs | `PascalCase` | `DHT_Read()` |
| Static funcs | `_lowerCase` | `_calc_crc()` |
| Macros | `UPPER_SNAKE` | `SERVO_PWM_FREQ` |
| Struct fields | `snake_case` | `last_value` |

### ISR / Critical Sections
```c
__disable_irq();
// read/write shared volatile data
__enable_irq();

// ISR declaration:
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
```

## 6. Common Pitfalls

| # | Pitfall | Solution |
|---|---------|----------|
| 1 | `I2C_ReadReg(addr, reg, &val)` — wrong arg count | Use `uint8_t val = I2C_ReadReg(addr, reg);` |
| 2 | `pinMode(pin, OUTPUT)` — Arduino macro undefined | Use `pinMode(pin, PIN_MODE_OUTPUT);` |
| 3 | `USART_SimpleInit(baud)` — missing pin_config | Use `USART_SimpleInit(baud, USART_PINS_DEFAULT);` |
| 4 | `USART_SendByte()` / `USART_ReadByte()` | Use `USART_WriteByte()` / `USART_Read()` |
| 5 | Lib function name conflicts with SimpleHAL | Add library-specific prefix (e.g., `I2CScan_Run()`) |
| 6 | Dead static functions causing warnings | Remove them — don't use `__attribute__((unused))` |
| 7 | Struct variable on stack | Must be global or static (stack only 2KB) |
| 8 | Missing `initialized` flag in struct | Always add `uint8_t initialized;` field |
| 9 | Timer conflict (SimpleTIM + SimplePWM same TIM) | Choose one per timer instance |
| 10 | `Timer_Init()` called manually | Not needed — auto-initialized via constructor |

## 7. Startup Sequence

```
Power On / Reset
  → _start (startup_ch32v00x.S)
    → handle_reset
      → SystemInit() (HSI PLL → 48MHz)
      → __libc_init_array()
        → SimpleDelay_AutoInit()  ← constructor: SysTick @ 1ms
      → main()
        → NVIC_PriorityGroupConfig()
        → SystemCoreClockUpdate()
        → ... user code ...
```

## 8. Init Order (in main())

```c
int main(void) {
    // 1. SystemCoreClockUpdate();     // required for all timing
    // 2. Timer_Init auto-called — do NOT call manually
    // 3. Init peripherals as needed
    // 4. Init device libraries
    // ...

    while (1) {
        // main loop
    }
}
```

## 9. Printf Control

```c
#define ENABLE_PRINTF  1   // Development — printf enabled
#define ENABLE_PRINTF  0   // Production — saves ~2KB flash

#if ENABLE_PRINTF
    SDI_Printf_Enable();   // Enable SDI printf (debug interface)
#endif
```

## 10. Memory Budget (Typical)

| Resource | Used | Free |
|----------|------|------|
| Flash | ~1.5KB–5KB | ~11KB–14.5KB |
| RAM | ~200–500 bytes | ~1.5KB–1.8KB |

Max Flash: 16,384 bytes | Max RAM: 2,048 bytes

## 11. Interrupt Vector Table (Key Entries)

| IRQn | Handler | Peripheral |
|------|---------|-----------|
| 16 | WWDG_IRQHandler | Window Watchdog |
| 17 | PVD_IRQHandler | Power Voltage Detector |
| 18 | FLASH_IRQHandler | Flash |
| 20 | EXTI7_0_IRQHandler | External Interrupt 7..0 |
| 21 | AWU_IRQHandler | Auto Wake-Up |
| 22-28 | DMA1_Channel1..7_IRQHandler | DMA Channels |
| 29 | ADC1_IRQHandler | ADC |
| 30 | I2C1_EV_IRQHandler | I2C1 Event |
| 32 | USART1_IRQHandler | USART1 |
| 33 | SPI1_IRQHandler | SPI1 |
| 34-37 | TIM1_BRK/UP/TRG_COM/CC_IRQHandler | TIM1 |
| 38 | TIM2_IRQHandler | TIM2 |

All handlers `.weak` in `startup_ch32v00x.S` — override by defining with `__attribute__((interrupt("WCH-Interrupt-fast")))`.

## 12. Timer Resource Conflicts

| Resource | Used By | Note |
|----------|---------|------|
| SysTick | SimpleDelay | Auto-init via constructor — do NOT touch |
| TIM1 | SimplePWM (ch1-4) **or** SimpleTIM | Choose ONE |
| TIM2 | SimplePWM (ch1-4) **or** SimpleTIM **or** SimpleTIM_Ext | Choose ONE |
| IWDG | SimpleIWDG | LSI clock |
| WWDG | SimpleWWDG | PCLK clock |
