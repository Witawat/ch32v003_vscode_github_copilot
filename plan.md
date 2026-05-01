# CH32V003/006 — Development Plan

> ไฟล์นี้ติดตามแผนการพัฒนาโปรเจกต์ CH32V003/006
> อ่านโดย GitHub Copilot agent ทุกครั้งที่ทำงานกับโปรเจกต์นี้

---

## 🎯 Current Plan: Add 3 New Servo-Related Libraries

**สถานะ:** 🟡 In Progress
**วันที่เริ่ม:** 2026-05-01

**TL;DR:** Create 3 new libraries for servo ecosystem — multi-servo cluster with easing curves (dual backend: hw-PWM/PCA9685), ESC (BLDC controller), and servo calibration tester. All feasible on CH32V003/006.

---

### Phase 1 — ServoCluster (Multi-Servo + Easing)

- **Folder:** `User/Lib/ServoCluster/`
- **Backend:** Dual — `Servo.h` (hardware PWM, 8 ch) หรือ `PCA9685` (I2C, 16 ch)
- **API:** Init, AddServo, MoveTo(angle, duration, easing), MoveAll, SetEasing, SetSpeed, Update, IsMoving, Stop/StopAll
- **Easing:** 10 curves (LINEAR, QUAD_IN/OUT/IN_OUT, CUBIC_IN/OUT/IN_OUT, SINE_IN/OUT/IN_OUT)
- **Template:** PCA9685 (multi-channel) + Servo (pulse) + PIR (state machine non-blocking)

### Phase 2 — ESC (BLDC Motor Controller)

- **Folder:** `User/Lib/ESC/`
- **Backend:** SimplePWM 50Hz (1-4 ESCs)
- **API:** Init, Arm, SetThrottle(0-100%), SetThrottleMicroseconds(us), Calibrate, Stop, Disarm, IsArmed
- **Template:** Servo (pulse control, calibration pattern)

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
