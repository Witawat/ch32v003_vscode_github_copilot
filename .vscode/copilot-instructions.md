# CH32V003 — GitHub Copilot Instructions

> ไฟล์นี้โหลดโดย GitHub Copilot อัตโนมัติสำหรับ project นี้  
> **สำหรับรายละเอียดครบถ้วน**: ดูที่ [AGENTS.md](./AGENTS.md) และ [guidelines.md](./guidelines.md)

---

## 🎯 Quick Reference

**MCU**: CH32V003 (RISC-V RV32EC, 48MHz, 16KB Flash, 2KB RAM)  
**Toolchain**: RISC-V GCC12 (`riscv-wch-elf-`) from MounRiver Studio 2  
**Build**: `.\build.bat` | **Upload**: `.\upload.bat` via WCH-Link

### ✋ DO NOT MODIFY
`Core/`, `Debug/`, `Peripheral/`, `Startup/`, `Ld/` — Vendor files, never edit  
✅ Write all code in `User/` only

---

## 📌 Essential Info (See AGENTS.md for Full Details)

For complete API reference, pin mappings, library patterns, and examples, see:
- **[AGENTS.md](./AGENTS.md)** — Comprehensive AI agent guide with quick start, API reference, and common pitfalls
- **[guidelines.md](./guidelines.md)** — Complete coding standards and library development guide (Thai)
- **[README.md](../README.md)** — Project overview and SimpleHAL documentation

---

## 🔑 Key Points (Summary)

### GPIO Pin Numbers
```
PA1=0, PA2=1 | PC0=10...PC7=17 | PD2=20...PD7=25
```

### Critical Initialization Order
```c
SystemCoreClockUpdate();  // Call FIRST
// Timer_Init() auto-called by SimpleDelay — DON'T call manually
USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);  // Optional
```

### Device Library Pattern
Every library in `User/Lib/` must:
1. Include `"../../SimpleHAL/SimpleHAL.h"`
2. Have `uint8_t initialized;` in struct
3. Check NULL + initialized in all public functions
4. Use `static` for internal vars/functions
5. Author: `@author CH32V003 Library Team`

### Memory Constraints
- **Flash**: 16 KB total (keep < 12KB)
- **RAM**: 2 KB total (critical!)
- Prefer static allocation over malloc
- Check `output/CH32V003.map` after build

---

## ⚠️ Common Pitfalls

1. **ISR Rules**: Keep ISRs short — only set flags. Never use `printf`, `Delay_Ms`, `I2C_Write`, or `USART_Print` in ISR
2. **volatile**: Variables modified in ISR must be declared `volatile`
3. **No infinite busy-wait**: Always use timeout with `Get_CurrentMs()` or counter
4. **Enable Clock**: Call `RCC_APB2PeriphClockCmd(...)` before using any peripheral
5. **Resource Conflicts**: 
   - TIM1/TIM2 can't be used for both PWM and SimpleTIM simultaneously
   - Don't use SimpleI2C_Soft on pins already used by hardware I2C

---

## 📝 Coding Standards

- **Naming**: Variables `snake_case` | Constants `UPPER_SNAKE_CASE` | Functions `Module_ActionName` | Types `PascalCase_t`
- **Error Handling**: Return error codes, use enums with `LIBNAME_OK = 0`
- **Config Macros**: Use `#ifndef` to allow overrides
- **Production**: Disable printf with `DISABLE_PRINTF PRINTF_OFF` to save Flash

For complete guidelines, see [guidelines.md](./guidelines.md).
