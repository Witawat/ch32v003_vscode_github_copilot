# ข้อจำกัดของไลบรารี — CH32V003

> **Updated:** 2026-06-26 | SimpleHAL v2.0 | 71 libraries audited

---

## SimpleHAL v2.0 Breaking Changes

| เรื่อง | รายละเอียด | ผลกระทบ |
|-------|-----------|---------|
| **`Timer_Init()`** | ต้องเรียกเองใน `main()` หลัง `SystemCoreClockUpdate()` (เดิมมี constructor auto-call) | 54 ไลบรารีที่ใช้ `Delay_Ms/Us` ทำงานได้ต่อเมื่อผู้ใช้เรียก `Timer_Init()` |
| **SPI Speed Enum** | ค่า enum สลับ (SPI_125KHZ=7, SPI_12MHZ=0) — ใช้ชื่อ enum ตรงปลอดภัย | กระทบเฉพาะผู้ใช้ค่าตัวเลขดิบ |
| **SPI Remap Pins** | SPI remap แก้เป็น PD1/PD2/PD3/PD0 (เดิม PC6/PC8/PC7/PC5 — ผิด) | ตรวจสอบ wiring ถ้าเคยใช้ SPI remap |
| **GPIO Pin Enum** | เพิ่ม PD0(18), PD1(19) | ใช้ PD1 เป็น GPIO ได้ (รวม SOP-8) |

---

## Package Compatibility

| แพ็กเกจ | GPIO | SPI HW | I2C HW | PWM | ADC | USART |
|---------|:---:|:---:|:---:|:---:|:---:|:---:|
| TSSOP-20 (F4P6) | ✅ 18 pins | ✅ | ✅ | ✅ 8 ch | ✅ 8 ch | ✅ |
| QFN-20 (F4U6) | ✅ 18 pins | ✅ | ✅ | ✅ 8 ch | ✅ 8 ch | ✅ |
| SOP-16 (A4M6) | ⚠️ ~14 pins | ✅ | ✅ | ⚠️ บาง ch | ⚠️ 4 ch | ✅ |
| SOP-8 (J4M6) | ⚠️ 6 pins | ❌ ใช้ soft | ❌ ใช้ soft | ⚠️ 2 ch | ⚠️ 4 ch | ✅ |

> **SOP-8:** PC5-PC7 ไม่มี → SPI hardware ใช้ไม่ได้ → ใช้ `shiftOut/shiftIn` software  
> **SOP-8:** PC2 ไม่มี → I2C hardware ใช้ไม่ได้ → ใช้ `SimpleI2C_Soft` แทน

---

## ข้อจำกัดเฉพาะไลบรารี

### 📺 Display / LED

| ไลบรารี | ข้อจำกัด | รายละเอียด |
|---------|---------|-----------|
| **WS2815Matrix** | สูงสุด 8×8 | RAM 2KB — `matrix_buffer` 256B, `temp_buffer` 256B (static) |
| **OLED** | 1 buffer ร่วม | 3 ขนาด (128x64/128x32/64x48) ใช้ buffer เดียว — แสดงผลทีละขนาด |
| **OLED** | Font 8x16/12x16 | ยังไม่มีข้อมูลฟอนต์จริง — fallback เป็น Font_6x8 อัตโนมัติ |
| **MAX7219** | SPI init เอง | ผู้ใช้ต้องเรียก `SPI_SimpleInit(SPI_MODE0, speed, pins)` ก่อน `MAX7219_Init` |
| **MAX7219** | RunningLight | `while(1)` infinite loop — ใช้ใน main loop แบบ blocking |
| **NeoPixel** | FillGradient | ต้องมี ≥ 2 LEDs |
| **NeoPixel** | malloc | ใช้ `malloc` บน RAM 2KB — หลีกเลี่ยงการเรียก Init ซ้ำ |
| **P10** | Panel สูงสุด 32×16 | Buffer static 192B (RGB) — config ใหญ่กว่าถูก reject |
| **TM1637** | DisplayNumber | รองรับ INT16_MIN |
| **TM1650** | WaitKey | Timeout 5s (press) + 2s (release) — ไม่ block ถาวร |

### 🌡️ Sensor

| ไลบรารี | ข้อจำกัด | รายละเอียด |
|---------|---------|-----------|
| **DHT** | IRQ ปิด ~5ms | อ่านค่า 1 ครั้งปิด interrupt ชั่วคราว — อาจพลาด interrupt สำคัญ |
| **DHT** | Timing calibration | Overhead 3µs/loop — ปรับเทียบอัตโนมัติสำหรับ CH32V003 @ 48MHz |
| **DS18B20** | 1-Wire timing | ผ่าน Simple1Wire — IRQ ปิดระหว่าง bit-bang (~790µs/reset) |
| **MQGas** | ADC auto-init | เรียก `ADC_EnableChannel()` ใน Init — ไม่ต้องเรียก `ADC_SimpleInit` แยก |
| **NTC10K** | Guard division | `t0_kelvin=0` / `b_value=0` guard — return 0°C |

### 📡 Communication

| ไลบรารี | ข้อจำกัด | รายละเอียด |
|---------|---------|-----------|
| **ESP01** | HTTP URL ≤ 200 chars | Path + host รวมกันเกิน 200 ตัว → `ESP01_ERROR_OVERFLOW` |
| **nRF24L01** | SPI Mode 0 | ผู้ใช้ต้องตั้ง SPI Mode 0 ด้วยตนเอง |
| **nRF24L01** | TX timeout | `Delay_Ms(1)` ใน busy-wait loop — CPU usage ลดลง |
| **SoftUART** | Baud > 38400 | Timing error สะสม ~8% ต่อ 10 bits — ใช้ baud ≤ 38400 |
| **SoftUART** | Baud = 0 | Return error — ป้องกัน DIV/0 |
| **HC05** | Flush timeout | 100ms timeout — ไม่ block ถาวรหากมี noise |
| **GPS** | NMEA checksum | ไม่ตรวจสอบ — ใช้ cross-check จาก field validity แทน |
| **PMS5003** | USART_Flush | รีเซ็ต frame buffer เมื่อเริ่มอ่านใหม่ — frame บางส่วนถูกทิ้ง |
| **Modbus** | Master อย่างเดียว | ไม่รองรับโหมด Slave — ใช้ USART1 ตัวเดียว → มี instance ได้ตัวเดียว |
| **Modbus** | RAM ~1.3KB (DMA) | DMA buffer 256B + capture 258B + protocol buffers (static) — ระวังชนกับ OLED (1KB) |
| **Modbus** | DMA_CH2 + CH3 | โหมด DMA ยึด 2 channels — ห้ามใช้กับ `DMA_analogReadStart` หรือ DMA อื่น |
| **Modbus** | IDLE hook ตัวเดียว | override `USART_IdleHook()` ได้ 1 ตัวต่อโปรเจกต์ — ห้ามชนกับไลบรารีอื่น |
| **Modbus** | DE/RE ไม่อัตโนมัติ | RS-485 ต้องควบคุม DE/RE pin เอง (ตัวอย่าง ex04) |

### ⚙️ Motor / Servo / I/O

| ไลบรารี | ข้อจำกัด | รายละเอียด |
|---------|---------|-----------|
| **ESC** | PWM duty | ใช้ `PWM_GetPeriod()` — ทำงานถูกต้องกับทุก PWM frequency |
| **ServoCluster** | PWM duty | ใช้ `PWM_GetPeriod()` — ไม่ hardcode 65535 |
| **ServoCluster** | speed_pct guard | `speed_pct=0` → clamp เป็น 1; `duration_ms=0` → clamp เป็น 1 |
| **INA219** | Error sentinel | ใช้ `NAN` (from `<math.h>`) — ตรวจสอบด้วย `isnan()` |
| **AS5600** | Atomic read | Multi-byte I2C read — ป้องกัน torn read (ค่าฉีกระหว่างเปลี่ยน) |
| **AS5600** | Error sentinel | Return `0xFFFF` เมื่อ I2C error |
| **PCA9685** | I2C checks | ตรวจสอบ return value ทุก `_read_reg`/`_write_reg` — device ไม่ค้างเงียบ |

### 🔐 Flash / Storage

| ไลบรารี | ข้อจำกัด | รายละเอียด |
|---------|---------|-----------|
| **W25Qxx** | SPI init | ผู้ใช้ต้องตั้งค่า SPI เองก่อน — ไลบรารีใช้ `SPI_Transfer` เท่านั้น |
| **AT24Cxx** | EEPROM endurance | ~1,000,000 write cycles — หลีกเลี่ยงการเขียนใน loop |

---

## Timer Conflict Map

| Resource | ใช้โดย | หมายเหตุ |
|----------|--------|---------|
| SysTick | SimpleDelay, **54 ไลบรารี** | ต้องเรียก `Timer_Init()` ใน `main()` |
| TIM1 | SimplePWM (PWM1_CH1-4), SimpleTIM, Servo, ESC, L298N, ServoCluster | **เลือกอย่างใดอย่างหนึ่ง** |
| TIM2 | SimplePWM (PWM2_CH1-4), SimpleTIM, SimpleTIM_Ext, ServoCluster, P10 | **เลือกอย่างใดอย่างหนึ่ง** |
| IWDG | SimpleIWDG | LSI clock อิสระ |
| USART1 | SimpleUSART, ESP01, HC05, GPS, PZEM004T, PMS5003, TJC, **Modbus** | **ใช้ร่วมกันไม่ได้** — หยุด USART debug print ก่อนใช้ device |
| DMA_CH2 | Modbus TX (โหมด DMA) | ใช้กับ DMA_USART TX — ห้ามใช้กับ DMA อื่นพร้อมกัน |
| DMA_CH3 | Modbus RX (โหมด DMA) | circular buffer 256B — ห้ามใช้กับ DMA อื่นพร้อมกัน |

---

## Known Unfixed Issues (ต้อง redesign)

| เรื่อง | รายละเอียด | Workaround |
|-------|-----------|-----------|
| **WS2815Matrix RAM** | 32×32 matrix ต้องการ RAM ~4KB — เกิน CH32V003 | ลดเหลือ 8×8 |
| **OLED Font 8x16/12x16** | ยังไม่มีข้อมูล bitmap ฟอนต์จริง | Fallback เป็น Font_6x8 |
| **DHT/DS18B20 IRQ** | `Delay_Us` ใน IRQ-disabled section — อาจพลาด SysTick | ลดความถี่การอ่าน (≥ 2 วินาที) |
| **SoftUART RX Buffer** | Ring buffer ไม่ถูกเติมข้อมูล — `SoftUART_Available` คืน 0 เสมอ | ใช้ `SoftUART_ReadByte` พร้อม timeout |
