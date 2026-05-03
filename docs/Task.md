# CH32V003 Task List

## Session: Fix Build Errors & Add New Libraries

---

### Phase 0 — Fix Build Errors

| # | รายการ | สถานะ |
|---|--------|-------|
| 0.1 | `I2C_ReadReg` API เปลี่ยน signature → แก้ BMP280, DS3231, MPU6050 ให้ใช้ `I2C_ReadRegMulti` | ✅ Done |
| 0.2 | `USART_SimpleInit` ต้องการ 2 args + rename `SendByte`/`ReadByte` → แก้ HC05 | ✅ Done |
| 0.3 | Arduino macros (`INPUT`, `OUTPUT`, `INPUT_PULLUP`) ไม่มี → แก้ HX711, KeyMatrix, nRF24L01, W25Qxx ให้ใช้ `PIN_MODE_*` | ✅ Done |
| 0.4 | `I2CScan.h` ชนกับ `I2C_Scan()` ใน SimpleI2C.h → rename เป็น `I2CScan_Run()` | ✅ Done |
| 0.5 | `_read_reg_multi` ใน nRF24L01.c ไม่ได้ใช้งาน → ลบออก | ✅ Done |

---

### Phase 1 — Relay Module

| # | รายการ | สถานะ |
|---|--------|-------|
| 1.1 | สร้าง `User/Lib/Relay/Relay.h` | ✅ Done |
| 1.2 | สร้าง `User/Lib/Relay/Relay.c` | ✅ Done |

**ฟังก์ชัน:** `Relay_Init`, `Relay_On`, `Relay_Off`, `Relay_Toggle`, `Relay_Set`, `Relay_IsOn`
รองรับ Active High / Active Low

---

### Phase 2 — OH49E Hall Effect Sensor Module

| # | รายการ | สถานะ |
|---|--------|-------|
| 2.1 | สร้าง `User/Lib/OH49E/OH49E.h` | ✅ Done |
| 2.2 | สร้าง `User/Lib/OH49E/OH49E.c` | ✅ Done |

**ฟังก์ชัน:** `OH49E_Init`, `OH49E_ReadRaw`, `OH49E_ReadVoltage`, `OH49E_IsFieldDetected`, `OH49E_GetFieldDirection`, `OH49E_GetFieldStrength`, `OH49E_SetThreshold`

---

### Phase 3 — L298N DC Motor Driver Module

| # | รายการ | สถานะ |
|---|--------|-------|
| 3.1 | สร้าง `User/Lib/L298N/L298N.h` | ✅ Done |
| 3.2 | สร้าง `User/Lib/L298N/L298N.c` | ✅ Done |

**ฟังก์ชัน:** `L298N_Init`, `L298N_Run`, `L298N_Stop`, `L298N_Brake`, `L298N_SetSpeed`, `L298N_SetDirection`

---

### Phase 4 — RC522 RFID Reader Module

| # | รายการ | สถานะ |
|---|--------|-------|
| 4.1 | สร้าง `User/Lib/RC522/RC522.h` | ✅ Done |
| 4.2 | สร้าง `User/Lib/RC522/RC522.c` | ✅ Done |

**ฟังก์ชัน:** `RC522_Init`, `RC522_IsCardPresent`, `RC522_ReadUID`, `RC522_Halt`, `RC522_Reset`, `RC522_GetVersion`

---

### Phase 5 — PCF8574 I2C GPIO Expander Module

| # | รายการ | สถานะ |
|---|--------|-------|
| 5.1 | สร้าง `User/Lib/PCF8574/PCF8574.h` | ✅ Done |
| 5.2 | สร้าง `User/Lib/PCF8574/PCF8574.c` | ✅ Done |

**ฟังก์ชัน:** `PCF8574_Init`, `PCF8574_PinMode`, `PCF8574_Write`, `PCF8574_Read`, `PCF8574_WritePort`, `PCF8574_ReadPort`, `PCF8574_Toggle`

---

### Phase 6 — Build Verification

| # | รายการ | สถานะ |
|---|--------|-------|
| 6.1 | Build ผ่าน (Exit Code: 0) หลังแก้ error ทั้งหมด | ✅ Done |
| 6.2 | Build ผ่าน (Exit Code: 0) หลังเพิ่ม 5 modules ใหม่ | ✅ Done |

---

### Phase 7 — Add 6 New Low-Resource Modules

| # | รายการ | สถานะ |
|---|--------|-------|
| 7.1 | สร้าง `User/Lib/SoilMoisture_YL69/SoilMoisture.h` | ✅ Done |
| 7.2 | สร้าง `User/Lib/SoilMoisture_YL69/SoilMoisture.c` | ✅ Done |
| 7.3 | สร้าง `User/Lib/SoilMoisture_YL69/README.md` | ✅ Done |
| 7.4 | สร้าง `User/Lib/FlameSensor_KY026/FlameSensor.h` | ✅ Done |
| 7.5 | สร้าง `User/Lib/FlameSensor_KY026/FlameSensor.c` | ✅ Done |
| 7.6 | สร้าง `User/Lib/FlameSensor_KY026/README.md` | ✅ Done |
| 7.7 | สร้าง `User/Lib/SoundSensor_KY038/SoundSensor.h` | ✅ Done |
| 7.8 | สร้าง `User/Lib/SoundSensor_KY038/SoundSensor.c` | ✅ Done |
| 7.9 | สร้าง `User/Lib/SoundSensor_KY038/README.md` | ✅ Done |
| 7.10 | สร้าง `User/Lib/RainSensor_YL83/RainSensor.h` | ✅ Done |
| 7.11 | สร้าง `User/Lib/RainSensor_YL83/RainSensor.c` | ✅ Done |
| 7.12 | สร้าง `User/Lib/RainSensor_YL83/README.md` | ✅ Done |
| 7.13 | สร้าง `User/Lib/WaterFlow_YFS201/WaterFlow.h` | ✅ Done |
| 7.14 | สร้าง `User/Lib/WaterFlow_YFS201/WaterFlow.c` | ✅ Done |
| 7.15 | สร้าง `User/Lib/WaterFlow_YFS201/README.md` | ✅ Done |
| 7.16 | สร้าง `User/Lib/GPS_NEO6M/GPS.h` | ✅ Done |
| 7.17 | สร้าง `User/Lib/GPS_NEO6M/GPS.c` | ✅ Done |
| 7.18 | สร้าง `User/Lib/GPS_NEO6M/README.md` | ✅ Done |

**ฟังก์ชัน (SoilMoisture_YL69):** `SoilMoisture_Init`, `SoilMoisture_Read`, `SoilMoisture_ReadRaw`, `SoilMoisture_IsDry`, `SoilMoisture_Calibrate`

**ฟังก์ชัน (FlameSensor_KY026):** `FlameSensor_Init`, `FlameSensor_ReadRaw`, `FlameSensor_ReadIntensity`, `FlameSensor_IsFlameDetected`, `FlameSensor_SetThreshold`

**ฟังก์ชัน (SoundSensor_KY038):** `SoundSensor_Init`, `SoundSensor_ReadRaw`, `SoundSensor_ReadLevel`, `SoundSensor_IsClapDetected`, `SoundSensor_GetPeak`

**ฟังก์ชัน (RainSensor_YL83):** `RainSensor_Init`, `RainSensor_ReadRaw`, `RainSensor_ReadLevel`, `RainSensor_IsRaining`, `RainSensor_GetIntensity`

**ฟังก์ชัน (WaterFlow_YFS201):** `WaterFlow_Init`, `WaterFlow_GetPulseCount`, `WaterFlow_GetFlowRate`, `WaterFlow_GetTotalVolume`, `WaterFlow_Reset`

**ฟังก์ชัน (GPS_NEO6M):** `GPS_Init`, `GPS_Update`, `GPS_IsFixValid`, `GPS_GetLatitude`, `GPS_GetLongitude`, `GPS_GetAltitude`, `GPS_GetSpeed`, `GPS_GetSatellites`, `GPS_GetDateTime`

---

### Phase 7B — Build Verification

| # | รายการ | สถานะ |
|---|--------|-------|
| 7.19 | Build ผ่าน (Exit Code: 0) หลังเพิ่ม 6 modules ใหม่ | ✅ Done |

---

### Phase 8 — Add 3 New Servo-Related Libraries

| # | รายการ | สถานะ |
|---|--------|-------|
| 8.1 | สร้าง `User/Lib/ServoCluster/ServoCluster.h` | ✅ Done |
| 8.2 | สร้าง `User/Lib/ServoCluster/ServoCluster.c` | ✅ Done |
| 8.3 | สร้าง `User/Lib/ServoCluster/README.md` | ✅ Done |
| 8.4 | สร้าง `User/Lib/ESC/ESC.h` | ✅ Done |
| 8.5 | สร้าง `User/Lib/ESC/ESC.c` | ✅ Done |
| 8.6 | สร้าง `User/Lib/ESC/README.md` | ✅ Done |
| 8.7 | สร้าง `User/Lib/ServoTester/ServoTester.h` | ✅ Done |
| 8.8 | สร้าง `User/Lib/ServoTester/ServoTester.c` | ✅ Done |
| 8.9 | สร้าง `User/Lib/ServoTester/README.md` | ✅ Done |

**ฟังก์ชัน (ServoCluster):** `Init`, `AddServo`, `MoveTo`, `MoveAll`, `SetEasing`, `SetSpeed`, `Update`, `IsMoving`, `IsAllDone`, `Stop`, `StopAll`

**ฟังก์ชัน (ESC):** `Init`, `Arm`, `SetThrottle`, `SetThrottleMicroseconds`, `Calibrate`, `Stop`, `Disarm`, `IsArmed`

**ฟังก์ชัน (ServoTester):** `Init`, `Sweep`, `FindCenter`, `FindPulseRange`, `SetPulse`, `GetCurrentPulse`

---

### Phase 8B — Build Verification

| # | รายการ | สถานะ |
|---|--------|-------|
| 8.10 | Build ผ่าน (Exit Code: 0) หลังเพิ่ม 3 servo modules | ✅ Done |

---

### Phase 9 — P10 LED Display Library (2026-05-03)

| # | รายการ | สถานะ |
|---|--------|-------|
| 9.1 | สร้าง `User/Lib/P10/P10.h` | ✅ Done |
| 9.2 | สร้าง `User/Lib/P10/P10.c` | ✅ Done |
| 9.3 | สร้าง `User/Lib/P10/README.md` | ✅ Done |

**ฟังก์ชัน:** P10 LED Matrix Panel Driver — Single/Dual/RGB color, timer ISR scan, configurable resolution

---

### Phase 10 — WS2812Matrix Library (2026-05-03)

| # | รายการ | สถานะ |
|---|--------|-------|
| 10.1 | สร้าง `User/Lib/WS2812Matrix/WS2812Matrix.h` | ✅ Done |
| 10.2 | สร้าง `User/Lib/WS2812Matrix/WS2812Matrix.c` | ✅ Done |
| 10.3 | สร้าง `User/Lib/WS2812Matrix/WS2812M_Fonts.h` | ✅ Done |
| 10.4 | สร้าง `User/Lib/WS2812Matrix/README.md` | ✅ Done |
| 10.5 | เพิ่ม font rendering + text display (v1.1) | ✅ Done |
| 10.6 | เพิ่ม sprite/bitmap + effects + buffer utils (v1.1) | ✅ Done |

**ฟังก์ชัน:** WS2812 8x8 LED Matrix — Instance struct + SimpleGPIO, Zigzag/Snake, Drawing primitives, ASCII + Thai fonts, Scrolling text, Sprites, Effects, Buffer utilities

---

### Phase 11 — MAX7219 v1.1 Upgrade (2026-05-03)

| # | รายการ | สถานะ |
|---|--------|-------|
| 11.1 | สร้าง `User/Lib/MAX7219/max7219_fonts_thai.h` | ✅ Done |
| 11.2 | อัปเดต `MAX7219.h` — เพิ่ม API + config macros | ✅ Done |
| 11.3 | อัปเดต `MAX7219.c` — เพิ่ม implementation | ✅ Done |
| 11.4 | อัปเดต `README.md` + `MAX7219_Documentation_TH.md` | ✅ Done |

**ฟังก์ชัน:** Thai UTF-8 rendering (DrawCharThai/DrawStringThai), Vertical scrolling, Wipe effects (4 dirs), Blink, Sparkle, MarqueeBorder, RainEffect, RunningLight, Buffer utilities (Shift, ScrollBuffer, ProgressBar), Config macros

---

### Phase 11B — Build Verification

---

## Git Commit Message

```
fix(libs): แก้ไขข้อผิดพลาด SimpleHAL API และเพิ่ม 5 module ใหม่

แก้ไขข้อผิดพลาดจากการเปลี่ยน API ของ SimpleHAL:
- BMP280, DS3231, MPU6050: เปลี่ยน I2C_ReadReg (3 args) เป็น I2C_ReadRegMulti
- HC05: อัปเดต USART_SimpleInit ให้รับ 2 args และเปลี่ยนชื่อ SendByte/ReadByte
- HX711, KeyMatrix, nRF24L01, W25Qxx: เปลี่ยน Arduino GPIO macros เป็น PIN_MODE_* enum
- I2CScan: เปลี่ยนชื่อ I2C_Scan() เป็น I2CScan_Run() เพื่อหลีกเลี่ยงการชนกับ SimpleI2C.h
- nRF24L01: ลบฟังก์ชัน static _read_reg_multi ที่ไม่ได้ใช้งานออก

เพิ่ม library module ใหม่:
- Relay: ควบคุม relay แบบ Active High / Active Low
- OH49E: เซนเซอร์ Hall Effect แบบ linear ผ่าน ADC
- L298N: ควบคุมมอเตอร์ DC พร้อมปรับความเร็วด้วย PWM
- RC522: อ่าน RFID ผ่าน SPI (REQA, anti-collision, select, halt)
- PCF8574: ขยาย GPIO ผ่าน I2C แบบ 8-bit quasi-bidirectional
```

---

```
feat(libs): เพิ่ม 6 module ใหม่ (Sensor + Flow + GPS) และแผนพัฒนาโปรเจกต์

เพิ่ม library module ใหม่สำหรับ CH32V003/CH32V006:
- SoilMoisture_YL69: YL-69 Soil Moisture Sensor ผ่าน ADC (0-100%, calibrate ได้)
- FlameSensor_KY026: KY-026 Flame Sensor ผ่าน ADC + digital (dual mode)
- SoundSensor_KY038: KY-038 Sound Sensor ผ่าน ADC (clap detection, peak tracking)
- RainSensor_YL83: YL-83 Rain Sensor ผ่าน ADC (intensity: NONE/LIGHT/MODERATE/HEAVY)
- WaterFlow_YFS201: YF-S201 Water Flow Sensor ผ่าน GPIO interrupt (L/min, total L)
- GPS_NEO6M: NEO-6M GPS Module ผ่าน UART (parse $GPGGA/$GPRMC, lat/lon/alt/speed)

เพิ่มไฟล์โปรเจกต์:
- plan.md: แผนพัฒนาถาวรสำหรับ Copilot agent
- Task.md: อัปเดต Phase 7 (18 tasks + build verify)

Build: Exit Code 0, 0 warnings, Flash 10% RAM 20%
```

---

```
feat(libs): เพิ่ม 3 module ด้าน Servo (Cluster + ESC + Tester)

เพิ่ม library module ใหม่สำหรับ CH32V003/CH32V006:
- ServoCluster: ควบคุม servo หลายตัวพร้อมกัน non-blocking
  dual backend (HW-PWM 8ch / PCA9685 16ch)
  รองรับ 10 easing curves (linear, quad, cubic, sine)
- ESC: ควบคุม BLDC มอเตอร์ผ่าน ESC มาตรฐาน PWM 50Hz
  arm/disarm safety, throttle 0-100%, auto-calibrate
- ServoTester: เครื่องมือ calibrate servo
  auto-sweep, find center, find pulse range

Build: Exit Code 0, 0 warnings, Flash 10% RAM 20%
```
