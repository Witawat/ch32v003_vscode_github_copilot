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

## Git Commit Message

```
fix(libs): fix SimpleHAL API compatibility errors and add 5 new modules

Fix build errors caused by SimpleHAL API changes:
- BMP280, DS3231, MPU6050: replace I2C_ReadReg (3-arg) with I2C_ReadRegMulti
- HC05: update USART_SimpleInit to 2-arg form, rename SendByte/ReadByte
- HX711, KeyMatrix, nRF24L01, W25Qxx: replace Arduino GPIO macros with PIN_MODE_* enum
- I2CScan: rename I2C_Scan() to I2CScan_Run() to avoid conflict with SimpleI2C.h
- nRF24L01: remove unused static function _read_reg_multi

Add new library modules:
- Relay: active high/low relay control
- OH49E: linear hall effect sensor via ADC
- L298N: DC motor driver with PWM speed control
- RC522: MFRC522 RFID reader over SPI (REQA, anti-collision, select, halt)
- PCF8574: I2C GPIO expander (8-bit quasi-bidirectional I/O)
```
