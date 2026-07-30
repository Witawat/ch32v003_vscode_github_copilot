# web-simplehal — แผนแก้ไข (Audit Remediation Plan)

ตรวจสอบเมื่อ 2026-07-30 โดยเทียบ `docs/web-simplehal/*.html` (20 ไฟล์: ch00–ch19 + answers + index) กับโค้ดจริงใน
`User/SimpleHAL/*.h/.c` (19 header) โดยตรวจทีละบทแบบละเอียด (ไม่ใช่แค่ grep ชื่อฟังก์ชัน) รวมถึงตรวจ arity, ลำดับ parameter, และตารางอ้างอิงตัวเลข/พิน

**สรุปภาพรวม**: รูปแบบปัญหาเด่นชัดคือ **ไฟล์เนื้อหาหลัก (ch0X-xxx.html) เขียนโค้ดตัวอย่างแบบ "เดา API" โดยไม่ตรงกับ header จริง** ในขณะที่ **ไฟล์ `_answers.html` เกือบทั้งหมดถูกต้อง** (คนละคน/คนละรอบเขียนกัน และ answers ผ่านการตรวจจริงมากกว่า) ปัญหาแบ่งเป็น 3 ระดับความรุนแรง:

1. **จุดเดียว/ชื่อเดียวซ้ำหลายไฟล์** (เช่น `USART_SimpleInit`) — แก้แบบ find-replace ได้เกือบทั้งหมด
2. **arity/signature ผิดกระจายในไฟล์เดียว** — ต้องอ่านทั้งไฟล์แต่ไม่ต้องเขียนใหม่ทั้งบท
3. **ทั้งบทเขียนผิด pattern พื้นฐาน** (ch17-1wire ไม่มี `bus` handle เลยทั้งบท) — ต้องเขียนใหม่ทั้งบท

---

## Phase 0 — Quick fixes ซ้ำข้ามไฟล์ (ผลกระทบสูงสุด ใช้เวลาน้อยสุด ทำก่อน)

บั๊กเดียวกันซ้ำในหลายไฟล์ แก้แบบ pattern เดียวได้ทีเดียวหลายจุด:

1. **`USART_SimpleInit` ผิด arity/ชื่อ constant — เกิดใน ch01, ch05, ch06, ch07, ch08, ch11, ch12 (7 ไฟล์)**
   - รูปแบบผิดที่พบ: `USART_SimpleInit(USART_BAUD_115200)` (ch01), `USART_SimpleInit(USART_115200)` (ch05, ch06, ch07, ch08, ch11, ch12) — ทั้งสองแบบเป็น constant ที่ไม่มีอยู่จริง และเรียกด้วย 1 arg
   - ของจริง (`SimpleUSART.h`): `void USART_SimpleInit(USART_BaudRate baud, USART_PinConfig pin_config)` — ต้องเป็น `USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT)`
   - Reference ที่ถูกอยู่แล้ว: `ch02_answers.html`, `ch04_answers.html`, `ch10_answers.html`, `ch11_answers.html`, `ch12_answers.html` ใช้รูปแบบถูกต้องอยู่แล้ว — ใช้เป็นต้นแบบตอน copy-fix
   - **Action**: grep `USART_SimpleInit(USART_` ทั้งโฟลเดอร์ แก้เป็น `USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT)` ทุกจุด (รวมในตาราง/prose ที่พูดถึงชื่อ constant ด้วย ไม่ใช่แค่ใน `<code>` block)

2. **`ADC_Read(PA2)` / `ADC_Read(PA1)` ใช้ GPIO pin constant แทน ADC channel enum — เกิดใน ch10, ch11**
   - ปัญหา: `PA2` (จาก `SimpleGPIO.h`) กับ `ADC_CH_1` (จาก `SimpleADC.h`) มีค่าตัวเลขบังเอิญใกล้กันแต่คนละ enum คนละความหมาย — ถ้าใส่ `PA2` ตรงๆ จะอ่านผิด channel แบบเงียบๆ (ไม่ compile error แต่ผลลัพธ์ผิด — อันตรายกว่าบั๊ก compile-fail เพราะตรวจจับยาก)
   - ของจริงที่ถูก: `ADC_Read(ADC_CH_PA2)` หรือ `analogRead(PA2)` (macro คนละตัวที่แปลง pin→channel ให้อัตโนมัติ) — ดู `ch10_answers.html`/`ch11_answers.html` ที่ใช้ถูกอยู่แล้ว
   - **Action**: หาทุกจุดที่เรียก `ADC_Read(` ตามด้วย GPIO pin constant (`PA*`/`PC*`/`PD*`) โดยตรง แก้เป็น `ADC_CH_PA2`/`ADC_CH_PA1` ที่ตรงกับ pin จริง หรือเปลี่ยนไปใช้ `analogRead(pin)` แทนถ้าต้องการ API ระดับสูง

3. **`ADC_Read()` เรียกโดยไม่มี arg — เกิดใน ch18, ch19**
   - ของจริง: `uint16_t ADC_Read(ADC_Channel channel)` ต้องมี channel เสมอ — ตรวจดูบริบทแต่ละจุดว่าตั้งใจอ่าน channel ไหนแล้วเติม arg ให้ถูก

4. **`ADC_CH0`/`ADC_CH3`/`ADC_CH7` (ไม่มี underscore) — เกิดใน ch08**
   - ของจริง: `ADC_CH_0`, `ADC_CH_3`, `ADC_CH_7` (มี underscore คั่น) — แก้แบบ find-replace เติม `_` หลัง `CH`

5. **ตาราง `ch15-iwdg.html` timeout ผิดหลักสิบเท่า (บรรทัด ~70-79)**
   - ตารางปัจจุบัน: 25.6ms, 51.2ms, 102.4ms, 204.8ms, 409.6ms, 819.2ms, 1638ms
   - ค่าจริงจาก `SimpleIWDG.h` (บรรทัด 158-169): 512ms, 1024ms, 2048ms, 4096ms, 8192ms, 16384ms, 32768ms
   - หน้าเดียวกันมีประโยค "Timeout: 1ms – 32.7 วินาที" ซึ่ง**ตรงกับค่าจริง** แต่ขัดแย้งกับตารางของตัวเอง — ใช้ประโยคนี้ยืนยันว่าต้องแก้ตาราง ไม่ใช่แก้ประโยค
   - **Action**: แทนที่ตารางทั้งหมดด้วยค่าจาก header โดยตรง

6. **ตาราง PVD levels `ch18-pwr.html` ชื่อผิด (บรรทัด ~106-111)**
   - ผิด: `PVD_2_9V`, `PVD_3_3V`, `PVD_3_7V`, `PVD_4_4V`
   - ถูก (`SimplePWR.h:39-46`): `PWR_PVD_2V9`, `PWR_PVD_3V3`, `PWR_PVD_3V7`, `PWR_PVD_4V4`

7. **ตาราง AWU prescaler `ch18-pwr.html` มีค่าที่ไม่มีจริง (บรรทัด ~95-100)**
   - ผิด: `/1`, `/160`, `/1600`, `/61440`
   - ถูก (`SimplePWR.h:52-67,320-336`): ชุดจริงคือ 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 10240, 61440 — แทนที่ตารางทั้งชุด

8. **macro ชื่อผิด `ch19-arduino.html:100`**: `USART_PRINT_EXTENSIONS` ไม่มีจริง ของจริงคือ `ENABLE_USART_PRINTLN` / `ENABLE_USART_PRINTFLOAT` (`SimpleArduino.h:199-205`)

**ตรวจสอบเพิ่มเติมก่อนปิด phase**: หลัง grep-replace ข้อ 1-4 แล้ว ให้ grep ซ้ำอีกรอบเพื่อยืนยันไม่เหลือ `USART_115200`, `USART_BAUD_115200`, `ADC_CH0`/`ADC_CH3`/`ADC_CH7` (ไม่มี underscore) หลงเหลือในทั้งโฟลเดอร์

---

## Phase 1 — จุดเดียว/ไม่กี่จุดต่อไฟล์ แต่ไม่ซ้ำแพทเทิร์นข้ามไฟล์ (แก้เป็นไฟล์ต่อไฟล์)

### 1.1 `ch02-gpio.html` — GPIO batch functions ผิด signature ทั้งหมด
- `pinModeMultiple(PORT_C, 0x0F, PIN_MODE_OUTPUT)` (บรรทัด 128) — ของจริงคือ macro 2-arg `pinModeMultiple(pins_array, mode)` รับ array ของ pin ไม่ใช่ port+mask
- `digitalWriteMultiple(PORT_C, 0x0F, 0x05)` (บรรทัด 129) — ของจริง 2-arg `digitalWriteMultiple(pins_array, values_array)` (สอง array ไม่ใช่ port+mask+value)
- `portWrite(PORT_C, 0xAA)` (บรรทัด 130), `portRead(PORT_C)` (บรรทัด 131) — `PORT_C` ไม่มีอยู่จริงในโค้ดเลย ของจริงรับ `GPIO_TypeDef*` เช่น `GPIOC`
- **Action**: เขียนตัวอย่างใหม่ตามแพทเทิร์นที่ `ch02_answers.html` ใช้อยู่แล้ว (`pinModeMultiple(leds, PIN_MODE_OUTPUT)` โดย `leds` เป็น array)

### 1.2 `ch03-delay.html` — `Start_Timer`/`Reset_Timer` ขาด arg
- `Start_Timer(timer, ms)` เรียก 2 args ทุกจุด (บรรทัด 90, 106, 145, 150, 221-223) — ของจริง `void Start_Timer(Timer_t *timer, uint32_t ms, uint8_t repeat)` ต้องมี 3 args
- `Reset_Timer(timer)` เรียก 1 arg (บรรทัด 92-93) — ของจริงต้องการ 2 args `Reset_Timer(Timer_t *timer, uint8_t repeat)`
- **Action**: เติม arg `repeat` (0 หรือ 1 ตามบริบทตัวอย่าง) ทุกจุดเรียก — ใช้ `ch03_answers.html` เป็นต้นแบบ (มี `Start_Timer(&t1, 100, 1)` ถูกอยู่แล้ว)

### 1.3 `ch04-usart.html` — คำอธิบาย/parameter ผิดความหมาย (ไม่ผิด arity แต่เข้าใจผิด)
- `USART_PrintHex(0xAB, 2)` (บรรทัด 116) — arg ที่ 2 จริงคือ `uint8_t uppercase` (ค่า boolean 0/1) ไม่ใช่จำนวนหลัก (digits) ต้องเขียนคำอธิบายใหม่ ไม่ใช่แค่เปลี่ยนตัวเลข
- `USART_Available()` (บรรทัด 117) อธิบายว่าคืน "จำนวน byte" — จริงคืนแค่ 1 (มีข้อมูล) หรือ 0 (ไม่มี) ตาม header comment
- **Action**: แก้คำอธิบาย 2 จุดนี้ ฟังก์ชันอื่นในไฟล์นี้ (`USART_SimpleInit` นับรวมใน Phase 0 แล้ว, `USART_Print`, `USART_WriteByte`, `USART_PrintNum`, `USART_Read`, `USART_ReadBytes`, `USART_Flush`, `USART_RxByteHook`) ตรวจแล้วถูกต้อง ไม่ต้องแตะ

### 1.4 `ch05-i2c.html` — ตาราง pin remap ผิด
- ตาราง (บรรทัด ~64): `I2C_PINS_PARTIAL_REMAP` ระบุ `SCL=PD0, SDA=PD1` — ผิด ของจริง (`SimpleI2C.h`) คือ `SCL=PD2, SDA=PD1` (ค่า PD0/PD1 จริงๆ คือของ `I2C_PINS_REMAP` คนละตัว)
- **Action**: แก้ตัวเลขในตารางให้ตรงกับ 3 โหมด remap จริงใน header (Default / Remap / PartialRemap)

### 1.5 `ch06-i2c-soft.html` — arity ผิด 3 จุด
- `I2C_Soft_Init(PA1, PA2)` (บรรทัด 98, 127) — ขาด arg ที่ 3 `speed` ของจริง `I2C_Soft_Init(uint8_t scl_pin, uint8_t sda_pin, I2C_Soft_Speed speed)`
- `I2C_Soft_Read(0x50, 0x00, &val2, 1)` (บรรทัด 107) — เรียก 4 args (addr, reg, buf, len) แต่ของจริง 3 args `I2C_Soft_Read(uint8_t addr, uint8_t* data, uint16_t len)` ไม่มี register byte แยก (ต้องเขียน reg ด้วย `I2C_Soft_WriteByte`/`I2C_Soft_Write` ก่อนแล้วค่อยเรียก Read)
- `I2C_Soft_ReadByte()` (บรรทัด 174, 176) — ของจริงต้องมี arg `ack`: `I2C_Soft_ReadByte(uint8_t ack)`
- **Action**: เขียนตัวอย่างการอ่านแบบมี register ใหม่ทั้งหมด (แยก write-register แล้วค่อย read ตาม pattern I2C bus จริง)

### 1.6 `ch07-spi.html` — arity + constant ปลอม
- `SPI_SimpleInit(SPI_CLOCK_DIV_16, SPI_PINS_DEFAULT)` (บรรทัด 114, 154, 175) — `SPI_CLOCK_DIV_16/4/2` ไม่มีอยู่จริงเลย และของจริงต้องการ 3 args `SPI_SimpleInit(SPI_Mode mode, SPI_Speed speed, SPI_PinConfig pin_config)` (ขาด `mode`, ชื่อ speed enum จริงคือ `SPI_1MHZ` ฯลฯ)
- `SPI_SetCSPin(MAX7219_CS)` (บรรทัด 115, 155) — 1 arg แต่ของจริงต้องการ 2 args `SPI_SetCSPin(GPIO_TypeDef* port, uint16_t pin)`
- **Action**: เขียนใหม่ทุกจุดเรียก `SPI_SimpleInit`/`SPI_SetCSPin` ให้ตรง signature จริง อ้างอิงชื่อ speed enum จาก `SimpleSPI.h` โดยตรง (อย่าเดา)

### 1.7 `ch08-adc.html` — arity ผิดหลายฟังก์ชัน (ชื่อ constant แก้ไปแล้วใน Phase 0 ข้อ 4)
- `ADC_SimpleInit(ADC_CH0)` (บรรทัด 119) — ของจริง `ADC_SimpleInit(void)` ไม่รับ arg เลย
- `ADC_ToVoltage(raw)` (บรรทัด 123) — ของจริงต้องการ 2 args `ADC_ToVoltage(uint16_t adc_value, float vref)`
- `ADC_GetBatteryPercent(LIION_42V)` (บรรทัด 141) — `LIION_42V` ไม่มีจริง ของจริง 3 args `ADC_GetBatteryPercent(float vdd, float v_min, float v_max)`
- คำอธิบาย `analogReadMilliVolts` (บรรทัด 192-193) ผิด — จริงเป็น macro คำนวณตรงจาก `analogRead()*3300/1023` ไม่ได้เรียก `ADC_ReadVoltage()`
- **Action**: แก้ทั้ง 4 จุดตาม signature จริงใน `SimpleADC.h`

### 1.8 `ch09-pwm.html` — ตาราง pin mapping ผิด (function call ถูกหมดแล้ว)
- ตาราง (บรรทัด 63-70): ผิด 7 ใน 8 แถว — ของจริง (`SimplePWM.h`): PWM1_CH1=PD2 (ถูกอยู่แล้ว), PWM1_CH2=PA1, PWM1_CH3=PC3, PWM1_CH4=PC4, PWM2_CH1=PD4, PWM2_CH2=PD3, PWM2_CH3=PC0, PWM2_CH4=PD7
- **Action**: แทนที่ตารางทั้งชุดด้วยค่าจาก header ตรงๆ (เป็นแค่ตาราง ไม่กระทบโค้ดตัวอย่าง)

### 1.9 `ch10-opamp.html` — arity/return-type/argument-order ผิดหลายฟังก์ชัน
- `OPAMP_Init(CHP0, CHN0, OPAMP_MODE_NON_INVERTING)` (บรรทัด 144) — ของจริง 2 args เท่านั้น ไม่มี mode param: `OPAMP_Init(OPAMP_Channel_Positive pos_channel, OPAMP_Channel_Negative neg_channel)`
- `uint32_t cfg = OPAMP_GetConfig();` (บรรทัด 143) — ของจริง return `void` และรับ 2 out-pointer args: `OPAMP_GetConfig(OPAMP_Channel_Positive* pos, OPAMP_Channel_Negative* neg)`
- `OPAMP_ConfigVoltageFollower()`, `ConfigNonInverting()`, `ConfigInverting()`, `ConfigComparator()` เรียกไม่มี arg (บรรทัด 146-149) — ของจริงต้องการ 1 arg (VoltageFollower) หรือ 2 args (อีก 3 ตัว)
- `OPAMP_CalculateR2NonInv(2.0, 10000)` / `CalculateR2Inv(2.0, 10000)` (บรรทัด 152-153) — **สลับลำดับ arg** ของจริงคือ `(uint32_t r1, float desired_gain)` เอา r1 ก่อน gain — อันตราย เพราะ compile ผ่านแต่ผลลัพธ์ผิด
- `ADC_Read(PA2)`/`ADC_Read(PA1)` — รวมอยู่ใน Phase 0 ข้อ 2 แล้ว
- **Action**: เขียนใหม่ section การเรียกใช้ OPAMP ทั้งหมด อ้างอิง `ch10_answers.html` ที่ถูกอยู่แล้วเป็นต้นแบบ

### 1.10 `ch11-tim.html` — arity ผิด + struct ปลอม
- `TIM_SimpleInit(TIM_2)` (บรรทัด 88, 132, 170, 202, 253) — ของจริงต้องการ 2 args `TIM_SimpleInit(TIM_Instance timer, uint32_t frequency_hz)`
- `TIM_AdvancedInit(TIM_1, &cfg)` (บรรทัด 98) — ไม่มี config struct/pointer API จริง ของจริงรับ 4 scalar args `TIM_AdvancedInit(TIM_Instance timer, uint16_t prescaler, uint16_t period, TIM_Mode mode)`
- `ADC_Read(PA2)` — รวมอยู่ใน Phase 0 ข้อ 2 แล้ว
- **Action**: เขียนใหม่ทุกจุดเรียก `TIM_SimpleInit`/`TIM_AdvancedInit` อ้างอิง `ch11_answers.html` เป็นต้นแบบ

### 1.11 `ch18-pwr.html` — ผิดหลายจุด (ตารางแก้ใน Phase 0 ข้อ 6-7 แล้ว)
- `float days = PWR_CalculateBatteryLife(1000,1,5,5); // ~200 days` (บรรทัด 250-252) — return type จริงคือ `uint32_t` **ชั่วโมง** ไม่ใช่ float วัน ด้วย input นี้ผลจริงคือ ~1000 ชั่วโมง (≈41.7 วัน) ไม่ใช่ ~200 วัน — ต้องแก้ทั้งชนิดตัวแปรและคำอธิบายผลลัพธ์
- `ADC_Read()` ไม่มี arg (บรรทัด 184, 218) — รวมอยู่ใน Phase 0 ข้อ 3 แล้ว
- **Action**: แก้ตัวอย่างคำนวณ battery life ใหม่ทั้งหมด คำนวณผลลัพธ์จริงจาก `SimplePWR.c` ก่อนใส่ตัวเลขในเอกสาร (อย่าเดา)

### 1.12 `ch19-arduino.html` — arity ผิด 2 ฟังก์ชัน (macro แก้ใน Phase 0 ข้อ 8 แล้ว)
- `USART_PrintlnHex(num)` (บรรทัด 106) — ของจริงต้องการ 2 args `(uint32_t num, uint8_t uppercase)`
- `USART_PrintFloat(num)` / `USART_PrintlnFloat(num)` (บรรทัด 107-108) — ของจริงต้องการ 2 args `(float val, uint8_t decimal_places)`
- `ADC_Read()` ไม่มี arg (บรรทัด 124, 173) — รวมอยู่ใน Phase 0 ข้อ 3 แล้ว
- **Action**: เติม arg ที่ขาดทุกจุด

---

## Phase 2 — ผิดหลายฟังก์ชันที่เกี่ยวโยงกัน ต้อง refactor ทั้ง section (ยากกว่า Phase 1 เพราะ semantics เปลี่ยน ไม่ใช่แค่เติม arg)

### 2.1 `ch12-tim-ext.html` — out-parameter pattern เข้าใจผิดทั้งบท
- `Time_t t = Stopwatch_GetTime();` (บรรทัด 103) และ `Time_t r = Countdown_GetTime();` (บรรทัด 121) — เขียนเป็น return-by-value แต่ของจริงเป็น out-pointer ทั้งคู่: `void Stopwatch_GetTime(Time_t* time)` / `void Countdown_GetTime(Time_t* time)`
- `Stopwatch_GetTimeString(buf, format)` / `Countdown_GetTimeString(buf, fmt)` (บรรทัด 106, 122, 170, 198, 235, 286, 366) — ขาด arg ที่ 3 `mode` ทุกจุด: ของจริง `(char* buffer, TimeFormat_t format, TimeDisplayMode_t mode)`
- `Countdown_Init(t)` โดย `t` เป็น `Time_t` struct (บรรทัด 114) — ของจริงรับ 3 scalar args `Countdown_Init(uint16_t hours, uint8_t minutes, uint8_t seconds)` ไม่มี struct overload
- `Time_ToString(time, buf, size, mode, format)` (บรรทัด 132) — ของจริง 4 args ไม่มี `size` และลำดับ format/mode สลับกับที่เอกสารเขียน: `Time_ToString(Time_t* time, char* buffer, TimeFormat_t format, TimeDisplayMode_t mode)`
- `Time_FromSeconds(seconds, mode)` (บรรทัด 133) — ของจริง 3 args ขาด output pointer: `Time_FromSeconds(uint32_t total_seconds, Time_t* time, TimeDisplayMode_t mode)`
- **Action**: เขียนใหม่ทั้งบท (ยกเว้นส่วน USART ที่แก้ใน Phase 0 ข้อ 1 แล้ว) อ้างอิง `ch12_answers.html` ที่ใช้ 3-arg `GetTimeString` และ `Countdown_InitFromSeconds` ถูกต้องอยู่แล้วเป็นต้นแบบหลัก — เพราะทุกฟังก์ชันในบทนี้เกี่ยวโยงกันเป็น pattern เดียว (out-pointer + mode enum) จึงควรแก้พร้อมกันทีเดียว ไม่ใช่ทีละจุด

### 2.2 `ch13-dma.html` + `ch13_answers.html` — struct-config pattern เข้าใจผิด
- `DMA_SimpleInit(DMA_CH1)` (บรรทัด ~93) — ของจริงรับ struct pointer เดียว `void DMA_SimpleInit(DMA_Config_t* config)` ไม่ใช่ channel enum ตรงๆ — ต้องสอนวิธี populate struct ก่อน
- `DMA_MemCopyAsync(dst, src, size, cb)` (บรรทัด ~119) — ของจริงไม่มี callback param เลย: `void DMA_MemCopyAsync(DMA_Channel channel, void* dst, const void* src, uint16_t size)` — ต้องอธิบายว่า "เช็คเสร็จ" ทำผ่าน `DMA_GetStatus`/callback แยกที่ set ไว้ก่อนหน้า ไม่ใช่ pass เข้าไปตอนเรียก
- `DMA_USART_GetReceivedCount()` (บรรทัด ~144) — ของจริงต้องการ 2 args `(DMA_Channel channel, uint16_t buffer_size)`
- **`ch13_answers.html` บรรทัด ~129**: เรียก `DMA_USART_Transmit(DMA_CH2, msg, MSG_SIZE)` โดยไม่เคยเรียก `DMA_USART_InitTx()` ก่อน — ตาม `SimpleDMA.c:445-460` ฟังก์ชัน Transmit แค่ rewrite `MADDR`/`CNTR` แล้ว start channel ที่ config ไว้แล้วเท่านั้น ไม่ได้ตั้งค่า peripheral address/direction/`USART_DMACmd` ให้ — ถ้าไม่เรียก InitTx ก่อน ตัวอย่างจะไม่ทำงานจริงตามที่อธิบาย
- **Action**: เขียนใหม่ทั้ง 2 ไฟล์ ตรวจ `SimpleDMA.h/.c` ให้ครบทุก field ของ `DMA_Config_t` ก่อนเขียนตัวอย่าง init struct และเพิ่มการเรียก `DMA_USART_InitTx()` ที่ขาดใน answers

---

## Phase 3 — ทั้งบทเขียนผิด pattern พื้นฐาน (ยากสุด ต้องเขียนใหม่ทั้งไฟล์)

### 3.1 `ch17-1wire.html` — ไม่มี `bus` handle เลยทั้งบท (วิกฤตสุดในชุดนี้)
ทุกฟังก์ชันใน `Simple1Wire.h` ต้องรับ `OneWire_Bus* bus` เป็น arg แรกเสมอ (ได้มาจาก `OneWire_Init()`) แต่ทั้งบทไม่เคยเก็บหรือส่ง handle นี้เลยแม้แต่จุดเดียว ทำให้โค้ดทั้งบทคอมไพล์ไม่ผ่าน — รุนแรงกว่าปัญหาทุกบทก่อนหน้าเพราะเป็น pattern พื้นฐานที่ผิดซ้ำทุกบรรทัด ไม่ใช่ arg ขาดจุดเดียว:
- `OneWire_Init(GPIOA, GPIO_Pin_1)` (บรรทัด 156, 208, 262) — ของจริงคือ `OneWire_Bus* OneWire_Init(uint8_t pin)` รับ SimpleGPIO pin เดียว (เช่น `PA1`) ไม่ใช่ `(port, pin)` แยก และ **ต้องเก็บค่า return เป็น handle** เพื่อใช้ต่อ
- `OneWire_Reset()`, `OneWire_SkipROM()`, `OneWire_WriteByte(0x44)`, `OneWire_ReadByte()`, `OneWire_VerifyCRC(raw,9)`, `OneWire_ResetSearch()`, `OneWire_Select(roms[i])` (บรรทัด 159-299 และตาราง API) — ทุกตัวขาด `bus` เป็น arg แรก
- `OneWire_Search(roms[count])` (บรรทัด 214, 268) — ของจริง `bool OneWire_Search(OneWire_Bus* bus)` ไม่มี output buffer ในตัว ต้องเรียก `OneWire_GetAddress(bus, rom)` แยกหลังจากนั้นเพื่อดึงค่า
- `OneWire_GetAddress(idx)` ในตาราง API (บรรทัด 103) — ของจริง `void OneWire_GetAddress(OneWire_Bus* bus, uint8_t* rom)` รับ bus handle + output buffer ไม่ใช่ index
- **Action**: เขียนใหม่ทั้งไฟล์ตั้งแต่ต้น — เริ่มจากอธิบาย `OneWire_Bus* bus = OneWire_Init(pin);` เป็นขั้นตอนแรกที่ทุก section ต้องอ้างถึง แล้วไล่แก้ทุกฟังก์ชันให้ส่ง `bus` เป็น arg แรกตาม `Simple1Wire.h` จริงทุกจุด รวมถึงแก้ loop การ search ROM ให้ใช้ `OneWire_ResetSearch(bus)` + `while(OneWire_Search(bus)) { OneWire_GetAddress(bus, rom); ... }` แทน `OneWire_Search(roms[count])` เดิม

---

## ลำดับการทำงานที่แนะนำ (ง่าย → ยาก)

1. **Phase 0** (ต่ำกว่า 1 ชั่วโมง) — 8 ข้อ find-replace ซ้ำข้ามไฟล์ ผลกระทบสูงสุดต่อเวลาที่ใช้น้อยที่สุด ทำก่อนเสมอ
2. **Phase 1** (ครึ่งวัน) — ไล่ทีละไฟล์ตามลำดับ 1.1 → 1.12 แต่ละไฟล์แก้ได้อิสระ ไม่ผูกกัน จึงแบ่งงานคู่ขนานได้
3. **Phase 2** (1 วัน) — `ch12-tim-ext.html` และ `ch13-dma.html`+`ch13_answers.html` ต้องอ่านทั้งไฟล์และเข้าใจ pattern (out-pointer / struct-config) ก่อนแก้ ไม่ใช่แค่เติม arg
4. **Phase 3** (ยากสุด, เขียนใหม่ทั้งบท) — `ch17-1wire.html` เพียงไฟล์เดียวแต่ใช้เวลามากสุดเพราะต้องรื้อโครงสร้างทั้งบท
5. **หลังแก้เสร็จทุก phase**: เทียบกับ `ch03_answers.html`, `ch10_answers.html`, `ch11_answers.html`, `ch12_answers.html` อีกรอบเพื่อยืนยันว่าไฟล์เนื้อหาหลักกับ answers สอดคล้องกัน (สอนอะไร ตอบแบบนั้น)
6. **ป้องกันไม่ให้เกิดซ้ำ**: เอกสารชุดนี้มีปัญหารูปแบบเดียวกับ `docs/web-tutorial/AUDIT_REMEDIATION_PLAN.md` (เขียนโค้ดตัวอย่างแบบเดา API ไม่ผ่านการ compile จริง) — ควรใช้ script ตรวจสอบร่วมกันทั้งสองชุดเอกสาร (extract ชื่อฟังก์ชันจาก `<code>` block แล้ว grep หาใน header จริงที่เกี่ยวข้อง เตือนเมื่อไม่พบ) รันเป็น pre-commit หรือ CI step
