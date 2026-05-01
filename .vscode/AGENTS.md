# CH32V003 — AI Agent Instructions

> **Purpose**: Help AI coding agents understand this CH32V003 embedded project and be immediately productive  
> **Last Updated**: 2026-05-01  
> **Full Guidelines**: See [guidelines.md](./guidelines.md) for complete reference

---

## 🎯 Project Overview

**CH32V003** is a RISC-V RV32EC microcontroller (48MHz, 16KB Flash, 2KB RAM) developed by WCH. This project provides:

- **SimpleHAL**: Arduino-style Hardware Abstraction Layer for easy peripheral access
- **Device Libraries**: 50+ ready-to-use drivers for sensors, displays, and modules
- **Build System**: Make-based compilation with VS Code integration
- **Toolchain**: RISC-V Embedded GCC12 (`riscv-wch-elf-`) from MounRiver Studio 2

---

## ⚡ Quick Start for Agents

### Build & Upload Commands

```bash
# Build (Ctrl+Shift+B in VS Code)
.\build.bat

# Clean build artifacts
.\clean.bat

# Rebuild (clean + build)
.\rebuild.bat

# Flash to device via WCH-Link
.\upload.bat
```

### VS Code Tasks

Use `run_task` tool with these task IDs:
- `shell: Build CH32V003 (MounRiver)` — Default build task
- `shell: Clean CH32V003` — Clean object files
- `shell: Rebuild CH32V003` — Full rebuild
- `shell: Upload CH32V003 (WCH-Link)` — Flash firmware

---

## 📁 Critical File Structure

### ✋ DO NOT MODIFY (Vendor Files)
```
Core/           # RISC-V core implementation
Debug/          # WCH debug utilities (SDI Printf)
Peripheral/     # Standard Peripheral Library (inc/, src/)
Startup/        # Assembly startup code
Ld/             # Linker script (Link.ld)
```

### ✅ WRITE CODE HERE (User Directory)
```
User/
├── main.c / main.h              # Application entry point
├── ch32v00x_conf.h              # Enable/disable peripheral headers
├── ch32v00x_it.c/.h             # Interrupt handlers
├── SimpleHAL/                   # HAL library (GPIO, ADC, PWM, etc.)
└── Lib/                         # Device libraries (50+ drivers)
```

**Rule**: All new code must go in `User/` or subdirectories. Never modify vendor directories.

---

## 🔧 SimpleHAL API Reference

### GPIO Operations

```c
#include "SimpleHAL/SimpleHAL.h"

// Pin numbering (enum GPIO_Pin):
// PA1=0, PA2=1
// PC0=10, PC1=11, ..., PC7=17
// PD2=20, PD3=21, ..., PD7=25

pinMode(PC0, PIN_MODE_OUTPUT);      // Set pin mode
digitalWrite(PC0, HIGH);            // Write digital value
uint8_t val = digitalRead(PC1);     // Read digital value
digitalToggle(PC0);                 // Toggle pin state

attachInterrupt(PC1, callback, FALLING);  // RISING, FALLING, CHANGE
detachInterrupt(PC1);

uint16_t adc = analogRead(PD5);     // 10-bit ADC (0-1023)
analogWrite(PC0, 50);               // PWM duty cycle % (uses SimplePWM)
```

### Timing & Delay

```c
SystemCoreClockUpdate();            // Update clock (call first!)
// Timer_Init() auto-called by SimpleDelay constructor — no need to call manually

Delay_Ms(500);                      // Blocking delay
Delay_Us(100);

Timer_t myTimer = Timer_Create();   // Non-blocking timer
Timer_Start(&myTimer, 1000);        // 1 second timeout
if (Timer_IsExpired(&myTimer)) { ... }
```

### USART (Serial Communication)

```c
USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);  // TX=PD5, RX=PD6
USART_Print("Hello\r\n");
USART_Printf("Value: %d\r\n", 42);
char line[64];
USART_ReadLine(line, sizeof(line));  // Blocking read until \n
```

### I2C Communication

```c
I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);  // SCL=PC2, SDA=PC1
// Or use software I2C on any pins:
I2C_Soft_Init(SCL_PIN, SDA_PIN, I2C_100KHZ);

I2C_WriteByte(device_addr, register_addr, data);
uint8_t data = I2C_ReadByte(device_addr, register_addr);
I2C_WriteBuffer(addr, reg, buffer, length);
```

### SPI Communication

```c
SPI_SimpleInit(SPI_MODE_0, SPI_PINS_DEFAULT);  // MOSI=PD6, MISO=PD5, SCK=PD4
SPI_Transfer(data);                    // Send/receive byte
SPI_TransferBuffer(tx_buf, rx_buf, len);
```

### PWM Output

```c
PWM_Init(PWM1_CH1, 1000);             // 1kHz on PD2
PWM_SetDuty(PWM1_CH1, 50);            // 50% duty cycle
PWM_Start(PWM1_CH1);
PWM_Stop(PWM1_CH1);
```

### Timer Interrupts

```c
TIM_Config(TIM_1, 1000);              // 1kHz interrupt
TIM_AttachInterrupt(TIM_1, myISR);
TIM_Start(TIM_1);
```

---

## 📚 Device Library Pattern

When creating or modifying device libraries in `User/Lib/`:

### Required Structure

```
User/Lib/<LibraryName>/
├── <LibraryName>.h       # Header with API
├── <LibraryName>.c       # Implementation
└── README.md             # Documentation (Thai + English)
```

### Mandatory Conventions

1. **Include SimpleHAL**: Use `"../../SimpleHAL/SimpleHAL.h"`
2. **Header Guard**: `#ifndef __<LIBRARYNAME>_H`
3. **Author Tag**: `@author CH32V003 Library Team` (or specific author if noted)
4. **Initialization Flag**: Every struct MUST have `uint8_t initialized;` field
5. **Null Checks**: All public functions MUST check for NULL pointers and initialization state
6. **Static Internals**: All internal variables/functions MUST be `static`
7. **ISR Safety**: Use `__disable_irq()` / `__enable_irq()` for critical sections

### Example Template

```c
// MySensor.h
#ifndef __MYSENSOR_H
#define __MYSENSOR_H

#include "../../SimpleHAL/SimpleHAL.h"

typedef struct {
    uint8_t initialized;
    uint8_t sda_pin;
    uint8_t scl_pin;
    // ... other fields
} MySensor_t;

bool MySensor_Init(MySensor_t *sensor, uint8_t sda, uint8_t scl);
float MySensor_ReadTemperature(MySensor_t *sensor);
void MySensor_Deinit(MySensor_t *sensor);

#endif
```

```c
// MySensor.c
#include "MySensor.h"

static bool validate_sensor(MySensor_t *sensor) {
    return sensor != NULL && sensor->initialized;
}

bool MySensor_Init(MySensor_t *sensor, uint8_t sda, uint8_t scl) {
    if (sensor == NULL) return false;
    
    sensor->sda_pin = sda;
    sensor->scl_pin = scl;
    sensor->initialized = 1;
    return true;
}
```

---

## 🧠 Memory & Resource Constraints

| Resource | Total | Usage Guideline |
|----------|-------|-----------------|
| **Flash** | 16 KB | Keep code < 12KB to leave room for bootloader |
| **RAM** | 2 KB | Critical! Monitor stack + heap usage |
| **Stack** | ~512B | Avoid deep recursion, large local arrays |
| **Heap** | Minimal | Prefer static allocation over malloc |

### Tips for RAM-Constrained Development

- Use `static` buffers instead of stack allocation for large arrays
- Avoid `printf` with complex format strings (use `USART_Printf` sparingly)
- Prefer `uint8_t` over `int` when possible
- Use bit-fields and packed structs for sensor data
- Check `output/CH32V003.map` after build for memory usage

---

## 🔌 Pin Reference

### GPIO Pins Available

| Port | Pins | Count |
|------|------|-------|
| GPIOA | PA1, PA2 | 2 |
| GPIOC | PC0–PC7 | 8 |
| GPIOD | PD2–PD7 | 6 |
| **Total** | | **16 pins** |

### PWM-Capable Pins

| Pin | Timer Channel | Alias |
|-----|--------------|-------|
| PD2 | TIM1_CH1 | `PWM1_CH1` |
| PA1 | TIM1_CH2 | `PWM1_CH2` |
| PC3 | TIM1_CH3 | `PWM1_CH3` |
| PC4 | TIM1_CH4 | `PWM1_CH4` |
| PD4 | TIM2_CH1 | `PWM2_CH1` |
| PD3 | TIM2_CH2 | `PWM2_CH2` |
| PC0 | TIM2_CH3 | `PWM2_CH3` |
| PD7 | TIM2_CH4 | `PWM2_CH4` |

### ADC Channels

| Channel | Pin | Constant |
|---------|-----|----------|
| 0 | PA2 | `ADC_CH_PA2` |
| 1 | PA1 | `ADC_CH_PA1` |
| 2 | PC4 | `ADC_CH_PC4` |
| 3 | PD2 | `ADC_CH_PD2` |
| 4 | PD3 | `ADC_CH_PD3` |
| 5 | PD5 | `ADC_CH_PD5` |
| 6 | PD6 | `ADC_CH_PD6` |
| 7 | PD4 | `ADC_CH_PD4` |
| 8 | Internal Vref | `ADC_CH_VREFINT` (~1.2V) |
| 9 | Calibration | `ADC_CH_VCALINT` |

### I2C Pin Configurations

| Config | SCL | SDA | Constant |
|--------|-----|-----|----------|
| Default | PC2 | PC1 | `I2C_PINS_DEFAULT` |
| Remap | PD0 | PD1 | `I2C_PINS_REMAP` |

### USART Pin Configurations

| Config | TX | RX | Constant |
|--------|----|----|----------|
| Default | PD5 | PD6 | `USART_PINS_DEFAULT` |
| Remap1 | PD0 | PD1 | `USART_PINS_REMAP1` |
| Remap2 | PD6 | PD5 | `USART_PINS_REMAP2` |

---

## 🛠️ Common Pitfalls & Solutions

### 1. Forgetting `SystemCoreClockUpdate()`

**Problem**: Delays and timers don't work correctly  
**Solution**: Always call `SystemCoreClockUpdate()` at start of `main()`

### 2. Calling `Timer_Init()` Manually

**Problem**: Double initialization causes issues  
**Solution**: `Timer_Init()` is auto-called by SimpleDelay constructor — don't call it yourself

### 3. Using Wrong Pin Numbers

**Problem**: Confusing physical pin numbers with enum values  
**Solution**: Use enum constants (`PC0`, `PD2`, etc.), not raw numbers

### 4. RAM Overflow

**Problem**: Program crashes or behaves unpredictably  
**Solution**: Check `output/CH32V003.map` for RAM usage, reduce static buffers

### 5. Not Checking Initialization

**Problem**: Functions crash when called before init  
**Solution**: Always check `initialized` flag in public API functions

### 6. Modifying Vendor Files

**Problem**: Updates break your changes  
**Solution**: NEVER modify `Core/`, `Peripheral/`, `Startup/`, `Ld/`, `Debug/`

---

## 📖 Documentation Links

For detailed information, refer to these files:

- **[guidelines.md](./guidelines.md)** — Complete coding standards, API reference, library patterns
- **[README.md](../README.md)** — Project overview and quick start guide
- **[COMPILER_SETUP.md](../COMPILER_SETUP.md)** — Toolchain installation guide
- **SimpleHAL Module Docs**: `User/SimpleHAL/readme/*/README.md`
- **Device Library Docs**: `User/Lib/*/README.md`

---

## 🤖 Agent-Specific Tips

### When Writing New Code

1. **Check existing libraries first**: Look in `User/Lib/` before writing new drivers
2. **Follow naming conventions**: Use `PascalCase` for functions, `UPPER_CASE` for macros
3. **Add error handling**: Return `bool` or error codes, never assume success
4. **Document in Thai + English**: README files should have both languages
5. **Test with minimal example**: Provide a complete, compilable example in README

### When Debugging

1. **Check build output**: Look for warnings in `output/CH32V003.map`
2. **Verify pin assignments**: Cross-reference with pin tables above
3. **Monitor RAM usage**: 2KB is very limited — watch for stack overflow
4. **Use printf debugging**: Enable with `#define ENABLE_PRINTF 1` in main.c
5. **Check initialization order**: Ensure peripherals are initialized before use

### When Creating New Libraries

1. **Study existing examples**: Look at `User/Lib/DHT/`, `User/Lib/BMP280/`, etc.
2. **Follow the template**: Use the pattern shown in "Device Library Pattern" section
3. **Write comprehensive README**: Include wiring diagram, API reference, examples
4. **Test thoroughly**: Verify on real hardware before committing
5. **Update guidelines.md**: Add new library to the status table

---

## 📊 Library Status Summary

As of 2026-05-01, **50+ device libraries** are available:

### Phase 1 (Basic Sensors)
✅ DHT, HCSR04, Servo, Button, Buzzer, DS18B20, IR, LCD1602_I2C, MAX7219, NeoPixel, NTC10K, OLED, PIR, RotaryEncoder, TJC, TM1637, WS2815Matrix

### Phase 2 (Advanced Modules)
✅ StepperMotor, ShiftReg595, AT24Cxx, DS3231, HX711

### Phase 3 (Complex Sensors)
✅ MPU6050, BMP280, KeyMatrix, MQGas, nRF24L01

### Phase 4 (Precision Instruments)
✅ BH1750, SHT3x, INA219, MCP4725, ADS1115, PCA9685, W25Qxx, VL53L0X, HC05

### Phase 5 (Industrial/IoT)
✅ DRV8825, ESC, ESP01, FlameSensor_KY026, GPS_NEO6M, L298N, OH49E, PCF8574, PMS5003, PZEM004T, PZEM004Tv3, RainSensor_YL83, RC522, RCWL0516, Relay, ServoCluster, ServoTester, SoilMoisture_YL69, SoundSensor_KY038, TMC220x, TMC5160, WaterFlow_YFS201

See [guidelines.md §15](./guidelines.md#15-lib-ที่มีอยู่แล้วและสถานะ) for complete details.

---

## 🔄 Version History

- **SimpleHAL v1.9.0** — Current version
- **Toolchain**: GCC12 riscv-wch-elf (from MounRiver Studio 2)
- **Target**: CH32V003 (also compatible with CH32V006)

---

*This file is auto-loaded by GitHub Copilot and other AI agents. Keep it concise and actionable.*
