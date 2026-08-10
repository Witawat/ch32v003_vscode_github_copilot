# AGENTS.md — Development workflow for CH32V003 SimpleHAL

## Pre-commit / pre-PR checklist

### เมื่อแก้ไข SimpleHAL API (`User/SimpleHAL/`)

ทุกครั้งที่เปลี่ยน function signature, เพิ่ม/ลบ/เปลี่ยนชื่อฟังก์ชันใน `User/SimpleHAL/*.h`:

1. ✅ รัน `scripts\validate-tutorial-api.ps1` เพื่อตรวจสอบว่า `docs/web-tutorial/*.html` ยังเรียก API ได้ถูกต้อง
2. ✅ ถ้าสคริปต์พบชื่อฟังก์ชันที่ไม่รู้จัก → ตรวจสอบว่าเป็น false positive หรือเป็น API ใน tutorial ที่ต้องอัปเดต
3. ✅ อัปเดต `docs/web-simplehal-api/` ให้ตรงกับ API ใหม่
4. ✅ เพิ่มตัวอย่างใน `User/Examples/` ถ้าเป็นฟีเจอร์ใหม่

### เมื่อแก้ไข Lib API (`User/Lib/`)

ทุกครั้งที่เปลี่ยน function signature ใน `User/Lib/*/*.h`:

1. ✅ รัน `scripts\validate-tutorial-api.ps1`
2. ✅ ตรวจสอบและอัปเดตบทเรียนที่เกี่ยวข้องใน `docs/web-tutorial/chapx_lib_<module>.html`
3. ✅ ตรวจสอบและอัปเดตหน้า catalog overview (`chapx_lib_display/elec/env1/...`) ถ้ามีการอ้างอิงถึงโมดูลนั้น
4. ✅ ตรวจสอบ `docs/web-tutorial/chapx_recipes.html` ถ้าโมดูลนั้นปรากฏในหน้าสูตรสำเร็จ

### สคริปต์ตรวจสอบ

```powershell
# ตรวจสอบแบบเร็ว — แสดงรายการ แต่ไม่ fail (ใช้ตอน dev)
powershell -File scripts\validate-tutorial-api.ps1

# ตรวจสอบแบบเข้มงวด — fail CI ถ้าพบปัญหา (ใช้ใน PR gate)
powershell -File scripts\validate-tutorial-api.ps1 -Strict
```

### Commit message format

ใช้ conventional commits:
- `fix(SimpleHAL):` — แก้บั๊กใน SimpleHAL
- `fix(Lib):` — แก้บั๊กใน User/Lib
- `feat(SimpleHAL):` — ฟีเจอร์ใหม่ใน SimpleHAL
- `docs(web-tutorial):` — อัปเดตบทเรียน
- `docs(web-simplehal-api):` — อัปเดต API reference

### โครงสร้างโปรเจกต์

```
CH32V003/
├── User/
│   ├── SimpleHAL/          # 19 modules — hardware abstraction layer
│   ├── Lib/                # 72 modules — device drivers
│   └── Examples/           # 16 workshop folders (102 example .c files)
├── Peripheral/             # WCH standard peripheral library
├── docs/
│   ├── web-tutorial/       # 123+72 HTML tutorial pages
│   ├── web-simplehal/      # SimpleHAL overview site
│   └── web-simplehal-api/  # SimpleHAL API reference
├── scripts/
│   ├── validate-tutorial-api.ps1  # Fab API detector
│   └── ...                         # Build scripts
└── AGENTS.md               # This file
```

### ข้อควรระวัง

- **ห้ามเดา API ใน tutorial** — ฟังก์ชันทุกตัวที่เขียนใน `<code>` block ต้องมีอยู่จริงใน `.h` file
- **instance-based pattern** — ไลบรารีส่วนใหญ่ใช้ handle/instance เป็น arg แรก (เช่น `LCD_Init(&lcd, ...)`) — ห้ามละ instance
- **ห้ามใช้ API ที่ deprecated** — เช่น `PWM_SimpleInit` → ใช้ `PWM_Init`, `IR_GetCode` → ใช้ `IR_GetData`
