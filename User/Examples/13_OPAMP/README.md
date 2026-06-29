# 13_OPAMP — ตัวอย่าง OPAMP

## ไฟล์ในโฟลเดอร์

| ไฟล์ | คำอธิบาย |
|------|----------|
| `ex01_Voltage_Follower.c` | Voltage Follower (Buffer, Gain=1) |
| `ex02_NonInverting_Amp.c` | Non-Inverting Amplifier (Gain>1) |
| `ex03_Inverting_Amp.c` | Inverting Amplifier (Gain<0) |
| `ex04_Comparator.c` | Comparator (HIGH/LOW output) |
| `ex05_OPAMP_with_ADC.c` | OPAMP + ADC อ่าน output |

## Quick Start

```c
// Voltage Follower
OPAMP_SimpleInit(OPAMP_MODE_VOLTAGE_FOLLOWER);
OPAMP_Enable();

// Comparator
OPAMP_ConfigComparator(OPAMP_CHP0, OPAMP_CHN0);
OPAMP_Enable();
```

OPAMP ใช้ได้กับทุกรุ่น CH32V003 (ซิลิคอนเดียวกัน)
เชื่อมต่อผ่าน EXTEN register โดยตรง ไม่ต้องเปิด RCC clock แยก
