# CH32V003 Build Errors — Case Study (2026-04-30)

## 1. I2C_ReadReg — Too Many Arguments

**ไฟล์ที่เกิด:** BMP280.c, DS3231.c, MPU6050.c

**สาเหตุ:**
API เดิมในไฟล์เหล่านั้นใช้ `I2C_ReadReg(addr, reg, *val)` (3 args, คืน `I2C_Status`)
แต่ SimpleI2C.h เปลี่ยน signature เป็น `uint8_t I2C_ReadReg(addr, reg)` (2 args, คืนค่าที่อ่านได้โดยตรง)

**แก้ไข:**
เปลี่ยนไปใช้ `I2C_ReadRegMulti(addr, reg, val, 1)` ซึ่งคืน `I2C_Status` และเขียนค่าลง pointer ตามเดิม

---

## 2. USART_SimpleInit / USART_SendByte / USART_ReadByte

**ไฟล์ที่เกิด:** HC05.c

**สาเหตุ:**
- `USART_SimpleInit(baudrate)` → API เพิ่ม param ที่ 2: `USART_SimpleInit(baud, pin_config)` บังคับเสมอ
- `USART_SendByte` ถูกเปลี่ยนชื่อเป็น `USART_WriteByte`
- `USART_ReadByte()` ถูกเปลี่ยนชื่อเป็น `USART_Read()`

**แก้ไข:**
```c
USART_SimpleInit((USART_BaudRate)baudrate, USART_PINS_DEFAULT);
USART_WriteByte(byte);
USART_Read();
```

---

## 3. INPUT / OUTPUT / INPUT_PULLUP Undeclared

**ไฟล์ที่เกิด:** HX711.c, KeyMatrix.c, nRF24L01.c, W25Qxx.c

**สาเหตุ:**
SimpleGPIO ใช้ `GPIO_PinMode` enum ไม่ใช่ macro แบบ Arduino
ไม่มีการ define `INPUT`, `OUTPUT`, `INPUT_PULLUP`

**แก้ไข:**
| เก่า | ใหม่ |
|------|------|
| `OUTPUT` | `PIN_MODE_OUTPUT` |
| `INPUT` | `PIN_MODE_INPUT` |
| `INPUT_PULLUP` | `PIN_MODE_INPUT_PULLUP` |

---

## 4. I2C_Scan — Conflicting Types

**ไฟล์ที่เกิด:** I2CScan.c, I2CScan.h

**สาเหตุ:**
SimpleI2C.h เพิ่มฟังก์ชัน `uint8_t I2C_Scan(uint8_t*, uint8_t)` ซึ่งชนกับ
`void I2C_Scan(void)` ของ I2CScan library

**แก้ไข:**
เปลี่ยนชื่อฟังก์ชันใน I2CScan library เป็น `I2CScan_Run()` ทั้ง .h และ .c

---

## 5. Unused Static Function Warning

**ไฟล์ที่เกิด:** nRF24L01.c

**สาเหตุ:**
`static void _read_reg_multi(...)` ถูกเขียนไว้เป็น counterpart ของ `_write_reg_multi`
แต่ไม่มีที่ใดในไฟล์เรียกใช้จริง → GCC ออก warning `-Wunused-function`

**แก้ไข:**
ลบฟังก์ชันออก เนื่องจากเป็น dead code และไม่มีประโยชน์จริง

---

## Pattern สรุป

- เมื่อ SimpleHAL เปลี่ยน API signature → lib ที่ใช้งานต้องอัปเดตตาม
- Static helper ที่ไม่ถูกเรียก → ลบออก ไม่ใช้ `__attribute__((unused))`
- ชื่อฟังก์ชันใน lib ต้องไม่ชนกับ SimpleHAL → ใช้ prefix ของ lib เอง (เช่น `I2CScan_Run`)
- SimpleGPIO ใช้ `GPIO_PinMode` enum เสมอ ไม่มี Arduino-style macros
