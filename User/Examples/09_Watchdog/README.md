# 09_Watchdog — ตัวอย่าง Watchdog

## ไฟล์ในโฟลเดอร์

| ไฟล์ | คำอธิบาย |
|------|----------|
| `ex01_IWDG_Simple.c` | IWDG พื้นฐาน (IWDG_SimpleInit, IWDG_Feed) |
| `ex02_IWDG_Advanced.c` | IWDG ขั้นสูง (IWDG_Init, IWDG_WasResetCause) |
| `ex03_WWDG_Simple.c` | WWDG พื้นฐาน (WWDG_SimpleInit, WWDG_Refresh) |
| `ex04_WWDG_Interrupt.c` | WWDG + Early Wakeup Interrupt |
| `ex05_WWDG_Advanced.c` | WWDG ขั้นสูง (custom prescaler) |

## IWDG vs WWDG

| คุณสมบัติ | IWDG | WWDG |
|----------|------|------|
| Clock | LSI (~40kHz) | PCLK1 |
| Prescaler | 4-256 | 1-8 |
| Counter | 12-bit | 7-bit (down) |
| Max Timeout | ~32.7s | ~87ms |
| Window | ไม่มี | มี (0x40-0x7F) |
