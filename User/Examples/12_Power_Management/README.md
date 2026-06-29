# 12_Power_Management — ตัวอย่าง Power Management

## ไฟล์ในโฟลเดอร์

| ไฟล์ | คำอธิบาย |
|------|----------|
| `ex01_Sleep_Mode.c` | Sleep Mode (CPU หยุด peripherals ทำงาน) |
| `ex02_Standby_AWU.c` | Standby + AWU timer |
| `ex03_Standby_External_Wake.c` | Standby + External interrupt wake |
| `ex04_PVD_Monitor.c` | PVD ตรวจจับแรงดันตก |
| `ex05_Battery_Life_Estimation.c` | คำนวณอายุแบตเตอรี่ |
| `ex06_Wakeup_Pin.c` | Wakeup Pin + AWU |

## Power Modes

| Mode | กินไฟ | CPU | RAM | Wake up |
|------|:---:|:---:|:---:|---------|
| Normal | ~5-10mA | ✅ | ✅ | - |
| Sleep | ~1-2mA | ❌ | ✅ | Any interrupt |
| Standby | ~2-5µA | ❌ | ❌ | AWU/EXTI/NRST |

> ⚠️ `PWR_EnableWakeupPin()` — PA0 ไม่มีในทุกแพ็กเกจ CH32V003 ใช้ AWU หรือ EXTI แทน

## AWU Prescaler Quick Reference

| Prescaler | Time/Count | Max (63) |
|:---:|------|------|
| 1024 | ~8ms | ~504ms |
| 4096 | ~32ms | ~2s |
| 10240 | ~80ms | ~5s |
| 61440 | ~480ms | ~30s |
