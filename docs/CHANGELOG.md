## [2025-12-22 13:47] - SimpleADC v1.1 - Battery Monitoring Enhancement

### เพิ่มเติม (Added)
- **SimpleADC Battery Monitoring**: เพิ่มฟีเจอร์วัดแรงดันแบตเตอรี่และแสดงเปอร์เซ็นต์โดยไม่ต้องใช้วงจรภายนอก
  - **Internal Vref Reading**: อ่านแรงดันอ้างอิงภายใน (ADC Channel 8)
  - **VDD Calculation**: คำนวณแรงดัน VDD จริงจาก Vrefint
  - **VDD Compensation**: ชดเชยความผันผวนของ VDD อัตโนมัติ
  - **Battery Percentage**: คำนวณเปอร์เซ็นต์แบตเตอรี่ (0-100%)
  - **Multi-Battery Support**: รองรับ Li-ion, Alkaline, CR2032
  - **2 New Examples**: Internal Vref Reading และ Battery Monitor
  - **Thai Documentation**: README 300+ บรรทัด

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/SimpleADC.h` (MODIFIED)
- เพิ่ม Internal Channel definitions:
  - `ADC_CH_8`, `ADC_CH_9` (Channel 8-9)
  - `ADC_CH_VREFINT`, `ADC_CH_VCALINT` (aliases)
- เพิ่ม Vref constants:
  - `ADC_VREFINT_VOLTAGE` (1.2V)
  - `ADC_VREFINT_CAL` (512)
- เพิ่ม Function declarations:
  - `ADC_ReadVrefInt()` - อ่าน Internal Vref
  - `ADC_GetVDD()` - คำนวณ VDD จริง
  - `ADC_ReadVoltageCompensated()` - อ่านแรงดันพร้อมชดเชย VDD
  - `ADC_GetBatteryPercent()` - คำนวณเปอร์เซ็นต์แบตเตอรี่

#### `User/SimpleHAL/SimpleADC.c` (MODIFIED)
- Implement `ADC_ReadVrefInt()`:
  - ใช้ ADC_Channel_Vrefint (Channel 8)
  - Sample time 241 cycles สำหรับ internal channel
- Implement `ADC_GetVDD()`:
  - อ่าน Vrefint 10 ครั้งและหาค่าเฉลี่ย
  - สูตร: VDD = 1.2V × 1023 / Vrefint_ADC
  - Zero-check protection
- Implement `ADC_ReadVoltageCompensated()`:
  - อ่าน VDD จริงก่อน
  - ใช้ VDD ที่คำนวณได้แทน 3.3V
- Implement `ADC_GetBatteryPercent()`:
  - สูตร: (vdd - v_min) / (v_max - v_min) × 100
  - Clamp ผลลัพธ์ให้อยู่ใน 0-100%

#### `User/SimpleHAL/Examples/SimpleADC_Examples.c` (MODIFIED)
- เพิ่ม `Example_ADC_InternalVref()`:
  - อ่านและแสดงค่า Vrefint (raw ADC)
  - คำนวณและแสดง VDD จริง
  - เปรียบเทียบกับ 3.3V (แสดง [HIGH]/[LOW]/[OK])
- เพิ่ม `Example_ADC_BatteryMonitor()`:
  - วัดแรงดัน VDD (แบตเตอรี่) real-time
  - คำนวณและแสดงเปอร์เซ็นต์ (0-100%)
  - แสดง progress bar (20 characters)
  - เตือนเมื่อแบตต่ำ (< 20%): [LOW BATTERY!]
  - แสดงสถานะ: [Good], [Medium], [LOW BATTERY!]
  - รองรับหลายประเภทแบต (Li-ion, Alkaline, CR2032)
- อัปเดต `SimpleADC_Examples_Main()`:
  - เพิ่ม comments สำหรับ Example 8 และ 9

#### `User/SimpleHAL/SimpleADC/README.md` (NEW)
- เอกสารภาษาไทยฉบับสมบูรณ์ 300+ บรรทัด
- ครอบคลุม:
  - ภาพรวม SimpleADC (External + Internal channels)
  - คุณสมบัติเด่น (ใช้งานง่าย, วัดแบต, แม่นยำสูง)
  - ADC Channels (ตาราง External 0-7, Internal 8-9)
  - การใช้งานพื้นฐาน (Init, Read, Average)
  - **Battery Monitoring** (ฟีเจอร์ใหม่):
    - ทำไมต้องใช้ VDD Compensation
    - วิธีการทำงาน (3 ขั้นตอน)
    - ตัวอย่างการใช้งาน (วัด VDD, คำนวณ %, ตัวอย่างสมบูรณ์)
    - ตารางแรงดันมาตรฐาน (Li-ion, Alkaline, CR2032, NiMH)
  - API Reference (Basic + Battery Monitoring functions)
  - ตัวอย่างการใช้งาน (9 examples)
  - Tips & Tricks (ลด Noise, ประหยัดพลังงาน, Calibration)
  - เวอร์ชัน (v1.0 และ v1.1)

### Technical Details

**Internal Channels:**
- **Channel 8 (Vrefint)**: Internal Reference Voltage ≈ 1.2V
- **Channel 9 (Vcalint)**: Internal Calibration Voltage (50-75% VDD)

**VDD Calculation:**
```
VDD = VREFINT_VOLTAGE × ADC_MAX_VALUE / ADC_ReadVrefInt()
    = 1.2V × 1023 / Vrefint_ADC
```

**Battery Percentage:**
```
Percent = (VDD - V_min) / (V_max - V_min) × 100%
Clamped to 0-100%
```

**Battery Voltage Standards:**
| Type | Full | Empty | Example |
|------|------|-------|---------|
| Li-ion/Li-Po | 4.2V | 3.0V | `ADC_GetBatteryPercent(vdd, 3.0, 4.2)` |
| Alkaline 2xAA | 3.2V | 2.0V | `ADC_GetBatteryPercent(vdd, 2.0, 3.2)` |
| CR2032 | 3.0V | 2.0V | `ADC_GetBatteryPercent(vdd, 2.0, 3.0)` |

### สรุป
- **รวมโค้ด**: ~500 บรรทัด (core + examples + docs)
- **ไฟล์แก้ไข**: 3 ไฟล์ (SimpleADC.h, SimpleADC.c, SimpleADC_Examples.c)
- **ไฟล์ใหม่**: 1 ไฟล์ (README.md)
- **ตัวอย่างใหม่**: 2 examples (Example 8, 9)
- **เอกสาร**: README 300+ บรรทัดภาษาไทย
- **API Functions**: 4 ฟังก์ชันใหม่
- **Internal Channels**: 2 channels (Vrefint, Vcalint)
- **Battery Types**: 4 types (Li-ion, Alkaline, CR2032, NiMH)
- **SimpleADC Version**: 1.0 → 1.1

---

## [2025-12-22 13:16] - SimplePWR Library v1.0 - Power Management

### เพิ่มเติม (Added)
- **SimplePWR Library**: Library สำหรับจัดการพลังงานแบบครบวงจร
  - **Sleep Mode**: CPU หยุดทำงาน, Peripherals ทำงานต่อ (~1-2mA)
  - **Standby Mode**: Deep Sleep ประหยัดพลังงานสูงสุด (~5µA, ประหยัด 99.9%)
  - **PVD (Power Voltage Detector)**: ตรวจจับแรงดันต่ำ 8 ระดับ (2.9V-4.4V)
  - **AWU (Auto Wake-Up)**: ตั่งเวลาปลุกอัตโนมัติ (1ms-30s)
  - **Wake-up Pin Support**: ตื่นจาก external interrupt (PA0)
  - **Battery Life Calculator**: คำนวณอายุแบตเตอรี่
  - **Power Estimation**: ประมาณการใช้พลังงานใน Standby mode
  - **7 Examples**: ตัวอย่างจากพื้นฐานถึงขั้นสูง
  - **Thai Documentation**: เอกสาร 1000+ บรรทัด พร้อมทฤษฎี power management

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/SimplePWR.h` (NEW)
- เพิ่ม Power Mode constants (ENTRY_WFI, ENTRY_WFE)
- เพิ่ม PVD Voltage Levels (2.9V-4.4V)
- เพิ่ม AWU Prescaler constants (1-61440)
- เพิ่ม Helper Macros (LSI_FREQ, AWU_TIMEOUT_MS, AWU_CALC_WINDOW)
- เพิ่ม Basic API: Sleep, Standby, StandbyUntilInterrupt
- เพิ่ม Advanced API: EnterSleepMode, EnterStandbyMode, ConfigureAWU, EnablePVD, DisablePVD, GetPVDStatus, GetAWUTimeout
- เพิ่ม Utility API: WasStandbyWakeup, ClearStandbyFlag, EnableWakeupPin, DisableWakeupPin, EstimateStandbyCurrent, CalculateBatteryLife
- เพิ่ม Reference Tables (AWU Timeout, Power Consumption)
- รวม ~400 บรรทัด

#### `User/SimpleHAL/SimplePWR.c` (NEW)
- Implement PWR peripheral initialization
- Implement Sleep mode entry (WFI/WFE)
- Implement Standby mode entry พร้อม AWU configuration
- Implement AWU parameter selection (automatic prescaler)
- Implement PVD configuration และ status checking
- Implement Wake-up source detection
- Implement Battery life calculation
- Implement Power consumption estimation
- รวม ~350 บรรทัด

#### `User/SimpleHAL/Examples/PWR/01_BasicSleep.c` (NEW)
- ตัวอย่าง Sleep Mode พื้นฐาน
- LED blink พร้อม sleep intervals
- เปรียบเทียบ power consumption

#### `User/SimpleHAL/Examples/PWR/02_StandbyMode.c` (NEW)
- ตัวอย่าง Standby Mode พร้อม auto wake-up
- ตรวจสอบ wake-up source
- System reset behavior

#### `User/SimpleHAL/Examples/PWR/03_PVD_Monitor.c` (NEW)
- ตัวอย่าง Power Voltage Detector
- ตรวจจับแรงดันต่ำ
- Warning system

#### `User/SimpleHAL/Examples/PWR/04_AutoWakeUp.c` (NEW)
- ตัวอย่าง AWU timer configuration
- Precise wake-up intervals
- Timing calculation

#### `User/SimpleHAL/Examples/PWR/05_LowPowerBlink.c` (NEW)
- Ultra-low-power LED blink
- Battery life optimization
- Power consumption analysis

#### `User/SimpleHAL/Examples/PWR/06_PowerManagement.c` (NEW)
- Complete power management system
- State machine (Active, Idle, Sleep, Standby)
- Automatic power mode adjustment

#### `User/SimpleHAL/Examples/PWR/07_BatteryMonitor.c` (NEW)
- Battery monitoring application
- ADC + PVD integration
- Battery percentage calculation
- Emergency shutdown

#### `User/SimpleHAL/Examples/PWR/README.md` (NEW)
- เอกสารภาษาไทยฉบับสมบูรณ์ 1000+ บรรทัด
- ครอบคลุม: บทนำ, ทฤษฎี (Sleep vs Standby, PVD, AWU)
- การติดตั้งและเริ่มต้นใช้งาน
- การใช้งานพื้นฐาน (Sleep, Standby, PVD, AWU)
- การใช้งานขั้นสูง (AWU configuration, Wake-up detection, Battery calculation)
- ตัวอย่างการใช้งาน (7 examples)
- ตารางข้อมูลอ้างอิง (AWU Prescaler, PVD Levels, Power Consumption)
- เทคนิคการประหยัดพลังงาน (6 techniques)
- Troubleshooting (5 ปัญหาพบบ่อย)

#### `User/SimpleHAL/SimpleHAL.h` (MODIFIED)
- เพิ่ม `#include "SimplePWR.h"`
- อัปเดท version 2.1.0 → 1.9.0

### Technical Details

**Power Modes:**
- Active: 3-5 mA (full speed operation)
- Sleep: 1-2 mA (CPU stopped, peripherals running)
- Standby: ~5 µA (everything off, AWU/PVD optional)

**PVD Thresholds:**
- 8 levels: 2.9V, 3.1V, 3.3V, 3.5V, 3.7V, 3.9V, 4.1V, 4.4V
- Hysteresis: ~100mV
- Use cases: Battery monitoring, brownout detection

**AWU Timer:**
- LSI Clock: 128kHz (±25% accuracy)
- Prescaler: 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 10240, 61440
- Window: 0-63
- Max timeout: ~30 seconds

**Battery Life Example:**
```
CR2032 (220mAh) + Standby Mode (5µA):
Battery life = 220mAh / 0.005mA = 44,000 hours = 5 years!
```

### สรุป
- **รวมโค้ด**: ~3,500 บรรทัด (core + examples + docs)
- **ไฟล์ใหม่**: 10 ไฟล์ (2 core + 7 examples + 1 README)
- **ตัวอย่าง**: 7 ตัวอย่าง (basic: 4, advanced: 3)
- **เอกสาร**: README 1000+ บรรทัดภาษาไทย
- **API Functions**: 20+ ฟังก์ชัน
- **Power Modes**: 2 modes (Sleep, Standby)
- **PVD Levels**: 8 voltage thresholds
- **AWU Prescalers**: 15 prescaler values
- **Battery Life**: Up to 5+ years on coin cell
- **SimpleHAL Version**: 1.9.0

---

## [2025-12-22 13:00] - SimpleDMA Library v1.0

### เพิ่มเติม (Added)
- **SimpleDMA Library**: Library สำหรับควบคุม DMA (Direct Memory Access) แบบครบวงจร
  - **7 DMA Channels**: รองรับ DMA1_Channel1-7 พร้อม priority management
  - **3 Transfer Modes**: Memory-to-Memory, Peripheral-to-Memory, Memory-to-Peripheral
  - **Circular Buffer Mode**: รองรับ ring buffer สำหรับข้อมูลต่อเนื่อง
  - **Priority Levels**: 4 ระดับ (Low, Medium, High, Very High)
  - **Callback System**: Transfer Complete และ Error callbacks
  - **Peripheral Integration**: รองรับ ADC, USART, SPI พร้อม helper functions
  - **Blocking/Non-blocking**: รองรับทั้ง 2 โหมด
  - **8 Examples**: ตัวอย่างจากพื้นฐานถึงขั้นสูง
  - **Thai Documentation**: เอกสาร 1000+ บรรทัด พร้อมทฤษฎี DMA

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/SimpleDMA.h` (NEW)
- เพิ่ม DMA_Channel enum (DMA_CH1-7)
- เพิ่ม DMA_Direction enum (M2M, P2M, M2P)
- เพิ่ม DMA_Priority enum (Low, Medium, High, Very High)
- เพิ่ม DMA_DataSize enum (Byte, HalfWord, Word)
- เพิ่ม DMA_Mode enum (Normal, Circular)
- เพิ่ม DMA_Status enum (Idle, Busy, Complete, Error)
- เพิ่ม DMA_Config_t structure
- เพิ่ม Callback types (TransferComplete, Error)
- เพิ่ม Basic API: SimpleInit, Start, Stop, GetStatus, WaitComplete, Reset, GetRemainingCount
- เพิ่ม Callback API: SetTransferCompleteCallback, SetErrorCallback
- เพิ่ม Memory API: MemCopy, MemCopyAsync, MemSet
- เพิ่ม ADC API: ADC_Init, ADC_InitMultiChannel
- เพิ่ม USART API: USART_InitTx, USART_InitRx, USART_Transmit, USART_GetReceivedCount
- เพิ่ม SPI API: SPI_Init, SPI_TransferBuffer
- เพิ่ม Helper API: GetChannelBase, GetChannelIRQn, EnableInterrupt
- รวม ~500 บรรทัด

#### `User/SimpleHAL/SimpleDMA.c` (NEW)
- Implement DMA initialization พร้อม configuration
- Implement Start/Stop/Reset functions
- Implement Status tracking และ GetStatus
- Implement Memory transfer functions (Copy, Set)
- Implement ADC integration (continuous, multi-channel)
- Implement USART integration (TX, RX, circular buffer)
- Implement SPI integration (full-duplex transfer)
- Implement Callback system
- Implement 7 Interrupt handlers (DMA1_Channel1-7_IRQHandler)
- Implement Helper functions (channel mapping, IRQ mapping)
- รวม ~700 บรรทัด

#### `User/SimpleHAL/Examples/DMA/01_MemCopy_Basic.c` (NEW)
- ตัวอย่าง Memory-to-Memory transfer พื้นฐาน
- เปรียบเทียบความเร็วกับ memcpy()
- แสดง throughput calculation

#### `User/SimpleHAL/Examples/DMA/02_MemCopy_Async.c` (NEW)
- ตัวอย่าง Non-blocking memory copy
- ใช้ callbacks
- แสดง concurrent CPU operation

#### `User/SimpleHAL/Examples/DMA/03_ADC_DMA.c` (NEW)
- ADC continuous conversion กับ DMA
- Multi-channel sampling
- Circular buffer mode
- ค่าเฉลี่ยจาก buffer

#### `User/SimpleHAL/Examples/DMA/04_USART_DMA_TX.c` (NEW)
- USART transmission ด้วย DMA
- ส่งข้อมูลขนาดใหญ่
- CPU ทำงานอื่นได้ระหว่างส่ง

#### `User/SimpleHAL/Examples/DMA/05_USART_DMA_RX.c` (NEW)
- USART reception ด้วย DMA
- Circular buffer mode
- Wrap-around handling

#### `User/SimpleHAL/Examples/DMA/06_SPI_DMA.c` (NEW)
- SPI transfer ด้วย DMA
- Full-duplex communication
- High-speed data transfer

#### `User/SimpleHAL/Examples/DMA/07_DoubleBuffer.c` (NEW)
- Double buffering technique
- Ping-pong buffers
- Maximum throughput

#### `User/SimpleHAL/Examples/DMA/08_MultiChannel.c` (NEW)
- ใช้หลาย channels พร้อมกัน
- Priority management
- Channel coordination

#### `User/SimpleHAL/Examples/DMA/README.md` (NEW)
- เอกสารภาษาไทยฉบับสมบูรณ์ 1000+ บรรทัด
- ครอบคลุม: ภาพรวม DMA, ทฤษฎี (DMA คืออะไร, Transfer modes, Priority)
- Hardware Setup (Channel mapping, Priority levels)
- การใช้งานพื้นฐาน (Memory transfer, Callbacks, Blocking/Non-blocking)
- การใช้งานขั้นกลาง (ADC+DMA, USART+DMA, SPI+DMA, Circular buffer)
- เทคนิคขั้นสูง (Double buffering, Priority management, Error handling, Multi-channel)
- Best Practices (Channel selection, Memory alignment, Buffer management, Interrupt priority, Performance optimization)
- API Reference (40+ ฟังก์ชัน)
- Troubleshooting (5 ปัญหาพบบ่อย)

#### `User/SimpleHAL/SimpleHAL.h` (MODIFIED)
- เพิ่ม `#include "SimpleDMA.h"`
- อัปเดท version 2.0.0 → 2.1.0
- เพิ่ม DMA ในรายการ peripherals ที่รองรับ

### Technical Details

**DMA Channels:**
- 7 channels (DMA1_Channel1-7)
- Hardware priority: CH1 > CH2 > ... > CH7
- Software priority: Very High > High > Medium > Low

**Transfer Modes:**
- Normal: ถ่ายโอนครั้งเดียวแล้วหยุด
- Circular: ถ่ายโอนวนซ้ำ (ring buffer)

**Data Sizes:**
- Byte (8-bit)
- HalfWord (16-bit)
- Word (32-bit)

**Peripheral Support:**
- ADC: Continuous conversion, Multi-channel
- USART: TX/RX with circular buffer
- SPI: Full-duplex transfer
- Memory: Copy, Set operations

### สรุป
- **รวมโค้ด**: ~4,000 บรรทัด (core + examples + docs)
- **ไฟล์ใหม่**: 11 ไฟล์ (2 core + 8 examples + 1 README)
- **ตัวอย่าง**: 8 ตัวอย่าง (basic: 2, intermediate: 4, advanced: 2)
- **เอกสาร**: README 1000+ บรรทัดภาษาไทย
- **API Functions**: 40+ ฟังก์ชัน
- **DMA Channels**: 7 channels
- **Transfer Modes**: 3 modes (M2M, P2M, M2P)
- **Priority Levels**: 4 levels
- **Peripheral Integration**: ADC, USART, SPI
- **SimpleHAL Version**: 2.1.0

---

## [2025-12-22 12:47] - SimpleHAL Completeness Review & Recommendations

### เพิ่มเติม (Added)
- **SimpleHAL Review Report**: รายงานตรวจสอบความครบถ้วนของ Library เทียบกับ CH32V003 Hardware
  - วิเคราะห์โมดูลที่สมบูรณ์แล้ว: GPIO, USART, I2C, SPI, ADC, TIM, PWM, Flash, WDT, OPAMP, Delay, 1-Wire
  - ข้อเสนอแนะการพัฒนาเพิ่มเติม:
    - **SimpleDMA**: รองรับ DMA 7 channels สำหรับ USART/ADC/SPI
    - **SimplePWR**: จัดการโหมดประหยัดพลังงาน Sleep/Stop/Standby
    - **Internal Sensors**: การอ่านค่า Internal Temp และ Vrefint
    - **SimpleSYS**: การเข้าถึง Unique ID (UID) และ Reset Source detection
    - **SimpleRCC**: การปรับแต่ง Clock frequency
    - **SimpleBuffer**: ระบบ Ring Buffer สำหรับ Serial data

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `readme/2025-12-22_System_SimpleHALReview.md` (NEW)
- เอกสารสรุปผลการตรวจสอบและคำแนะนำการพัฒนาในอนาคต

#### `readme/INDEX.md` (MODIFIED)
- เพิ่มรายการเอกสาร SimpleHAL Review

---


### เพิ่มเติม (Added)
- **SimpleI2C_Soft Library**: Library สำหรับสร้างสัญญาณ I2C ผ่าน GPIO ทั่วไป (Bit-bang)
  - **Blind Testing**: รองรับการส่งข้อมูลโดยไม่ต้องรอ ACK (ignore_ack) เหมาะสำหรับการทดสอบกับ Logic Analyzer
  - **Flexible Pins**: สามารถเลือกใช้คู่ GPIO pins ใดก็ได้ใน CH32V003
  - **Adjustable Speed**: ปรับความเร็วได้ผ่านค่าหน่วงเวลา (delay_us) โดยดีฟอลต์ที่ ~100kHz
  - **Full Protocol**: รองรับ Start, Stop, WriteByte, ReadByte, Write (multi), Read (multi)
  - **Thai Documentation**: คู่มือการใช้งานและหลักการทดสอบด้วย Logic Analyzer

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/SimpleI2C_Soft.h` (NEW)
- นิยาม `I2C_Soft_Init`, `I2C_Soft_Write`, `I2C_Soft_Read`
- นิยามสถานะ `I2C_Soft_Status`
- นิยามฟังก์ชันระดับต่ำ (Start, Stop, WriteByte, ReadByte)

#### `User/SimpleHAL/SimpleI2C_Soft.c` (NEW)
- Implement bit-banging logic โดยใช้ `digitalWrite` และ `Delay_Us`
- จัดการสัญญาณ SDA/SCL ในรูปแบบ Open-Drain simulation

#### `User/SimpleHAL/Examples/I2C_LogicAnalyzer_Test.c` (NEW)
- ตัวอย่างการเขียนโปรแกรมเพื่อส่งข้อมูลออกไปอย่างต่อเนื่องสำหรับตรวจสอบด้วย Logic Analyzer

#### `readme/2025-12-22_I2C_LogicAnalyzerTesting.md` (NEW)
- เอกสารอธิบายปัญหาของ Hardware I2C (ACK requirement) และวิธีแก้ปัญหาด้วย Software I2C

#### `readme/INDEX.md` (MODIFIED)
- เพิ่มรายการเอกสาร Software I2C

---

## [2025-12-22 10:31] - DS18B20 Digital Temperature Sensor Library v1.0 + Simple1Wire v1.0

### เพิ่มเติม (Added)
- **Simple1Wire Library**: Generic 1-Wire protocol library สำหรับ CH32V003 (SimpleHAL component)
  - **Low-Level Protocol**: Reset pulse, Write/Read bit/byte functions
  - **ROM Commands**: Skip ROM, Read ROM, Match ROM, Search ROM, Alarm Search
  - **ROM Search Algorithm**: ค้นหา devices ทั้งหมดบน bus (รองรับ multi-device)
  - **CRC8 Calculation**: Dallas/Maxim CRC8 algorithm (polynomial: x^8 + x^5 + x^4 + 1)
  - **Interrupt-Safe**: Disable interrupts ระหว่าง critical timing
  - **Precise Timing**: ใช้ SimpleDelay (`Delay_Us`) สำหรับ timing แม่นยำ
  - **All GPIO Pins**: รองรับทุก GPIO pins (PA1-PA2, PC0-PC7, PD2-PD7)
  - **SimpleHAL v2.0.0**: อัพเดท SimpleHAL เป็น version 2.0.0

- **DS18B20 Library**: Library สำหรับวัดอุณหภูมิด้วย DS18B20 digital sensor
  - **Temperature Range**: -55°C ถึง +125°C (±0.5°C accuracy)
  - **Resolution**: 9-12 bit (0.5°C - 0.0625°C)
  - **Single/Multi-Sensor**: รองรับ single และ multi-sensor บน bus เดียวกัน
  - **Blocking/Non-Blocking**: รองรับทั้ง 2 โหมดการอ่าน
  - **Alarm Threshold**: ตั้งค่า high/low threshold พร้อม alarm search
  - **EEPROM Storage**: บันทึก configuration (resolution, alarm) ลง EEPROM
  - **CRC Validation**: ตรวจสอบ CRC8 สำหรับความน่าเชื่อถือ
  - **Parasite Power**: รองรับ parasite power mode
  - **7 Examples**: ตัวอย่างจากพื้นฐานถึงขั้นสูง
  - **Thai Documentation**: เอกสาร 1000+ บรรทัด พร้อมทฤษฎี 1-Wire protocol

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/Simple1Wire.h` (NEW)
- เพิ่ม OneWire_Bus structure (pin, ROM, search state)
- เพิ่ม 1-Wire timing constants (RESET_PULSE, PRESENCE_WAIT, WRITE_0/1_LOW, READ_LOW/WAIT)
- เพิ่ม ROM command constants (SKIP_ROM, READ_ROM, MATCH_ROM, SEARCH_ROM, ALARM_SEARCH)
- เพิ่ม Low-Level Protocol API: Init, Reset, WriteBit, ReadBit, WriteByte, ReadByte, WriteBytes, ReadBytes
- เพิ่ม ROM Command API: SkipROM, ReadROM, MatchROM, Select
- เพิ่ม Search API: ResetSearch, Search, AlarmSearch, GetAddress
- เพิ่ม CRC API: CRC8, VerifyCRC
- เพิ่ม Utility API: Depower, GetBusByPin
- รวม ~450 บรรทัด

#### `User/SimpleHAL/Simple1Wire.c` (NEW)
- Implement 1-Wire timing (reset: 480µs, presence: 60-240µs, write 0: 60µs, write 1: 10µs, read: 3µs+10µs)
- Implement ROM commands (Skip, Read, Match, Select)
- Implement ROM search algorithm (Maxim AN187) พร้อม discrepancy tracking
- Implement CRC8 calculation (Dallas/Maxim polynomial)
- Implement interrupt-safe sections (`__disable_irq()` / `__enable_irq()`)
- Implement multi-bus support (สูงสุด 4 buses)
- รวม ~550 บรรทัด

#### `User/Lib/DS18B20/DS18B20.h` (NEW)
- เพิ่ม DS18B20_Device structure (bus, ROM, resolution, use_rom, last_temperature)
- เพิ่ม DS18B20 command constants (CONVERT_T, READ_SCRATCHPAD, WRITE_SCRATCHPAD, COPY_SCRATCHPAD, RECALL_E2, READ_POWER)
- เพิ่ม Resolution enum (9-12 bit)
- เพิ่ม Initialization API: Init, InitWithROM
- เพิ่ม Temperature Reading API: StartConversion, StartConversionAll, ReadTemperature, ReadTemperatureF, ReadTemperatureBlocking, IsConversionDone
- เพิ่ม Configuration API: SetResolution, GetResolution, SetAlarm, GetAlarm, SaveToEEPROM, LoadFromEEPROM
- เพิ่ม Multi-Sensor API: SearchDevices, SearchAlarm
- เพิ่ม Utility API: ReadPowerSupply, GetConversionTime, VerifyDevice, CelsiusToFahrenheit, FahrenheitToCelsius
- รวม ~500 บรรทัด

#### `User/Lib/DS18B20/DS18B20.c` (NEW)
- Implement temperature conversion (raw → °C/°F) พร้อม resolution masking
- Implement scratchpad read/write (9 bytes)
- Implement resolution configuration (9-12 bit)
- Implement alarm threshold management (TH, TL)
- Implement EEPROM save/load (Copy Scratchpad, Recall E²)
- Implement multi-sensor search (ใช้ Simple1Wire ROM search)
- Implement CRC validation (ใช้ OneWire_VerifyCRC)
- Implement power supply detection (normal vs parasite)
- รวม ~600 บรรทัด

#### `User/Lib/DS18B20/Examples/01_BasicReading.c` (NEW)
- ตัวอย่างพื้นฐาน: อ่านอุณหภูมิแบบ blocking
- แสดง ROM address
- แสดงอุณหภูมิ °C และ °F

#### `User/Lib/DS18B20/Examples/02_MultiSensor.c` (NEW)
- ค้นหา sensors ทั้งหมดบน bus
- แสดง ROM addresses
- อ่านอุณหภูมิจากทุก sensors พร้อมกัน

#### `User/Lib/DS18B20/Examples/03_AlarmThreshold.c` (NEW)
- ตั้งค่า alarm thresholds (TH, TL)
- บันทึกลง EEPROM
- ค้นหา sensors ที่มี alarm

#### `User/Lib/DS18B20/Examples/04_HighResolution.c` (NEW)
- เปรียบเทียบความละเอียด 9-12 bit
- แสดงเวลา conversion
- แสดงความแม่นยำ

#### `User/Lib/DS18B20/Examples/05_ParasitePower.c` (NEW)
- ตรวจสอบ power mode
- อ่านอุณหภูมิในโหมด parasite power
- ตรวจสอบค่าผิดปกติ (85°C, -127°C)

#### `User/Lib/DS18B20/Examples/06_ROMSearch.c` (NEW)
- ค้นหา devices ทั้งหมดบน bus
- แสดง ROM address แบบละเอียด (Family Code, Serial Number, CRC)
- ตรวจสอบ CRC

#### `User/Lib/DS18B20/Examples/07_Advanced.c` (NEW)
- Non-blocking temperature reading พร้อม state machine
- Multi-sensor พร้อม different resolutions
- Alarm search
- EEPROM configuration

#### `User/Lib/DS18B20/README.md` (NEW)
- เอกสารภาษาไทยฉบับสมบูรณ์ 1000+ บรรทัด
- ครอบคลุม: ภาพรวม DS18B20, 1-Wire Protocol (timing diagrams, ROM commands, Function commands)
- Hardware Setup (Normal power, Parasite power, Multi-sensor, Pull-up resistor selection)
- การใช้งานพื้นฐาน (Single sensor, Non-blocking, Resolution, Alarm)
- การใช้งานขั้นสูง (Multi-sensor, Alarm search, State machine)
- ROM Addressing (64-bit structure, การอ่าน, การแยก)
- Parasite Power Mode (หลักการ, ข้อดี/ข้อเสีย, การใช้งาน)
- Troubleshooting (4 ปัญหาพบบ่อย)
- API Reference (30+ ฟังก์ชัน)

#### `User/SimpleHAL/SimpleHAL.h` (MODIFIED)
- เพิ่ม `#include "Simple1Wire.h"`
- อัพเดท version 1.9.0 → 2.0.0

#### `User/test_simplehal_compile.c` (MODIFIED)
- เพิ่ม Simple1Wire test functions (13 functions)
- ทดสอบ: Init, Reset, Write/Read bit/byte, ROM commands, Search, CRC

#### `User/test_userlib_compile.c` (MODIFIED)
- เพิ่ม `#include "Lib/DS18B20/DS18B20.h"`
- เพิ่ม `test_ds18b20_library()` function
- ทดสอบ: Init, Temperature reading, Configuration, Multi-sensor, Utility functions

#### `readme/INDEX.md` (MODIFIED)
- เพิ่มรายการ Simple1Wire v1.0 (SimpleHAL)
- เพิ่มรายการ DS18B20 v1.0 (User/Lib)

### Technical Details

**1-Wire Protocol Timing:**
```
Reset Pulse:     480-960 µs low
Presence Pulse:  60-240 µs low (from device)
Write 1:         1-15 µs low
Write 0:         60-120 µs low
Read:            1-15 µs low + sample @ 15 µs
```

**ROM Search Algorithm:**
- ใช้ Maxim Application Note 187
- Binary tree search พร้อม discrepancy tracking
- รองรับ multi-device bus
- CRC validation

**DS18B20 Resolution:**
| Resolution | Precision | Conversion Time |
|-----------|-----------|-----------------|
| 9-bit     | 0.5°C     | 93.75 ms       |
| 10-bit    | 0.25°C    | 187.5 ms       |
| 11-bit    | 0.125°C   | 375 ms         |
| 12-bit    | 0.0625°C  | 750 ms         |

### สรุป
- **รวมโค้ด**: ~4,000 บรรทัด (Simple1Wire + DS18B20 + examples + docs)
- **ไฟล์ใหม่**: 12 ไฟล์ (4 core + 7 examples + 1 README)
- **ตัวอย่าง**: 7 ตัวอย่าง (basic: 3, intermediate: 2, advanced: 2)
- **เอกสาร**: README 1000+ บรรทัดภาษาไทย
- **API Functions**: 50+ ฟังก์ชัน (Simple1Wire: 20+, DS18B20: 30+)
- **1-Wire Timing**: Standard Dallas/Maxim specification
- **Temperature Range**: -55°C ถึง +125°C
- **Max Devices**: ไม่จำกัด (ขึ้นกับ bus capacitance)
- **SimpleHAL Version**: 2.0.0

---

## [2025-12-22 10:12] - Rotary Encoder KY-040 Library v1.0

### เพิ่มเติม (Added)
- **Rotary Encoder Library**: Library สำหรับควบคุม KY-040 Rotary Encoder แบบครบวงจร
  - **Interrupt-based Quadrature Decoding**: ตรวจจับการหมุนแม่นยำ 100% ด้วย state machine
  - **Direction Detection**: ตรวจจับทิศทาง CW (Clockwise) และ CCW (Counter-Clockwise)
  - **Position Counter**: นับตำแหน่งแบบ unlimited range (signed 32-bit)
  - **Button Support**: รองรับการกดปุ่ม (press, release, long press >500ms, double click <300ms)
  - **Acceleration Mode**: หมุนเร็ว = ค่าเปลี่ยนเร็วขึ้น (x2, x4, x8 based on RPM)
  - **Debouncing**: กำจัด noise อัตโนมัติ (configurable 2-50ms)
  - **Callback System**: Event-driven programming (5 callbacks)
  - **Min/Max Limits**: จำกัดช่วงค่าได้
  - **7 Examples**: ตัวอย่างจากพื้นฐานถึงขั้นสูง
  - **Thai Documentation**: เอกสาร 1000+ บรรทัด พร้อมทฤษฎี quadrature encoding

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/Lib/RotaryEncoder/RotaryEncoder.h` (NEW)
- เพิ่ม RotaryDirection enum (NONE, CW, CCW)
- เพิ่ม RotaryEvent enum (7 events)
- เพิ่ม RotaryEncoder structure (configuration และ state)
- เพิ่ม Configuration constants (DEBOUNCE_MS, LONG_PRESS_MS, DOUBLE_CLICK_MS, ACCEL_THRESHOLD_RPM)
- เพิ่ม Initialization API: Init, Reset
- เพิ่ม Position Control API: GetPosition, SetPosition, GetDirection, HasChanged, SetLimits, ClearLimits
- เพิ่ม Button Control API: IsButtonPressed, WaitForButton, UpdateButton
- เพิ่ม Advanced Settings API: SetDebounceTime, SetAcceleration
- เพิ่ม Callback API: OnRotate, OnButtonPress, OnButtonRelease, OnButtonLongPress, OnButtonDoubleClick
- เพิ่ม Internal API: Update, CLK_ISR, DT_ISR
- รวม ~450 บรรทัด

#### `User/Lib/RotaryEncoder/RotaryEncoder.c` (NEW)
- Implement quadrature_table[4][4] - Gray code transition table
- Implement Rotary_ReadState() - อ่าน 2-bit state (CLK, DT)
- Implement Rotary_ProcessRotation() - quadrature decoding พร้อม debouncing
- Implement Rotary_Init() - ตั้งค่า pins, interrupts, และ initial state
- Implement Position Control functions (Get/Set/Direction/HasChanged/Limits)
- Implement Button Control functions (IsPressed/Wait/Update)
- Implement Button Detection (press, release, long press, double click)
- Implement Acceleration calculation (RPM-based factor: 1x, 2x, 4x, 8x)
- Implement Callback system (5 callback functions)
- Implement Multi-encoder support (สูงสุด 8 encoders)
- รวม ~420 บรรทัด

#### `User/Lib/RotaryEncoder/Examples/01_BasicRotation.c` (NEW)
- ตัวอย่างพื้นฐาน: อ่านตำแหน่งและทิศทาง
- แสดงผลทาง USART
- Polling mode

#### `User/Lib/RotaryEncoder/Examples/02_ButtonControl.c` (NEW)
- การใช้งานปุ่ม: press, release, long press, double click
- รีเซ็ตตำแหน่งด้วยปุ่ม
- LED indicator

#### `User/Lib/RotaryEncoder/Examples/03_MenuNavigation.c` (NEW)
- ระบบเมนูแบบง่าย (5 รายการ)
- หมุนเพื่อเลือก, กดเพื่อยืนยัน
- กดค้างเพื่อย้อนกลับ
- Position limits

#### `User/Lib/RotaryEncoder/Examples/04_VolumeControl.c` (NEW)
- ควบคุมระดับ 0-100
- แสดงผลด้วย PWM (LED brightness)
- Bar graph visualization
- Acceleration mode toggle

#### `User/Lib/RotaryEncoder/Examples/05_DistanceMeasure.c` (NEW)
- วัดระยะทาง/มุมจากจำนวน pulses
- คำนวณ mm, cm, degrees
- Statistics tracking (min, max, average)
- Wheel diameter configuration

#### `User/Lib/RotaryEncoder/Examples/06_AdvancedCallbacks.c` (NEW)
- Event-driven programming
- Callback functions ทั้ง 5 ตัว
- State machine implementation
- Statistics display

#### `User/Lib/RotaryEncoder/Examples/07_Acceleration.c` (NEW)
- เปรียบเทียบ normal vs acceleration mode
- แสดง acceleration factor real-time
- Wide range support (0-1000)
- Performance comparison

#### `User/Lib/RotaryEncoder/README.md` (NEW)
- เอกสารภาษาไทยฉบับสมบูรณ์ 1000+ บรรทัด
- ครอบคลุม: ภาพรวม, ทฤษฎี Rotary Encoder (Quadrature encoding, State machine, Gray code)
- Hardware Setup (วงจรต่อ, รายการอุปกรณ์, ข้อควรระวัง)
- การติดตั้งและ Quick Start (5 ตัวอย่าง)
- API Reference ครบถ้วน (25+ ฟังก์ชัน)
- ตัวอย่างการใช้งาน (5 categories)
- เทคนิคขั้นสูง (8 techniques: Multi-encoder, Acceleration tuning, Debounce optimization, EEPROM persistence, Interrupt optimization, Memory optimization, Advanced button patterns, Velocity calculation)
- Troubleshooting (8 ปัญหาพบบ่อย)

### Technical Details

**Quadrature Encoding:**
- ใช้ 2 signals (CLK และ DT) ที่มี phase shift 90°
- State machine 4 states (00, 01, 10, 11)
- Gray code transitions (เปลี่ยน 1 bit ต่อครั้ง)
- Lookup table สำหรับ decode direction

**Acceleration Algorithm:**
```c
// คำนวณ RPM จาก time between rotations
uint32_t rpm = (60000 / time_diff) / 20;  // 20 pulses/rotation

// กำหนด acceleration factor ตาม RPM
if (rpm > 200)      factor = 8;  // Very fast
else if (rpm > 120) factor = 4;  // Fast
else if (rpm > 60)  factor = 2;  // Medium
else                factor = 1;  // Normal
```

**Button Detection:**
- Press/Release: Edge detection
- Long Press: Duration > 500ms
- Double Click: 2 clicks within 300ms
- Debouncing: Software debounce (configurable)

### สรุป
- **รวมโค้ด**: ~3,000 บรรทัด (core + examples + docs)
- **ไฟล์ใหม่**: 10 ไฟล์ (2 core + 7 examples + 1 README)
- **ตัวอย่าง**: 7 ตัวอย่าง (basic: 2, intermediate: 3, advanced: 2)
- **เอกสาร**: README 1000+ บรรทัดภาษาไทย
- **API Functions**: 25+ ฟังก์ชัน
- **Callbacks**: 5 callback functions
- **Features**: Quadrature decoding, Direction detection, Position counter, Button support, Acceleration, Debouncing, Callbacks, Limits
- **Max Encoders**: 8 encoders พร้อมกัน
- **Pin Requirements**: CLK/DT ต้องรองรับ EXTI, SW pin ใดก็ได้

---

## [2025-12-22 09:53] - Buzzer Control Library v1.0

### เพิ่มเติม (Added)
- **Buzzer Library**: Library สำหรับควบคุม Buzzer แบบครบวงจร
  - **Active High/Low Support**: รองรับทั้ง Active High และ Active Low buzzer
  - **Frequency Control**: ควบคุมความถี่ 20Hz - 20kHz
  - **Musical Notes**: โน้ตเพลงครบถ้วน (C3-C6) รวม 48 โน้ต
  - **Melody Playback**: เล่นทำนองพร้อม Note structures
  - **Rhythm Patterns**: สร้างจังหวะต่างๆ
  - **Blocking/Non-blocking**: รองรับทั้ง 2 โหมด
  - **Volume Control**: ปรับระดับเสียง 0-100%
  - **Pre-defined Patterns**: SOS, Alarm, Success, Error, Startup
  - **Frequency Sweep**: เสียงไซเรน (siren effect)
  - **7 Examples**: ตัวอย่างจากพื้นฐานถึงขั้นสูง
  - **Thai Documentation**: เอกสาร 1000+ บรรทัด

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/Lib/Buzzer/Buzzer.h` (NEW)
- เพิ่ม BuzzerType enum (BUZZER_ACTIVE_HIGH, BUZZER_ACTIVE_LOW)
- เพิ่ม Note structure (frequency, duration)
- เพิ่ม Melody structure (notes, length, repeat)
- เพิ่ม BuzzerState enum (IDLE, PLAYING, PAUSED)
- เพิ่ม Musical Notes definitions (48 โน้ต: C3-C6)
- เพิ่ม Note Duration constants (WHOLE, HALF, QUARTER, EIGHTH, SIXTEENTH)
- เพิ่ม Initialization API: Init, SetVolume
- เพิ่ม Basic Control API: On, Off, Toggle, Beep, BeepMultiple
- เพิ่ม Tone Control API: Tone, ToneAsync, NoTone
- เพิ่ม Melody API: PlayMelody, PlayMelodyAsync, Update
- เพิ่ม Advanced API: Stop, Pause, Resume, IsBusy, GetState, FrequencySweep
- เพิ่ม Pre-defined Patterns: PlaySOS, PlayAlarm, PlaySuccess, PlayError, PlayStartup
- รวม ~550 บรรทัด

#### `User/Lib/Buzzer/Buzzer.c` (NEW)
- Implement pin_to_pwm_channel() - แปลง GPIO pin เป็น PWM channel
- Implement set_pwm_frequency() - ตั้งค่าความถี่ PWM
- Implement set_pwm_duty() - ตั้งค่า duty cycle (รองรับ Active Low inversion)
- Implement start_tone() / stop_tone() - เริ่ม/หยุดเล่นโทนเสียง
- Implement Buzzer_Init() - เริ่มต้นการใช้งาน
- Implement Buzzer_SetVolume() - ปรับระดับเสียง
- Implement Basic Control functions (On, Off, Toggle, Beep, BeepMultiple)
- Implement Tone Control functions (Tone, ToneAsync, NoTone)
- Implement Melody functions (PlayMelody, PlayMelodyAsync, Update)
- Implement Advanced functions (Stop, Pause, Resume, IsBusy, GetState, FrequencySweep)
- Implement Pre-defined Patterns (SOS, Alarm, Success, Error, Startup)
- รวม ~550 บรรทัด

#### `User/Lib/Buzzer/Examples/01_BasicBeep.c` (NEW)
- ตัวอย่างการใช้งานพื้นฐาน
- Simple On/Off, Single Beep, Multiple Beeps
- Toggle และ Volume Control
- รวม ~120 บรรทัด

#### `User/Lib/Buzzer/Examples/02_ToneControl.c` (NEW)
- ควบคุมโทนเสียงที่ความถี่ต่างๆ
- Musical Scale (C Major, C Minor, Chromatic)
- Frequency Sweep และ Siren Effects
- Octave Comparison
- รวม ~180 บรรทัด

#### `User/Lib/Buzzer/Examples/03_MelodyPlayer.c` (NEW)
- เล่นทำนองต่างๆ
- Happy Birthday, Jingle Bells, Twinkle Twinkle Little Star
- Super Mario Bros Theme, Nokia Ringtone
- Custom Melody
- รวม ~200 บรรทัด

#### `User/Lib/Buzzer/Examples/04_RhythmPatterns.c` (NEW)
- สร้างจังหวะต่างๆ
- Morse Code (SOS), Notification Sounds
- Alarm Patterns, Game Sounds
- Door Bell, Phone Ringtone
- รวม ~180 บรรทัด

#### `User/Lib/Buzzer/Examples/05_NonBlocking.c` (NEW)
- การใช้งานแบบ Non-blocking
- Async Tone, Background Melody
- Event-driven Beeps, Pause/Resume
- Multi-tasking
- รวม ~200 บรรทัด

#### `User/Lib/Buzzer/Examples/06_AlarmSystem.c` (NEW)
- ระบบเตือนภัยต่างๆ
- Fire Alarm, Burglar Alarm, Gas Leak Alarm
- Multi-stage Warning, Emergency Alert
- Escalating Warning
- รวม ~220 บรรทัด

#### `User/Lib/Buzzer/Examples/07_MusicNotes.c` (NEW)
- เล่นโน้ตเพลงจริง
- Scales (Major, Minor, Chromatic)
- Arpeggios, Musical Intervals, Chords
- Classical Pieces (Für Elise, Canon in D, Ode to Joy)
- Rhythm Variations
- รวม ~250 บรรทัด

#### `User/Lib/Buzzer/README.md` (NEW)
- เอกสารภาษาไทยฉบับสมบูรณ์ 1000+ บรรทัด
- ครอบคลุม: ภาพรวม, ทฤษฎี Buzzer (Active vs Passive, High vs Low)
- Hardware Setup (วงจร Active High/Low พร้อม transistor)
- การติดตั้งและ Quick Start
- API Reference ครบถ้วน (30+ ฟังก์ชัน)
- ตัวอย่างการใช้งาน (7 categories)
- เทคนิคขั้นสูง (Melody creation, Tempo, Chord, Pattern, Volume fade, Memory optimization)
- Musical Notes Reference (ตารางความถี่ทุกโน้ต)
- Troubleshooting (6 ปัญหาพบบ่อย)

### สรุป
- **รวมโค้ด**: ~2,500 บรรทัด (core + examples + docs)
- **ไฟล์ใหม่**: 9 ไฟล์ (2 core + 7 examples)
- **ตัวอย่าง**: 7 ตัวอย่าง (basic: 2, intermediate: 3, advanced: 2)
- **เอกสาร**: README 1000+ บรรทัดภาษาไทย
- **API Functions**: 30+ ฟังก์ชัน
- **Musical Notes**: 48 โน้ต (C3-C6)
- **Pre-defined Patterns**: 5 patterns
- **PWM Pins Support**: 8 pins (PA1, PC0, PC3-4, PD2-4, PD7)
- **Features**: Tone control, Melody playback, Rhythm patterns, Volume control, Frequency sweep

---

## [2025-12-22 09:38] - SimpleDelay v1.1 - SysTick-Based Delay Functions

### ปรับปรุง (Improved)
- **Delay Functions Accuracy**: ปรับปรุง `Delay_Us()` และ `Delay_Ms()` ให้ใช้ SysTick timer แทน NOP loops
  - **ความแม่นยำสูงขึ้น**: ใช้ hardware timer (`Get_CurrentUs`/`Get_CurrentMs`) แทน software loop
  - **ไม่ขึ้นกับ Optimization**: Compiler optimization ไม่กระทบความแม่นยำ
  - **Overflow-Safe**: ใช้ unsigned arithmetic เพื่อจัดการ overflow อัตโนมัติ
  - **ยังคงเป็น Blocking**: ทำงานแบบ blocking delay ตามเดิม
  - **Backward Compatible**: API ยังคงเหมือนเดิม 100%

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/SimpleDelay.c` (MODIFIED)
- ปรับปรุง `Delay_Us()` (บรรทัด 73-79):
  - เปลี่ยนจาก NOP loop เป็นการใช้ `Get_CurrentUs()`
  - เพิ่ม zero-check optimization (`if (n == 0) return`)
  - ใช้ unsigned arithmetic: `while ((Get_CurrentUs() - start) < n)`
  - ลบ volatile loop และ `__asm__("nop")` ออก
- ปรับปรุง `Delay_Ms()` (บรรทัด 88-92):
  - เปลี่ยนจาก loop ของ `Delay_Us(1000)` เป็นการใช้ `Get_CurrentMs()`
  - เพิ่ม zero-check optimization (`if (n == 0) return`)
  - ใช้ unsigned arithmetic: `while ((Get_CurrentMs() - start) < n)`
  - ไม่มีการสะสมของความผิดพลาด
- อัปเดต documentation ทั้ง 2 ฟังก์ชัน

#### `User/SimpleHAL/SimpleDelay.h` (MODIFIED)
- อัปเดต `Delay_Us()` documentation:
  - เพิ่ม: "ใช้ SysTick timer สำหรับความแม่นยำสูง (ความละเอียด ~1us)"
  - เพิ่ม: "รองรับ overflow อัตโนมัติด้วย unsigned arithmetic"
  - เพิ่ม: "ไม่ขึ้นกับ compiler optimization"
- อัปเดต `Delay_Ms()` documentation:
  - เพิ่ม: "ใช้ SysTick timer สำหรับความแม่นยำสูง (ความละเอียด 1ms)"
  - เพิ่ม: "รองรับ overflow อัตโนมัติด้วย unsigned arithmetic"
  - เพิ่ม: "ไม่ขึ้นกับ compiler optimization"

### Technical Details

**Overflow-Safe Arithmetic:**
```c
// ตัวอย่าง: ถ้า timer overflow
uint32_t start = 0xFFFFFFF0;  // ใกล้ overflow
uint32_t current = 0x00000010; // overflow แล้ว
uint32_t elapsed = current - start; // = 0x20 (32) ✅ ถูกต้อง
```

**ก่อนแก้ไข (NOP-based):**
```c
void Delay_Us(uint32_t n) {
  while (n--) {
    for (volatile uint32_t i = 0; i < 3; i++) {
      __asm__("nop");
    }
  }
}
```

**หลังแก้ไข (SysTick-based):**
```c
void Delay_Us(uint32_t n) {
  if (n == 0) return;
  uint32_t start = Get_CurrentUs();
  while ((Get_CurrentUs() - start) < n) {
    // รอจนกว่าเวลาจะผ่านไป
  }
}
```

### ข้อดี
- ✅ **ความแม่นยำสูงขึ้น**: ใช้ hardware timer แทน software loop
- ✅ **ไม่ขึ้นกับ CPU clock**: ใช้ SysTick timer ที่มีความถี่คงที่
- ✅ **ไม่ขึ้นกับ optimization**: Compiler optimization ไม่กระทบ
- ✅ **รองรับ overflow**: ใช้ unsigned arithmetic
- ✅ **Backward compatible**: API เดิมยังใช้ได้ 100%
- ✅ **Zero overhead**: เพิ่ม zero-check เพื่อ performance

### สรุป
- **ไฟล์ที่แก้ไข**: 2 ไฟล์ (SimpleDelay.c, SimpleDelay.h)
- **ฟังก์ชันที่ปรับปรุง**: 2 ฟังก์ชัน (Delay_Us, Delay_Ms)
- **Breaking Changes**: ไม่มี
- **Backward Compatible**: ✅ ใช่
- **Performance**: เท่าเดิมหรือดีกว่า (มี zero-check)
- **Accuracy**: ดีขึ้นมาก (hardware timer vs software loop)

---

## [2025-12-22 09:19] - NTC10K Temperature Sensor Library v1.0

### เพิ่มเติม (Added)
- **NTC10K Library**: Library สำหรับวัดอุณหภูมิด้วย NTC 10K thermistor
  - **Calculation Methods**: รองรับทั้ง Steinhart-Hart equation (แม่นยำสูง) และ Beta equation (เร็ว)
  - **Voltage Divider**: รองรับทั้ง NTC-to-GND และ NTC-to-VCC configuration
  - **Calibration System**: Single-point และ two-point calibration พร้อม gain correction
  - **Averaging & Filtering**: ลด noise ด้วย multi-sample averaging
  - **Multi-Sensor Support**: รองรับสูงสุด 4 sensors พร้อมกัน
  - **Temperature Monitoring**: Threshold detection พร้อม callback system
  - **Temperature Range**: -40°C ถึง +125°C
  - **7 Examples**: ตัวอย่างจากพื้นฐานถึงโปรเจกต์จริง
  - **Thai Documentation**: README พร้อมวงจร และสูตรคำนวณ

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/Lib/NTC10K/NTC10K.h` (NEW)
- เพิ่ม NTC configuration structures (NTC_Config, NTC_Instance)
- เพิ่ม calculation method enums (Beta, Steinhart-Hart)
- เพิ่ม divider configuration enums (NTC-to-GND, NTC-to-VCC)
- เพิ่ม Initialization API: Init, InitWithConfig
- เพิ่ม Core API: ReadTemperature, ReadTemperatureF, ReadResistance, ReadADC
- เพิ่ม Advanced API: ReadAverage, SetCalibration, SetCalibrationAdvanced, SetBValue, SetCoefficients, SetMethod
- เพิ่ม Monitoring API: SetThreshold, SetCallback, EnableThreshold, Update
- เพิ่ม Utility API: CelsiusToFahrenheit, FahrenheitToCelsius, IsValid, UpdateAll, GetInstanceByChannel
- รวม ~450 บรรทัด

#### `User/Lib/NTC10K/NTC10K.c` (NEW)
- Implement CalculateResistance() - คำนวณความต้านทานจาก ADC value (รองรับทั้ง 2 divider configs)
- Implement CalculateTempBeta() - Beta equation: 1/T = 1/T0 + (1/B)×ln(R/R0)
- Implement CalculateTempSteinhartHart() - Steinhart-Hart equation: 1/T = A + B×ln(R) + C×(ln(R))³
- Implement ApplyCalibration() - ใช้ offset และ gain correction
- Implement CheckThresholds() - ตรวจสอบขีดจำกัดและเรียก callback
- Implement multi-sensor management พร้อม instance tracking
- รวม ~420 บรรทัด

#### `User/Lib/NTC10K/Examples/01_Basic_Temperature.c` (NEW)
- ตัวอย่างพื้นฐาน: อ่านและแสดงอุณหภูมิ
- แสดงการใช้ NTC_Init() และ NTC_ReadTemperature()
- แสดงค่า ADC, ความต้านทาน, และอุณหภูมิ (°C และ °F)

#### `User/Lib/NTC10K/Examples/02_With_Averaging.c` (NEW)
- ตัวอย่างการใช้ averaging เพื่อลด noise
- เปรียบเทียบ single reading vs averaged reading
- แสดง noise level indicator

#### `User/Lib/NTC10K/Examples/03_Calibration.c` (NEW)
- ตัวอย่างการ calibrate sensor แบบ step-by-step
- คำนวณ offset จากอุณหภูมิอ้างอิง
- ทดสอบความแม่นยำหลัง calibration

#### `User/Lib/NTC10K/Examples/04_Multi_Sensor.c` (NEW)
- ตัวอย่างการใช้ 3 sensors พร้อมกัน
- แสดงผลแบบตาราง
- คำนวณค่าเฉลี่ย, สูงสุด, ต่ำสุด, และ delta

#### `User/Lib/NTC10K/Examples/05_Temperature_Monitor.c` (NEW)
- ระบบเฝ้าระวังอุณหภูมิพร้อม threshold detection
- LED indicators (Red=High, Blue=Low, Green=Normal)
- Callback functions สำหรับแจ้งเตือน
- Bar graph visualization

#### `User/Lib/NTC10K/Examples/06_Data_Logger.c` (NEW)
- บันทึกข้อมูลอุณหภูมิ (100 samples)
- คำนวณสถิติ (min, max, average)
- Export ข้อมูลเป็น CSV format

#### `User/Lib/NTC10K/Examples/07_Project_ThermoController.c` (NEW)
- โปรเจกต์ควบคุมอุณหภูมิแบบสมบูรณ์
- ควบคุม heater/cooler ด้วย relay
- On-off control พร้อม hysteresis
- ตั้งค่าอุณหภูมิเป้าหมายด้วยปุ่ม
- LED status indicator

#### `User/Lib/NTC10K/README.md` (NEW)
- คู่มือการใช้งานภาษาไทย
- Voltage divider circuits (2 แบบ) พร้อม ASCII diagram
- สูตรคำนวณความต้านทานและอุณหภูมิ
- Beta equation และ Steinhart-Hart equation
- Quick start guide
- API reference summary
- Troubleshooting guide

#### `User/SimpleHAL/SimpleHAL.h` (MODIFIED)
- อัปเดต version 1.8.0 → 1.9.0

### สรุป
- **รวมโค้ด**: ~1,500 บรรทัด (core + examples + docs)
- **ไฟล์ใหม่**: 10 ไฟล์ (2 core + 7 examples + 1 README)
- **ตัวอย่าง**: 7 ตัวอย่าง (basic: 3, intermediate: 2, advanced: 2)
- **เอกสาร**: README พร้อมวงจรและสูตรคำนวณ
- **API Functions**: 20+ ฟังก์ชัน
- **Calculation Methods**: 2 methods (Beta, Steinhart-Hart)
- **Divider Configs**: 2 configs (NTC-to-GND, NTC-to-VCC)
- **Temperature Range**: -40°C ถึง +125°C
- **Max Sensors**: 4 sensors พร้อมกัน
- **SimpleHAL Version**: 1.9.0

---

## [2025-12-22 07:20] - 🚨 CRITICAL BUGFIX: ADC Pin Mapping Correction

### แก้ไขบั๊กร้ายแรง (Critical Bugfix)
- **ADC Pin Mapping ผิดพลาด**: แก้ไขการแมป ADC pins ให้ตรงกับ CH32V003 datasheet
  - **ข้อมูลเดิม (ผิด)**: ADC รองรับ PD2-PD7 (6 pins)
  - **ข้อมูลใหม่ (ถูก)**: ADC รองรับ PA1, PA2, PC4, PD2-PD6 (8 pins)
  - **❌ PD7 ไม่รองรับ ADC!** (ข้อมูลเดิมผิด)

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/SimpleADC.h` (MODIFIED)
- แก้ไข `ADC_Channel` enum ให้ตรงกับ datasheet
- เปลี่ยนจาก `ADC_CH_A0-A7` เป็น `ADC_CH_0-7`
- เพิ่ม aliases: `ADC_CH_PA1`, `ADC_CH_PA2`, `ADC_CH_PC4`, `ADC_CH_PD2-6`, `ADC_CH_PD4`
- อัปเดต documentation ทั้งหมด

#### `User/SimpleHAL/SimpleADC.c` (MODIFIED)
- แก้ไข `GetADCChannel()` - ใช้ direct mapping (channel = enum value)
- เพิ่ม `GetGPIOPort()` - รองรับ GPIOA, GPIOC, GPIOD
- แก้ไข `GetGPIOPin()` - แมป pins ถูกต้องตาม datasheet
- แก้ไข `ADC_EnableChannel()` - เปิด clock ตาม port ที่ถูกต้อง
- อัปเดต `ADC_SimpleInit()` - เปิดทั้ง 8 channels

#### `User/SimpleHAL/SimpleGPIO.h` (MODIFIED)
- แก้ไข `IS_ADC_PIN()` macro: `PD2-PD7` → `PA1, PA2, PC4, PD2-PD6`
- อัปเดต `analogRead()` documentation และ examples
- แก้ไข error message: "Only PD2-PD7" → "Only PA1, PA2, PC4, PD2-PD6"

#### `User/SimpleHAL/SimpleGPIO.c` (MODIFIED)
- แก้ไข `mapPinToADC()` - เพิ่ม PA1, PA2, PC4, ลบ PD7
- แก้ไข `_analogRead_impl()` runtime validation

### ADC Channel Mapping (ถูกต้อง)

| Channel | GPIO | เดิม (ผิด) | ใหม่ (ถูก) |
|---------|------|-----------|-----------|
| CH 0    | PA2  | PD7 ❌    | PA2 ✅    |
| CH 1    | PA1  | -         | PA1 ✅    |
| CH 2    | PC4  | -         | PC4 ✅    |
| CH 3    | PD2  | PD2 ✅    | PD2 ✅    |
| CH 4    | PD3  | PD3 ✅    | PD3 ✅    |
| CH 5    | PD5  | PD5 ✅    | PD5 ✅    |
| CH 6    | PD6  | PD6 ✅    | PD6 ✅    |
| CH 7    | PD4  | PD4 ✅    | PD4 ✅    |

### Breaking Changes
- ❌ `ADC_CH_A0-A7` ตอนนี้แมปตาม hardware channels ที่ถูกต้อง:
  - `ADC_CH_A0` = Channel 0 (PA2) - เดิมเป็น PD2
  - `ADC_CH_A1` = Channel 1 (PA1) - เดิมเป็น PD3
  - `ADC_CH_A2` = Channel 2 (PC4) - เดิมเป็น PD4
  - `ADC_CH_A3` = Channel 3 (PD2) - ถูกต้องแล้ว
  - `ADC_CH_A4` = Channel 4 (PD3) - ถูกต้องแล้ว
  - `ADC_CH_A5` = Channel 5 (PD5) - ถูกต้องแล้ว
  - `ADC_CH_A6` = Channel 6 (PD6) - ถูกต้องแล้ว
  - `ADC_CH_A7` = Channel 7 (PD4) - ถูกต้องแล้ว
- ✅ แนะนำให้ใช้ชื่อใหม่: `ADC_CH_PA2`, `ADC_CH_PA1`, `ADC_CH_PC4`, `ADC_CH_PD2-6`, `ADC_CH_PD4`
- ✅ หรือใช้ `ADC_CH_0-7` (ตรงกับ channel number)

### Impact
- **High**: ผู้ใช้ที่พยายามใช้ ADC กับ PA1, PA2, PC4 จะได้ error (แต่ควรได้)
- **High**: ผู้ใช้ที่พยายามใช้ ADC กับ PD7 จะได้ error (ถูกต้อง - PD7 ไม่รองรับ ADC)
- **Medium**: ต้องอัปเดตโค้ดที่ใช้ `ADC_CH_A0-A7`

### สรุป
- **ไฟล์ที่แก้ไข**: 4 ไฟล์
- **ADC Pins ที่รองรับ**: 8 pins (PA1, PA2, PC4, PD2-PD6)
- **Breaking Changes**: ✅ มี (enum names เปลี่ยน)
- **Backward Compatible**: ❌ ไม่ใช่ (ต้องอัปเดตโค้ด)
- **Severity**: 🚨 CRITICAL (ข้อมูลเดิมผิดพลาด)

---

## [2025-12-22 07:12] - SimpleGPIO v1.3 - Hybrid Pin Validation

### เพิ่มเติม (Added)
- **Hybrid Pin Validation**: ระบบตรวจสอบ pin ที่ไม่รองรับแบบ compile-time และ runtime
  - **Compile-Time Check**: ตรวจสอบ constant pins ตอนคอมไพล์ด้วย `_Static_assert`
  - **Runtime Check**: ตรวจสอบ variable pins ตอน runtime (return 0 หรือไม่ทำงาน)
  - **Pin Support Macros**: `IS_ADC_PIN()` และ `IS_PWM_PIN()` สำหรับตรวจสอบ pin support
  - **Error Messages**: ข้อความ error ชัดเจน บอกว่า pin ไหนรองรับอะไร
  - **Zero Overhead**: ไม่มี performance overhead สำหรับ constant pins

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/SimpleGPIO.h` (MODIFIED)
- อัปเดต version 1.2 → 1.3
- เพิ่ม `IS_ADC_PIN()` macro: ตรวจสอบว่า pin รองรับ ADC (PD2-PD7)
- เพิ่ม `IS_PWM_PIN()` macro: ตรวจสอบว่า pin รองรับ PWM (PA1, PC0, PC3-4, PD2-4, PD7)
- แปลง `analogRead()` เป็น hybrid validation macro ด้วย `__builtin_choose_expr()`
- แปลง `analogWrite()` เป็น hybrid validation macro ด้วย `__builtin_choose_expr()`
- เพิ่ม `_analogRead_impl()` function declaration (internal implementation)
- เพิ่ม `_analogWrite_impl()` function declaration (internal implementation)
- อัปเดต documentation พร้อมตัวอย่างการใช้งานและ error messages

#### `User/SimpleHAL/SimpleGPIO.c` (MODIFIED)
- อัปเดต version 1.2 → 1.3
- เปลี่ยนชื่อ `analogRead()` → `_analogRead_impl()`
- เปลี่ยนชื่อ `analogWrite()` → `_analogWrite_impl()`
- เพิ่ม runtime validation ใน `_analogRead_impl()`: ตรวจสอบ PD2-PD7
- เพิ่ม runtime validation ใน `_analogWrite_impl()`: ตรวจสอบ PWM pins

### Technical Details

**GCC Built-in Functions ที่ใช้:**
- `__builtin_constant_p(expr)`: ตรวจสอบว่า expression เป็น compile-time constant
- `__builtin_choose_expr(cond, expr1, expr2)`: เลือก expression ตาม condition
- `_Static_assert(cond, msg)`: Compile-time assertion (C11)

**Validation Flow:**
```c
analogRead(pin) → __builtin_constant_p(pin)?
  ├─ Yes (constant) → _Static_assert(IS_ADC_PIN(pin)) → _analogRead_impl(pin)
  └─ No (variable)  → _analogRead_impl(pin) [runtime check inside]
```

### ตัวอย่างการใช้งาน

**Compile-Time Validation (Constants):**
```c
uint16_t value = analogRead(PD2);  // ✓ OK
analogWrite(PA1, 128);             // ✓ OK

uint16_t value = analogRead(PA1);  // ✗ Compile Error!
// error: analogRead: Pin does not support ADC! Only PD2-PD7 support ADC.

analogWrite(PA2, 128);             // ✗ Compile Error!
// error: analogWrite: Pin does not support PWM! Supported: PA1, PC0, PC3-4, PD2-4, PD7
```

**Runtime Validation (Variables):**
```c
uint8_t sensor_pin = get_sensor_pin();
uint16_t value = analogRead(sensor_pin);
if (value == 0) {
    // Pin ไม่รองรับ ADC หรือเกิดข้อผิดพลาด
}
```

### สรุป
- **Validation Types**: 2 types (compile-time + runtime)
- **Macros เพิ่ม**: 2 macros (IS_ADC_PIN, IS_PWM_PIN)
- **Functions เปลี่ยน**: 2 functions (analogRead, analogWrite → macros)
- **Implementation Functions**: 2 functions (_analogRead_impl, _analogWrite_impl)
- **Breaking Changes**: ไม่มี (API เดิมยังใช้ได้)
- **Backward Compatible**: ✅ ใช่
- **Performance**: Zero overhead สำหรับ constant pins
- **SimpleGPIO Version**: 1.3

---

## [2025-12-21 21:52] - TJC Library File Name Standardization

### เปลี่ยนแปลง (Changed)
- **TJC Library File Names**: เปลี่ยนชื่อไฟล์ให้สอดคล้องกับรูปแบบการตั้งชื่อของ libraries อื่นๆ
  - **Renamed**: `tjc_hmi.h` → `TJC.h`
  - **Renamed**: `tjc_hmi.c` → `TJC.c`
  - **Updated**: Header guards จาก `__TJC_HMI_H__` เป็น `__TJC_H__`
  - **Updated**: Documentation และ include statements ทั้งหมด
  - **Updated**: Build system (makefile) ให้ใช้ชื่อไฟล์ใหม่

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/Lib/TJC/TJC.h` (RENAMED from tjc_hmi.h)
- เปลี่ยนชื่อไฟล์จาก `tjc_hmi.h` เป็น `TJC.h`
- อัปเดต `@file` documentation
- อัปเดต header guard: `__TJC_HMI_H__` → `__TJC_H__`

#### `User/Lib/TJC/TJC.c` (RENAMED from tjc_hmi.c)
- เปลี่ยนชื่อไฟล์จาก `tjc_hmi.c` เป็น `TJC.c`
- อัปเดต `@file` documentation
- อัปเดต include: `#include "tjc_hmi.h"` → `#include "TJC.h"`

#### `User/Lib/TJC/Examples/example_tjc_usage.c` (MODIFIED)
- อัปเดต include statements (2 ตำแหน่ง): `tjc_hmi.h` → `TJC.h`

#### `User/test_userlib_compile.c` (MODIFIED)
- เพิ่ม `#include "Lib/TJC/TJC.h"`
- เพิ่ม `test_tjc_library()` function เพื่อทดสอบ compilation

#### `obj/User/Lib/TJC/subdir.mk` (MODIFIED)
- อัปเดต source file references: `tjc_hmi.c` → `TJC.c`
- อัปเดต object file references: `tjc_hmi.o` → `TJC.o`
- อัปเดต dependency file references: `tjc_hmi.d` → `TJC.d`

### สรุป
- **ไฟล์ที่เปลี่ยนชื่อ**: 2 ไฟล์ (TJC.h, TJC.c)
- **ไฟล์ที่แก้ไข**: 3 ไฟล์ (example_tjc_usage.c, test_userlib_compile.c, subdir.mk)
- **Breaking Changes**: ไม่มี (เป็นการเปลี่ยนชื่อไฟล์เท่านั้น)
- **Backward Compatible**: ❌ ต้องอัปเดต include statements ในโค้ดที่ใช้งาน TJC library
- **Naming Convention**: ตรงกับ libraries อื่นๆ (IR.h, NeoPixel.h, MAX7219.h)

---

## [2025-12-21 17:45] - SimpleGPIO v1.8 - Advanced GPIO Functions

### เพิ่มเติม (Added)
- **Advanced GPIO Functions**: เพิ่มฟังก์ชัน GPIO ขั้นสูง 4 ตัวใน SimpleGPIO
  - **digitalWriteMultiple()**: เขียนค่าไปหลาย pins พร้อมกัน (ใช้ macro คำนวณขนาดอัตโนมัติ)
  - **pulseIn()**: วัดความกว้าง pulse (microseconds) สำหรับ ultrasonic sensor, IR receiver
  - **shiftOut()**: Software SPI output สำหรับ 74HC595, LED matrix, shift register
  - **shiftIn()**: Software SPI input สำหรับ 74HC165, keypad matrix
  - **Bit Order Constants**: LSBFIRST, MSBFIRST

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/SimpleGPIO.h` (MODIFIED)
- เพิ่ม bit order constants: `LSBFIRST` (0), `MSBFIRST` (1)
- เพิ่ม `_digitalWriteMultiple()` function prototype (internal)
- เพิ่ม `digitalWriteMultiple()` macro (auto-calculate array size)
- เพิ่ม `pulseIn()` function prototype
- เพิ่ม `shiftOut()` function prototype
- เพิ่ม `shiftIn()` function prototype
- Documentation ครบถ้วนพร้อมตัวอย่างการใช้งาน

#### `User/SimpleHAL/SimpleGPIO.c` (MODIFIED)
- Implement `_digitalWriteMultiple()` - loop เรียก digitalWrite()
- Implement `pulseIn()` - ใช้ micros() วัดเวลา, มี timeout protection
- Implement `shiftOut()` - bit-bang SPI output (~100kHz)
- Implement `shiftIn()` - bit-bang SPI input (~100kHz)

#### `User/SimpleHAL/SimpleHAL.h` (MODIFIED)
- อัพเดท version 1.7.0 → 1.8.0

#### `User/SimpleHAL/README.md` (MODIFIED)
- อัพเดท version และ changelog

#### `readme/2025-12-21_Library_SimpleGPIO_Advanced_Functions.md` (NEW)
- เอกสารสรุปการเพิ่ม advanced functions
- Use cases: Ultrasonic, Shift Register, Multiple I/O
- Technical details และตัวอย่างโค้ด

### Use Cases

**1. Ultrasonic Distance Sensor (HC-SR04)**
```c
uint32_t duration = pulseIn(ECHO_PIN, HIGH, 30000);
float distance_cm = duration * 0.034 / 2;
```

**2. 74HC595 Shift Register (8-bit output)**
```c
digitalWrite(LATCH_PIN, LOW);
shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0b10101010);
digitalWrite(LATCH_PIN, HIGH);
```

**3. 74HC165 Shift Register (8-bit input)**
```c
digitalWrite(LOAD_PIN, HIGH);
uint8_t data = shiftIn(DATA_PIN, CLOCK_PIN, MSBFIRST);
```

**4. Multiple LED Control**
```c
const uint8_t leds[] = {PC0, PC1, PC2};
const uint8_t states[] = {HIGH, LOW, HIGH};
digitalWriteMultiple(leds, states);
```

### สรุป
- **ฟังก์ชันใหม่**: 4 ฟังก์ชัน (digitalWriteMultiple, pulseIn, shiftOut, shiftIn)
- **Constants**: 2 constants (LSBFIRST, MSBFIRST)
- **บรรทัดโค้ดเพิ่ม**: ~150 บรรทัด
- **Breaking Changes**: ไม่มี
- **Backward Compatible**: ✅ ใช่
- **SimpleHAL Version**: 1.8.0

---

## [2025-12-21 16:00] - SimpleGPIO v1.7 - PA Pin Support

### เพิ่มเติม (Added)
- **PA Pin Support**: เพิ่มการรองรับ PA1 และ PA2 pins ใน SimpleGPIO
  - **Digital I/O**: pinMode(), digitalWrite(), digitalRead(), digitalToggle() รองรับ PA1-PA2
  - **PWM Support**: PA1 รองรับ PWM1_CH2 (ใช้ผ่าน analogWrite)
  - **Interrupt Support**: attachInterrupt()/detachInterrupt() รองรับ PA pins
  - **Pin Numbering**: เปลี่ยน scheme ใหม่ (PA=0-1, PC=10-17, PD=20-25)
  - **Documentation**: อัพเดทเอกสารทุกไฟล์ให้ระบุ PA pins และข้อจำกัด

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/SimpleGPIO.h` (MODIFIED)
- เพิ่ม PA1 (=0) และ PA2 (=1) ใน GPIO_Pin enum
- เปลี่ยน PC pins: PC0-PC7 = 10-17 (เดิม 0-7)
- เปลี่ยน PD pins: PD2-PD7 = 20-25 (เดิม 10-17)
- อัพเดท documentation ทุกฟังก์ชันให้ระบุ PA pins
- ระบุชัดเจนว่า PA ไม่รองรับ ADC, PA1 รองรับ PWM

#### `User/SimpleHAL/SimpleGPIO.c` (MODIFIED)
- อัพเดท pin_map[] array เพิ่ม GPIOA entries (PA1, PA2)
- เพิ่ม PA1 ใน mapPinToPWM() สำหรับ analogWrite()
- GPIOA clock enable มีอยู่แล้ว (ไม่ต้องแก้ไข)

#### `User/SimpleHAL/SimpleHAL.h` (MODIFIED)
- อัพเดท version 1.6.0 → 1.7.0

#### `User/SimpleHAL/README.md` (MODIFIED)
- อัพเดท Pin Mapping section แสดง PA1-PA2
- เพิ่มตาราง PWM Support และ ADC Support
- อัพเดท version และ changelog

#### `readme/2025-12-21_Library_SimpleGPIO_PA_Support.md` (NEW)
- เอกสารสรุปการเพิ่ม PA pin support
- Objective, Implementation, Walkthrough
- Root Cause Analysis
- Breaking Changes warning

### สรุป
- **GPIO Ports**: GPIOA (PA1-PA2), GPIOC (PC0-PC7), GPIOD (PD2-PD7)
- **PWM Support**: PA1 (PWM1_CH2), PC0/3/4, PD2/3/4/7
- **ADC Support**: PD2-PD7 เท่านั้น (PA และ PC ไม่รองรับ)
- **Breaking Change**: Pin numbering เปลี่ยนแปลง (ใช้ชื่อ pin แทนตัวเลข)
- **SimpleHAL Version**: 1.7.0

---

## [2025-12-21 15:38] - WS2815 LED Matrix Library v1.0

### เพิ่มเติม (Added)
- **WS2815 LED Matrix Library**: Library ครบครันสำหรับควบคุม WS2815/WS2812 LED Matrix
  - **Matrix Support**: รองรับ 8x8 ถึง 32x32 pixels
  - **4 Wiring Patterns**: Zigzag Left/Right, Snake, Columns
  - **Coordinate System**: ระบบพิกัด (x, y) ใช้งานง่าย
  - **Graphics Primitives**: Pixel, Line (Bresenham), Rectangle, Circle (Midpoint), Bitmap, Sprite
  - **Text Rendering**: ASCII (5x7) + Thai (8x8) fonts พร้อม UTF-8 support
  - **Scrolling Text**: Horizontal/Vertical scrolling (non-blocking)
  - **Effects**: Fade in/out, Wipe, Slide transitions
  - **Pattern Generation**: Checkerboard, Gradient (H/V), Random
  - **Utilities**: Rotate, Mirror, Brightness control
  - **6 Examples**: Basic → Intermediate → Advanced
  - **Thai Documentation**: เอกสาร 800+ บรรทัด
  - **Total Code**: ~3,500 บรรทัด (core + fonts + examples + docs)

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/Lib/WS2815Matrix/SimpleWS2815Matrix.h` (NEW)
- เพิ่ม Matrix configuration structures (Matrix_Config_t, Point_t, Rect_t, Sprite_t, ScrollText_t)
- เพิ่ม Wiring pattern enums (4 patterns)
- เพิ่ม Initialization API: Init, SetWiringPattern, GetConfig
- เพิ่ม Basic Drawing API: SetPixel, GetPixel, Clear, Fill, Show
- เพิ่ม Shape Drawing API: DrawLine, DrawRect, FillRect, DrawCircle, FillCircle
- เพิ่ม Text API: DrawChar, DrawText, DrawCharThai, DrawTextThai, GetTextWidth
- เพิ่ม Scrolling API: ScrollTextInit, ScrollTextUpdate, ScrollTextStop
- เพิ่ม Sprite API: DrawSprite, DrawBitmap
- เพิ่ม Effects API: FadeIn, FadeOut, WipeTransition, SlideTransition
- เพิ่ม Utility API: Rotate, Mirror, Brightness, XYtoIndex, IsInBounds
- เพิ่ม Pattern API: Checkerboard, GradientH/V, Random
- รวม ~650 บรรทัด

#### `User/Lib/WS2815Matrix/SimpleWS2815Matrix.c` (NEW)
- Implement coordinate mapping สำหรับ 4 wiring patterns
- Implement Bresenham's line algorithm
- Implement Midpoint circle algorithm
- Implement filled shape algorithms
- Implement text rendering engine พร้อม UTF-8 support
- Implement Thai font lookup และ Unicode conversion
- Implement scrolling text engine (non-blocking)
- Implement sprite rendering พร้อม transparency
- Implement transition effects
- Implement pattern generation
- Implement buffer manipulation (rotate, mirror)
- รวม ~900 บรรทัด

#### `User/Lib/WS2815Matrix/fonts.h` (NEW)
- เพิ่ม font_5x7: Complete ASCII 32-126 (95 characters, 5x7 pixels)
- เพิ่ม font_thai_8x8: Thai consonants (43 characters, 8x8 pixels)
- เพิ่ม font_thai_numbers_8x8: Thai numbers (10 characters, 8x8 pixels)
- เพิ่ม ThaiCharMap_t และ thai_consonant_map สำหรับ Unicode mapping
- รวม ~700 บรรทัด

#### `User/Lib/WS2815Matrix/Examples/01_Basic_SetPixel.c` (NEW)
- ตัวอย่างการตั้งค่าพิกเซลแต่ละดวง
- แสดงการใช้ระบบพิกัด (x, y)
- แสดงการใช้สี RGB และ 32-bit color
- แสดงการอ่านค่าพิกเซล

#### `User/Lib/WS2815Matrix/Examples/02_Basic_Shapes.c` (NEW)
- ตัวอย่างการวาดรูปทรง (เส้น, สี่เหลี่ยม, วงกลม)
- แสดง outline และ filled shapes
- แสดงรูปทรงผสม (บ้าน)
- Animation: ขยาย/หด

#### `User/Lib/WS2815Matrix/Examples/03_Basic_Text.c` (NEW)
- ตัวอย่างการแสดงข้อความ ASCII
- ตัวอย่างการแสดงข้อความภาษาไทย
- แสดงเลขไทย
- แสดงสีต่างๆ

#### `User/Lib/WS2815Matrix/Examples/04_Basic_Patterns.c` (NEW)
- Checkerboard pattern
- Gradient patterns (H/V)
- Random noise

#### `User/Lib/WS2815Matrix/Examples/05_Intermediate_Scrolling.c` (NEW)
- Horizontal scrolling text
- Vertical scrolling text
- Speed control
- Thai text scrolling
- Multi-color scrolling

#### `User/Lib/WS2815Matrix/Examples/08_Advanced_Effects.c` (NEW)
- Fade in/out effects
- Wipe transition
- Slide transition
- Rainbow effect integration

#### `User/Lib/WS2815Matrix/README_TH.md` (NEW)
- เอกสารภาษาไทยฉบับสมบูรณ์ 800+ บรรทัด
- ครอบคลุม: บทนำ, คุณสมบัติ, WS2812 vs WS2815
- Hardware setup พร้อมการคำนวณกำลังไฟ
- Wiring patterns ทั้ง 4 แบบพร้อมภาพประกอบ
- การใช้งานขั้นพื้นฐาน (4 หัวข้อ)
- เทคนิคขั้นกลาง (4 หัวข้อ)
- เทคนิคขั้นสูง (4 หัวข้อ)
- API Reference ครบถ้วน
- Troubleshooting (6 ปัญหา)

#### `User/SimpleHAL/SimpleHAL.h` (MODIFIED)
- เพิ่ม `#include "../Lib/WS2815Matrix/SimpleWS2815Matrix.h"`
- อัปเดต version 1.5.0 → 1.6.0

### สรุป
- **รวมโค้ด**: ~3,500 บรรทัด (core + fonts + examples + docs)
- **ไฟล์ใหม่**: 10 ไฟล์ (3 core + 6 examples + 1 README)
- **ตัวอย่าง**: 6 ตัวอย่าง (basic: 4, intermediate: 1, advanced: 1)
- **เอกสาร**: 800+ บรรทัดภาษาไทย
- **API Functions**: 60+ ฟังก์ชัน
- **Fonts**: 2 fonts (ASCII 5x7, Thai 8x8)
- **Wiring Patterns**: 4 patterns
- **Matrix Sizes**: 8x8 ถึง 32x32
- **Graphics**: 5 primitives + text + sprites
- **Features**: Scrolling, Effects, Patterns, Utilities
- **SimpleHAL Version**: 1.6.0

---

## [2025-12-21 15:15] - MAX7219 LED Matrix Driver Library v1.0

### เพิ่มเติม (Added)
- **MAX7219 LED Matrix Driver Library**: Library ครบครันสำหรับควบคุม MAX7219 LED Matrix 8x8
  - **SPI Communication**: Hardware SPI พร้อม cascading support (1-8 matrices)
  - **Graphics Primitives**: Pixel, Line (Bresenham), Rectangle, Circle (Bresenham), Triangle, Bitmap
  - **Text Rendering**: ASCII fonts (5x7, 8x8) + Thai font (5x7)
  - **Scrolling Text**: Horizontal scrolling (non-blocking)
  - **Animation System**: Frame-based animation พร้อม loop support
  - **Sprite System**: Sprite rendering พร้อม transparency mask
  - **Effects**: Fade in/out, brightness control (0-15), invert
  - **14 Examples**: ตัวอย่างครบทุกระดับ (basic → advanced)
  - **Thai Documentation**: เอกสาร 1000+ บรรทัด
  - **Total Code**: ~5,000 บรรทัด (core + examples + docs)

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/Lib/MAX7219/SimpleMAX7219.h` (NEW)
- เพิ่ม MAX7219 register definitions (NOOP, DIGIT0-7, DECODE, INTENSITY, SCANLIMIT, SHUTDOWN, DISPLAYTEST)
- เพิ่ม MAX7219_Handle structure พร้อม buffer management (8 devices support)
- เพิ่ม MAX7219_Font structure สำหรับ font system
- เพิ่ม MAX7219_Animation และ MAX7219_Scroll structures
- เพิ่ม Initialization API: Init, SetIntensity, DisplayControl, Clear, Update, DisplayTest
- เพิ่ม Graphics API: SetPixel, GetPixel, DrawLine, DrawRect, DrawCircle, DrawTriangle, DrawBitmap
- เพิ่ม Text API: SetFont, DrawChar, DrawString, GetStringWidth
- เพิ่ม Scrolling API: StartScrollText, UpdateScroll, StopScroll
- เพิ่ม Animation API: StartAnimation, UpdateAnimation, StopAnimation
- เพิ่ม Sprite API: DrawSprite
- เพิ่ม Utility API: FadeIn, FadeOut, Invert
- รวม ~650 บรรทัด

#### `User/Lib/MAX7219/SimpleMAX7219.c` (NEW)
- Implement SPI communication protocol สำหรับ MAX7219
- Implement register write functions (single device และ cascaded)
- Implement display buffer management (8 bytes × 8 devices)
- Implement Bresenham's line algorithm
- Implement Bresenham's circle algorithm
- Implement filled shape algorithms (rectangle, circle, triangle)
- Implement text rendering engine พร้อม font lookup
- Implement scrolling engine (non-blocking)
- Implement animation state machine
- Implement sprite blitting พร้อม transparency
- Implement fade effects และ utility functions
- รวม ~850 บรรทัด

#### `User/Lib/MAX7219/max7219_fonts.h` (NEW)
- เพิ่ม font_5x7: Complete ASCII 32-127 (95 characters, 5x7 pixels)
- เพิ่ม font_8x8: Complete ASCII 32-127 (95 characters, 8x8 pixels)
- เพิ่ม font_thai_5x7: Thai characters subset (43 characters, 5x7 pixels)
- Font data รวม ~1,200 บรรทัด

#### `User/Lib/MAX7219/Examples/01_basic_display.c` (NEW)
- ตัวอย่างพื้นฐาน: Initialization, patterns (checkerboard, border, X, all on)
- Brightness control และ fade effects
- Blink effect

#### `User/Lib/MAX7219/Examples/02_text_display.c` (NEW)
- แสดงตัวอักษร ASCII ทีละตัว (A-Z, 0-9)
- แสดงคำสั้นๆ (HI, OK, GO)
- นับเลข และ countdown

#### `User/Lib/MAX7219/Examples/03_graphics_primitives.c` (NEW)
- Pixel animation
- Line drawing (diagonal, horizontal, vertical)
- Rectangle (outline, filled)
- Circle (outline, filled)
- Triangle (outline, filled)
- Combined shapes และ animated growing/shrinking

#### `User/Lib/MAX7219/Examples/04_brightness_control.c` (NEW)
- Fade in/out effects
- Pulse effect (heartbeat)
- Breathing effect
- Step brightness levels
- Random flicker effect

#### `User/Lib/MAX7219/Examples/05_scrolling_text.c` (NEW)
- Horizontal scrolling text (non-blocking)
- Multiple messages
- Continuous scrolling loop

#### `User/Lib/MAX7219/Examples/06_thai_text.c` (NEW)
- Thai font demonstration
- Mixed Thai/English text
- UTF-8 to Unicode conversion notes

#### `User/Lib/MAX7219/Examples/07_animation.c` (NEW)
- Bouncing ball animation (6 frames)
- Rotating line animation (8 frames)
- Spinner animation (4 frames)
- Custom growing square animation

#### `User/Lib/MAX7219/Examples/08_sprite_display.c` (NEW)
- Static sprites (smiley, heart, star, spaceship)
- Animated sprite movement (horizontal, vertical)
- Bouncing sprite
- Rotating sprites

#### `User/Lib/MAX7219/Examples/09_multiple_matrix.c` (NEW)
- Cascaded displays (4 matrices = 32x8 pixels)
- Wide graphics (line, rectangle across matrices)
- Long text display
- Wave animation

#### `User/Lib/MAX7219/Examples/10_game_snake.c` (NEW)
- Snake game implementation
- Button controls (4 directions)
- Collision detection
- Food spawning
- Game over และ restart

#### `User/Lib/MAX7219/Examples/11_clock_display.c` (NEW)
- Digital clock (HH:MM format)
- Blinking colon
- Scrolling messages every 10 seconds

#### `User/Lib/MAX7219/Examples/12_spectrum_analyzer.c` (NEW)
- ADC input reading
- Bar graph visualization
- Real-time update (50ms)

#### `User/Lib/MAX7219/Examples/13_weather_station.c` (NEW)
- Weather icons (sunny, cloudy, rainy)
- Temperature display
- Scrolling weather text
- Mode cycling

#### `User/Lib/MAX7219/Examples/14_custom_animation.c` (NEW)
- Plasma effect
- Ripple effect
- Matrix rain effect
- Starfield effect
- Smooth transition with easing

#### `User/Lib/MAX7219/MAX7219_Documentation_TH.md` (NEW)
- เอกสารภาษาไทยฉบับสมบูรณ์ 1000+ บรรทัด
- ครอบคลุม: บทนำ, ฮาร์ดแวร์, การติดตั้ง, การใช้งานพื้นฐาน
- การแสดงข้อความ, กราฟิกขั้นสูง, Animation และ Sprite
- เทคนิคขั้นสูง, API Reference ครบถ้วน
- การแก้ปัญหา, ตัวอย่างโปรเจกต์

#### `User/Lib/MAX7219/README.md` (NEW)
- Quick start guide
- Features overview
- Example descriptions (14 examples)
- API reference summary
- Link to full documentation

#### `User/SimpleHAL/SimpleHAL.h` (MODIFIED)
- เพิ่ม `#include "../Lib/MAX7219/SimpleMAX7219.h"`
- อัปเดต version 1.4.0 → 1.5.0

#### `readme/INDEX.md` (MODIFIED)
- เพิ่มรายการ MAX7219 LED Matrix Driver Library

#### `CHANGELOG.md` (MODIFIED)
- เพิ่มรายการการเปลี่ยนแปลงทั้งหมด

### สรุป
- **รวมโค้ด**: ~5,000 บรรทัด (core + fonts + examples + docs)
- **ไฟล์ใหม่**: 19 ไฟล์ (3 core + 14 examples + 2 docs)
- **ตัวอย่าง**: 14 ตัวอย่าง (basic: 4, intermediate: 5, advanced: 5)
- **เอกสาร**: 1000+ บรรทัดภาษาไทย + README
- **API Functions**: 30+ ฟังก์ชัน
- **Fonts**: 3 fonts (ASCII 5x7, ASCII 8x8, Thai 5x7)
- **Graphics**: 6 primitives (pixel, line, rect, circle, triangle, bitmap)
- **Features**: Scrolling, Animation, Sprites, Cascading (1-8 displays)
- **SimpleHAL Version**: 1.5.0

---

## [2025-12-21 14:42] - SimpleOPAMP Library v1.0

### เพิ่มเติม (Added)
- **SimpleOPAMP Library**: Library สำหรับควบคุม OPAMP (Operational Amplifier) ใน CH32V003
  - **4 Operating Modes**: Voltage Follower, Non-Inverting Amplifier, Inverting Amplifier, Comparator
  - **Arduino-style API**: ใช้งานง่าย พร้อม auto-configuration
  - **Gain Calculation**: Utilities สำหรับคำนวณ gain และค่าความต้านทาน
  - **Channel Selection**: เลือก input channels (CHP0/CHP1, CHN0/CHN1) ได้
  - **6 Examples**: ตัวอย่างครบทุกโหมดการใช้งาน
  - **Thai Documentation**: เอกสาร README 1000+ บรรทัด
  - **Total Code**: ~3,500 บรรทัด (core + examples + docs)

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/SimpleOPAMP.h` (NEW)
- เพิ่ม OPAMP mode enums (VOLTAGE_FOLLOWER, NON_INVERTING, INVERTING, COMPARATOR)
- เพิ่ม channel enums (OPAMP_CHP0/1, OPAMP_CHN0/1)
- เพิ่ม gain presets (1x, 2x, 4x, 8x, 16x)
- เพิ่ม Basic API: SimpleInit, Enable, Disable, SetMode
- เพิ่ม Advanced API: ConfigVoltageFollower, ConfigNonInverting, ConfigInverting, ConfigComparator
- เพิ่ม Utility functions: CalculateGainNonInv/Inv, CalculateR2NonInv/Inv, IsEnabled, GetConfig
- เพิ่ม documentation ภาษาไทยครบถ้วน พร้อมตัวอย่างวงจร
- รวม ~400 บรรทัด

#### `User/SimpleHAL/SimpleOPAMP.c` (NEW)
- Implement OPAMP initialization และ configuration
- Implement mode switching (voltage follower, amplifiers, comparator)
- Implement gain calculation utilities
- Implement channel selection และ status checking
- Internal state tracking สำหรับ current mode และ enable status
- รวม ~220 บรรทัด

#### `User/SimpleHAL/Examples/OPAMP/01_Basic_VoltageFollower.c` (NEW)
- ตัวอย่าง Voltage Follower (Buffer) พื้นฐาน
- แสดงการใช้งาน OPAMP เป็น buffer สำหรับ potentiometer
- Monitor input/output voltage ผ่าน ADC
- แสดงความแตกต่างระหว่าง input และ output

#### `User/SimpleHAL/Examples/OPAMP/02_NonInverting_Amplifier.c` (NEW)
- ตัวอย่าง Non-Inverting Amplifier
- แสดงการคำนวณ gain จากค่าความต้านทาน (R1, R2)
- Monitor actual gain และเปรียบเทียบกับค่าที่คาดหวัง
- แสดง error percentage และ saturation warning

#### `User/SimpleHAL/Examples/OPAMP/03_Inverting_Amplifier.c` (NEW)
- ตัวอย่าง Inverting Amplifier
- แสดงการกลับเฟสของสัญญาณ (180 องศา)
- แสดงการใช้งาน reference voltage (Vref)
- Monitor phase inversion และ saturation

#### `User/SimpleHAL/Examples/OPAMP/04_Comparator_Mode.c` (NEW)
- ตัวอย่าง Comparator Mode
- เปรียบเทียบ signal กับ threshold voltage
- ควบคุม LED ตามผลการเปรียบเทียบ
- แสดง digital output (HIGH/LOW)

#### `User/SimpleHAL/Examples/OPAMP/05_Signal_Conditioning.c` (NEW)
- ตัวอย่าง Signal Conditioning สำหรับ LM35 temperature sensor
- แสดงการขยายสัญญาณ 10 เท่า เพื่อเพิ่มความละเอียดของ ADC
- แสดงการคำนวณอุณหภูมิจากสัญญาณที่ขยายแล้ว
- Averaging เพื่อลด noise

#### `User/SimpleHAL/Examples/OPAMP/06_Advanced_Techniques.c` (NEW)
- ตัวอย่าง Auto-Ranging Amplifier
- Dynamic gain switching (2x, 5x, 10x) ตามขนาดสัญญาณ
- ป้องกัน saturation อัตโนมัติ
- แสดงเทคนิคการใช้ OPAMP ขั้นสูง

#### `User/SimpleHAL/Examples/OPAMP/README.md` (NEW)
- เอกสารภาษาไทยฉบับสมบูรณ์ 1000+ บรรทัด
- ครอบคลุม: OPAMP theory, โครงสร้าง CH32V003 OPAMP, Pin mapping
- การใช้งานพื้นฐาน: Voltage Follower, Non-Inverting, Inverting, Comparator
- การใช้งานขั้นสูง: Signal Conditioning, Offset Compensation, Multi-stage
- เทคนิคขั้นสูง: Auto-ranging, Noise reduction, Bandwidth considerations
- API Reference ครบถ้วน, Troubleshooting, ตารางสรุป

#### `User/SimpleHAL/SimpleHAL.h` (MODIFIED)
- เพิ่ม `#include "SimpleOPAMP.h"`
- อัปเดต version 1.3.0 → 1.4.0

#### `User/SimpleHAL/README.md` (MODIFIED)
- เพิ่ม OPAMP row ในตาราง peripherals
- เพิ่ม OPAMP ในรายการคุณสมบัติหลัก
- อัปเดต version 1.3.0 → 1.4.0
- อัปเดต "New in 1.4.0" description

#### `readme/2025-12-21_Library_SimpleOPAMP.md` (NEW)
- เอกสารสรุปการพัฒนา library
- Objective, Implementation, Walkthrough
- Root Cause Analysis
- ผลลัพธ์และการทดสอบ

#### `readme/INDEX.md` (MODIFIED)
- เพิ่มรายการ SimpleOPAMP library

#### `CHANGELOG.md` (MODIFIED)
- เพิ่มรายการการเปลี่ยนแปลงทั้งหมด

### สรุป
- **รวมโค้ด**: ~3,500 บรรทัด (core + examples + docs)
- **ไฟล์ใหม่**: 10 ไฟล์ (2 core + 6 examples + 1 README + 1 doc)
- **ตัวอย่าง**: 6 ตัวอย่าง (voltage follower → advanced techniques)
- **เอกสาร**: 1000+ บรรทัดภาษาไทย
- **API Functions**: 15+ ฟังก์ชัน (basic + advanced + utilities)
- **Operating Modes**: 4 โหมด (Voltage Follower, Non-Inverting, Inverting, Comparator)
- **SimpleHAL Version**: 1.4.0

---

## [2025-12-21 13:48] - SimpleFlash Library v1.0

### เพิ่มเติม (Added)
- **SimpleFlash Library**: Library สำหรับจัดเก็บข้อมูลใน Flash memory
  - **Configuration Management**: บันทึก/โหลด config พร้อม CRC16 validation
  - **Multiple Data Types**: รองรับ byte, half-word, word, string, struct
  - **Simplified API**: Macros ใช้งานง่าย (FLASH_SAVE_CONFIG, FLASH_LOAD_CONFIG, FLASH_WRITE_AUTO, FLASH_READ)
  - **Wear Leveling**: Circular buffer technique เพื่อยืดอายุ Flash
  - **Auto-Erase**: WriteWithErase functions สำหรับ modify-erase-write cycle
  - **Error Handling**: Status codes และ validation ครบถ้วน
  - **7 Examples**: ตัวอย่างจากพื้นฐานถึงขั้นสูง (simple usage, basic, config, string, struct, wear leveling)
  - **Thai Documentation**: เอกสาร README 400+ บรรทัด
  - **Total Code**: ~2,840 lines (core + examples + docs)

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/SimpleFlash.h` (NEW)
- เพิ่ม Flash memory constants และ configuration
- เพิ่ม FlashStatus enum (11 status codes)
- เพิ่ม Core API: Init, EraseAll, ErasePage
- เพิ่ม Read/Write functions: Byte, HalfWord, Word
- เพิ่ม String functions: ReadString, WriteString
- เพิ่ม Struct functions: ReadStruct, WriteStruct
- เพิ่ม Config management: SaveConfig, LoadConfig, IsConfigValid
- เพิ่ม Utility functions: CalculateCRC16, IsAddressValid, GetPageAddress
- เพิ่ม Advanced functions: WriteByteWithErase, WriteHalfWordWithErase, WriteWordWithErase
- เพิ่ม Simplified API macros: FLASH_SAVE_CONFIG, FLASH_LOAD_CONFIG, FLASH_WRITE_AUTO, FLASH_READ
- รวม ~460 บรรทัด

#### `User/SimpleHAL/SimpleFlash.c` (NEW)
- Implement flash operations ทั้งหมด
- Implement CRC16-CCITT calculation
- Implement modify-erase-write cycle
- Implement error handling และ verification
- รวม ~580 บรรทัด

#### `User/SimpleHAL/Examples/Flash/flash_simple_usage.c` (NEW)
- ตัวอย่างการใช้งานแบบง่ายที่สุดด้วย Simplified API macros
- แสดงการใช้ FLASH_SAVE_CONFIG/FLASH_LOAD_CONFIG
- แสดงการใช้ FLASH_WRITE_AUTO/FLASH_READ

#### `User/SimpleHAL/Examples/Flash/flash_basic_read_write.c` (NEW)
- ตัวอย่างการอ่าน/เขียน byte, half-word, word
- แสดงการใช้ WriteWithErase functions

#### `User/SimpleHAL/Examples/Flash/flash_config_storage.c` (NEW)
- ตัวอย่างการจัดเก็บ configuration พร้อม CRC validation
- แสดงการใช้ Flash_SaveConfig/Flash_LoadConfig
- แสดง factory reset

#### `User/SimpleHAL/Examples/Flash/flash_string_storage.c` (NEW)
- ตัวอย่างการจัดเก็บ string และ WiFi credentials

#### `User/SimpleHAL/Examples/Flash/flash_struct_storage.c` (NEW)
- ตัวอย่างการจัดเก็บ struct ที่ซับซ้อน
- แสดง nested structs และ arrays

#### `User/SimpleHAL/Examples/Flash/flash_wear_leveling.c` (NEW)
- ตัวอย่างเทคนิค wear leveling ขั้นสูง
- Circular buffer พร้อม timestamp-based slot management

#### `User/SimpleHAL/Examples/Flash/README.md` (NEW)
- เอกสารภาษาไทยครบถ้วน 400+ บรรทัด
- ครอบคลุม: Overview, Quick Start, Basic/Intermediate/Advanced usage
- Best Practices, API Reference, Troubleshooting

#### `User/SimpleHAL/SimpleHAL.h` (MODIFIED)
- เพิ่ม `#include "SimpleFlash.h"`

#### `readme/2025-12-21_Library_SimpleFlash.md` (NEW)
- เอกสารสรุปการพัฒนา library
- Objective, Implementation, Walkthrough
- Root Cause Analysis, Workflow diagram
- เทคนิคที่ใช้ (CRC16, Modify-Erase-Write, Generic Macros)

#### `readme/INDEX.md` (MODIFIED)
- เพิ่มรายการ SimpleFlash library

#### `CHANGELOG.md` (MODIFIED)
- เพิ่มรายการการเปลี่ยนแปลงทั้งหมด

### สรุป
- **รวมโค้ด**: ~2,840 บรรทัด (core + examples + docs)
- **ไฟล์ใหม่**: 10 ไฟล์ (2 core + 7 examples + 1 README)
- **ตัวอย่าง**: 7 ตัวอย่าง (simple → basic → advanced)
- **เอกสาร**: 400+ บรรทัดภาษาไทย
- **API Functions**: 30+ ฟังก์ชัน + 4 macros
- **Flash Storage**: 128 bytes (2 pages)
- **Features**: CRC16 validation, Wear leveling, Auto-erase

---

## [2025-12-21 13:00] - OLED I2C Library v1.0

### เพิ่มเติม (Added)
- **OLED I2C Library**: Library ครบครันสำหรับควบคุม OLED Display ผ่าน I2C
  - **SSD1306 Controller Support**: รองรับ OLED ที่ใช้ SSD1306 IC
  - **Multiple Display Sizes**: รองรับ 128x64, 128x32, 64x48 pixels
  - **Graphics Primitives**: Line, Rectangle, Circle, Triangle (outline & filled)
  - **Font System**: 6x8 ASCII font พร้อม framework สำหรับขยายผล
  - **Menu System**: Interactive menu framework พร้อม navigation
  - **Sprite Animation**: Sprite system สำหรับ animations
  - **Double Buffering**: Optional double buffering สำหรับ smooth animations
  - **Thai Text Framework**: UTF-8 support และ framework สำหรับภาษาไทย

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/Lib/OLED/oled_i2c.h` (NEW)
- เพิ่ม SSD1306 command definitions
- เพิ่ม OLED_Handle structure สำหรับจัดการ display state
- เพิ่ม display size enums (OLED_128x64, OLED_128x32, OLED_64x48)
- เพิ่ม buffering mode enums (single/double)
- เพิ่ม core API functions: Init, Update, Clear, Fill, DisplayOn, SetContrast
- เพิ่ม pixel operations: SetPixel, GetPixel
- เพิ่ม scrolling functions: StartScroll, StopScroll
- รวม ~350 บรรทัด

#### `User/Lib/OLED/oled_i2c.c` (NEW)
- Implement SSD1306 initialization sequence
- Implement I2C communication protocol
- Implement buffer management (static allocation)
- Implement screen update (full และ partial)
- Implement display control functions
- Implement hardware scrolling
- รวม ~450 บรรทัด

#### `User/Lib/OLED/oled_graphics.h` (NEW)
- เพิ่ม graphics primitives API
- เพิ่ม bitmap และ sprite structures
- เพิ่ม drawing functions: Line, Rect, Circle, Triangle
- เพิ่ม filled shape functions
- เพิ่ม bitmap rendering functions
- เพิ่ม sprite system API
- เพิ่ม advanced graphics: Progress bar, Graph
- รวม ~300 บรรทัด

#### `User/Lib/OLED/oled_graphics.c` (NEW)
- Implement Bresenham's line algorithm
- Implement Midpoint circle algorithm
- Implement filled shape algorithms
- Implement bitmap rendering
- Implement sprite system
- Implement progress bar และ graph plotting
- รวม ~550 บรรทัด

#### `User/Lib/OLED/oled_fonts.h` (NEW)
- เพิ่ม font structure definition
- เพิ่ม built-in font declarations (6x8, 8x16, 12x16)
- เพิ่ม text rendering API
- เพิ่ม text alignment enums
- เพิ่ม number formatting functions
- เพิ่ม multi-line และ scrolling text functions
- เพิ่ม UTF-8 helper functions
- รวม ~250 บรรทัด

#### `User/Lib/OLED/oled_fonts.c` (NEW)
- เพิ่ม 6x8 ASCII font data (complete, 95 characters)
- Implement text rendering engine
- Implement text alignment
- Implement number formatting (int, float)
- Implement multi-line text
- Implement scrolling text
- Implement UTF-8 to Unicode conversion
- Thai text framework (placeholder)
- รวม ~650 บรรทัด

#### `User/Lib/OLED/oled_menu.h` (NEW)
- เพิ่ม menu framework structures
- เพิ่ม menu item types (action, submenu, value editor)
- เพิ่ม menu styles (list, icon, full)
- เพิ่ม navigation API
- เพิ่ม value editor API
- เพิ่ม menu helper functions
- รวม ~250 บรรทัด

#### `User/Lib/OLED/oled_menu.c` (NEW)
- Implement menu navigation (up/down/select/back)
- Implement value editing (int, bool, list)
- Implement menu rendering (3 styles)
- Implement auto-scroll
- Implement callback system
- รวม ~400 บรรทัด

#### `User/Lib/OLED/examples/01_hello_world.c` (NEW)
- ตัวอย่างพื้นฐาน: แสดงข้อความ "Hello World"
- แสดงการใช้งาน OLED_Init, OLED_Clear, OLED_DrawString, OLED_Update

#### `User/Lib/OLED/examples/02_graphics_basic.c` (NEW)
- ตัวอย่างการวาดรูปทรงเรขาคณิต
- แสดง line, rectangle, circle, triangle
- แสดง outline และ filled shapes

#### `User/Lib/OLED/examples/07_menu_system.c` (NEW)
- ตัวอย่าง interactive menu system
- แสดงการใช้งาน menu navigation
- แสดงการใช้งาน value editor
- แสดงการใช้งาน callbacks

#### `User/Lib/OLED/README.md` (NEW)
- เอกสารภาษาไทยฉบับสมบูรณ์ 800+ บรรทัด
- ครอบคลุม: Overview, Hardware setup, Quick start, API reference
- การใช้งานขั้นพื้นฐาน, ขั้นกลาง, ขั้นสูง
- เทคนิคต่างๆ: Font creation, Image conversion, Optimization
- Troubleshooting และ Project examples

#### `readme/2025-12-21_Library_OLED_I2C.md` (NEW)
- เอกสารสรุปการพัฒนา library
- Objective, Implementation, Walkthrough
- Root Cause Analysis
- ผลลัพธ์และการทดสอบ

#### `readme/INDEX.md` (MODIFIED)
- เพิ่มรายการ OLED I2C Library

#### `CHANGELOG.md` (MODIFIED)
- เพิ่มรายการการเปลี่ยนแปลงทั้งหมด

### สรุป
- **รวมโค้ด**: ~3,200 บรรทัด (headers + implementations + examples)
- **ไฟล์ใหม่**: 11 ไฟล์ (8 library files + 3 examples)
- **ตัวอย่าง**: 3 ตัวอย่าง (hello world, graphics, menu)
- **เอกสาร**: 800+ บรรทัดภาษาไทย (README.md)
- **API Functions**: 50+ ฟังก์ชัน
- **Built-in Fonts**: 1 font (6x8 ASCII complete)
- **Display Sizes**: 3 ขนาด (128x64, 128x32, 64x48)
- **Graphics**: Line, Rect, Circle, Triangle, Bitmap, Sprite
- **Menu System**: List, Icon, Full screen styles

---

## [2025-12-21 12:40] - SimpleTIM Extensions v1.0

### เพิ่มเติม (Added)
- **SimpleTIM_Ext Library**: Extension library สำหรับ SimpleTIM เพิ่มฟังก์ชันการจับเวลาระดับสูง
  - **Stopwatch (นับขึ้น)**: เริ่มจาก 00:00:00 นับขึ้นไม่จำกัด
  - **Countdown (นับลง)**: ตั้งเวลาได้ไม่จำกัด พร้อม alarm callback
  - **3 รูปแบบเวลา**: HH:MM:SS, MM:SS, SS
  - **2 โหมดแสดงผล**: NORMALIZED (แปลงหน่วยปกติ), RAW (แสดงค่าตรงๆ)
  - **ไม่มีข้อจำกัดค่าเวลา**: รองรับ 450 วินาที, 146 นาที, 72 ชั่วโมง
  - **Pause/Resume/Reset**: ควบคุมการทำงานได้อย่างยืดหยุ่น
  - **Maximum time**: ~49 วัน (uint32_t milliseconds)

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/SimpleTIM_Ext.h` (NEW)
- เพิ่ม time format enums: TIME_FORMAT_HHMMSS, TIME_FORMAT_MMSS, TIME_FORMAT_SS
- เพิ่ม display mode enums: TIME_DISPLAY_NORMALIZED, TIME_DISPLAY_RAW
- เพิ่ม Time_t structure สำหรับเก็บเวลา (hours, minutes, seconds)
- เพิ่ม Stopwatch API functions (8 ฟังก์ชัน):
  - Init, Start, Stop, Reset
  - GetTime, GetTimeString, GetTotalSeconds, GetTotalMilliseconds
  - IsRunning
- เพิ่ม Countdown API functions (10 ฟังก์ชัน):
  - Init, InitFromSeconds, Start, Stop, Reset
  - GetTime, GetTimeString, IsFinished
  - SetAlarmCallback, GetRemainingSeconds, IsRunning
- เพิ่ม Utility functions (3 ฟังก์ชัน):
  - Time_ToString, Time_FromSeconds, Time_ToSeconds
- เพิ่ม buffer size recommendations (TIME_BUFFER_SIZE_*)
- เพิ่ม documentation ภาษาไทยครบถ้วน 400+ บรรทัด

#### `User/SimpleHAL/SimpleTIM_Ext.c` (NEW)
- Implement stopwatch functions พร้อม millisecond tracking
- Implement countdown functions พร้อม alarm callback system
- Implement time format conversion (NORMALIZED และ RAW modes)
- Implement string formatting สำหรับทุก time formats
- ใช้ TIM2 ที่ 1000Hz (1ms resolution) เป็น base timer
- Internal state management สำหรับ stopwatch และ countdown
- Helper functions สำหรับแปลง milliseconds เป็น Time_t
- รวม ~350 บรรทัด

#### `User/SimpleHAL/Examples/SimpleTIM_Ext_Examples.c` (NEW)
- เพิ่มตัวอย่างการใช้งาน 11 ตัวอย่าง:
  1. Basic Stopwatch (HH:MM:SS) - จับเวลาพื้นฐาน
  2. Basic Countdown (HH:MM:SS) - นับถอยหลังพื้นฐาน
  3. Stopwatch MM:SS Format - แสดงผลแบบนาที:วินาที
  4. Countdown MM:SS Format - Kitchen timer style
  5. Stopwatch SS Format - แสดงผลเป็นวินาที
  6. Countdown SS Format - Short intervals
  7. RAW Display Mode Demo - สาธิตการแสดงผลแบบ RAW (450s, 146min)
  8. Lap Timer - Stopwatch พร้อมบันทึก lap times
  9. Multi-Timer Management - ใช้ stopwatch และ countdown พร้อมกัน
  10. Kitchen Timer Project - Countdown พร้อม buzzer alarm และ LED
  11. Workout Interval Timer - Work/rest periods สำหรับออกกำลังกาย
- รวม ~650 บรรทัด

#### `readme/2025-12-21_Library_SimpleTIM_Extensions.md` (NEW)
- เพิ่มเอกสารภาษาไทยฉบับสมบูรณ์ 800+ บรรทัด:
  - ภาพรวมและคุณสมบัติหลัก
  - การติดตั้งและเริ่มต้นใช้งาน (Quick Start)
  - Stopwatch Usage - การใช้งานแบบนับขึ้น
  - Countdown Usage - การใช้งานแบบนับลง
  - Time Formats Guide - คู่มือรูปแบบเวลาทั้ง 3 แบบ
  - Display Modes Guide - NORMALIZED vs RAW พร้อมตัวอย่าง
  - Advanced Techniques - Lap timer, Multi-timer, Custom display
  - Project Examples - Pomodoro timer, Race timing system
  - API Reference ครบถ้วนทุกฟังก์ชัน
  - Troubleshooting และการแก้ปัญหา

#### `User/SimpleHAL/SimpleHAL.h` (MODIFIED)
- เพิ่ม `#include "SimpleTIM_Ext.h"` หลัง SimpleTIM.h
- อัปเดต documentation ระบุ TIM Ext module

#### `User/SimpleHAL/README.md` (MODIFIED)
- เพิ่ม TIM Ext row ในตาราง peripherals:
  - `| **TIM Ext** | SimpleTIM_Ext.h | Stopwatch และ Countdown timers |`

#### `readme/INDEX.md` (MODIFIED)
- เพิ่มรายการ SimpleTIM Extensions ในส่วน 2025-12-21:
  - SimpleTIM Extensions - Stopwatch & Countdown
  - ระบุคุณสมบัติหลัก: 3 formats, 2 modes, unlimited values

#### `CHANGELOG.md` (MODIFIED)
- เพิ่มรายการการเปลี่ยนแปลงทั้งหมด

### สรุป
- **รวมโค้ด**: ~1,400 บรรทัด (header + implementation + examples)
- **ไฟล์ใหม่**: 3 ไฟล์ (SimpleTIM_Ext.h, SimpleTIM_Ext.c, Examples)
- **ตัวอย่าง**: 11 ตัวอย่าง (basic → intermediate → advanced → projects)
- **เอกสาร**: 800+ บรรทัดภาษาไทย
- **API Functions**: 21 ฟังก์ชัน (8 Stopwatch + 10 Countdown + 3 Utility)
- **Time Formats**: 3 รูปแบบ (HH:MM:SS, MM:SS, SS)
- **Display Modes**: 2 โหมด (NORMALIZED, RAW)

---

## [2025-12-21 12:20] - SimpleGPIO v1.2 - Analog Functions

### เพิ่มเติม (Added)
- **Analog Functions**: เพิ่มฟังก์ชัน `analogRead()` และ `analogWrite()` ใน SimpleGPIO
  - `analogRead(pin)` - อ่านค่า ADC จาก PD2-PD7 (return 0-1023)
  - `analogWrite(pin, value)` - สร้าง PWM บน PC0, PC3-4, PD2-4, PD7 (0-255)
  - Auto-initialization - ไม่ต้อง init ADC/PWM เอง
  - Error handling - ตรวจสอบ pin อัตโนมัติ
  - Arduino-compatible API

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/SimpleGPIO.h` (MODIFIED)
- อัพเดทเวอร์ชัน 1.1 → 1.2
- เพิ่มคำอธิบาย analog I/O capabilities ใน header documentation
- เพิ่ม function prototypes:
  - `uint16_t analogRead(uint8_t pin)`
  - `void analogWrite(uint8_t pin, uint8_t value)`
- อัพเดท GPIO Pins documentation ระบุ ADC และ PWM support

#### `User/SimpleHAL/SimpleGPIO.c` (MODIFIED)
- อัพเดทเวอร์ชัน 1.1 → 1.2
- เพิ่ม includes: `SimpleADC.h`, `SimplePWM.h`
- เพิ่ม state tracking: `adc_initialized` สำหรับ ADC auto-init
- เพิ่ม helper functions:
  - `mapPinToADC()` - แมป GPIO pin → ADC channel (PD2-PD7)
  - `mapPinToPWM()` - แมป GPIO pin → PWM channel (PC0, PC3-4, PD2-4, PD7)
- เพิ่ม implementations:
  - `analogRead()` - อ่านค่า ADC พร้อม auto-init และ error handling
  - `analogWrite()` - สร้าง PWM พร้อม auto-init และ error handling

#### `User/SimpleHAL/Examples/SimpleGPIO_AnalogRead_Test.c` (NEW)
- ทดสอบ `analogRead()` กับทุก ADC pins (PD2-PD7)
- แสดงค่า ADC และ voltage ผ่าน USART
- ทดสอบ error handling (PC0 ไม่รองรับ ADC)

#### `User/SimpleHAL/Examples/SimpleGPIO_AnalogWrite_Test.c` (NEW)
- ทดสอบ `analogWrite()` กับ PWM pin (PC3)
- LED fade in/out effect
- แสดง brightness percentage ผ่าน USART

#### `User/SimpleHAL/Examples/SimpleGPIO_Analog_Combined_Test.c` (NEW)
- ทดสอบการใช้งาน ADC และ PWM ร่วมกัน
- Potentiometer (PD2) ควบคุม LED brightness (PC3)
- แสดงค่า ADC และ PWM ผ่าน USART

#### `readme/2025-12-21_Guide_SimpleGPIO_Analog_Functions.md` (NEW)
- คู่มือการใช้งาน analogRead() และ analogWrite() ฉบับสมบูรณ์
- Pin Compatibility Matrix (ADC และ PWM pins)
- ตัวอย่างการใช้งานพื้นฐาน: Potentiometer control, LED fade
- ตัวอย่างการใช้งานขั้นสูง: Multi-channel ADC, RGB LED, Temperature sensor, Servo control
- API Reference ครบถ้วน
- Best Practices และ Troubleshooting

#### `readme/2025-12-21_Library_SimpleGPIO_Analog_Functions.md` (NEW)
- เอกสารสรุปการพัฒนา analog functions
- Objective, Implementation, Walkthrough
- Pin Mapping table
- ผลลัพธ์และการทดสอบ

#### `readme/INDEX.md` (MODIFIED)
- เพิ่มรายการ SimpleGPIO Analog Functions guide

#### `CHANGELOG.md` (MODIFIED)
- เพิ่มรายการการเปลี่ยนแปลงทั้งหมด

### สรุป
- **ฟังก์ชันใหม่**: 2 ฟังก์ชัน (analogRead, analogWrite)
- **Helper functions**: 2 ฟังก์ชัน (mapPinToADC, mapPinToPWM)
- **Test examples**: 3 ไฟล์
- **Documentation**: 2 ไฟล์ (คู่มือการใช้งาน + สรุปการพัฒนา)
- **Code size**: ~150 บรรทัด (implementation)
- **รองรับ pins**: 6 ADC pins, 7 PWM pins

---


## [2025-12-21 11:23] - TM1637 7-Segment Display Library

### เพิ่มเติม (Added)
- **TM1637 Library**: Library สำหรับควบคุม 7-segment display ผ่าน IC Driver TM1637
  - Two-wire communication (CLK, DIO) ด้วย bit-banging
  - รองรับ 4-digit และ 6-digit displays
  - ควบคุมความสว่าง 8 ระดับ (0-7)
  - แสดงตัวเลข, ทศนิยม, เลขฐาน 16, เวลา
  - Scrolling text (blocking และ non-blocking)
  - Custom animations
  - Blinking effects

### ฟีเจอร์หลัก
- **20+ API Functions**: Init, Display (Number/Float/Hex/Time/Char/String), Brightness, Scroll, Animation, Blink
- **Segment Control**: Direct segment manipulation, custom patterns
- **Non-Blocking**: StartScroll/UpdateScroll, SetBlink/UpdateBlink patterns
- **Integration**: ใช้ร่วมกับ Timer library และ SimpleGPIO

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/Lib/TM1637/SimpleTM1637.h` (NEW)
- เพิ่ม TM1637 protocol definitions และ commands
- เพิ่ม segment bit definitions (SEG_A to SEG_G, SEG_DP)
- เพิ่ม TM1637_Handle structure
- เพิ่ม API functions ทั้งหมด 20+ ฟังก์ชัน
- เพิ่ม documentation ภาษาไทยครบถ้วน

#### `User/Lib/TM1637/SimpleTM1637.c` (NEW)
- Implement low-level protocol (START, STOP, WriteByte, ACK detection)
- เพิ่ม segment encoding tables (digits, hex, characters)
- Implement display functions (Number, Float, Hex, Time, Char, String)
- Implement scrolling (blocking และ non-blocking)
- Implement animation และ blinking
- รวม ~800 บรรทัด

#### `User/Lib/TM1637/Examples/01_Basic_Display.c` (NEW)
- 7 ตัวอย่างขั้นพื้นฐาน:
  - Simple counter (0-9999)
  - Brightness control (fade in/out)
  - Display time (HH:MM)
  - Temperature display
  - Leading zeros
  - Individual digit control
  - Display on/off

#### `User/Lib/TM1637/Examples/02_Intermediate_Display.c` (NEW)
- 8 ตัวอย่างขั้นกลาง:
  - Hexadecimal display
  - Character display (HELP, COOL, CAFE)
  - Custom segment patterns
  - Blinking display
  - Floating point numbers
  - Negative numbers
  - Progress bar
  - Countdown timer

#### `User/Lib/TM1637/Examples/03_Advanced_Effects.c` (NEW)
- 9 ตัวอย่างขั้นสูง:
  - Scrolling text (blocking และ non-blocking)
  - Loading animation
  - Circle animation
  - Wave animation
  - Countdown with animation
  - Progress with percentage
  - Snake animation
  - Multi-message display

#### `User/Lib/TM1637/Examples/04_Project_MultiFunction.c` (NEW)
- โปรเจคตัวอย่างครบวงจร:
  - Multi-mode display (Clock, Temperature, Counter, Stopwatch)
  - Button control สำหรับ mode switching
  - Non-blocking operation ด้วย state machine
  - Auto-brightness (simulated)
  - Integration กับ Timer library

#### `User/Lib/TM1637/TM1637_Documentation_TH.md` (NEW)
- เอกสารภาษาไทยครบถ้วน 1000+ บรรทัด:
  - บทนำและภาพรวม TM1637
  - Hardware setup และ wiring diagram
  - API Reference ทุกฟังก์ชัน
  - Tutorial ขั้นพื้นฐาน (5 หัวข้อ)
  - Tutorial ขั้นกลาง (5 หัวข้อ)
  - Tutorial ขั้นสูง (3 หัวข้อ)
  - เทคนิคและ Tips (5 หัวข้อ)
  - Troubleshooting (4 ปัญหา)
  - ตัวอย่างโปรเจค (4 โปรเจค)

#### `readme/2025-12-21_Library_TM1637.md` (NEW)
- เพิ่มเอกสารสรุปการพัฒนา library
- Objective, Implementation, Walkthrough
- Root Cause Analysis
- Workflow diagram (Mermaid)
- เทคนิคที่ใช้

#### `readme/INDEX.md` (MODIFIED)
- เพิ่มรายการ TM1637 library

#### `CHANGELOG.md` (MODIFIED)
- เพิ่มรายการการเปลี่ยนแปลงทั้งหมด

### สรุป
- **รวมโค้ด**: ~3,300 บรรทัด
- **ไฟล์ใหม่**: 8 ไฟล์
- **ตัวอย่าง**: 24 ตัวอย่าง (basic → intermediate → advanced → project)
- **เอกสาร**: 1000+ บรรทัดภาษาไทย

---

## [2025-12-21] - SimpleGPIO v1.1 - Pin Naming Refactoring

### เปลี่ยนแปลง (Changed)
- **SimpleGPIO Pin Naming**: เปลี่ยนจาก Arduino-style เป็น CH32V003 native
  - D0-D7 → PC0-PC7
  - A0-A7 → PD2, PD3, PD4, PD5, PD6, PD7

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/SimpleGPIO.h` (MODIFIED)
- เปลี่ยน `GPIO_Pin` enum จาก D0-D7, A0-A7 เป็น PC0-PC7, PD2-PD7
- อัปเดต version 1.0 → 1.1
- อัปเดต documentation และตัวอย่างโค้ด

#### `User/SimpleHAL/SimpleGPIO.c` (MODIFIED)
- อัปเดต version 1.0 → 1.1
- แก้ไข comments ใน pin_map array

#### `User/SimpleHAL/SimpleGPIO_Examples.c` (MODIFIED)
- แทนที่ชื่อ pin ทั้งหมด 10 ตัวอย่าง
- D0 → PC0, D1 → PC1, etc.

#### `User/SimpleHAL/SimplePWM.h` (MODIFIED)
- อัปเดต comments ลบการอ้างถึง Arduino pin names
- ระบุเฉพาะชื่อ pin จริง (PC0, PC3, PC4, PD2, PD3, PD4, PD7)

#### `User/SimpleHAL/SimplePWM_Examples.c` (MODIFIED)
- แทนที่ D0-D3 เป็น PC0-PC3

#### `User/SimpleHAL/SimpleTIM_Examples.c` (MODIFIED)
- แทนที่ D0-D2 เป็น PC0-PC2

#### `readme/2025-12-21_Guide_SimpleGPIO_Usage.md` (MODIFIED)
- อัปเดต Pin Mapping table
- แก้ไขตัวอย่างโค้ดทั้งหมด (~40+ ตัวอย่าง)

#### `readme/2025-12-21_Guide_Integration_GPIO_PWM_ADC.md` (MODIFIED)
- อัปเดต Pin Compatibility Matrix
- แก้ไขตัวอย่างโค้ดและโปรเจกต์ตัวอย่าง

#### `readme/2025-12-21_Refactoring_SimpleGPIO_PinNaming.md` (NEW)
- เพิ่มเอกสารการ refactoring
- Pin mapping table และ migration guide

#### `readme/INDEX.md` (MODIFIED)
- เพิ่มรายการ refactoring entry

### Breaking Changes
⚠️ **โค้ดเดิมจะใช้งานไม่ได้** - ต้องแก้ไขชื่อ pin ทั้งหมด  
⚠️ ใช้ Find & Replace: D0→PC0, D1→PC1, ... A0→PD2, A1→PD3, ...

---

## [2025-12-21] - SimpleHAL v1.1.0 - GPIO, TIM และ PWM Modules

### เพิ่มเติม (Added)
- **SimpleGPIO Module**: Arduino-style GPIO control
  - pinMode(), digitalWrite(), digitalRead(), digitalToggle()
  - attachInterrupt(), detachInterrupt() พร้อม callback
  - Pin naming: D0-D7, A0-A7
  - Interrupt modes: RISING, FALLING, CHANGE
  - Port operations: portWrite(), portRead()

- **SimpleTIM Module**: Timer peripheral control
  - Frequency-based initialization (Hz)
  - Auto-calculation ของ prescaler และ period
  - Interrupt callbacks สำหรับ update events
  - Timer control: Start, Stop, SetFrequency
  - Counter access และ advanced configuration

- **SimplePWM Module**: PWM output control
  - 8 PWM channels (TIM1: 4 channels, TIM2: 4 channels)
  - Duty cycle control (0-100% หรือ raw value)
  - Frequency adjustment
  - Arduino analogWrite() compatible
  - Pin remapping support
  - Polarity control (normal/inverted)

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/SimpleGPIO.h` (NEW)
- เพิ่ม Arduino-style GPIO API
- Pin definitions และ mode enums
- Interrupt mode definitions

#### `User/SimpleHAL/SimpleGPIO.c` (NEW)
- Implement GPIO control functions
- Pin mapping table สำหรับ CH32V003
- Interrupt callback management
- EXTI interrupt handler

#### `User/SimpleHAL/SimpleGPIO_Examples.c` (NEW)
- 10 ตัวอย่างการใช้งาน GPIO
- LED blink, button reading, interrupts
- Port operations, binary counter

#### `User/SimpleHAL/SimpleTIM.h` (NEW)
- เพิ่ม Timer configuration API
- Frequency-based และ advanced modes
- Interrupt callback support

#### `User/SimpleHAL/SimpleTIM.c` (NEW)
- Implement timer functions
- Auto-calculation algorithm
- TIM1 และ TIM2 interrupt handlers

#### `User/SimpleHAL/SimpleTIM_Examples.c` (NEW)
- 10 ตัวอย่างการใช้งาน Timer
- Timer interrupts, LED control, stopwatch
- Task scheduler, precise timing

#### `User/SimpleHAL/SimplePWM.h` (NEW)
- เพิ่ม PWM control API
- 8 channel definitions
- Duty cycle และ frequency functions

#### `User/SimpleHAL/SimplePWM.c` (NEW)
- Implement PWM functions
- Channel configuration management
- TIM1/TIM2 PWM mode setup

#### `User/SimpleHAL/SimplePWM_Examples.c` (NEW)
- 10 ตัวอย่างการใช้งาน PWM
- LED fade, servo control, RGB LED
- Motor speed, breathing effect

#### `User/SimpleHAL/SimpleHAL.h` (MODIFIED)
- เพิ่ม includes สำหรับ modules ใหม่
- อัปเดต version เป็น 1.1.0
- อัปเดต documentation

#### `readme/2025-12-21_Library_SimpleHAL_GPIO_TIM_PWM.md` (NEW)
- เพิ่มเอกสารครบถ้วนภาษาไทย
- อธิบายทั้ง 3 modules
- ตัวอย่างการใช้งานและผลลัพธ์

#### `readme/INDEX.md` (MODIFIED)
- เพิ่มรายการ SimpleHAL modules ใหม่

---

## [2025-12-19] - TJC HMI Library - Command Reception Feature

### เพิ่มเติม (Added)
- **ฟีเจอร์รับคำสั่งจาก TJC**: เพิ่มความสามารถในการรับและแยกคำสั่งที่ส่งมาจาก TJC HMI
  - รองรับรูปแบบ `CMD` หรือ `CMD|PARA1|PARA2|...`
  - รองรับตัวปิดท้าย `;` (optional)
  - รองรับ parameters สูงสุด 10 ตัว

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `Hardware/inc/tjc_hmi.h` (MODIFIED)
- เพิ่ม `TJC_MAX_PARAMS` constant (10 parameters)
- เพิ่ม `TJC_ReceivedCommand_t` structure สำหรับเก็บคำสั่งที่แยกแล้ว
- เพิ่ม `TJC_CommandCallback_t` callback type
- เพิ่ม `TJC_RegisterCommandCallback()` function declaration

#### `Hardware/src/tjc_hmi.c` (MODIFIED)
- เพิ่ม `command_callback` variable
- เพิ่ม `TJC_ProcessCommand()` function - แยกคำสั่งตามรูปแบบ `CMD|PARA1|PARA2`
- เพิ่มการตรวจจับคำสั่ง text ใน `TJC_ProcessResponse()` (ASCII 0x20-0x7E)
- เพิ่ม `TJC_RegisterCommandCallback()` function implementation

#### `User/example_tjc_usage.c` (MODIFIED)
- เพิ่ม `onTJC_Command()` callback example
- เพิ่มตัวอย่างการใช้งาน 3 แบบ:
  - `LED|ON` / `LED|OFF` - ควบคุม LED
  - `MOTOR|50` - ควบคุมความเร็วมอเตอร์
  - `RGB|255|128|0` - ควบคุมสี RGB
- เพิ่มการลงทะเบียน command callback

---

## [2025-12-19] - TJC HMI Library

### เพิ่มเติม (Added)
- **TJC HMI Library**: Library สำหรับควบคุม TJC HMI Display ผ่าน UART
  - ไฟล์: `Hardware/inc/tjc_hmi.h`, `Hardware/src/tjc_hmi.c`
  - ตัวอย่างการใช้งาน: `User/example_tjc_usage.c`

### ฟีเจอร์หลัก
- รองรับรูปแบบคำสั่ง `CMD|PARA1|PARA2|...` พร้อมตัวปิดท้าย `;` (optional)
- ตรวจจับและแปลความหมาย error codes ทั้งหมด 18 ตัว (ภาษาไทย)
- รองรับ UART pin remapping 3 แบบ (Default, Remap1, Remap2)
- Baud rate ปรับได้ตามต้องการ
- Circular buffer 256 bytes รองรับข้อความ 10-60 ตัวอักษร
- Callback system สำหรับ:
  - Error events
  - Touch events (Press/Release)
  - Touch coordinates (X, Y)
  - Numeric data
  - String data
  - System events (Startup, Sleep, Wake)

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `Hardware/inc/tjc_hmi.h` (NEW)
- เพิ่ม error code enums (18 codes) ตามเอกสาร TJC
- เพิ่ม return data type enums (13 types)
- เพิ่ม data structures: `TJC_TouchEvent_t`, `TJC_TouchCoord_t`, `TJC_NumericData_t`
- เพิ่ม callback function types (6 types)
- เพิ่ม public API functions (15 functions)

#### `Hardware/src/tjc_hmi.c` (NEW)
- Implement UART communication layer พร้อม pin remapping
- Implement circular buffer สำหรับรับข้อมูล
- Implement command parser รองรับ `CMD|PARA1|PARA2` และ `;`
- Implement response handler ตรวจจับ terminator `0xFF 0xFF 0xFF`
- Implement error handler แปลเป็นภาษาไทย
- Implement callback system สำหรับทุก event types

#### `User/example_tjc_usage.c` (NEW)
- เพิ่มตัวอย่างการใช้งานครบทุกฟีเจอร์
- เพิ่ม callback functions ทั้งหมด 6 ตัว
- เพิ่มตัวอย่างการส่งคำสั่งแบบต่างๆ
- เพิ่ม practical examples: แสดงค่าเซ็นเซอร์, ควบคุม LED, ปรับความสว่าง, รับข้อความ

#### `readme/2025-12-19_Hardware_TJC_HMI_Library.md` (NEW)
- เพิ่มเอกสารครบถ้วนพร้อม flowchart
- เพิ่ม root cause analysis
- เพิ่มคำแนะนำการใช้งานและการทดสอบ

#### `readme/INDEX.md` (MODIFIED)
- เพิ่มรายการ TJC HMI Library ลงใน index

---

## [2025-12-19] - Clangd Configuration

### แก้ไข (Fixed)
- แก้ไขการตั้งค่า clangd ให้ใช้ relative paths แทน absolute paths
- เพิ่มเอกสารอธิบายความแตกต่างระหว่าง relative และ absolute paths

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `.clangd` (MODIFIED)
- เปลี่ยนจาก absolute paths เป็น relative paths
- เพิ่ม RISC-V architecture flags
- เพิ่ม include paths ครบถ้วน 8 paths

#### `readme/2025-12-19_Config_ClangdConfiguration.md` (NEW)
- เพิ่มเอกสารการตั้งค่า clangd แบบสมบูรณ์
- เพิ่ม root cause analysis
- เพิ่ม workflow diagram

#### `readme/2025-12-19_Config_RelativePaths.md` (NEW)
- เพิ่มเอกสารอธิบาย relative vs absolute paths
- เพิ่มตัวอย่างการใช้งาน

---

## [2025-12-12] - Simple HAL Library

### เพิ่มเติม (Added)
- Simple HAL Library สำหรับ USART, I2C, SPI, ADC, NeoPixel
- รองรับ pin remapping ทุก peripheral
- มี comments ภาษาไทยครบถ้วน

### การเปลี่ยนแปลงของแต่ละไฟล์

#### `User/SimpleHAL/SimpleUSART.h`, `SimpleUSART.c` (NEW)
- เพิ่ม Arduino-style USART library
- รองรับ pin remapping (Default, Remap1, Remap2)

#### `User/SimpleHAL/SimpleI2C.h`, `SimpleI2C.c` (NEW)
- เพิ่ม Arduino-style I2C library
- รองรับ pin remapping

#### `User/SimpleHAL/SimpleSPI.h`, `SimpleSPI.c` (NEW)
- เพิ่ม Arduino-style SPI library
- รองรับ pin remapping

#### `User/SimpleHAL/SimpleADC.h`, `SimpleADC.c` (NEW)
- เพิ่ม flexible ADC library
- รองรับ single-shot และ continuous mode

#### `User/SimpleHAL/SimpleNeoPixel.h`, `SimpleNeoPixel.c` (NEW)
- เพิ่ม WS2812 NeoPixel library
- รองรับ 24MHz และ 48MHz อัตโนมัติ

---

## รูปแบบการบันทึก

```
## [YYYY-MM-DD] - หัวข้อการเปลี่ยนแปลง

### ประเภทการเปลี่ยนแปลง
- เพิ่มเติม (Added)
- แก้ไข (Fixed)
- เปลี่ยนแปลง (Changed)
- ลบออก (Removed)

### การเปลี่ยนแปลงของแต่ละไฟล์
#### `path/to/file` (NEW/MODIFIED/DELETED)
- รายละเอียดการเปลี่ยนแปลง
```
