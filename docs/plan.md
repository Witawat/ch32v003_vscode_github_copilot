# CH32V003/006 — Development Plan

> ไฟล์นี้ติดตามแผนการพัฒนาโปรเจกต์ CH32V003/006
> อ่านโดย GitHub Copilot agent ทุกครั้งที่ทำงานกับโปรเจกต์นี้

---

## ✅ Completed — 2026-05-25

### P10 LED Matrix Display Library
- **Folder:** `User/Lib/P10/`
- รองรับ Single/Dual/RGB color (compile-time config)
- Timer interrupt scan (TIM2), configurable resolution (32×16 / 16×16)
- Frame buffer 1-bit per pixel per color
- API: Init, SetPixel, Clear, Fill, Deinit, ScanHandler

### WS2812 8×8 LED Matrix Library
- **Folder:** `User/Lib/WS2812Matrix/`
- Instance struct + SimpleGPIO pins pattern
- 2 wiring patterns: Zigzag + Snake
- Drawing primitives (line, rect, circle)
- **v1.1:** Font rendering (ASCII 5x7 + Thai 8x8 + UTF-8), Scrolling text, Sprite/Bitmap, Effects (fade/wipe), Buffer utilities (rotate/flip)

### MAX7219 v1.0 → v1.1 Upgrade
- **Folder:** `User/Lib/MAX7219/`
- Thai UTF-8 rendering (DrawCharThai, DrawStringThai) — 44 consonants + Thai digits
- New effects: Wipe (4 dirs), Blink, Sparkle, MarqueeBorder, RainEffect, RunningLight
- Vertical scrolling, Buffer utilities (Shift, ScrollBuffer, ProgressBar)
- Config macros (MAX7219_ENABLE_THAI_FULL, MAX7219_ENABLE_EFFECTS)

---

## 🎯 Current Plan: None (Completed all scheduled work)

**สถานะ:** ✅ Done
**วันที่:** 2026-05-25

### Phase 3 — ServoTester (Calibration Tool)

- **Folder:** `User/Lib/ServoTester/`
- **Backend:** Servo.h (1 ch)
- **API:** Init, Sweep(start_us, end_us, step, delay), FindCenter, FindPulseRange, SetPulse, GetCurrentPulse
- **Template:** Servo (pulse) + I2CScan (diagnostic tool)

### Phase 4 — Build Verification

- Build ทั้งหมด → exit code 0, no warnings
- Memory: +264 bytes RAM, +5.5KB Flash → feasible (free: 1628/14612)

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
