# CH32V003/006 — Development Plan

> ไฟล์นี้ติดตามแผนการพัฒนาโปรเจกต์ CH32V003/006
> อ่านโดย GitHub Copilot agent ทุกครั้งที่ทำงานกับโปรเจกต์นี้

---

## 🎯 Current Plan: Add 6 New Low-Resource Modules

**สถานะ:** 🟡 In Progress
**วันที่เริ่ม:** 2026-05-01

**TL;DR:** Create 6 new libraries for commonly-used low-resource sensors/modules feasible on CH32V003/006 (2KB RAM, 16KB Flash). All follow existing lib patterns from `User/Lib/`.

---

### Phase 1 — ADC-Based Sensors (4 modules, ทำพร้อมกันได้)

| # | Folder | Model | Protocol | RAM |
|---|--------|-------|----------|-----|
| 1 | `SoilMoisture_YL69` | YL-69 Soil Moisture | ADC | ~16B |
| 2 | `FlameSensor_KY026` | KY-026 Flame Sensor | ADC + GPIO | ~16B |
| 3 | `SoundSensor_KY038` | KY-038 Sound Sensor | ADC | ~16B |
| 4 | `RainSensor_YL83` | YL-83 Rain Sensor | ADC | ~16B |

**Template:** `User/Lib/OH49E/OH49E.h` (ADC pattern), `User/Lib/PIR/PIR.h` (ADC+digital pattern)

### Phase 2 — Interrupt-Based (1 module)

| # | Folder | Model | Protocol | RAM |
|---|--------|-------|----------|-----|
| 5 | `WaterFlow_YFS201` | YF-S201 Water Flow | GPIO Interrupt | ~32B |

**Template:** `User/Lib/RotaryEncoder/RotaryEncoder.h` (interrupt counting)

### Phase 3 — UART-Based (1 module, depends on Phase 1-2)

| # | Folder | Model | Protocol | RAM |
|---|--------|-------|----------|-----|
| 6 | `GPS_NEO6M` | NEO-6M GPS | UART | ~256B |

**Template:** `User/Lib/PMS5003/PMS5003.h` (UART parsing)
**Note:** Uses USART1, same as debug output. Provide option to disable debug during GPS use.

### Phase 4 — Build Verification

- Build ทั้งหมด → exit code 0, no warnings
- Check pattern compliance: initialized flag, null checks, static internals

---

## 📋 Key Patterns (จาก project guidelines)

- New Lib path: `User/Lib/<Name>_<Model>/<Name>.h/.c/README.md`
- Include from lib: `"../../SimpleHAL/SimpleHAL.h"`
- Author in libs: `@author CH32V003 Library Team`
- Header guard: `#ifndef __<NAME>_H`
- Every struct needs `initialized` flag
- Every public function needs null + initialized check
- Internal vars/functions must be `static`
- ISR timing: `__disable_irq()` / `__enable_irq()`

---

## ✅ Excluded (Not Feasible on CH32V003/006)

| Module | Reason |
|--------|--------|
| TFT LCD (ST7735/ILI9341) | Framebuffer >2KB, RAM not enough |
| E-Ink (SSD1680) | Image buffer too large |
| Fingerprint R307 | Image buffer needs external RAM |
| MicroSD + FATFS | RAM ~512B, borderline |
| SIM800L | UART contention + AT buffer |
| LoRa SX1278 | SPI bus contention |
| CAN Bus MCP2515 | SPI + buffer constraints |
