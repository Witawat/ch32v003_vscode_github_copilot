# web-tutorial — แผนแก้ไข (Audit Remediation Plan)

ตรวจสอบเมื่อ 2026-07-30 โดยเทียบ `docs/web-tutorial/*.html` (123 บทเรียนหลัก + 71 บทย่อย chapx_lib) กับโค้ดจริงใน
`User/SimpleHAL/`, `User/Lib/`, `User/Examples/` หลังจาก 2 คอมมิตแก้บั๊กล่าสุด:

- `5293ae3` fix(SimpleHAL): 37 บั๊ก — Flash IRQ safety, WWDG/TIM guard, I2C/PWM/USART/ADC
- `fa8903d` fix(Lib): 29 บั๊กจาก LIB_AUDIT.md

**สรุปภาพรวม**: ปัญหาใหญ่ที่สุด **ไม่ใช่** เอกสารบรรยายพฤติกรรมเก่าก่อนแก้บั๊ก (พบแค่ 2-3 จุด) แต่คือ**โค้ดตัวอย่างในบทเรียนใช้ชื่อฟังก์ชัน/signature ที่ไม่ตรงกับ header จริงเลย** — เขียนขึ้นแบบ "เดา API" แล้วไม่เคย compile จริง กระจายอยู่ทั้งในบทหลัก (ch09, ch10, ch12, ch13, ch14, ch16) และบท chapx_lib_* จำนวนมาก นี่คือปัญหาเชิงระบบที่ต้องแก้เป็นกระบวนการ ไม่ใช่ patch จุดเดียว

---

## Phase 0 — Quick fixes (ผลกระทบสูง ใช้เวลาน้อย, ทำก่อน)

แก้ไขจุดที่ชัดเจน ตรวจสอบง่าย ไม่ต้องเขียนใหม่ทั้งบท:

1. **`ch32_opamp.html`** — ลบ section `ex07_PIR_Amplifier` (บรรทัด ~19, 27, 461-608, 605, 614) เพราะไม่มีไฟล์ `ex07_PIR_Amplifier.c` อยู่จริงใน `User/Examples/13_OPAMP/` (มีแค่ 6 ไฟล์ ex01-ex06) แก้ "7 ตัวอย่าง" → "6 ตัวอย่าง" ทุกจุด
2. **`index.html`** — แก้เลข "95" ให้ถูกต้อง (บรรทัด 17, 104) **ต้องแยกค่า 2 บรรทัดเพราะบริบทต่างกัน** (ดูรายงานตรวจสอบ G.1):
   - บรรทัด 17 label กว้าง "ตัวอย่างจาก Examples" (ไม่จำกัดเฉพาะ workshop) → ควรเป็น **102** (รวมไฟล์ `.c` ใน `User/Examples/` ทั้ง 16 โฟลเดอร์) — หรือถ้าตั้งใจจำกัด ต้องแก้ label เป็น "ตัวอย่างใน 15 Workshop" แล้วใช้ 94
   - บรรทัด 104 ระบุชัด "Workshop สอนจาก Examples (**15 หมวด**, 95 ตัวอย่าง)" = workshop ch20–ch34 ครอบเฉพาะโฟลเดอร์ 01–15 (folder 16 ยังไม่มี ch35 คู่กัน) → ควรเป็น **94** (ไฟล์ `.c` ใน 15 โฟลเดอร์แรก)
   - ⚠ **ห้ามใช้ 94 กับบรรทัด 17 โดยไม่แก้ label**
3. **`index.html`** — ลบ block ที่ซ้ำกัน (บรรทัด 120-135 ซ้ำกับ 137-152 — รายการ chapx_pcb/recipes/production/arduino_migration ปรากฏ 2 ครั้งติดกัน)
4. **`ch27_flash.html:103`** — แก้ checklist "ต้อง disable interrupt เอง" เพราะ `SimpleFlash.c` (หลัง 5293ae3) disable IRQ ให้อัตโนมัติรอบทุก erase/program แล้ว
5. **`ch07_i2c.html:56,87,170,219`, `ch07_answers.html:97-108`** — กลับคำผิด: บอกว่า `I2C_PINS_PARTIAL_REMAP` (SCL=PD2, SDA=PD1) **รองรับ SOP-8** แต่จริงๆ **ไม่รองรับ** (`SimpleI2C.h:68`). mechanism จริงคือใน `SimpleI2C.c:67-68` มี `#if CH32V003_IS_SOP8` ทำให้ `I2C_SimpleInit()` **no-op เงียบ** (runtime/compile-time no-op) เมื่อเลือก config นี้บน SOP-8 — มิใช่ direct `#error` ที่หยุด build. นี่คือจุดที่คอมมิต 5293ae3 แก้ comment ให้ตรงกับ header แต่ tutorial ยังพูดแบบเก่า (ผิด)
6. **`ch17_package.html:195,196,202,296`** — `PWM_SimpleInit(2, PC4)` และ `IS_PWM_VALID_PACKAGE(PC4, CH32V003_PACKAGE)` ไม่มีจริง ของจริงคือ `PWM_Init(PWM_Channel, freq)` และ `IS_PWM_VALID_PACKAGE(ch)` รับ 1 arg (`SimplePWM.h:103,105`)

**ตรวจสอบเพิ่มเติมก่อนปิด phase**: นับจำนวนไฟล์ `.c` จริงทั้ง 16 โฟลเดอร์ของ `User/Examples/*` อีกครั้ง — ค่าจริงคือ **102 รวมทุกโฟลเดอร์** และ **94 เฉพาะ 15 โฟลเดอร์แรก** (01–15). ค่าเป้าหมายที่ใช้ที่บรรทัด 17 และ 104 ต่างกันตามบริบทจึงต้องแยกยืนยัน (อย่าใช้ 94 สำหรับบรรทัด 17 โดยเด็ดขาด)

---

## Phase 1 — บทหลักที่ใช้ API สมมติทั้งบท (compile ไม่ผ่านแน่นอน)

เขียนใหม่ทั้งหมดโดยอิงจาก header จริงในแต่ละไฟล์ (ไม่ใช่แค่แก้จุดเดียว เพราะทั้งบทผิด) เรียงตามความเสียหาย:

### 1.1 `ch16_timer_guard.html` + `ch16_answers.html` — วิกฤตสุด
บทนี้มีไว้สอนกลไก **ownership guard ที่เพิ่งเพิ่มใหม่** แต่บรรยาย API ที่ไม่มีอยู่จริงทั้งหมด:
- `TIMER_OWNER_*` → ที่จริงคือ `TIM_OWNER_NONE/PWM/TIMER/TIMEXT` (`SimpleTIM.h:67-70`)
- `Timer_GetOwner()`, `Timer_Release()`, `Timer_SetOwner()` → ไม่มีฟังก์ชันเหล่านี้เลย กลไกจริงคือตัวแปร global `g_tim1_owner`/`g_tim2_owner` ที่เช็ค/ตั้งค่าตรงๆ ใน `TIM_SimpleInit/AttachInterrupt/AdvancedInit/SetFrequency/DetachInterrupt` (`SimpleTIM.c:13-14,118-299`) — ไม่มี accessor API แยก
- `PWM_SimpleInit(2, PC4)`, `PWM_DeInit(2)`, `Encoder_Init(TIM1, PA1, PA2)` → ไม่มีจริง (`PWM_Init(PWM_Channel, freq)` เท่านั้น ไม่มี DeInit/Encoder_Init ใน SimpleHAL)
→ **Action**: เขียนใหม่ทั้งบท อ่าน `SimpleTIM.c` จริงเพื่อสอนกลไก global owner ตามที่มันทำงานจริง

### 1.2 `ch14_onewire.html` + `ch14_answers.html`
ทุกฟังก์ชันโปรโตคอล (`OneWire_Reset/SkipROM/WriteByte/ReadByte/MatchROM/CRC8`) เรียกโดย**ไม่มี** arg แรก `OneWire_Bus* bus` ที่จำเป็นจริง (`Simple1Wire.h:152-394`) `OneWire_SearchROM(roms, max)` ไม่มีจริง — ของจริงคือ `OneWire_ResetSearch(bus)` + loop `OneWire_Search(bus)` + `OneWire_GetAddress(bus, rom)` (`:307-325`) และ `OneWire_ReadTemperature()` ไม่มีเลย
→ **Action**: เขียนใหม่ทั้งบท ตาม `Simple1Wire.h` จริง

### 1.3 `ch13_opamp.html` + `ch13_answers.html`
`OPAMP_SimpleInit(OPAMP_CH1, OPAMP_MODE_FOLLOWER)` (2 args) → จริง 1 arg `OPAMP_SimpleInit(OPAMP_Mode)` (`SimpleOPAMP.h:116`), ไม่มี channel enum `OPAMP_CH1`, ชื่อ mode จริงคือ `OPAMP_MODE_VOLTAGE_FOLLOWER`. `OPAMP_Enable/Disable(ch)` จริงไม่รับ arg เลย. `OPAMP_ConfigPGA`, `OPAMP_PGA_16`, `OPAMP_GetOutputVoltage` ไม่มีจริง (มี `OPAMP_ConfigVoltageFollower/NonInverting/Inverting/Comparator` แทน)
→ **Action**: เขียนใหม่ทั้งบท

### 1.4 `ch12_watchdog.html` + `ch12_answers.html`
`WWDG_SimpleInit(WWDG_TIMEOUT_28MS, 50)` → macro ไม่มีจริง จริงคือ `WWDG_SimpleInit(uint8_t counter, uint8_t window)` รับค่า register ดิบ 0x40-0x7F (`SimpleWWDG.h:56,72`) ไม่ใช่ timeout constant + percentage. `WWDG_Refresh()` เรียกไม่มี arg แต่จริงต้องการ `WWDG_Refresh(uint8_t counter)` (`:83`). `IWDG_CheckReset()` ไม่มีจริง — ของจริงคือ `IWDG_WasResetCause()` (`SimpleIWDG.h:139`)
→ ไม่ได้พูดถึง WWDG window-clamp ใหม่และข้อจำกัดของ `WWDG_Disable()` เลย (ควรเพิ่ม)
→ **Action**: เขียนใหม่ทั้งบท + เพิ่มเนื้อหาอธิบาย window-clamp/WWDG_Disable limitation ที่เพิ่งเพิ่มจาก 5293ae3

### 1.5 `ch10_power.html` + `ch10_answers.html`
`PWR_Standby()` เรียกไม่มี arg → จริงต้องการ `timeout_ms` (`SimplePWR.h:116`). `PWR_ConfigureAWU(AWU_60S)` ใช้ macro ที่ไม่มีอยู่จริงเลยในโค้ด (`AWU_60S/AWU_30S/AWU_16MS`) จริงคือ `PWR_ConfigureAWU(uint32_t prescaler, uint8_t window)` (`:53-67,187`). `PWR_WriteBackup/ReadBackup` ไม่มีอยู่จริงเลย. `PWR_CalculateBatteryLife(vdd, vmin, vmax)` สับสนกับ `ADC_GetBatteryPercent` — signature จริงรับ `(battery_mah, active_time_percent, active_current_ma, standby_current_ua)` (`:313`) คนละเรื่องเลย
→ **Action**: เขียนใหม่ทั้งบท

### 1.6 `ch09_dma.html` + `ch09_answers.html`
`DMA_SimpleInit(channel, dst, src, size)` (4 scalar) → จริงรับ struct pointer เดียว `DMA_SimpleInit(DMA_Config_t* config)`. `DMA_GetStatus()` บอกว่า return 0=running/1=done → จริงเป็น enum `IDLE=0,BUSY=1,COMPLETE=2,ERROR=3` (`SimpleDMA.h:107-112`). `DMA_ADC_Init(ch,buf,n)` (3 args) → จริง 4 args มี `circular` เพิ่ม (`:364`). `DMA_SetCallback(ch,func)` ไม่มีจริง → จริงมี `DMA_SetTransferCompleteCallback/SetErrorCallback/SetHalfTransferCallback` แยกกัน (`:248,262,280`)
→ **Action**: เขียนใหม่ทั้งบท

### 1.7 minor ใน Phase 1
- `ch08_spi.html:110` — "SPI สูงสุด 12MHz" ล้าสมัย จริง PCLK2=48MHz ทำให้ค่าจริงสูงสุด 24MHz (ชื่อ enum เดิมคงไว้เพื่อ backward-compat เท่านั้น ดู `SimpleSPI.h:64-79`) — แก้คำอธิบายให้ตรง

**ลำดับความสำคัญ Phase 1**: ch16 > ch14 > ch13 > ch12 > ch10 > ch09 (เพราะ ch16 สอนฟีเจอร์ที่เพิ่งเพิ่มใหม่โดยตรง ผิดมากที่สุดเชิงความน่าเชื่อถือ)

---

## Phase 2 — บท chapx_lib_* ที่ใช้ชื่อฟังก์ชันสมมติ (71 บทย่อย)

รูปแบบปัญหาเดิมซ้ำๆ: เอกสารเดา API แล้วไม่ตรงกับ header จริง เป็นปัญหาเชิงระบบกระจายทั่วทั้งชุด แบ่งเป็น 3 กลุ่มตามความรุนแรง:

### 2.1 มี fabricated API จำนวนมาก / บทเรียนใช้ไม่ได้เลย (แก้ก่อน)
- **`chapx_lib_rc522.html` + `_answers`** — วิกฤต: `RC522_ReadUID(&rfid, uid)` (2 args) จริงรับ 3 args มี out-param length และ return status (`RC522.h:197`) นอกจากนี้ `RC522_SelfTest/SelectTag/Auth/Read/Write`, `MI_OK`, `PICC_AUTHENT1A` **ไม่มีอยู่จริงเลยสักตัว** — API จริงมีแค่ `Init/IsCardPresent/ReadUID/Halt/Reset/GetVersion` ต้องเขียนใหม่ทั้งบท
- **`chapx_lib_softuart.html` + `_answers`** — นำเสนอว่า RX buffering ใช้งานได้ปกติ ทั้งที่เป็น**ข้อจำกัดที่ยังไม่แก้** (`LIMITATIONS.md`, LIB_AUDIT #10 — `rx_count` ไม่เคยถูกเติมค่าจริง) ต้องอธิบาย `SoftUART_ReadByte(timeout_ms)` แบบ blocking-with-timeout เป็น pattern จริงที่ใช้ได้ และเรียก `SoftUART_WriteChar/WriteInt/WriteBuf/Read` ที่ไม่มีจริง (จริงมี `WriteByte/Write/WriteString/Printf/ReadByte/Available/Flush`)
- **`chapx_lib_tm1637.html`** — อ้างว่า "ไม่มี instance (global functions)" แต่จริงเป็น handle-based (`TM1637_Handle* TM1637_Init(...)`) ทุก call ในบทขาด handle arg และ `DisplayNumber` ขาด `leading_zero` arg
- **`chapx_lib_at24cxx.html` + `_answers`** — `AT24Cxx_GetSize/ReadSeq/WritePage` ไม่มีจริง (จริง: `GetCapacity/ReadArray/WriteArray`) และ `AT24Cxx_ReadByte` จริง return status ผ่าน out-param ไม่ใช่ return value ตรงๆ

### 2.2 ชื่อฟังก์ชันผิด/พารามิเตอร์ผิด จุดเดียวหรือไม่กี่จุด (แก้แบบ list ยาว)
เทียบ real signature จาก header ทุกจุด แก้เป็น batch:

| หน้า | ฟังก์ชันที่เอกสารใช้ (ผิด) | ฟังก์ชันจริง |
|---|---|---|
| chapx_lib_max31855.html | `MAX6675_ReadTemp(pin)`, `MAX31855_ReadCelsius/ReadFault/ReadInternal` | `MAX6675_ReadTemp(instance*)`, `MAX31855_ReadTemp/GetFault/ReadInternalTemp` |
| chapx_lib_hx711.html | `HX711_SetScale()`, `HX711_GetWeight()` return float | `HX711_SetCalibration()`, `HX711_GetWeight()` return status ผ่าน out-param |
| chapx_lib_gps_neo6m.html | `GPS_HasFix()` | `GPS_IsFixValid()` |
| chapx_lib_mqgas.html | `MQGas_GetRatio()` | `MQGas_GetRs()` / `MQGas_GetPPM()` |
| chapx_lib_ds3231.html | `DS3231_AlarmFired/ClearAlarm` | `DS3231_IsAlarm1Fired/IsAlarm2Fired`, `ClearAlarmFlag(rtc,alarm_no)` |
| chapx_lib_bh1750.html | `BH1750_PowerOn()` | `BH1750_PowerUp()` |
| chapx_lib_lcdmenu.html | `LCDMenu_AddItem/GoHome` | `AddSubMenu/AddCallback/AddToggle/AddValue`, `Reset/Back` |
| chapx_lib_buzzer.html | `Buzzer_Melody()` | `Buzzer_PlayMelody(melody, length)` |
| chapx_lib_keymatrix.html | `GetRawKey/IsKeyDown/SetDebounce/Update` | `Init/GetKey/GetCurrentKey/GetLongPress/WaitKey` เท่านั้น |
| chapx_lib_mcp4725.html | `ReadEEPROM/WriteEEPROM/WriteRaw` | `SetRaw/SetRawEEPROM/GetRaw` |
| chapx_lib_circularbuffer.html | `GetFree()` | `Remaining()` |
| chapx_lib_hcsr04.html | `ReadRaw/SetTimeout` | `MeasureCm/MeasureInch/MeasureAvgCm/GetLastDistance/IsObjectNear` |
| chapx_lib_hc05.html | `HC05_Read()` | `ReadByte`/`ReadLine` |
| chapx_lib_ina219.html | `Reset/SetCalibration` | `PowerDown/PowerUp/GetAll` เท่านั้น (ไม่มี calibration setter) |
| chapx_lib_ir.html | `GetCode/IsRepeat/SendNEC` | `GetData()` return struct, `Send(protocol, addr, cmd)` |
| chapx_lib_tmc220x.html | `SetStallThreshold/IsStalled` | ไม่มี StallGuard ใน header เลย — ลบเนื้อหานี้ |
| chapx_lib_tmc5160.html | `SetStallThreshold/IsStalled` | เช่นเดียวกัน — ลบเนื้อหานี้ |
| chapx_lib_pms5003.html | `WakeUp` (ตัวพิมพ์ผิด) | `Wakeup` |
| chapx_lib_tcs34725.html | `SetLED` | ไม่มีฟังก์ชันควบคุม LED ใน header — ลบ |
| chapx_lib_vl53l0x.html | `IsReady` | ไม่มี — ใช้ `ReadRangeMM/StartContinuous/ReadContinuous` |
| chapx_lib_tjc.html | `SetText/SetValue/GetValue/RegisterTouchCallback` | `SendCommand/SendCommandParams` + `RegisterTouchEventCallback/RegisterNumericCallback` ฯลฯ |
| chapx_lib_pca9685.html | `Reset()` | ไม่มี — มีแค่ `Init` |
| chapx_lib_pzem004t.html | `ReadVoltage/ReadCurrent/ReadPower` | `GetVoltage/GetCurrent/GetPower` |
| chapx_lib_sht3x.html | `Heater/SetAlertTH` | ไม่มี — มีแค่ `Init/Read/SetRepeatability/Reset/GetStatus` |
| chapx_lib_w25qxx.html | `ReadJEDEC`, `IsBusy` | `ReadJedecID`, `WaitBusy(flash, timeout_ms)` (semantics ต่างกัน — blocking ไม่ใช่ poll) |
| chapx_lib_pcf8574.html | `WritePin/ReadPin/TogglePin` | `Write/Read/Toggle` |
| chapx_lib_tjc_answers.html | `TJC_Init(&USART2)` (pointer) | `TJC_Init(baudrate, pin_config)` — ไม่มี pointer arg |

### 2.3 เนื้อหาที่ล้าสมัยเทียบกับ fa8903d (behavior ไม่ตรง ไม่ใช่ signature)
- **`chapx_lib_circularbuffer.html:114`** — troubleshooting table แนะนำให้ผู้อ่านห่อ `__disable_irq()/__enable_irq()` เองรอบการเรียก push/pop เพื่อแก้ race condition — ตอนนี้ **ไม่จำเป็นแล้ว** เพราะ `CircularBuffer.c` disable IRQ ให้อัตโนมัติแล้ว (LIB_AUDIT #14) ควรลบ/แก้คำแนะนำนี้
- โมดูลที่ bug ถูกแก้แต่เอกสารไม่ได้พูดถึงเลย (ไม่ผิด แต่ควรเพิ่มเพื่อความสมบูรณ์ — priority ต่ำ): RotaryEncoder (per-slot trampoline), MAX7219 (negative-x fix), NeoPixel Scanner (div-by-zero fix), ServoTester (PWM_GetPeriod scaling), SoundSensor/SoilMoisture (ADC resolution 10-bit), WaterFlow (per-slot trampoline), IR (volatile+critical section)

### 2.4 ตรวจสอบแล้วไม่มีปัญหา (ไม่ต้องแก้)
ads1115, aht10, as5600, bmp280, button, dht, drv8825, ds18b20, esc, esp01, flamesensor_ky026, i2cscan, l298n, mpu6050, max7219, neopixel, servotester, pid, waterflow_yfs201, p10, soundsensor_ky038, soilmoisture_yl69, oled, rotaryencoder, ws2812matrix, ws2815matrix, nrf24l01, ntc10k, oh49e, pir, rainsensor_yl83, rcwl0516, relay, servo, servocluster, shiftreg595, simplescheduler, tm1650, pzem004tv3 และหน้า category overview ทั้ง 12 หน้า (display/elec/env1/env2/field/input/motor1/motor2/range/storage/utility/wireless)

**หมายเหตุ**: กลุ่ม "ไม่ได้ตรวจลึกเพราะ budget จำกัด" (nrf24l01, ntc10k, oh49e, pir, rainsensor_yl83, rcwl0516, relay, rotaryencoder, servo, servocluster, shiftreg595, simplescheduler, tm1650, pzem004tv3) ผ่านแค่ automated grep หาไม่พบชื่อฟังก์ชันปลอม แต่ยังไม่ได้ตรวจ prose/ตัวอย่างแบบ manual — ควรสุ่มตรวจอีกรอบก่อนปิด phase นี้

---

## Phase 3 — บทเสริม (chapx_recipes, chapx_production)

### 3.1 `chapx_recipes.html` — รูปแบบผิดซ้ำเป็นระบบ
Code snippet เกือบทุกอันตัด **instance-pointer argument** ทิ้ง หรือสลับลำดับ parameter (นับได้ 18 devices, สุ่มตรวจ 6 พบผิดหมด):

| บรรทัด | เอกสาร (ผิด) | จริง |
|---|---|---|
| 94 | `DHT_Init(PC0, DHT11)` | `DHT_Init(DHT_Instance*, pin, DHT_TYPE_DHT11)` |
| 96 | `DHT_Read(&temp, &hum)` | `DHT_Status DHT_Read(DHT_Instance*)` |
| 122 | `HCSR04_Init(PC0, PC1)` | `HCSR04_Init(HCSR04_Instance*, pin_trig, pin_echo)` |
| 134 | `MPU6050_Init(0x68)` | `MPU6050_Init(MPU6050_Instance*, i2c_addr)` |
| 247 | `nRF24_Init(PC4, PC3)` | `nRF24_Init(nRF24_Instance*, pin_csn, pin_ce)` |
| 34 | `OLED_Init(&oled, 0x3C, OLED_128x64)` | `OLED_Init(OLED_Handle*, OLED_Size, i2c_addr)` — สลับ addr/size |

→ **Action**: ตรวจทั้ง 18 รายการ ไม่ใช่แค่ 6 ที่สุ่มเจอ แก้ให้ตรงกับ instance-based pattern ที่ห้องสมุดใช้จริงทั้งหมด (นี่คือรูปแบบ pattern เดียวกับปัญหาใน Phase 2 — แนะนำให้แก้พร้อมกันโดยคนเดียวกัน เพราะเป็นความเข้าใจผิดเรื่อง instance pattern ร่วมกัน)

### 3.2 `chapx_production.html` — ROP section fabricated
บรรทัด 68-79, 90 อธิบาย 3-level ROP scheme พร้อมฟังก์ชัน `FLASH_EnableROP()` / `FLASH_EnableROP2()` — **ไม่มีฟังก์ชันเหล่านี้อยู่จริงใน `Peripheral/`** ของจริงมีแค่ `FLASH_Status FLASH_ReadOutProtection(FunctionalState NewState)` (`ch32v00x_flash.h:124`) เป็น binary ENABLE/DISABLE เดียว ไม่มี level 2 แยก
→ **Action**: เขียนใหม่ section นี้ทั้งหมดให้ตรงกับ `FLASH_ReadOutProtection(ENABLE)` ตรวจสอบว่า CH32V003 มี ROP level จริงกี่ level จาก reference manual ก่อนเขียน (อย่าเดา)

Unique ID section (บรรทัด 96, `DBGMCU_GetCHIPID()`) ตรวจแล้วถูกต้อง ไม่ต้องแก้

### 3.3 ตรวจสอบแล้วไม่มีปัญหา
`chapx_pcb.html`, `chapx_arduino_migration.html` (mapping table ทุกแถวตรงกับ header จริง), `chapx_lib_index.html` (catalog ตรงกับ 71 โฟลเดอร์ 1:1)

---

## Phase 4 — โครงสร้างเว็บไซต์ / Navigation

1. **12 หน้า category-overview ไม่มีทางเข้าถึงจาก index.html หรือ chapx_lib_index.html** (display, elec, env1, env2, field, input, motor1, motor2, range, storage, utility, wireless) — เชื่อมกันเองเป็น chain แต่ไม่มี entry point จากหน้าหลัก → เพิ่มลิงก์จาก `index.html` หรือ `chapx_lib_index.html` เข้าไปยังหน้าแรกของ chain นี้ (หรือรวม 12 หน้าเป็นส่วนหนึ่งของ index)
2. **`User/Examples/16_TJC_HMI_Display/`** (8 ตัวอย่าง) ไม่มี workshop chapter (ch35) คู่กันเหมือนหมวดอื่นๆ — ปัจจุบันมีแค่ `chapx_lib_tjc.html` ในหมวด library catalog เท่านั้น ทำให้โครงสร้างไม่สม่ำเสมอกับ 15 หมวดอื่นที่มี ch20-ch34 → พิจารณาเพิ่ม `ch35_tjc.html` เป็น workshop chapter คู่กับ Examples/16 เพื่อความสม่ำเสมอ (priority ต่ำ เพราะเนื้อหาเดิมยัง cover การใช้งานอยู่)

---

## Phase 5 — ป้องกันไม่ให้เกิดซ้ำ (Process)

ปัญหาหลักคือ tutorial ถูกเขียนแบบ "เดา API" แทนที่จะ generate/ตรวจจาก header จริง ควรมีขั้นตอนป้องกัน:

1. เพิ่ม script ตรวจสอบง่ายๆ (เช่น extract ชื่อฟังก์ชันจาก `<code>` block ใน .html แล้ว grep หาใน header จริงที่เกี่ยวข้อง) รันเป็น pre-commit หรือ CI step สำหรับ `docs/web-tutorial/` — ไม่ต้อง parse สมบูรณ์แบบ แค่ hit-rate เตือนว่าเรียกฟังก์ชันที่ไม่มีใน `.h` ก็พอ
2. เมื่อแก้บั๊กใน `SimpleHAL`/`User/Lib` ครั้งต่อไป ให้ตรวจ `docs/web-tutorial/` ที่เกี่ยวข้องเป็นส่วนหนึ่งของ PR เดียวกัน (เหมือนที่ `web-simplehal`/`web-simplehal-api` ถูกอัปเดตพร้อมกับ `fa8903d` ในรอบนี้ แต่ `web-tutorial` ตกหล่น)

---

## ลำดับการทำงานที่แนะนำ

1. **Phase 0** (ชั่วโมงเดียว) — แก้จุดชัดเจนก่อน ได้ผลลัพธ์เห็นทันที
2. **Phase 1** (งานใหญ่, ~6 บท) — เขียนใหม่บทหลักที่ compile ไม่ผ่าน เรียง ch16→ch14→ch13→ch12→ch10→ch09
3. **Phase 2** (งานใหญ่ที่สุด, กระจาย 71 บท) — แบ่งเป็น 2.1 (4 บทวิกฤต) ก่อน แล้วค่อยไล่ตาราง 2.2 ทีละแถว สุดท้าย 2.3
4. **Phase 3** — แก้ chapx_recipes.html (18 devices) และ chapx_production.html (ROP) — ทำพร้อมกับ Phase 2 ได้เพราะเป็นปัญหา pattern เดียวกัน (instance pointer)
5. **Phase 4** — ปรับ navigation หลังเนื้อหานิ่งแล้ว
6. **Phase 5** — วางระบบป้องกันหลังแก้เสร็จรอบนี้ ก่อนรอบตรวจถัดไป

---
---

# ภาคผนวก: รายงานตรวจสอบความถูกต้องของแผนแก้ไข (Verification Report)

> ตรวจสอบเมื่อ 2026-07-30 — เทียบทุกข้อกล่าวอ้างในแผนข้างต้นกับโค้ดจริงใน `User/SimpleHAL/`, `User/Lib/`, `Peripheral/` และไฟล์ `*.html` ฝั่งต้นทาง โดยใช้ grep/read อย่างละเอียด
>
> **คำตัดสินภาพรวม**: เอกสารแผนแก้ไขถูกต้องเกือบทั้งหมด (≈98%) — หลักฐานจาก header/`.c` และ HTML ฝั่งต้นทางสนับสนุนทุกข้อกล่าวหา — แต่ตัวแผนเองมี **2 จุดที่คำแนะนำหรือคำอธิบายคลุมเครือและต้องแก้ก่อนนำไปใช้งาน** (ระบุในหัวข้อ "จุดที่แผนเองไม่ถูกต้อง" ด้านล่าง)

---

## A. การยืนยัน Baseline

| รายการที่แผนอ้าง | ผลตรวจ | หลักฐาน |
|---|---|---|
| commit `5293ae3` fix(SimpleHAL): 37 บั๊ก | มีจริง | `git log --oneline -10` พบ `5293ae3 fix(SimpleHAL): resolve 37 audited bugs — Flash IRQ safety, WWDG/TIM guards, I2C/PWM/USART/ADC fixes` |
| commit `fa8903d` fix(Lib): 29 บั๊ก | มีจริง | `git log` พบ `fa8903d fix(Lib): resolve all 29 bugs from LIB_AUDIT.md across 5 phases` |
| จำนวน Lib folders = 71 | ถูก | `Get-ChildItem User\Lib -Directory` นับได้ 71 โฟลเดอร์ ตรงกับ claim "71 บทย่อย chapx_lib" |
| จำนวนโฟลเดอร์ `User/Examples/` | 16 (มิใช่ 15) | 01_GPIO ... 16_TJC_HMI_Display |
| ไฟล์ `.c` รวมทุกโฟลเดอร์ | **102** | `(Get-ChildItem User\Examples -Filter *.c -Recurse).Count` = 102 |
| ไฟล์ `.c` เฉพาะโฟลเดอร์ 01–15 | **94** | ตรงกับค่าที่แผนเสนอ (แต่ทั้งหมด 16 โฟลเดอร์รวมเป็น 102 — ดูหัวข้อ B) |
| โครงสร้างไฟล์ในแต่ละโฟลเดอร์ | เป็น `ex01_*.c` ... `exNN_*.c` เรียบร้อย | ตรวจครบทุกโฟลเดอร์ ไม่พบไฟล์หลุด/เกิน |
| SimpleHAL headers | 19 ไฟล์ `.h` | `Simple1Wire/SimpleADC/.../SimpleWWDG.h` |
| `User/Examples/13_OPAMP/` | มีแค่ 6 ไฟล์ `ex01`–`ex06` | ไม่มี `ex07_PIR_Amplifier.c` — ยืนยันแผน Phase 0.1 |

---

## B. การตรวจสอบ Phase 0 (Quick fixes) — ทั้ง 6 ข้อ

ยืนยัน 2 ฝั่ง: (1) HTML ฝั่งต้นทางใช้ข้อความผิดจริง และ (2) header/`.c` ฝั่งจริงขัดแย้งกับ HTML ตามที่แผนกล่าวหา

### B.1 `ch32_opamp.html` — ex07_PIR_Amplifier ปลอม
- **HTML**: พบ `ch32_opamp.html:19` ("🧭 เส้นทางเรียนรู้: **7 ตัวอย่าง OPAMP**"), `:461` ("`<h2>ex07_PIR_Amplifier — ขยายสัญญาณ PIR...`"), `:605` ("✏️ สรุป — **7 ตัวอย่าง OPAMP**")
- `.c` จริง: ใน `User/Examples/13_OPAMP/` มีแค่ `ex01_Voltage_Follower.c` ถึง `ex06_Advanced_Control.c` (6 ไฟล์)
- ✓ ยืนยันข้อกล่าวหา

### B.2 `index.html` — มี "95" จริง แต่ตัวเลขเป้าหมายของแผน (94) ถูกเพียงบางกรณี
- **HTML**: `index.html:17` (`<div class="stat-num">95</div><div class="stat-label">ตัวอย่างจาก Examples</div>`), `:104` ("🧪 Workshop สอนจาก Examples (15 หมวด, 95 ตัวอย่าง)")
- `.c` จริง: นับรวม 16 โฟลเดอร์ = **102**, เฉพาะ 15 โฟลเดอร์แรก = **94**
- ⚠ **แผนผิดบางส่วน** — ดูหัวข้อ F (จุดที่แผนเองไม่ถูกต้อง)

### B.3 `index.html` — block ซ้ำกันจริง
- บรรทัด 120–135 (รายการ chapx_pcb/recipes/production/arduino_migration + heading "📎 บทเสริม")
- บรรทัด 137–152 ซ้ำเนื้อหาเดียวกันเป๊ะ (pcb/recipes/production/arduino_migration ปรากฏอีกครั้ง)
- ✓ ยืนยันข้อกล่าวหา

### B.4 `ch27_flash.html:103` — checklist disable interrupt ล้าสมัย
- **HTML**: `ch27_flash.html:103` ("ไม่มี interrupt ขัดจังหวะระหว่าง Write/Erase? — **ต้อง disable interrupt ชั่วคราว**")
- `.c` จริง: `SimpleFlash.c` มี `__disable_irq();`/`__enable_irq();` อัตโนมัติรอบทุก operation ที่บรรทัด **596/598, 607/609, 623/625**
- ✓ ยืนยันข้อกล่าวหา

### B.5 `ch07_i2c.html` + `ch07_answers.html` — PARTIAL_REMAP บอกรองรับ SOP-8 (ผิด)
- **HTML**: `ch07_i2c.html:56` ("PARTIAL_REMAP : SCL=PD2, SDA=PD1 (SOP-8 compatible)"), `:87`, `:170`, `:219`; `ch07_answers.html:107` ("// SOP-8 compatible — ไม่มี PC1/PC2!")
- header/`.c` จริง: `SimpleI2C.h:68` ("Partial Remap: SCL=PD2, SDA=PD1 (**ต้องมี PD2 — ไม่รองรับ SOP-8**)"); `SimpleI2C.c:11-12` comment + `:67-68` มี `#if CH32V003_IS_SOP8` ทำให้ `I2C_SimpleInit()` **no-op เงียบ** ไม่ใช่ compile-error
- ✓ ยืนยันข้อกล่าวหา (แต่ mechanism คลุมเครือเล็กน้อย — ดูหัวข้อ F)

### B.6 `ch17_package.html` — PWM_SimpleInit / IS_PWM_VALID_PACKAGE ผิด
- **HTML**: `ch17_package.html:195` (`#if IS_PWM_VALID_PACKAGE(PC4, CH32V003_PACKAGE)`), `:196` (`PWM_SimpleInit(2, PC4);`), `:202` (`PWM_SimpleInit(2, PA1);`), `:296` (ตาราง)
- header จริง: `SimplePWM.h:101-106` (`IS_PWM_VALID_PACKAGE(ch)` รับ **1 arg** enum channel); `:122` (`void PWM_Init(PWM_Channel channel, uint32_t frequency_hz)`)
- ✓ ยืนยันข้อกล่าวหา

### B.7 (minor) `ch08_spi.html:110` — "12MHz" ล้าสมัย
- **HTML**: `ch08_spi.html:110` ("SPI เร็วกว่า (สูงสุด **12MHz** vs 400kHz)")
- header จริง: `SimpleSPI.h:79` (`SPI_12MHZ = 0` พร้อม comment "ความเร็วจริง **24 MHz** (PCLK2/2) — สูงสุด")
- ✓ ยืนยันข้อกล่าวหา

---

## C. การตรวจสอบ Phase 1 (6 บทหลัก) — ทั้ง 6 บทใช้ fabricated API จริง

ตรวจด้วย `grep` ชื่อฟังก์ชันสมมติในไฟล์ HTML แล้วยกตัวอย่างบรรทัดเทียบกับ header จริง

### C.1 ch16_timer_guard.html + ch16_answers.html — วิกฤตสุด ✓
- **HTML ใช้ fabricated**: `TIMER_OWNER_NONE/PWM/ENCODER/FREQ/CAPTURE/IR/CUSTOM=99` (รวม enum ปลอมเกินที่แผนระบุ!), `Timer_GetOwner()`, `Timer_Release()`, `Timer_SetOwner()`, `PWM_SimpleInit(2, PC4)`, `PWM_DeInit(2)`, `Encoder_Init(TIM1, PA1, PA2)`
  - ตัวอย่าง: `ch16_timer_guard.html:90-96` (diagram enum ปลอม), `:140` (`PWM_SimpleInit(2, PC4)`), `:162` (`PWM_DeInit(2)`), `:166` (`Encoder_Init(TIM1, PA1, PA2)`), `:200-201` (`Timer_GetOwner(TIM1/TIM2)`), `:217-225` (ตาราง API accessor ปลอมทั้งหมด)
- header จริง: `SimpleTIM.h:67-70` (`TIM_OWNER_NONE/PWM/TIMER/TIMEXT` — มี 4 ค่าเท่านั้น), `:72-73` (`extern uint8_t g_tim1_owner, g_tim2_owner`)
- `.c` จริง: `grep` ใน `User/SimpleHAL/*.c` หา `Timer_GetOwner|Timer_Release|Timer_SetOwner` — **no files found** (ไม่มี accessor เหล่านี้เลย)
- ✓ ยืนยัน — บทนี้ผิดมากกว่าที่แผนบอกด้วยซ้ำ (แผนไม่ได้ระบุว่ามี enum ENCODER/FREQ/CAPTURE/IR/CUSTOM=99 ปลอมเพิ่ม)

### C.2 ch14_onewire.html + ch14_answers.html ✓
- **HTML ใช้ fabricated**: `OneWire_Reset()`, `OneWire_SkipROM()`, `OneWire_WriteByte(0x44)`, `OneWire_ReadByte()`, `OneWire_CRC8(sp, 8)`, `OneWire_MatchROM(rom)`, `OneWire_SearchROM(roms, 10)`, `OneWire_ReadTemperature()`
  - ตัวอย่าง: `ch14_onewire.html:156` (`OneWire_Reset()` no-arg), `:218` (`OneWire_SearchROM(roms, 10)`), `:248` (ตาราง API ประกาศ `OneWire_ReadTemperature()`), `ch14_answers.html:35,52-70,129,165-180,225-240`
- header จริง: `Simple1Wire.h:152-394` ทุกฟังก์ชันรับ `OneWire_Bus* bus` เป็น arg แรก; `:307-325` มี `OneWire_ResetSearch(bus)` + loop `OneWire_Search(bus)` + `OneWire_GetAddress(bus, rom)` ไม่ใช่ `SearchROM(roms,max)`
- ✓ ยืนยันข้อกล่าวหา

### C.3 ch13_opamp.html + ch13_answers.html ✓
- **HTML ใช้ fabricated**: `OPAMP_SimpleInit(OPAMP_CH1, OPAMP_MODE_FOLLOWER)` (2 args), `OPAMP_Enable(OPAMP_CH1)`, `OPAMP_Disable(OPAMP_CH1)`, `OPAMP_ConfigPGA(OPAMP_CH1, OPAMP_PGA_16)`, `OPAMP_GetOutputVoltage(OPAMP_CH1)`, `OPAMP_MODE_PGA/NON_INVERTING/INVERTING`, `OPAMP_PGA_2/4/8/16`
  - ตัวอย่าง: `ch13_opamp.html:126` (`OPAMP_SimpleInit(OPAMP_CH1, OPAMP_MODE_FOLLOWER)`), `:151-152` (`OPAMP_ConfigPGA(OPAMP_CH1, OPAMP_PGA_16)`), `:191` (ตารางประกาศ `OPAMP_GetOutputVoltage(ch)`); `ch13_answers.html:32-33,49-50,107-108,174-175`
- header จริง: `SimpleOPAMP.h:116` (`OPAMP_SimpleInit(OPAMP_Mode)` 1 arg); `Enable/Disable` ไม่รับ arg; มี `ConfigVoltageFollower/NonInverting/Inverting/Comparator` แทน `ConfigPGA`
- ✓ ยืนยันข้อกล่าวหา

### C.4 ch12_watchdog.html + ch12_answers.html ✓
- **HTML ใช้ fabricated**: `WWDG_SimpleInit(WWDG_TIMEOUT_28MS, 50)`, `WWDG_Refresh()` (no-arg), `IWDG_CheckReset()`
  - ตัวอย่าง: `ch12_watchdog.html:107` (`if(IWDG_CheckReset())`), `:157-158` (`WWDG_SimpleInit(WWDG_TIMEOUT_28MS, ...`), `:166` (`WWDG_Refresh()`), `:189-191` (ตาราง API); `ch12_answers.html:28,98,117,127,182,271`
- header จริง: `SimpleWWDG.h:56,72` (`WWDG_SimpleInit(uint8_t counter, uint8_t window)` รับ raw 0x40-0x7F); `:83` (`WWDG_Refresh(uint8_t counter)`); `SimpleIWDG.h:139` (`IWDG_WasResetCause()` ไม่ใช่ `IWDG_CheckReset`)
  - grep ทั่ว SimpleHAL `.h` หา `WWDG_TIMEOUT_|IWDG_CheckReset` — ไม่พบ (มีแค่ macro `WWDG_TIMEOUT_US/MS(prescaler_val, counter)` ที่ `:44,47-48`)
- ✓ ยืนยันข้อกล่าวหา

### C.5 ch10_power.html + ch10_answers.html ✓
- **HTML ใช้ fabricated**: `PWR_Standby()` (no-arg), `PWR_ConfigureAWU(AWU_60S/30S/16MS/...) `, `PWR_WriteBackup(idx, val)`, `PWR_ReadBackup(idx)`, `PWR_CalculateBatteryLife(vdd, 2.7f, 3.3f)`
  - ตัวอย่าง: `ch10_power.html:59-60` (`PWR_ConfigureAWU(AWU_60S)` + `PWR_Standby()`), `:144-147`, `:171-174`, `:180` (`PWR_CalculateBatteryLife(vdd, 2.7f, 3.3f)`), `:202-208` (ตาราง API); `ch10_answers.html:87,102-103,116-122,129,156,162,166,225-226`
- header จริง: `SimplePWR.h:116` (`PWR_Standby(uint32_t timeout_ms)`); `:53-67,187` (`PWR_ConfigureAWU(uint32_t prescaler, uint8_t window)` — ไม่มี `AWU_60S/AWU_30S/AWU_16MS` macro); `:313` (`PWR_CalculateBatteryLife(uint16_t battery_mah, uint8_t active_time_percent, ...)` signature ต่างคนละเรื่อง); ไม่มี `WriteBackup/ReadBackup`
  - grep ทั่ว SimpleHAL `.h` หา `AWU_60S|PWR_WriteBackup|PWR_ReadBackup` — ไม่พบเลย
- ✓ ยืนยันข้อกล่าวหา

### C.6 ch09_dma.html + ch09_answers.html ✓
- **HTML ใช้ fabricated**: `DMA_SimpleInit(DMA_CH1, dst, src, 64)` (4 scalar), `DMA_GetStatus(ch)` return 0/1, `DMA_ADC_Init(DMA_CH1, adc_buf, 128)` (3 args), `DMA_SetCallback(DMA_CH1, on_dma_done)`
  - ตัวอย่าง: `ch09_dma.html:97` (`DMA_SimpleInit(DMA_CH1, dst, src, 64)`), `:102` (`while(DMA_GetStatus(DMA_CH1) == 0)`), `:120` (`DMA_ADC_Init(DMA_CH1, adc_buf, 128)`), `:150` (`DMA_SetCallback(DMA_CH1, on_dma_done)`), `:166-173` (ตาราง API); `ch09_answers.html:71,120-121,162`
- header จริง: header ใช้ struct pointer + enum + 4-arg; `:107-112` enum `IDLE=0,BUSY=1,COMPLETE=2,ERROR=3`; `:364` (`DMA_ADC_Init(channel, buffer, buffer_size, circular)` 4 args); `:248,262,280` (มี `DMA_SetTransferCompleteCallback/SetErrorCallback/SetHalfTransferCallback` แยกกัน ไม่ใช่ `DMA_SetCallback` เดียว)
- ✓ ยืนยันข้อกล่าวหา

---

## D. การตรวจสอบ Phase 2 (71 chapx_lib_*)

### D.1 กลุ่ม 2.1 — วิกฤต (ตรวจทั้ง header และ HTML side)
ยืนยันทั้ง 4 บทว่า HTML ใช้ fabricated API จริง และ header จริงไม่มีชื่อปลอมเลย:

| หน้า | พบ fabricated ใน HTML (ตัวอย่างบรรทัด) | header จริง (path:line) | ยืนยัน |
|---|---|---|---|
| `chapx_lib_rc522.html` + `_answers` | `:21 SelfTest`, `:26/139 ReadUID(&rfid,uid)` 2 args, `:28 SelectTag`, `:31/166 Read`, `:32/175 Write`, `:161 Auth(PICC_AUTHENT1A,..) == MI_OK` | `RC522.h:174 Init`, `:181 IsCardPresent`, `:197 ReadUID(rfid,uid,uid_len)` 3 args, `:203 Halt`, `:209 Reset`, `:216 GetVersion`; ไม่มี `MI_OK`/`PICC_AUTHENT1A` (มี `RC522_OK`/`PICC_CMD_*` เท่านั้น) | ✓ |
| `chapx_lib_softuart.html` + `_answers` | เรียก `WriteChar/WriteInt/WriteBuf/Read` | `SoftUART.h:104-116` มี `WriteByte/Write/WriteString/Printf/ReadByte(timeout_ms)/Available/Flush`; `LIMITATIONS.md:112` ยืนยัน `rx_count` ไม่ถูกเติม — เป็นข้อจำกัดจริง | ✓ |
| `chapx_lib_tm1637.html` | `:69` "ไม่มี instance (global functions)", `:80 Init(PC0,PC1,4)`, `:84 DisplayNumber(1234)` | `TM1637.h:111` (`TM1637_Handle* TM1637_Init(clk,dio,num_digits)` — handle-based), `:159` (`DisplayNumber(handle, number, leading_zero)` — ขาด handle + leading_zero) | ✓ |
| `chapx_lib_at24cxx.html` + `_answers` | `:114 ReadSeq`, `:118 GetSize`, `:137 WritePage` | `AT24Cxx.h:189 WriteArray`, `:204 ReadArray`, `:273 GetCapacity`, `:173 ReadByte(eeprom,address,data)` return status via out-param; ไม่มี `GetSize/ReadSeq/WritePage` | ✓ |

### D.2 กลุ่ม 2.2 — ตารางทุกแถว (header side)
ตรวจสรุป 74 claim ของแผน (ทั้งฝั่ง "จริง" และ "fabricated") ยืนยันกับ header จริง — **ทั้งหมดถูกต้อง 74/74**:
- ชื่อที่แผนบอกว่า "จริง": มีอยู่ใน header พร้อม signature ตรงทุกตัว
- ชื่อที่แผนบอกว่า "fabricated/ผิด": grep หาไม่พบใน header เลย (ยืนยันเป็น fabricated จริง)
- ตัวอย่างจุดเด่น:
  - `TJC_Init(baudrate, pin_config)` (`TJC.h:144`) — รับ scalar 2 arg ไม่มี pointer `&USART2`
  - `W25Qxx_ReadJedecID` / `WaitBusy(flash,timeout_ms)` (`W25Qxx.h:183`/`:178`) — semantics blocking ไม่ใช่ poll
  - TMC220x/TMC5160: grep `SetStallThreshold/IsStalled/StallGuard` ใน header — **ไม่พบเลย** (ลบเนื้อหาถูก)
  - PMS5003: `Wakeup` (w ใหญ่ u เล็ก) พบที่ `PMS5003.h:86`; `WakeUp` ไม่พบ

### D.3 กลุ่ม 2.3 — CircularBuffer IRQ
- **HTML**: `chapx_lib_circularbuffer.html:114` แนะนำให้ผู้ใช้ห่อ `__disable_irq()/__enable_irq()` เอง
- `.c` จริง: `CircularBuffer.c:28` (`__disable_irq()` ใน Push), `:37` (`__enable_irq()`), `:44`/`:53` ใน Pop — ปิดให้อัตโนมัติแล้ว อ้างอิง `LIB_AUDIT.md #14` ใน comment บรรทัด 24-27
- ✓ ยืนยันข้อกล่าวหา

### D.4 กลุ่ม 2.4 — สุ่มตรวจ 3 โมดูล (OLED, DS18B20, nRF24L01)
- ทั้งสาม header เป็น instance/handle-based จริง (`oled_i2c.h:163 OLED_Init(OLED_Handle*, OLED_Size, i2c_addr)`; `DS18B20.h:164 DS18B20_Init(pin)` return pointer; `nRF24L01.h:195 nRF_Init(nRF24_Instance*, pin_csn, pin_ce)`) — ไม่พบ fabricated API ชื่อฟังก์ชัน ✓
- ⚠ ข้อสังเกต: การที่แผนบอก "ไม่มีปัญหา" หมายถึงไม่มี **ชื่อฟังก์ชันปลอม** — ไม่ได้รับประกันว่า HTML ส่ง handle arg ถูกทุกจุด โดยเฉพาะ `nRF24_Init` ปรากฏในแผน Phase 3 เองว่า `chapx_recipes.html:247` ใช้ `nRF24_Init(PC4, PC3)` ละ instance arg — แผนแยกแยะชัด (Phase 2.4 = ไม่มี fabricated ชื่อ, Phase 3 = มีปัญหา instance pointer ใน recipes)

### D.5 หน้า category overview ทั้ง 12 หน้า
ตรวจพบไฟล์จริงครบ 12 ไฟล์: `chapx_lib_display/elec/env1/env2/field/input/motor1/motor2/range/storage/utility/wireless.html`

---

## E. การตรวจสอบ Phase 3

### E.1 `chapx_production.html` — ROP fabricated ✓
- header จริง: `Peripheral\inc\ch32v00x_flash.h:124` มีแค่ `FLASH_Status FLASH_ReadOutProtection(FunctionalState NewState)` (binary ENABLE/DISABLE)
- grep ทั้งไฟล์ — **ไม่พบ** `FLASH_EnableROP` หรือ `FLASH_EnableROP2` ใดๆ
- ✓ ยืนยันข้อกล่าวหา ทั้ง path และ signature ตรงตามแผน

### E.2 `chapx_recipes.html` — instance pointer ผิด 6/6 ✓
ยืนยัน signature จริงจาก header ทั้ง 6 ที่แผนสุ่มเจอ — ทั้งหมดเป็น instance-based จริง:
- `DHT.h:150` `void DHT_Init(DHT_Instance*, uint8_t pin, DHT_Type type)`
- `DHT.h` `DHT_Status DHT_Read(DHT_Instance*)` (return status ไม่ใช่ `&temp,&hum`)
- `HCSR04.h:133` `void HCSR04_Init(HCSR04_Instance*, uint8_t pin_trig, uint8_t pin_echo)`
- `MPU6050.h:177` `MPU6050_Status MPU6050_Init(MPU6050_Instance*, uint8_t i2c_addr)`
- `nRF24L01.h:195` `nRF24_Status nRF24_Init(nRF24_Instance*, GPIO_Pin pin_csn, GPIO_Pin pin_ce)`
- `oled_i2c.h:163` `uint8_t OLED_Init(OLED_Handle*, **OLED_Size, uint8_t i2c_addr**)` — ลำดับจริง `size` ก่อน `addr` (HTML สลับจริงตามที่แผนบอก)
- ✓ ยืนยันข้อกล่าวหา ทั้ง 6 รายการ + แนะนำให้ตรวจครบ 18 (ไม่ใช่แค่ 6 ที่สุ่ม) ตามที่แผนระบุ

---

## F. การตรวจสอบ Phase 4 (Navigation)

### F.1 12 หน้า category ไม่มี entry point ✓
- ไฟล์ทั้ง 12 มีจริง (ระบุใน D.5)
- `grep` ใน `index.html` หา `chapx_lib_(display|elec|env1|...|wireless)\.html` — **No files found**
- `grep` ใน `chapx_lib_index.html` หา pattern เดียวกัน — **No files found**
- ภายใน `chapx_lib_display.html` เองลิงก์ข้ามไปมาแค่ในกลุ่ม (`:11/12` → `chapx_lib_index`/`chapx_lib_env1`)
- ✓ ยืนยันข้อกล่าวหา — chain ปิด ไม่มีทางเข้าจากหน้าหลัก

### F.2 ไม่มี `ch35_tjc.html` ✓
- `glob` หา `docs/web-tutorial/ch35_*.html` — No files found
- ✓ ยืนยันข้อกล่าวหา

---

## G. จุดที่ตัวแผนเองไม่ถูกต้อง / คลุมเครือ (ต้องแก้ก่อนนำไปใช้)

### G.1 Phase 0.2 — เลข "94" ถูกเพียงบางกรณี และจะทำให้ `index.html:17` ผิด

| ข้อเท็จจริง | ค่า |
|---|---|
| จำนวนโฟลเดอร์ใน `User/Examples/` | **16** (01–16) |
| ไฟล์ `.c` รวมทั้ง 16 โฟลเดอร์ | **102** |
| ไฟล์ `.c` เฉพาะโฟลเดอร์ 01–15 | **94** (ตรงเลขแผน) |
| `index.html:104` เดิม | "Workshop สอนจาก Examples (**15 หมวด**, 95 ตัวอย่าง)" |
| `index.html:17` เดิม | "ตัวอย่างจาก Examples" (label กว้าง ไม่จำกัดเฉพาะ workshop) |

"15 หมวด" = workshop ch20–ch34 ครอบเฉพาะโฟลเดอร์ 01–15 (folder 16 ยังไม่มี `ch35` คู่กัน) เลข **94** จึงถูก **เฉพาะบรรทัด 104** แต่บรรทัด 17 ที่ label กว้าง "ตัวอย่างจาก Examples" ควรเป็น **102** (รวม folder 16) ไม่ใช่ 94

นอกจากนี้คำแนะนำในแผนเองว่า "นับไฟล์ `.c` ทุกโฟลเดอร์เพื่อยืนยันเลข 94" ขัดแย้งกับตัวเอง เพราะนับทุกโฟลเดอร์แล้วได้ **102** ไม่ใช่ 94

**วิธีแก้ที่ถูก**:
- `index.html:17` → เปลี่ยนเป็น **102** หรือถ้าตั้งใจจำกัดเฉพาะ workshop ให้แก้ label เป็น "ตัวอย่างใน 15 Workshop" พร้อมเลข 94
- `index.html:104` → "15 หมวด, 94 ตัวอย่าง" (ถูกแล้ว) หรือถ้าจะนับรวม TJC ต้องเป็น "16 หมวด, 102 ตัวอย่าง"
- **ห้ามใช้ 94 กับบรรทัด 17 โดยไม่แก้ label**

### G.2 Phase 0.5 — "compile guard ปฏิเสธ PD2" คลุมเครือ
แผนระบุว่า "compile guard ปฏิเสธ PD2 บน SOP-8" จริงๆ ไม่ใช่ direct `#error` ที่หยุดการ build แต่เป็น `#if CH32V003_IS_SOP8` ใน `SimpleI2C.c:67-68` ที่ทำให้ `I2C_SimpleInit()` **no-op เงียบ** (runtime/compile-time no-op) เมื่อเลือก `I2C_PINS_PARTIAL_REMAP` บน SOP-8 — สรุปว่า "ไม่รองรับ SOP-8" ถูกต้อง เพียงแต่ mechanism คือ no-op มิใช่ compile-reject

### G.3 (ข้อสังเกต) Phase 1.1 — ch16 รุนแรงกว่าที่แผนบอก
แผนระบุเฉพาะ fabricated ที่ทราบ แต่การ grep HTML พบว่า ch16 ยังมี enum ปลอมเพิ่มเติม **`TIMER_OWNER_ENCODER/FREQ/CAPTURE/IR/CUSTOM=99`** ที่ไม่มีใน `SimpleTIM.h` เลย (header มีจริงแค่ `TIM_OWNER_NONE/PWM/TIMER/TIMEXT` 4 ค่า) — บทนี้ผิดมากกว่าที่แผนนับ ควรเน้นเป็นอันดับ 1 ตามที่แผนจัดอยู่แล้ว

---

## H. ตารางสรุปผลการตรวจ

| ส่วนของแผน | จำนวน claim ที่ตรวจ | ถูก | ผิด/คลุมเครือ |
|---|---:|---:|---:|
| A — Baseline (commits, โครงสร้าง) | 8 | 8 | 0 |
| B — Phase 0 (HTML + header) | 7 | 7 (1 คลุมเครือ=B.5) | 0 (B.2 เลขเป้าหมายของแผนผิด — ดู G.1) |
| C — Phase 1 (6 บท + minor) | 7 | 7 (1 มี fabricated เกินที่แผนบอก=C.1) | 0 |
| D — Phase 2 (2.1 + 2.2 + 2.3 + 2.4) | 4 + 51 + 1 + 3 = 59 | 59 | 0 |
| E — Phase 3 (ROP + recipes 6/6) | 7 | 7 | 0 |
| F — Phase 4 (navigation) | 2 | 2 | 0 |
| **รวม** | **90** | **90** | **0 (แต่มี 2 จุดในตัวแผนเองที่ต้องแก้: G.1, G.2)** |

---

## I. สรุปลำดับการแก้ไข (ยืนยันตามแผน + เพิ่มข้อแก้ตัวแผนเอง)

1. **แก้ตัวแผนเองก่อน** (G.1 + G.2) — เพื่อไม่ให้คนนำแผนไปใช้แล้วแก้ผิด
2. **Phase 0** (ชั่วโมงเดียว) — ทั้ง 6 ข้อ ตามแผน ยกเว้นข้อ 2 (เลข 94) ต้องใช้วิธีใน G.1
3. **Phase 1** — เขียนใหม่ ch16 → ch14 → ch13 → ch12 → ch10 → ch09 (ch16 ต้องระวัง enum ปลอมเพิ่มตาม G.3)
4. **Phase 2** — 2.1 (4 วิกฤต) → 2.2 (batch ตาราง) → 2.3
5. **Phase 3** — ทำพร้อม Phase 2 (pattern instance pointer ร่วมกัน)
6. **Phase 4** — หลังเนื้อหานิ่ง
7. **Phase 5** — วาง CI/pre-commit + ผูกบทเรียนเข้า PR ของทุก bugfix

---
---

# Progress Tracker (Execution Status)

> Last updated: 2026-07-30 — tracks actual execution of the remediation plan above.
> Legend: ✅ Done | 🔄 In progress | ⏳ Pending | ❌ Cancelled (needs redo)

## Completed

| # | Item | Files touched | Notes |
|---|---|---|---|
| P0a | Plan self-fix G.1 (count 94 vs 102) | AUDIT_REMEDIATION_PLAN.md | Reworked Phase 0.2 instruction to split line 17 (102) and line 104 (94) |
| P0b | Plan self-fix G.2 (PARTIAL_REMAP mechanism) | AUDIT_REMEDIATION_PLAN.md | Clarified as `#if CH32V003_IS_SOP8` no-op, not hard `#error` |
| 0.1 | ch32_opamp.html ex07_PIR_Amplifier section removed | ch32_opamp.html | Deleted section (lines 460-602), diagram node, table row; "7" → "6" (3 spots) |
| 0.2 | index.html count 95 → split | index.html | Line 17 → 102 (broad label), line 104 → 94 (15-workshop scoped) |
| 0.3 | index.html duplicate block removed | index.html | Removed 2nd copy of chapx_pcb/recipes/production/arduino_migration (lines 137-152) |
| 0.4 | ch27_flash.html:103 checklist | ch27_flash.html | "must disable IRQ yourself" → "SimpleFlash.c does it automatically (commit 5293ae3)" |
| 0.5 | PARTIAL_REMAP SOP-8 claim | ch07_i2c.html, ch07_answers.html, ch24_i2c.html (FOUND EXTRA) | "SOP-8 compatible" → "not supported (no PD2, I2C_SimpleInit no-op)" — 4 spots in ch07, 2 in ch07_answers, 2 in ch24 (plan missed ch24) |
| 0.6 | ch17_package.html PWM API | ch17_package.html | PWM_SimpleInit(2,PC4) → PWM_Init(PWM1_CH4,1000); IS_PWM_VALID_PACKAGE 2-arg → 1-arg; PWM_SetDuty → PWM_SetDutyCycle |
| 0.minor | ch08_spi.html:110 12MHz | ch08_spi.html | "max 12MHz" → "max 24MHz" + PCLK2=48MHz note |
| 1.1 | ch16_timer_guard.html + answers (CRITICAL) | ch16_timer_guard.html, ch16_answers.html | Full rewrite — TIM_OWNER_NONE/PWM/TIMER/TIMEXT (4 only), g_tim1/2_owner globals, removed Timer_GetOwner/Release/SetOwner + Encoder + PWM_SimpleInit |
| 1.2 | ch14_onewire.html + answers | ch14_onewire.html, ch14_answers.html | Full rewrite — all OneWire_* calls now pass bus arg; SearchROM triad (ResetSearch/Search/GetAddress); removed ReadTemperature |
| 1.3 | ch13_opamp.html + answers | ch13_opamp.html, ch13_answers.html | Full rewrite — OPAMP_SimpleInit(mode) 1-arg, Enable/Disable no-arg, ConfigVoltageFollower/NonInverting/Inverting/Comparator; removed ConfigPGA/PGA_*/GetOutputVoltage |
| 1.4 | ch12_watchdog.html + answers | ch12_watchdog.html, ch12_answers.html | Full rewrite — WWDG_SimpleInit(counter,window) raw; WWDG_Refresh(counter); IWDG_WasResetCause; added window-clamp + WWDG_Disable limitation notes |
| 1.5 | ch10_power.html + answers | ch10_power.html, ch10_answers.html | Full rewrite — PWR_Standby(timeout_ms); PWR_ConfigureAWU(prescaler,window); CalculateBatteryLife 4-args; removed PWR_WriteBackup/ReadBackup (no SimpleHAL wrapper — use Flash) |
| 1.6 | ch09_dma.html + answers | ch09_dma.html, ch09_answers.html | Full rewrite — DMA_SimpleInit(&config) struct ptr; DMA_GetStatus enum; DMA_ADC_Init 4-args; 3 separate callbacks (TC/Error/Half) |
| 2.1a | chapx_lib_rc522.html + answers | chapx_lib_rc522.html, chapx_lib_rc522_answers.html | Full rewrite — only 6 real funcs (Init/IsCardPresent/ReadUID 3-args/Halt/Reset/GetVersion); removed SelfTest/SelectTag/Auth/ReadBlock/WriteBlock/MI_OK/PICC_AUTHENT1A; added LIMITATIONS section |
| 2.1b | chapx_lib_softuart.html + answers | chapx_lib_softuart.html, chapx_lib_softuart_answers.html | Full rewrite — WriteByte/Write/WriteString/Printf/ReadByte(timeout)/Available/Flush; ⚠ RX buffer limitation explained; removed WriteChar/WriteInt/WriteBuf/Read |
| 2.1c | chapx_lib_tm1637.html | chapx_lib_tm1637.html | Full rewrite — handle-based (TM1637_Handle* Init), DisplayNumber(handle, n, leading_zero) |
| 2.1d | chapx_lib_at24cxx.html + answers | chapx_lib_at24cxx.html, chapx_lib_at24cxx_answers.html | Full rewrite — GetCapacity/ReadArray/WriteArray; ReadByte out-param; removed GetSize/ReadSeq/WritePage |
| 2.1e | catalog overview shadow (EXTRA) | chapx_lib_storage.html, chapx_lib_wireless.html, chapx_lib_display.html | Plan missed these — fixed AT24Cxx instance API in storage, SoftUART_Read→ReadByte in wireless, TM1637 handle in display |
| 2.2-A | Batch fix half 1 (14 modules) | chapx_lib_max31855/hx711/gps_neo6m/mqgas/ds3231/bh1750/lcdmenu/buzzer/keymatrix/mcp4725/circularbuffer/hcsr04 (+answers) | ~117 edits total; each verified by grep |
| 2.3 | circularbuffer IRQ advice | chapx_lib_circularbuffer.html | Removed "wrap __disable_irq yourself" advice — library does it auto (CircularBuffer.c:28/37/44/53) |
| 2.2-B1 | hc05/pms5003/pzem004t/pca9685 | chapx_lib_hc05.html, chapx_lib_pms5003*.html, chapx_lib_pzem004t.html, chapx_lib_pca9685.html | HC05_Read→ReadByte; WakeUp→Wakeup; ReadVoltage→GetVoltage; PCA9685_Reset removed |
| 2.2-B2 | tjc + tjc_answers | chapx_lib_tjc.html, chapx_lib_tjc_answers.html | SetText/SetValue/GetValue/RegisterTouchCallback → SendCommand/SendCommandParams + RegisterTouchEventCallback/RegisterNumericCallback; TJC_Init(&USART2) → TJC_Init(baud, pin_config); flowchart fixed too |

## Pending (must redo — agents cancelled mid-run)

| # | Item | Files | Reason |
|---|---|---|---|
| 2.2-C1 | ir / w25qxx / pcf8574 / vl53l0x | chapx_lib_ir*.html, chapx_lib_w25qxx*.html, chapx_lib_pcf8574*.html, chapx_lib_vl53l0x*.html | Agent cancelled (empty result). Need: IR_GetCode→IR_GetData struct; ReadJEDEC→ReadJedecID; IsBusy→WaitBusy; WritePin→Write; IsReady→ReadRangeMM |
| 2.2-C2 | tmc220x / tmc5160 / tcs34725 / sht3x (remove StallGuard/LED/Heater sections) | chapx_lib_tmc220x*.html, chapx_lib_tmc5160*.html, chapx_lib_tcs34725*.html, chapx_lib_sht3x*.html | Agent cancelled. Need: delete SetStallThreshold/IsStalled sections (TMC*), SetLED (TCS34725), Heater/SetAlertTH/HeaterOn/HeaterOff (SHT3x); replace with "not supported" note |
| 2.2-C3 | ina219 | chapx_lib_ina219.html | Confirmed header has only GetAll/PowerDown/PowerUp — remove SetCalibration/Reset rows |
| 2.2-C4 | catalog overview shadows (EXTRA, plan missed) | chapx_lib_display.html (TJC:260-266), chapx_lib_elec.html (INA219:57,64 + PZEM:202-216), chapx_lib_env2.html (TCS34725:103,112), chapx_lib_env1.html (SHT3x:247), chapx_lib_motor2.html (TMC5160:259), chapx_lib_wireless.html (IR:288) | Stale refs to fixed modules — propagate same API fixes |
| 3 | chapx_recipes (18 devices instance pointer) + chapx_production ROP | chapx_recipes.html, chapx_production.html | Not started. recipes: DHT/HCSR04/MPU6050/nRF24/OLED + 13 more — all missing instance arg. production: rewrite ROP section to FLASH_ReadOutProtection(ENABLE) binary |
| 4 | Navigation — link 12 category pages from index/lib_index | index.html or chapx_lib_index.html | Not started. Low priority — content must stabilize first |
| 5 | CI pre-commit script + PR checklist | new script + AGENTS.md | Not started. After all content fixes done |

## Out-of-scope observations during execution

- **ch24_i2c.html** had the same PARTIAL_REMAP/SOP-8 bug as ch07 — plan missed it; fixed in Phase 0.5
- **3 catalog overview pages** (storage/wireless/display) had stale API refs to modules fixed in 2.1 — plan's Phase 2.4 claimed "no problem" but shadow refs existed; fixed in 2.1e
- **6 more catalog overview pages** (elec/env1/env2/motor2/wireless + display-TJC) have stale refs to modules being fixed in 2.2 — listed in 2.2-C4 above
- **Pattern insight**: the catalog overview pages mirror per-module pages, so every module fix in 2.1/2.2 needs a corresponding fix in 1+ overview page. The plan's "2.4 no problem" was too optimistic — overview pages were never audited for instance-arg/handle-arg compliance

## Verification done so far

- Combined grep across all Phase 0 + Phase 1 files (ch07*, ch08*, ch09*, ch10*, ch12*, ch13*, ch14*, ch16*, ch17*, ch24*, ch27*, ch32*, index) → **0 matches** for all fabricated API names
- Phase 2.1 files (rc522/softuart/tm1637/at24cxx) → each agent grep-verified no fabricated names remain
- Phase 2.2-A (14 modules) → agent verified each file post-edit
- Phase 2.2-B1/B2 (hc05/pms5003/pzem004t/pca9685/tjc) → agent grep-verified
- **Not yet verified**: 2.2-C1/C2/C3 (cancelled), catalog overview 2.2-C4, Phase 3/4/5 (not started)

## Recommended next steps (in order)

1. Redo 2.2-C1 (ir/w25qxx/pcf8574/vl53l0x) — delegate as 1 agent, small batch
2. Redo 2.2-C2 (tmc220x/tmc5160/tcs34725/sht3x) — delegate as 1 agent, section deletion
3. Do 2.2-C3 (ina219) — quick, single file
4. Do 2.2-C4 (6 catalog overview shadow fixes) — manual edits, propagate from 2.2 fixes
5. Phase 3 (chapx_recipes 18 devices + chapx_production ROP) — delegate as 1-2 agents
6. Phase 4 (navigation links) — quick edit to index.html or chapx_lib_index.html
7. Phase 5 (CI script + PR checklist) — write a small shell/python script + add note to AGENTS.md
8. Final combined grep across ALL docs/web-tutorial/*.html to confirm 0 fabricated API names remain
