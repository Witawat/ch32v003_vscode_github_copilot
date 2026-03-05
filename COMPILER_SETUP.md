# CH32V003 — Compiler & Build Setup

## Overview

โปรเจคนี้ใช้ **RISC-V Embedded GCC12** จาก MounRiver Studio 2 ร่วมกับ Windows batch scripts เป็น build system  
ไม่ใช้ Makefile เนื่องจาก MounRiver Studio 2 ไม่มี `make.exe` ใน toolchain path

---

## Toolchain

| Item         | Value                                      |
|--------------|--------------------------------------------|
| Compiler     | `riscv-wch-elf-gcc` (GCC 12.2.0)          |
| Prefix       | `riscv-wch-elf-`                           |
| Architecture | `rv32ecxw` / `ilp32e`                      |
| Multilib     | `rv32ec_xw/ilp32e`                         |

**Path:**
```
C:\MounRiver\MounRiver_Studio2\resources\app\resources\win32\components\WCH\
  Toolchain\RISC-V Embedded GCC12\bin\
```

> **หมายเหตุ:** ห้ามใช้ GCC8 (`riscv-none-embed-`) กับ CH32V003  
> เพราะขาด `rv32ec` multilib → linker error: `can't link RVE with other target`

---

## Build System Files

| ไฟล์          | หน้าที่                                           |
|---------------|---------------------------------------------------|
| `build.bat`   | Compile ทุกไฟล์ → link → สร้าง HEX + แสดง memory usage |
| `clean.bat`   | ลบ `obj\` และ `output\`                           |
| `rebuild.bat` | เรียก `clean.bat` แล้ว `build.bat`               |
| `upload.bat`  | Flash firmware ผ่าน WCH-Link ด้วย OpenOCD        |

### VS Code Tasks (`.vscode/tasks.json`)

| Task Label                      | Shortcut       |
|---------------------------------|----------------|
| Build CH32V003 (MounRiver)      | `Ctrl+Shift+B` |
| Clean CH32V003                  | Terminal → Run Task |
| Rebuild CH32V003                | Terminal → Run Task |
| Upload CH32V003 (WCH-Link)      | Terminal → Run Task |

---

## Compiler Flags

```bat
set ARCH=-march=rv32ecxw -mabi=ilp32e

:: C flags
-msmall-data-limit=0 -msave-restore -Os -fmessage-length=0
-fsigned-char -ffunction-sections -fdata-sections -fno-common
-Wunused -Wuninitialized -g

:: Linker flags (ARCH ต้องใส่ทั้ง compile และ link เสมอ)
-T"Ld/Link.ld" -nostartfiles -Xlinker --gc-sections
-Wl,-Map="output/CH32V003.map" --specs=nano.specs --specs=nosys.specs -lprintf
```

> **สำคัญ:** `-march=rv32ecxw -mabi=ilp32e` ต้องอยู่ทั้งใน `CFLAGS` และ `LDFLAGS`  
> หากขาดใน LDFLAGS linker จะเลือก multilib ผิด

---

## Include Paths

```
Debug/
Core/
User/
Peripheral/inc/
User/SimpleHAL/
```

ไฟล์ที่ถูก compile (ใน `build.bat`):
- `Core/core_riscv.c`
- `Debug/debug.c`
- `Peripheral/src/*.c`
- `User/main.c`, `system_ch32v00x.c`, `ch32v00x_it.c`
- `User/SimpleHAL/*.c`
- `User/Lib/**/*.c` (recursive — ทุก Lib ถูก compile อัตโนมัติ)
- `Startup/startup_ch32v00x.S` (Assembly)

---

## Output Structure

```
CH32V003/
├── obj/                  ← Object files (*.o), objects.rsp  [intermediates]
└── output/               ← Final firmware
    ├── CH32V003.elf       (สำหรับ GDB debugger)
    ├── CH32V003.hex       (สำหรับ flash ผ่าน WCH-Link)
    └── CH32V003.map       (memory map)
```

> `obj/` และ `output/` ถูกสร้างใหม่ทุกครั้งที่ rebuild — ห้าม commit เข้า version control

---

## Memory (CH32V003)

| Region | Size  | ที่อยู่       |
|--------|-------|--------------|
| Flash  | 16 KB | `0x00000000` |
| RAM    | 2 KB  | `0x20000000` |

Build จะแสดง memory usage หลัง compile สำเร็จ:
```
Flash:  3572 / 16384 bytes  (21%)  [Free: 12812 bytes]
RAM:    420  / 2048  bytes  (20%)  [Free: 1628  bytes]
```

---

## Upload / Flash

ใช้ OpenOCD จาก MounRiver Studio 2 ผ่าน **WCH-Link** (SWD):

```
Tool:   OpenOCD\OpenOCD\bin\openocd.exe
Config: OpenOCD\OpenOCD\bin\wch-riscv.cfg
```

```bat
openocd.exe -f "wch-riscv.cfg" -c "program {output/CH32V003.elf} verify reset exit"
```

---

## VS Code IntelliSense (`.vscode/c_cpp_properties.json`)

Compiler path ที่ใช้อยู่:
```
C:\MounRiver\MounRiver_Studio2\resources\app\resources\win32\components\WCH\
  Toolchain\RISC-V Embedded GCC12\bin\riscv-wch-elf-gcc.exe
```

---

## การแก้ไขเส้นทาง (ถ้า MounRiver ติดตั้งในตำแหน่งอื่น)

แก้ไขตัวแปร `TOOLCHAIN_BASE` ใน `build.bat`:
```bat
set TOOLCHAIN_BASE=C:\MounRiver\MounRiver_Studio2\resources\app\resources\win32\components\WCH\Toolchain
```

แก้ไข `compilerPath` ใน `.vscode/c_cpp_properties.json`

แก้ไข `OPENOCD_DIR` ใน `upload.bat`

---

## Troubleshooting

### `[ERROR] Compiler not found`
- ตรวจสอบว่า MounRiver Studio 2 ติดตั้งอยู่
- ตรวจสอบ path ใน `build.bat` (ตัวแปร `TOOLCHAIN_BASE`)
- ต้องใช้ `TOOLCHAIN_CHOICE=2` (GCC12) เสมอ

### Build ผ่านแต่ Flash/RAM แสดงผิดพลาด
- เกิดจาก `size.exe` path มี spaces
- `build.bat` ใช้ temp file (`obj\size.tmp`) แก้ปัญหานี้แล้ว
- ถ้าค้างให้รัน `clean.bat` ก่อน

### Upload ล้มเหลว
- ตรวจสอบว่า WCH-Link ต่อสายและไฟติด
- ต้องมี driver WCH-Link ติดตั้งแล้ว (ติดตั้งพร้อม MounRiver Studio 2)
- ตรวจสอบว่า `output\CH32V003.elf` มีอยู่ (build ก่อน upload)

### Linker error: `can't link RVE`
- สาเหตุ: ใช้ GCC8 (`riscv-none-embed-`) แทน GCC12
- แก้ไข: ตรวจสอบใน `build.bat` ว่า `TOOLCHAIN_CHOICE=2`

### Object files เก่าทำให้ build error
- รัน `rebuild.bat` (clean + build) แทน `build.bat`

---

## References

- แผ่นข้อมูล CH32V003: จัดทำโดย WCH (Nanjing Qinheng Microelectronics)
- RISC-V ISA Spec: https://riscv.org/
- Programming guidelines: `.vscode/guidelines.md`

---

**อัปเดตล่าสุด**: 2026-03-04  
**Toolchain**: RISC-V Embedded GCC12 (riscv-wch-elf-, v12.2.0)  
**SimpleHAL**: v1.9.0
