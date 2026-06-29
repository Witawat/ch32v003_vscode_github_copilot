# 07_ADC — ตัวอย่าง ADC

## ไฟล์ในโฟลเดอร์

| ไฟล์ | คำอธิบาย |
|------|----------|
| `ex01_Simple_AnalogRead.c` | ADC พื้นฐาน (ADC_SimpleInit, ADC_Read, ADC_ToVoltage) |
| `ex02_MultiChannel_Average.c` | หลาย channel + average (ADC_ReadMultiple, ADC_ToPercent) |
| `ex03_Battery_Monitor.c` | วัดแบตเตอรี่ (ADC_GetVDD, ADC_GetBatteryPercent) |
| `ex04_Compensated_Read.c` | ชดเชย VDD (ADC_ReadVrefInt, ADC_ReadVoltageCompensated) |
| `ex05_DMA_Continuous_ADC.c` | DMA ADC ต่อเนื่อง (DMA_analogReadStart, DMA_analogReadBusy) |

## ADC Channels ต่อแพ็กเกจ

| Channel | Pin | SOP-8 | SOP-16 |
|:---:|:---:|:---:|:---:|
| 0 | PA2 | ⚠️ | ⚠️ |
| 1 | PA1 | ❌ | ⚠️ |
| 2 | PC4 | ✅ | ✅ |
| 3 | PD2 | ❌ | ✅ |
| 4 | PD3 | ❌ | ✅ |
| 5 | PD5 | ✅ | ✅ |
| 6 | PD6 | ✅ | ✅ |
| 7 | PD4 | ✅ | ✅ |

`ADC_SimpleInit()` จะเปิดเฉพาะ channels ที่มีในแพ็กเกจปัจจุบันอัตโนมัติ

## analogRead จาก SimpleGPIO

```c
// Compile-time check — ใช้ pin ที่รองรับ ADC
uint16_t val = analogRead(PD2);   // OK — PD2 รองรับ ADC
// uint16_t val = analogRead(PD7);  // ERROR — PD7 ไม่รองรับ ADC!
```
