# CH32V003 — VS Code + GitHub Copilot Project Template

> **by [MAKER WITAWAT](https://www.makerwitawat.com)**

โปรเจกต์เทมเพลตสำหรับพัฒนา **CH32V003** (RISC-V 32-bit MCU) ด้วย **VS Code** และ **GitHub Copilot**  
พร้อม **SimpleHAL** — ไลบรารี Hardware Abstraction Layer สไตล์ Arduino และคอลเลกชัน Library สำหรับ Peripheral ยอดนิยม

---

## 🎯 จุดเด่น

- ✅ Build / Clean / Rebuild / Upload ได้จาก VS Code Tasks (`Ctrl+Shift+B`)
- ✅ **SimpleHAL** ใช้งานง่ายเหมือน Arduino (`pinMode`, `digitalWrite`, `Delay_Ms`)
- ✅ มี Library สำเร็จรูปสำหรับ Sensor และ Display ยอดนิยม
- ✅ รองรับ **GitHub Copilot** ช่วย Generate Code สำหรับ CH32V003 โดยเฉพาะ
- ✅ Toolchain: **RISC-V Embedded GCC12** จาก MounRiver Studio 2

---

## 📦 Requirements

| รายการ | รายละเอียด |
|--------|-----------|
| **IDE** | [VS Code](https://code.visualstudio.com/) |
| **Toolchain** | [MounRiver Studio 2](http://www.mounriver.com/) (ใช้เฉพาะ Toolchain + OpenOCD) |
| **Programmer** | WCH-Link (USB Debugger/Programmer) |
| **MCU** | CH32V003F4P6 / F4U6 / J4M6 หรือ variant อื่น ๆ |

> ดูวิธีติดตั้ง Toolchain ทั้งหมดได้ที่ [COMPILER_SETUP.md](COMPILER_SETUP.md)

---

## 🚀 Quick Start

### 1. Clone โปรเจกต์

```bash
git clone https://github.com/Witawat/ch32v003_vscode_github_copilot.git
cd ch32v003_vscode_github_copilot
```

### 2. เปิดใน VS Code

```bash
code .
```

### 3. Build

กด `Ctrl+Shift+B` หรือไปที่ **Terminal → Run Build Task**

### 4. Upload Firmware

**Terminal → Run Task → Upload CH32V003 (WCH-Link)**  
(ต่อ WCH-Link เข้า CH32V003 ก่อน)

---

## 🗂️ โครงสร้างโปรเจกต์

```
CH32V003/
├── Core/                   # RISC-V Core (core_riscv.c/h)
├── Debug/                  # Debug utility (SDI Printf)
├── Ld/                     # Linker script (Link.ld)
├── Peripheral/             # CH32V003 Standard Peripheral Library
│   ├── inc/                # Header files (GPIO, ADC, TIM, USART, I2C, SPI ...)
│   └── src/                # Source files
├── Startup/                # startup_ch32v00x.S
├── User/
│   ├── main.c / main.h     # ⭐ จุดเริ่มต้นของโปรแกรม
│   ├── SimpleHAL/          # ⭐ SimpleHAL Library
│   └── Lib/                # ⭐ Device Libraries
├── build.bat               # Build script
├── clean.bat               # Clean script
├── rebuild.bat             # Clean + Build
├── upload.bat              # Flash ผ่าน WCH-Link
└── .vscode/tasks.json      # VS Code Tasks
```

---

## 🔧 SimpleHAL Library

**SimpleHAL** คือ Hardware Abstraction Layer ที่ออกแบบมาให้ใช้ง่ายแบบ Arduino บน CH32V003

```c
#include "SimpleHAL/SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    // GPIO
    pinMode(PC0, PIN_MODE_OUTPUT);

    // USART
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    USART_Print("Hello, CH32V003!\r\n");

    // I2C
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);

    while (1) {
        digitalToggle(PC0);
        Delay_Ms(500);
    }
}
```

### Peripherals ที่รองรับ

| Module | Header | คำอธิบาย |
|--------|--------|----------|
| **GPIO** | `SimpleGPIO.h` | Digital I/O, External Interrupts |
| **ADC** | `SimpleADC.h` | Analog อ่านค่า, Battery Monitor, Internal Vref |
| **PWM** | `SimplePWM.h` | PWM Output (8 channels) |
| **Timer** | `SimpleTIM.h` | Timer Interrupts |
| **Timer Ext** | `SimpleTIM_Ext.h` | Stopwatch, Countdown |
| **USART** | `SimpleUSART.h` | Serial Communication |
| **I2C (HW)** | `SimpleI2C.h` | I2C Hardware |
| **I2C (SW)** | `SimpleI2C_Soft.h` | I2C Software (any pin) |
| **SPI** | `SimpleSPI.h` | SPI Communication |
| **1-Wire** | `Simple1Wire.h` | 1-Wire Protocol (DS18B20) |
| **DMA** | `SimpleDMA.h` | DMA Transfer |
| **Flash** | `SimpleFlash.h` | Flash Memory Storage |
| **OPAMP** | `SimpleOPAMP.h` | Operational Amplifier |
| **PWR** | `SimplePWR.h` | Power Management |
| **IWDG** | `SimpleIWDG.h` | Independent Watchdog |
| **WWDG** | `SimpleWWDG.h` | Window Watchdog |
| **Delay** | `SimpleDelay.h` | Delay Functions |

---

## 📚 Device Libraries (`User/Lib/`)

| Library | คำอธิบาย | Interface |
|---------|----------|-----------|
| **ADS1115** | 16-bit ADC | I2C |
| **AT24Cxx** | EEPROM | I2C |
| **BH1750** | Light Sensor | I2C |
| **BMP280** | Pressure/Temperature | I2C |
| **Button** | Push Button with debounce | GPIO |
| **Buzzer** | Passive Buzzer พร้อม Melody, Tone, Beep | GPIO/PWM |
| **DHT** | DHT11/DHT22 Temp/Humidity | GPIO |
| **DRV8825** | Stepper Driver | GPIO |
| **DS18B20** | 1-Wire Temperature Sensor | 1-Wire |
| **DS3231** | RTC Module | I2C |
| **ESC** | ESC Motor Control (BLDC) | PWM |
| **ESP01** | ESP8266 WiFi Module | USART |
| **FlameSensor_KY026** | Flame Sensor | ADC/GPIO |
| **GPS_NEO6M** | GPS Module | USART |
| **HC05** | Bluetooth Module | USART |
| **HCSR04** | Ultrasonic Distance | GPIO |
| **HX711** | Load Cell ADC | GPIO |
| **I2CScan** | I2C Bus Scanner | I2C |
| **INA219** | Current/Power Monitor | I2C |
| **IR** | Infrared Remote Receiver (NEC Protocol) | GPIO |
| **KeyMatrix** | Keypad Matrix | GPIO |
| **L298N** | Motor Driver | GPIO/PWM |
| **LCD1602_I2C** | LCD 16x2 ผ่าน I2C (PCF8574) | I2C |
| **LCDMenu** | Menu System for LCD1602/2004 (4-btn) | I2C + GPIO |
| **MAX7219** | LED Matrix / 7-Segment Display Driver (v1.1: Thai UTF-8 + Effects) | SPI |
| **MCP4725** | 12-bit DAC | I2C |
| **MPU6050** | IMU Gyro/Accel | I2C |
| **MQGas** | MQ Gas Sensors | ADC |
| **NeoPixel** | WS2812B RGB LED Strip | GPIO |
| **nRF24L01** | 2.4GHz Radio | SPI |
| **NTC10K** | Thermistor Temperature Sensor (10kΩ NTC) | ADC |
| **OH49E** | Hall Effect Sensor | ADC |
| **OLED** | SSD1306 OLED Display (I2C) พร้อม Fonts, Graphics, Menu | I2C |
| **P10** | P10 LED Panel Driver (Single/Dual/RGB, timer scan) | GPIO |
| **PCA9685** | 16-ch PWM Driver | I2C |
| **PCF8574** | I/O Expander | I2C |
| **PIR** | PIR Motion Sensor | GPIO |
| **PMS5003** | PM2.5 Sensor | USART |
| **PZEM004T** | AC Energy Meter | USART |
| **PZEM004Tv3** | AC Energy Meter v3 | USART |
| **RainSensor_YL83** | Rain Sensor | ADC/GPIO |
| **RC522** | RFID Reader | SPI |
| **RCWL0516** | Microwave Radar | GPIO |
| **Relay** | Relay Control | GPIO |
| **RotaryEncoder** | Rotary Encoder พร้อม Button | GPIO |
| **Servo** | RC Servo Motor | PWM |
| **ServoCluster** | Multi-Servo Manager | PWM |
| **ServoTester** | Servo Test Utility | PWM |
| **ShiftReg595** | 74HC595 Shift Register | GPIO |
| **SHT3x** | Temp/Humidity Sensor | I2C |
| **SoilMoisture_YL69** | Soil Moisture Sensor | ADC |
| **SoundSensor_KY038** | Sound Sensor | ADC |
| **StepperMotor** | Stepper Motor | GPIO |
| **TJC** | TJC/Nextion HMI Display | USART |
| **TM1637** | TM1637 4-Digit 7-Segment Display | GPIO |
| **TM1650** | 4-Digit 7-Segment Display | I2C |
| **TMC220x** | Stepper Driver (UART) | USART |
| **TMC5160** | Stepper Driver (SPI) | SPI |
| **VL53L0X** | ToF Distance Sensor | I2C |
| **W25Qxx** | SPI Flash Memory | SPI |
| **WaterFlow_YFS201** | Water Flow Sensor | GPIO |
| **WS2812Matrix** | WS2812 8×8 LED Matrix (Instance struct, fonts, effects) | GPIO |
| **WS2815Matrix** | WS2815 12V RGB LED Matrix | GPIO |

---

## 🔨 VS Code Tasks

| Task | คีย์ลัด / วิธีรัน | หน้าที่ |
|------|------------------|---------|
| **Build** | `Ctrl+Shift+B` | Compile และ Link |
| **Clean** | Terminal → Run Task | ลบ output และ obj |
| **Rebuild** | Terminal → Run Task | Clean แล้ว Build ใหม่ |
| **Upload** | Terminal → Run Task | Flash ผ่าน WCH-Link |

---

## ⚙️ Toolchain Details

| รายการ | ค่า |
|--------|-----|
| Compiler | `riscv-wch-elf-gcc` (GCC 12.2.0) |
| Architecture | `rv32ecxw` / `ilp32e` |
| Optimization | `-Os` |
| Debug | `-g` |
| Linker Script | `Ld/Link.ld` |

> **หมายเหตุ:** ต้องใช้ **GCC12** (`riscv-wch-elf-`) เท่านั้น  
> ห้ามใช้ GCC8 (`riscv-none-embed-`) เพราะขาด `rv32ec` multilib

---

## 🤖 GitHub Copilot Tips

โปรเจกต์นี้ออกแบบมาให้ทำงานร่วมกับ GitHub Copilot ได้ดี เพียงบอก Copilot ว่า:

- `"ใช้ SimpleHAL บน CH32V003"` — Copilot จะ Generate Code ที่เหมาะสม
- `"สร้าง function อ่าน ADC ด้วย SimpleADC"` — ได้โค้ดพร้อมใช้
- `"เขียน I2C read/write สำหรับ OLED SSD1306"` — ใช้ Library ที่มีอยู่แล้ว

---

## 📋 Pinout CH32V003F4P6 (TSSOP-20)

```
        VCC  GND
PA1  PA2  PC0  PC1  PC2  PC3  PC4  PC5  PC6  PC7
PD0  PD1  PD2  PD3  PD4  PD5  PD6  PD7
```

| ขา | ฟังก์ชั่น |
|----|----------|
| PC0–PC7 | GPIO, ADC (PC4=ADC CH2, PC5=ADC CH3 ...) |
| PD1 | SWIO (Debug/Program) |
| PD5, PD6 | USART TX, RX |
| PC1, PC2 | I2C SDA, SCL |
| PC5, PC6 | SPI SCK, MOSI |

---

## 📄 License

MIT License — ใช้งาน แก้ไข แจกจ่ายได้อย่างอิสระ

---

## 👤 Author

**MAKER WITAWAT**  
🌐 [makerwitawat.com](https://www.makerwitawat.com)  
📦 [github.com/Witawat](https://github.com/Witawat)
