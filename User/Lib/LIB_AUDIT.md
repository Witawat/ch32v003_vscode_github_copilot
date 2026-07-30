# การตรวจสอบบั๊ก User/Lib — CH32V003

> **วันที่ตรวจสอบ:** 2026-07-29
> **วันที่อัปเดตสถานะ:** 2026-07-30 — ตรวจสอบโค้ดจริงเทียบกับบั๊กทั้ง 29 จุด
> **ขอบเขต:** ทั้ง 71 ไลบรารีใน `User/Lib/` (ไม่รวม `User/SimpleHAL/` — ดู [PLAN.md](../../PLAN.md) แยกต่างหาก)
> **สถานะ:** แก้ไขแล้ว **29/29 จุด** (commit `fa8903d` แก้ 27 จุด, แก้เพิ่มอีก 2 จุดที่เหลือเมื่อ 2026-07-30)
> เทียบกับ [LIMITATIONS.md](LIMITATIONS.md) เดิม (2026-06-26) แล้ว — เฉพาะบั๊กที่ยังไม่เคยบันทึก
> หรือ workaround เดิมไม่พอ ถูกยกมาที่นี่

**สรุป:** พบบั๊กใหม่ **29 จุด** — 🔴 ร้ายแรง 13, 🟡 สำคัญ 12, 🟢 เล็กน้อย 4
**สถานะปัจจุบัน:** ✅ แก้ไขแล้วทั้งหมด 29/29 จุด

---

## 🔴 ร้ายแรง (Crash / Hang / ใช้งานไม่ได้เลย)

### 1. RotaryEncoder — ISR callback signature ผิด → crash ทันทีที่หมุน — ✅ แก้ไขแล้ว
**ไฟล์:** `RotaryEncoder.c:81-82`
```c
attachInterrupt(pin_clk, (void (*)(void))Rotary_CLK_ISR, CHANGE);
```
`Rotary_CLK_ISR`/`Rotary_DT_ISR` รับ parameter `RotaryEncoder*` แต่ `attachInterrupt()` เป็น
`void (*)(void)` (ไม่มี parameter) — ทุกครั้งที่ CLK/DT edge เกิดขึ้น ISR จะ dereference ค่าขยะใน
argument register เป็น `encoder` → undefined behavior/hard fault ทันทีที่หมุน encoder
> ไฟล์มีกลไกที่ถูกต้องอยู่แล้ว (`Rotary_EXTI_Handler(pin)` + `Rotary_FindEncoder`, บรรทัด 375-396)
> แต่ `Rotary_Init` ไม่ได้เรียกใช้ — เป็น dead code ที่ควรถูกเรียกแทน

### 2. ServoTester — คำนวณ duty cycle ผิดทั้งไฟล์ → เครื่องมือใช้งานไม่ได้เลย — ✅ แก้ไขแล้ว
**ไฟล์:** `ServoTester.c:14-23`
```c
duty = (uint16_t)(((uint32_t)pulse_us * 65535) / period);
```
Hardcode ช่วง 0-65535 ทั้งที่ `PWM_SetDutyCycleRaw()` ต้องการค่าตาม `PWM_GetPeriod()` (ARR ของ timer)
— LIMITATIONS.md บันทึกไว้แล้วว่า pattern นี้ถูกแก้ใน ESC และ ServoCluster แต่ ServoTester ตกหล่น
ทุกฟังก์ชัน (`Sweep`, `SetPulse`, `FindCenter`, `FindPulseRange`) ให้ duty ผิด/ถูก clip — เครื่องมือ
ไม่สามารถสร้างพัลส์ servo ที่ถูกต้องได้เลย

### 3. RC522 — อ่าน UID ผิดสำหรับการ์ด 7-byte UID (พบได้บ่อยมาก) — ✅ แก้ไขแล้ว
**ไฟล์:** `RC522.c:139-194` (`_anticoll`/`_select`)
ไม่รองรับ cascade level (CL2/CL3) ตาม ISO14443-3 — การ์ด UID 7 byte (NTAG213/215/216,
Mifare Ultralight — พบทั่วไป) จะได้ cascade tag `0x88` เก็บเป็น byte จริงของ UID และ
`RC522_ReadUID` report `uid_len=4` เสมอ ทั้งที่ผิด `_select()` ก็ไม่เช็ค SAK "UID not complete"
bit ด้วย → UID ที่ได้ผิด/สั้นเกินไปแบบเงียบๆ

### 4. MAX31855 — enum ชนกับ macro ชื่อเดียวกัน → fault status ผิดทุกครั้ง — ✅ แก้ไขแล้ว
**ไฟล์:** `MAX31855.h:81-94`
ประกาศ `enum { MAX31855_FAULT_OC=2, ... }` แล้วอีก 3 บรรทัดถัดมา `#define MAX31855_FAULT_OC 0x01`
ทับชื่อเดิม — ทุกจุดที่ใช้ชื่อนี้ (รวมใน `.c`) จะถูกแทนที่เป็นค่า macro ไม่ใช่ enum จึงชนกับ
`MAX31855_ERROR` (=1) แยกไม่ออกว่า error จาก NULL/param หรือ thermocouple open circuit

### 5. MAX6675 — อ่านอุณหภูมิผิดที่ ≥512°C — ✅ แก้ไขแล้ว
**ไฟล์:** `MAX31855.c:105-122` (`MAX6675_ReadTemp`)
MAX6675 เป็น 12-bit **unsigned** แต่โค้ด treat เป็น two's-complement signed (ลอกมาจาก MAX31855
ซึ่งเป็น signed จริง) — ค่าอุณหภูมิ ≥512°C (อยู่ในช่วงที่เอกสารบอกว่ารองรับ 0-1024°C) จะโดน
sign-extend กลายเป็นค่าติดลบผิดๆ

### 6. MAX7219 — negative x ทำให้ scroll text พังทุกครั้ง — ✅ แก้ไขแล้ว
**ไฟล์:** `MAX7219.c:201-234` (`SetPixel`/`GetPixel`)
เช็คแค่ `device_idx`/`y` ไม่เช็ค `x < 0` — `MAX7219_UpdateScroll` (บรรทัด 473) ตั้งใจลด
`offset` ต่ำกว่า 0 ทุกรอบ scroll (ฟีเจอร์หลักของ library) ทำให้ shift ด้วยจำนวนติดลบ (UB บน
RV32 wrap ผ่าน low 5 bits) — ทุก animation scroll แนวนอนพังจริง ไม่ใช่ edge case

### 7-8. NeoPixel — Scanner effect แฮงค์เครื่อง / หาร 0 — ✅ แก้ไขแล้ว
**ไฟล์:** `NeoPixel.c:481` และ `NeoPixel.c:820`
- บรรทัด 481: `neo_num_leds - eye_size - 2` เป็น signed แต่เก็บใน `uint16_t i` — ถ้า
  `neo_num_leds <= eye_size + 2` (เช่น strip 4 ดวง eye_size=3 ซึ่งเป็น config ที่เป็นไปได้จริง)
  ค่าติดลบ wrap เป็น ~65534 → loop วิ่งหมื่นรอบพร้อม `Delay_Ms()` ทุกรอบ = ค้างเป็นนาที
- บรรทัด 820: `max_pos = (neo_num_leds - eye_size - 2) * 2` เมื่อ `neo_num_leds == eye_size+2`
  พอดี `max_pos=0` → `effect->step % max_pos` หารด้วย 0

### 9. SoftUART — `Init()` ไม่กัน baud=0 ทั้งที่เอกสารอ้างว่ากันแล้ว — ✅ แก้ไขแล้ว
**ไฟล์:** `SoftUART.c:24-43`
LIMITATIONS.md เขียนว่า "Baud = 0 → Return error" แต่การกันนั้นมีแค่ใน `SoftUART_SetBaud`
ไม่มีใน `SoftUART_Init` — เรียก `SoftUART_Init(&uart, tx, rx, 0)` ตรงๆ จะหาร 0
(`bit_time_us = 1000000/0`) ทำให้ `_delay_bit()` ค้างยาวนานตั้งแต่ byte แรก

### 10. SoftUART — RX ring buffer ไม่เคยถูกเติมข้อมูล — ✅ แก้ไขแล้ว
**ไฟล์:** `SoftUART.c:72-111` — `SoftUART_Available()` คืน `rx_count` ที่ไม่เคยถูกเพิ่มค่าที่ไหนเลย
คืน 0 เสมอ ตรงตามที่ LIMITATIONS.md บันทึกไว้
> **แก้ไข (2026-07-30):** ไลบรารีนี้เป็น polling-only ไม่มี ISR รับข้อมูลพื้นหลังจริง จึงลบ
> `rx_buffer`/`rx_head`/`rx_tail`/`rx_count` (dead code) ออกทั้งหมด แล้วเปลี่ยน `SoftUART_Available()`
> เป็น best-effort check (เช็คว่า start bit กำลังมาบนขา RX หรือไม่) แทนการคืนค่า counter ที่ไม่เคย
> ถูกอัปเดต — อัปเดตเอกสารใน `SoftUART.h`/`README.md` ให้ตรงกับพฤติกรรมจริงด้วย

### 11. WaterFlow_YFS201 — นับพัลส์ผิดถ้าใช้หลายตัวพร้อมกัน — ✅ แก้ไขแล้ว
**ไฟล์:** `WaterFlow.c:24-37` (`WaterFlow_PulseISR`)
ISR ตัวเดียวถูก attach แยกกันทุก instance แต่ตัว handler เองวน loop ทุก instance ที่ลงทะเบียนไว้
แล้วเพิ่ม `pulse_count` ให้ **ทุกตัวที่พินอ่านได้ HIGH ขณะนั้น** ไม่ใช่แค่ตัวที่ trigger interrupt
จริง — ถ้าใช้ 2+ เซนเซอร์พร้อมกัน จะนับผิดเพี้ยนข้ามตัวกัน (ใช้ตัวเดียวไม่กระทบ)

### 12. TM1637 — `DisplayNumber` พังที่ INT16_MIN (ขัดแย้งกับ LIMITATIONS.md) — ✅ แก้ไขแล้ว
**ไฟล์:** `TM1637.c:255-259`
LIMITATIONS.md อ้างว่า "รองรับ INT16_MIN" แต่การ negate ค่า INT16_MIN overflow (คลาสสิก
`abs(INT_MIN)` UB) ทำให้ค่ายังติดลบอยู่ แล้ว `% 10` ได้ digit ติดลบ cast เป็น `uint8_t` (248)
ไป index `DIGIT_SEGMENTS[digit]` (array 10 ช่อง) เกินขอบเขต — อ่านค่าขยะมาแสดงผล

### 13. TM1650 — บั๊กเดียวกับ TM1637 ที่ INT16_MIN (ไม่เคยบันทึกไว้) — ✅ แก้ไขแล้ว
**ไฟล์:** `TM1650.c:186-189` — root cause เดียวกับข้อ 12 แต่ไม่มีใน LIMITATIONS.md เลย

---

## 🟡 สำคัญ (ทำงานผิดในกรณีทั่วไป/ข้อมูลผิดเงียบๆ)

### 14. CircularBuffer — ไม่ thread-safe จริงตามที่เอกสารอ้าง — ✅ แก้ไขแล้ว
**ไฟล์:** `CircularBuffer.c:20-38` — header อ้างว่า "Thread-safe สำหรับ single-producer,
single-consumer" แต่ `count++`/`count--` เป็น non-atomic read-modify-write ไม่มี
`__disable_irq()` ป้องกันเลย ถ้า Push มาจาก ISR (use case หลักตามเอกสาร) ชนกับ Pop ใน main loop
จะเกิด count drift ถาวร (คนละบั๊กกับ SoftUART ที่ buffer ไม่ถูกเติมเลย)

### 15. AT24Cxx — bounds check โดน bypass ด้วย integer overflow — ✅ แก้ไขแล้ว
**ไฟล์:** `AT24Cxx.c:142,187` — `address + len > capacity` เป็น `uint32_t + uint16_t` ที่
overflow ได้ (เช่น address=0xFFFFFFF0, len=0x20 → wrap เป็น 0x10 ผ่านการเช็ค) เขียนไปที่
ตำแหน่งหน่วยความจำผิดแทนที่จะ return error

### 16. GPS_NEO6M — `fix_valid` ไม่เคยถูกล้างเมื่อ GPS หลุด lock — ✅ แก้ไขแล้ว
**ไฟล์:** `GPS.c:182-226` (`_parse_gprmc`) — set `fix_valid=1` เมื่อ status='A' แต่ไม่มี else
clear เมื่อ status='V' (void) — `GPS_IsFixValid()` อาจรายงานค่าเก่าค้างว่า valid ทั้งที่หลุด lock แล้ว

### 17. SoundSensor_KY038 — สมมติ ADC 12-bit ผิด (ชิปนี้ 10-bit) → ตรวจจับเสียงปรบมือไม่ได้เลย — ✅ แก้ไขแล้ว
**ไฟล์:** `SoundSensor.c:36-37` — หารด้วย 4095.0f แต่ `ADC_Read()` คืนค่าสูงสุด 1023 (10-bit)
`level` จึงไม่มีทางเกิน ~0.25 ขณะที่ threshold default = 0.5f → `IsClapDetected()`
**คืนค่า true ไม่ได้เลยด้วย setting ปกติ**

### 18. SoilMoisture_YL69 — บั๊กเดียวกัน (สมมติ ADC 12-bit) — ✅ แก้ไขแล้ว
**ไฟล์:** `SoilMoisture.c:24` — `dry_value=4095` default แต่ ADC จริงสูงสุด 1023 ก่อน
calibrate ค่าความชื้นที่อ่านได้จะถูกบีบอยู่ใน ~0-25% ของช่วงจริงเสมอ

### 19. OLED — ไม่มี `initialized` guard เลยทั้งไฟล์ + bug ใน FillTriangle — ✅ แก้ไขแล้ว
**ไฟล์:** `oled_i2c.c` (ไม่มีจุดใดเช็ค `oled->initialized`), `oled_graphics.c:301,313`
(`FillTriangle` swap ค่า `int16_t` ผ่าน `swap_uint8` ที่สลับแค่ byte ต่ำ — พังถ้าพิกัดติดลบ
คือสามเหลี่ยมที่ขอบจอบางส่วน)

### 20. PID — ไม่กัน `dt=0` → output เป็น NaN/Inf ตลอดไป — ✅ แก้ไขแล้ว
**ไฟล์:** `PID.c:116` — `derivative = (error - prev_error) / dt` ไม่เช็ค `dt != 0` ทั้งใน
compute และตอน Init เลย NaN ยังผ่าน `_clamp()` ได้ด้วย (เปรียบเทียบกับ NaN เป็น false เสมอ)

### 21. P10 — ความกว้างที่ไม่ใช่เลขคูณ 8 ทำให้ pixel เพี้ยนข้ามแถว — ✅ แก้ไขแล้ว
**ไฟล์:** `P10.c:93-323` — `P10_Init` validate แค่ `0 < width <= 64` ไม่บังคับเป็นเลขคูณ 8
ทำให้คอลัมน์สุดท้ายของแถวคำนวณ `byte_index` เพี้ยนไปโดนแถวถัดไป

### 22. LCD1602_I2C — ไม่มี NULL/initialized guard เลย กระทบ LCDMenu ด้วย — ✅ แก้ไขแล้ว
**ไฟล์:** `lcd1602_i2c.c` — ทุกฟังก์ชันไม่เช็ค `lcd == NULL` หรือ initialized เลย ต่างจาก
ไลบรารีอื่นในโปรเจกต์ทั้งหมด และไม่เช็ค return status ของ `I2C_Write` เลยด้วย — LCDMenu ที่ถือ
`LCD1602_Handle*` ไว้ก็รับผลกระทบตามไปด้วยเพราะพึ่งพา handle ที่อาจไม่ได้ init จริง
> **แก้ไข (2026-07-30):** ส่วน NULL/initialized guard แก้ไปแล้วในคอมมิตก่อนหน้า ส่วนที่เหลือคือ
> `I2C_Write` return value: เปลี่ยน `LCD_ExpanderWrite`/`LCD_PulseEnable`/`LCD_WriteNibble`/
> `LCD_WriteByte`/`LCD_SendCommand`/`LCD_SendData` ให้คืนค่า `I2C_Status` แล้วให้ `LCD_Init`
> เช็ค status ตลอด init sequence — ถ้า PCF8574 ไม่ตอบสนอง (I2C NACK/timeout) จะไม่ตั้ง
> `lcd->initialized = 1` อีกต่อไป ป้องกันโค้ดที่เช็คแค่ flag คิดว่า init สำเร็จทั้งที่ฮาร์ดแวร์ไม่ตอบสนอง

### 23. NTC10K — `InitWithConfig` ไม่เช็ค NULL param — ✅ แก้ไขแล้ว
**ไฟล์:** `NTC10K.c:137-146` — ต่างจาก `NTC_Init()` ปกติที่ปลอดภัย ฟังก์ชันนี้ crash ทันทีถ้า
ส่ง `config` เป็น NULL

### 24. IR — ข้อมูลที่ ISR แชร์ไม่มี `volatile` และไม่มี critical section — ✅ แก้ไขแล้ว
**ไฟล์:** `IR.c:15-25,68-117,137-139,551-563` — `ir_rx` struct ถูก ISR เขียนและ main loop
อ่าน/เขียนโดยไม่มี `volatile` หรือ `__disable_irq()` เลย ขัดกับ pattern ที่ AGENT.MD กำหนดไว้เอง
`IR_GetRawData()` ยัง return pointer เข้าไปใน buffer ที่ ISR แก้ไขระหว่างนั้นได้อีก

### 25. HX711 — `initialized=1` ถูกตั้งก่อนเช็คว่า init สำเร็จจริง — ✅ แก้ไขแล้ว
**ไฟล์:** `HX711.c:75-99` — set flag ก่อน dummy read ยืนยัน ถ้า read timeout ฟังก์ชัน return
error แต่ `initialized` ค้างเป็น 1 — โค้ดที่เช็คแค่ flag (ไม่เช็ค return code) จะคิดว่า init
สำเร็จทั้งที่ฮาร์ดแวร์ไม่ตอบสนอง

---

## 🟢 เล็กน้อย

### 26. Buzzer — `FrequencySweep` หาร 0 โดยไม่ตั้งใจ (ไม่กระทบผลลัพธ์จริง) — ✅ แก้ไขแล้ว
**ไฟล์:** `Buzzer.c:420-442` — เช็ค `step_ms==0` แต่ไม่เช็ค `duration_ms==0` ก่อนหาร แม้ RISC-V
integer div/0 ไม่ trap และผลลัพธ์ถูกทิ้งเพราะ loop ไม่ทำงาน แต่เป็นความไม่สอดคล้องกับ pattern
guard ที่ project ใช้ที่อื่น

### 27. HC05 — มี field `rx_buf`/`rx_head`/`rx_tail` ที่ไม่เคยถูกใช้จริง — ✅ แก้ไขแล้ว
**ไฟล์:** `HC05.c` — dead/vestigial state ไม่ใช่บั๊กเชิงพฤติกรรม (I/O จริงผ่าน
`USART_Available`/`USART_Read` ตรงๆ) แต่ควร cleanup

### 28. WS2812Matrix/WS2815Matrix — UTF-8 truncated sequence อ่านเกิน buffer — ✅ แก้ไขแล้ว
**ไฟล์:** `WS2812Matrix.c:353-434`, `WS2815Matrix.c:694-703` — `utf8_to_unicode` ไม่เช็คว่า
string จบกลาง multi-byte sequence ก่อนอ่าน `text[1]`/`text[2]` อาจอ่านเกิน NUL terminator

### 29. WS2815Matrix — gradient pattern หาร 0 ถ้า width/height=1 — ✅ แก้ไขแล้ว
**ไฟล์:** `WS2815Matrix.c:616-638` (`PatternGradientH`/`V`) — ไม่กัน `width-1`/`height-1` = 0

---

## ✅ ไลบรารีที่ตรวจแล้วสะอาด (ไม่พบบั๊กใหม่)

ADS1115, AHT10, AS5600, BH1750, BMP280, Button, DHT, DRV8825, DS18B20, DS3231, ESC, ESP01,
FlameSensor_KY026, HCSR04, I2CScan, INA219, KeyMatrix, L298N, LCDMenu (ยกเว้นพึ่งพา LCD1602_I2C
ที่มีปัญหา), MCP4725, MPU6050, MQGas, nRF24L01, OH49E, PCA9685, PCF8574, PIR, PMS5003, PZEM004T,
PZEM004Tv3, RainSensor_YL83, RCWL0516, Relay, Servo, ServoCluster, ShiftReg595, SHT3x,
SimpleScheduler, TCS34725, TJC, TMC220x, TMC5160, VL53L0X, W25Qxx, StepperMotor

---

## สรุปลำดับความสำคัญถ้าจะแก้ (เดิม)

1. **ตัวที่ทำให้ crash/hang ทันที** — RotaryEncoder (#1), NeoPixel Scanner (#7-8), MAX7219
   scroll (#6), SoftUART Init (#9)
2. **ตัวที่ใช้งานไม่ได้เลยตามฟีเจอร์หลัก** — ServoTester (#2), RC522 UID (#3), SoundSensor (#17)
3. **ตัวที่ข้อมูลผิดเงียบๆ** — MAX31855/MAX6675 (#4-5), SoilMoisture (#18), GPS (#16),
   AT24Cxx (#15)
4. **ตัวที่ไม่มี safety guard พื้นฐาน** — LCD1602_I2C (#22), OLED (#19), PID (#20), NTC10K (#23)
5. ที่เหลือ (#10-14, #21, #24-29) ตามความสำคัญของ Lib ที่ product ใช้จริง

## สถานะล่าสุด (2026-07-30)

✅ **แก้ไขครบทั้ง 29/29 จุดแล้ว**

- 27 จุดแรก แก้ไปแล้วในคอมมิต `fa8903d fix(Lib): resolve all 29 bugs from LIB_AUDIT.md across 5 phases`
- 2 จุดที่เหลือ แก้เพิ่มเมื่อ 2026-07-30:
  - **#10 SoftUART RX buffer** — ลบ dead ring-buffer fields ออก เปลี่ยน `SoftUART_Available()`
    เป็น best-effort start-bit check แทน (ดู `SoftUART.c`, `SoftUART.h`, `README.md`)
  - **#22 LCD1602_I2C ไม่เช็ค I2C_Write return** — ทำให้ chain การเขียน I2C (`LCD_ExpanderWrite`
    → `LCD_PulseEnable` → `LCD_WriteNibble`/`LCD_WriteByte` → `LCD_SendCommand`/`LCD_SendData`)
    คืนค่า `I2C_Status` และให้ `LCD_Init` เช็ค status ก่อนตั้ง `initialized = 1` (ดู `lcd1602_i2c.c`)
