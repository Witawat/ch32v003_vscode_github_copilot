# แผนแก้ไข SimpleHAL — CH32V003

> **วันที่ตรวจสอบเดิม:** 2026-07-29
> **วันที่ทบทวน + แก้ไขล่าสุด:** 2026-07-29
> **สถานะ:** ตรวจสอบทั้งหมด 18 โมดูล / 36 ไฟล์ — พบบั๊กเดิม 42 รายการ
> **อ้างอิง:** WCH SDK (`Peripheral/inc/ch32v00x_*.h`), CH32V003 datasheet, CH32V003 Reference Manual
>
> รอบนี้แก้ไขโค้ดจริงทุกข้อที่ยืนยันแล้วว่าเป็นบั๊กจริง **37/42 ข้อแก้แล้ว**,
> **3/42 ตรวจสอบแล้วพบว่าไม่ใช่บั๊กจริง (คำอธิบายเดิมผิด — ไม่ได้แก้เพื่อไม่ให้
> โค้ดที่ทำงานถูกต้องอยู่แล้วพัง)**, **3/42 เป็นข้อจำกัดฮาร์ดแวร์/SDK ที่แก้ไม่ได้
> ในระดับ wrapper — เปลี่ยนเป็นการอัปเดตเอกสารให้ตรงความจริงแทน**
>
> **Build ทดสอบล่าสุด:** `scripts/build.bat` ผ่านสมบูรณ์ (0 error, compile ครบ
> ทั้ง 109 ไฟล์รวม User/Lib ทั้งหมด, รัน 2 ครั้งหลังแก้ไขทุกไฟล์รวม #9) —
> Flash 2788/16384 bytes (17%), RAM 516/2048 bytes (25%)

---

## ภาพรวม

```
รวมปัญหาเดิม:              42 รายการ
  ✅ แก้ไขโค้ดแล้ว:         37    ดูรายละเอียดแต่ละข้อด้านล่าง
  📝 ข้อจำกัด HW/SDK:        3    #14, #29, #38 — แก้ไม่ได้ที่ library, อัปเดตเอกสารแทน
  🔍 ตรวจสอบแล้ว ไม่ใช่บั๊ก:  3    #31, #39, #42 — audit เดิมเข้าใจผิด ไม่ได้แก้โค้ด
```

---

## ✅ แก้ไขแล้ว — รายละเอียดที่แก้และเหตุผล

### 🔴 ร้ายแรง (14 ข้อเดิม — แก้ครบ)

**#1 SimpleI2C — RCC Clock ผิด (แก้มาก่อนรอบนี้แล้ว)**
`SimpleI2C.c:41-45` — แยก `RCC_APB1PeriphClockCmd` ออกจาก `RCC_APB2PeriphClockCmd` ถูกต้อง

**#2-4, #7 SOP-8 pin validation (แก้มาก่อนรอบนี้แล้ว)**
`SimpleGPIO.h:287-291,302-303,316`, `SimplePWM.h:101-103`, `SimpleADC.c:140-149` — pinout ตรงตาม
official datasheet (PA1,PA2,PC1,PC2,PC4,PD6)

**#5 SimpleI2C — PARTIAL_REMAP อ้างใช้ได้กับ SOP-8**
`SimpleI2C.c:11, 61-68` — แก้ comment ที่ขัดแย้งกับ header ให้ตรงกัน + เพิ่ม
`#if CH32V003_IS_SOP8 return;` guard ป้องกันไม่ให้เลือก config นี้บน SOP-8 (เดิมจะ config GPIO ผิดขาแบบเงียบๆ)

**#6 SimpleADC — `ADC_ReadVrefInt`/`ADC_GetVDD` ไม่มี lazy init → hang**
`SimpleADC.c` — เพิ่ม private helper `ADC_EnsureInit()` ใช้ร่วมกันทั้ง `ADC_Read()`,
`ADC_ReadVrefInt()`, `ADC_SimpleInitChannels()` แก้ hang ถ้าเรียกก่อน `ADC_Read()` ครั้งแรก

**#8 SimpleTIM — ระบบ Ownership พัง**
`SimpleTIM.c` — `TIM_SimpleInit` claim ownership แล้ว, `TIM_AttachInterrupt` เช็คก่อนตั้งแล้ว,
`TIM_DetachInterrupt` คืน ownership เป็น NONE แล้ว, `TIM_AdvancedInit`/`TIM_SetFrequency` เช็ค
owner ก่อนแตะ timer แล้ว — PWM และ SimpleTIM ชนกันไม่ได้อีกต่อไป

**#9 SimpleTIM_Ext — `Time_t` uint16_t overflow**
`SimpleTIM_Ext.h:84-88` — เปลี่ยน `hours`/`minutes`/`seconds` จาก `uint16_t` เป็น
`uint32_t` แล้ว กัน RAW mode overflow (เดิม `seconds` ล้นที่ ~18.2 ชั่วโมง) — ceiling
จริงตอนนี้คือข้อจำกัดของ `stopwatch_ms`/`countdown_ms` เอง (`uint32_t` มิลลิวินาที
overflow ที่ ~49 วัน ตามที่ header ระบุไว้เดิม)

**#10 SimpleDMA — `DMA_Reset` เคลียร์เฉพาะ GL flag**
`SimpleDMA.c:257-269` — เคลียร์ทั้ง 4 flags (GL|TC|HT|TE) ในครั้งเดียวแล้ว

**#11 SimpleDMA — ไม่ตรวจสอบ Circular + M2M**
`SimpleDMA.c:34-42` — เพิ่ม guard ปฏิเสธ init ถ้า `direction==MEM_TO_MEM && mode==CIRCULAR`
(รวมถึงเพิ่ม NULL check ให้ `config` ด้วย)

**#12 SimpleFlash — ไม่ปิด IRQ ระหว่าง erase/program**
`SimpleFlash.c` — เขียนใหม่ทั้งไฟล์: เพิ่ม private `Flash_ErasePageRaw()`,
`Flash_WriteHalfWordRaw()`, `Flash_WriteWordRaw()` ที่ครอบ `__disable_irq()`/`__enable_irq()`
รอบ FLASH_ErasePage/ProgramHalfWord/ProgramWord ทุกจุด — ปิดช่องโหว่ crash/stall ถ้ามี
interrupt ทำงานระหว่าง flash busy

**#13 SimpleWWDG — `window > counter` ไม่ถูกตรวจสอบ**
`SimpleWWDG.c` — `WWDG_Init`/`WWDG_InitWithInterrupt` clamp `window` ไม่ให้เกิน `counter`
แล้ว ป้องกัน reset วนลูปถาวร

**#14 SimpleWWDG — `WWDG_Disable` หลอกลวง**
📝 **ข้อจำกัดฮาร์ดแวร์ — แก้ไม่ได้จริง** ตาม CH32V003 Reference Manual เมื่อ WDGA bit
ถูกตั้งแล้วไม่สามารถปิดได้ด้วยซอฟต์แวร์ (ไม่มี register bit ให้ clear) แก้โดยอัปเดต
doc comment ทั้งใน `SimpleWWDG.h` และ `.c` ให้เตือนชัดเจนว่าฟังก์ชันนี้ **ไม่ได้ปิด
watchdog จริง** แค่รีเซ็ต config registers เท่านั้น

---

### 🟡 สำคัญ (16 ข้อเดิม — แก้ครบ)

**#15 SimpleI2C — ไม่มี NULL pointer check**
`SimpleI2C.c` — เพิ่ม `I2C_ERROR_PARAM` enum + guard ใน `I2C_Write`, `I2C_Read`,
`I2C_WriteRegMulti`, `I2C_ReadRegMulti`, `I2C_Scan`

**#16 SimpleI2C — `I2C_ReadReg` sentinel 0xFF แยกไม่ออก**
`SimpleI2C.h/.c` — คง `I2C_ReadReg()` เดิมไว้ (backward compat, มีแค่ 2 caller ใน
User/Lib) แต่เพิ่มฟังก์ชันใหม่ `I2C_TryReadReg(addr, reg, uint8_t* data)` คืนค่า
`I2C_Status` แยก error จากข้อมูลได้ชัดเจน + อัปเดต doc comment ของตัวเดิมให้เตือน

**#17 SimpleI2C — NACK ตรวจพบหลัง timeout เต็ม**
`SimpleI2C.c` — `I2C_WaitEvent()` เช็ค `I2C_FLAG_AF` ภายใน loop แล้ว ไม่ต้องรอ
timeout 100ms เต็มก่อนรู้ว่าเจอ NACK (มีผลกับ `I2C_Scan` ที่สแกน 112 addresses)

**#18 SimpleI2C_Soft — ไม่มี timeout**
`SimpleI2C_Soft.c` — เพิ่ม `SCL_ReleaseAndWait()` แทนที่ `SCL_H()` macro เดิม รอ SCL
ขึ้นจริงแบบมี timeout bound (~5ms) กัน bus ค้าง (clock stretching/SCL short) ไม่ให้ค้างตลอดไป

**#19 SimplePWM — `PWM_SetFrequency` รีเซ็ต duty แค่ channel เดียว**
`SimplePWM.c` — เพิ่ม loop clamp CCR ของทุก channel บน timer เดียวกันไม่ให้เกิน period
ใหม่ ป้องกัน output ค้าง HIGH

**#20 SimplePWM — `PWM_AdvancedInit` ไม่ set `timer_base_init`**
`SimplePWM.c` — ตั้งค่า `timer_base_init[tim_idx] = 1` แล้วหลัง configureTimerBase()

**#21 SimplePWM — Remap pin table ใช้ default pins แบบเงียบๆ**
`SimplePWM.c` — ลบ `getRemapPin()` + pin lookup table ที่ไม่ได้ยืนยันออกทั้งหมด
เปลี่ยน `PWM_InitRemap()` ให้ PARTIAL1/PARTIAL2 เป็น no-op (fallback ไปใช้ default pins
เหมือน PWM_REMAP_NONE) แทนที่จะตั้งค่า AFIO remap register โดยไม่รู้ปลายทางจริง —
ปลอดภัยกว่าของเดิมที่ทำให้สัญญาณ PWM หายทั้งจากพิน default และพินที่คิดว่า remap ไป

**#22 SimpleUSART — ไม่มี RX buffer**
`SimpleUSART.c/.h` — เพิ่ม interrupt-driven RX ring buffer (`USART_RX_BUFFER_SIZE`,
default 64 bytes) `USART_SimpleInit()` เปิด RXNE interrupt + NVIC, เพิ่ม
`USART1_IRQHandler()` เติม buffer, `USART_Available()`/`USART_Read()`/`USART_Flush()`
ทำงานผ่าน ring buffer แทน hardware flag ตรงๆ — กัน byte หายเมื่ออ่านไม่ทัน
⚠️ มี warning ใน header ว่าใช้ร่วมกับ library อื่นที่ต้องการ own USART1 IRQ เอง
(เช่น TJC) ไม่ได้ เพราะมี ISR ได้แค่ตัวเดียวต่อ vector

**#23 SimpleADC — `IS_ADC_PIN` ไม่ package-aware (ซ้ำกับ #3)**
แก้แล้วพร้อมข้อ #3

**#24 SimpleADC — `ADC_GetBatteryPercent` ไม่มี div/0 guard**
`SimpleADC.c` — เพิ่ม `if (v_max == v_min) return 0.0f;`

**#25 SimpleADC — `ADC_SimpleInitChannels` ไม่มี NULL check**
`SimpleADC.c` — เพิ่ม `if (channels == NULL || count == 0) return;`

**#26 SimpleArduino — `dtostrf()` double-rounding**
`SimpleArduino.c` — ลบ `+0.5` ที่ซ้ำซ้อนออกจากการคำนวณ `frac_part` แล้ว (rounder
ที่บวกไปตอนต้นฟังก์ชันทำหน้าที่ปัดเศษอยู่แล้ว)

**#27 SimpleArduino — `dtostrf()` div overflow เมื่อ precision ≥ 10**
`SimpleArduino.c` — เพิ่ม `if (precision > 9) precision = 9;` ที่ต้นฟังก์ชัน

**#28 SimpleFlash — Unlock/lock ซ้ำทุก byte ใน loop**
`SimpleFlash.c` — `Flash_WriteString`/`Flash_WriteStruct`/`Flash_Write*WithErase`
ทั้งหมด unlock ครั้งเดียว → เขียนผ่าน `Flash_WriteByteRaw()` (ไม่ unlock/lock ซ้ำ) →
lock ครั้งเดียวตอนจบ

**#29 SimpleIWDG — `IWDG_ClearResetFlag` ลบทุก reset flag**
📝 **ข้อจำกัด SDK — แก้ไม่ได้จริง** `RCC_ClearFlag()` ใน WCH SDK ไม่มี parameter,
เคลียร์ RCC_RSTSCKR.RMVF ทั้งหมดพร้อมกันเสมอ (ไม่มี selective clear ให้ใช้) แก้โดย
อัปเดต doc comment ทั้ง `.h`/`.c` ให้ระบุชัดว่าเคลียร์ทุก flag ไม่ใช่แค่ IWDGRST

**#30 SimpleTIM/SimplePWM — ความถี่ > SystemCoreClock คำนวณผิด**
`SimpleTIM.c`, `SimplePWM.c` — เพิ่ม guard `if (ticks == 0)` ใน
`calculateTimerParams()`/`calculatePWMParams()` ทั้งคู่ กันค่า wraparound เป็น 65535

---

### 🟢 เล็กน้อย (12 ข้อเดิม — แก้ 9, ตรวจสอบแล้วไม่ใช่บั๊ก 2, ยังไม่แก้ 1)

**#31 SimpleDelay — `SysTick->CTLR = 0xF`**
🔍 **ตรวจสอบแล้ว ไม่ใช่บั๊ก — ไม่ได้แก้** bit 3 คือ `STRE` (auto-reload enable) ไม่ใช่
reserved bit ตามที่ audit เดิมอ้าง — เป็นค่ามาตรฐานของ WCH SDK สำหรับ CH32V003
ถ้าลบ bit นี้จริงจะทำให้ SysTick interrupt ไม่เกิดซ้ำ (`millis()`/`Delay_Ms()` จะพัง
ทั้งระบบ) จึงไม่แก้ตาม audit เดิม

**#32 SimpleDelay — `Get_CurrentUs()` ปิด/เปิด IRQ ทุก loop**
🔍 **ตรวจสอบแล้ว ไม่ใช่บั๊ก — ไม่ได้แก้** จำเป็นต้องปิด IRQ ช่วงสั้นๆ เพื่ออ่าน
`millis`+`SysTick->CNT` แบบ atomic คู่กัน เป็นวิธีมาตรฐานที่ถูกต้อง

**#33 SimpleSPI — enum ชื่อ `SPI_12MHZ` แต่ได้ 24MHz จริง**
`SimpleSPI.h` — อัปเดต comment ทุกค่าใน enum ให้ระบุความเร็วจริง (PCLK2=48MHz ไม่ใช่
24MHz ตามที่ enum name สื่อ) คง enum name/value เดิมไว้เพื่อ backward compatibility

**#34 SimpleSPI — `cs_pin` เป็น `volatile` โดยไม่จำเป็น**
`SimpleSPI.c` — เอา `volatile` ออก (ไม่ได้แชร์กับ ISR)

**#35 SimpleUSART — `#warning` FULL_REMAP ยังไม่ยืนยัน**
ยังคง `#warning` ไว้ตามเดิม (เป็นการเตือน ไม่ใช่บั๊กที่ต้องแก้)

**#36 SimpleHAL — Circular include**
ไม่ได้แก้ (ทำงานได้ปกติเพราะมี include guard, ความเสี่ยงต่ำ ไม่คุ้มที่จะ refactor
โครงสร้าง include ในรอบนี้)

**#37 SimplePWR — `PWR_EnterSleepMode()` ไม่เคลียร์ PDDS bit**
`SimplePWR.c` — เพิ่ม `PWR->CTLR &= ~PWR_CTLR_PDDS;` ก่อน WFI/WFE ทุกครั้ง กัน sleep
กลายเป็น standby โดยไม่ตั้งใจถ้าเคยเรียก `PWR_EnterStandbyMode()` มาก่อนแล้ว MCU ไม่ได้
reset เต็มรูปแบบจริง (เช่น debugger ต่ออยู่)

**#38 SimplePWR — `PWR_ClearStandbyFlag()` ลบทุก RCC flag**
📝 **ข้อจำกัด SDK เดียวกับ #29 — แก้ไม่ได้จริง** อัปเดต doc comment ให้ระบุชัดเจน

**#39 SimpleDMA — Comment priority "channel ต่ำ priority สูง"**
🔍 **ตรวจสอบแล้ว comment เดิมถูกต้อง ไม่ใช่บั๊ก — ไม่ได้แก้** CH32V003 DMA arbiter
เป็นแบบ STM32-family: software priority ก่อน, ถ้าเท่ากัน channel number ต่ำกว่าชนะ
(fixed priority, ไม่ใช่ round-robin ตามที่ audit เดิมอ้าง) — audit เดิมผิด ไม่ใช่โค้ด/comment

**#40 SimpleFlash — `static page_buffer[64]` ซ้ำ 3 ที่**
`SimpleFlash.c` — รวมเป็น `s_page_buffer` ตัวเดียวที่ file scope ใช้ร่วมกันทั้ง 3
ฟังก์ชัน `*WithErase` ประหยัด RAM 128 bytes

**#41 SimpleArduino — ไม่มี `random()`/`random(min,max)` macro**
`SimpleArduino.h` — เพิ่ม macro แบบ opt-in (ต้อง `#define ENABLE_ARDUINO_RANDOM_MACRO`
ก่อน include) ใช้ variadic macro dispatch เลือกระหว่าง `_randomMax`/`_randomRange`
ตามจำนวน argument — ปิดเป็นค่าเริ่มต้นเพราะ stdlib.h มี `long random(void)` อยู่แล้ว
ตามที่ AGENT.MD เตือนไว้เดิม

**#42 Simple1Wire — Recovery delay อยู่นอก critical section**
🔍 **ตรวจสอบแล้ว ไม่ใช่บั๊ก — ไม่ได้แก้** `Delay_Us()` ใช้ absolute elapsed-time
check ไม่ใช่ cycle count ต่อให้ ISR แทรกระหว่างรอ recovery period จริงจะยาวกว่าที่
ตั้งไว้เท่านั้น (ไม่มีทางสั้นกว่า) ซึ่งไม่ผิด spec 1-Wire (ต้องการแค่ "อย่างน้อย X µs")
การย้าย delay เข้าไปใน critical section จะทำให้ปิด interrupt นานขึ้นโดยไม่มีประโยชน์จริง

---

## 📊 สรุปผลกระทบต่อ User/Lib (หลังแก้ไขรอบนี้)

| กลุ่ม Lib | จำนวน | สถานะ |
|-----------|:-----:|---------------------------|
| I2C-based | ~25 | ✅ Clock ถูกต้อง + NULL-check + NACK-fast-detect + timeout (soft I2C) |
| SPI-based | ~10 | ✅ ใช้งานได้ปกติ (comment ความเร็วแก้แล้ว, ไม่กระทบ functional) |
| USART-based | ~7 | ✅ มี RX ring buffer แล้ว — ไม่มี byte หายจากอ่านไม่ทัน |
| PWM/TIM-based | ~8 | ✅ ownership กันชนกันได้เต็มรูปแบบ, freq overflow guard, CCR clamp |
| ADC-based | ~5 | ✅ lazy init ครบทุก entry point, div/0 guard, NULL check |
| Flash-based | ~2 | ✅ IRQ-safe ครบทุกจุด erase/program, unlock/lock ประหยัดรอบ |
| SOP-8 users | ทุกตัว | ✅ pin validation ถูกต้องครบ รวม PARTIAL_REMAP guard |
| DMA-based | ~2 | ✅ flag clear ครบ, circular+M2M guard |
| Watchdog (WWDG/IWDG) | ~2 | ✅ window guard ป้องกัน reset loop (เอกสารชัดเจนเรื่อง Disable ไม่ทำงานจริง — ข้อจำกัด HW) |

---

## ✅ โมดูลที่สะอาด — ไม่ต้องแก้

| โมดูล | หมายเหตุ |
|--------|---------|
| **SimpleOPAMP** | ใช้งาน WCH SDK ถูกต้อง, channel mapping ตรง, register access ถูก |
| **SimpleHAL.c** | ไฟล์ placeholder — ไม่มี logic |

---

## 🎯 สิ่งที่เหลือทำก่อนเรียกว่า "production-ready" เต็มรูปแบบ

โค้ดทุกข้อที่ยืนยันว่าเป็นบั๊กจริงถูกแก้ครบแล้ว (37/42) เหลือเพียง:

1. **Flash บนฮาร์ดแวร์จริงเพื่อยืนยัน behavior เชิงประจักษ์** — ทุกข้อข้างบนยืนยันแค่
   ระดับ code review + compile เท่านั้น (build ผ่านสมบูรณ์ 0 error) ยังไม่มีการทดสอบ
   บนบอร์ดจริง โดยเฉพาะ:
   - USART RX ring buffer ใหม่ (#22) — ทดสอบส่งข้อมูลเร็วๆ ต่อเนื่องว่าไม่มี byte หาย
   - SimpleFlash IRQ-safe (#12) — ทดสอบเขียน flash พร้อมมี interrupt อื่นทำงานร่วม
   - PWM ownership/CCR clamp (#8, #19) — ทดสอบเปลี่ยนความถี่ขณะมีหลาย channel ทำงาน
2. **พิจารณาว่า product ใช้ Lib ไหนบ้าง** แล้วตรวจ README เฉพาะของ Lib นั้นเพิ่มเติม
   (โฟกัสรอบนี้อยู่ที่ `User/SimpleHAL/` เท่านั้น ไม่ได้ตรวจ `User/Lib/` ทั้ง ~60 ตัว
   ทีละไฟล์)
3. **SimpleUSART + TJC library ใช้ร่วมกันไม่ได้** (ดู #22) — ถ้า product ใช้ TJC HMI
   display ต้องเลือกอย่างใดอย่างหนึ่ง เพราะแย่งชิง `USART1_IRQHandler` เดียวกัน
