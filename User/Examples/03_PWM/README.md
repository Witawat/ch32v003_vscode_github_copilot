# 03_PWM — ตัวอย่าง PWM

## ไฟล์ในโฟลเดอร์

| ไฟล์ | คำอธิบาย |
|------|----------|
| `ex01_LED_Fade.c` | Fade LED (PWM_Init, PWM_SetDutyCycle 0-100%) |
| `ex02_Servo_Control.c` | ควบคุม Servo SG90 50Hz |
| `ex03_Multiple_PWM.c` | 3 PWM channels พร้อมกัน (TIM1 + TIM2) |
| `ex04_PWM_Advanced.c` | Advanced: AdvancedInit, SetPolarity, GetDutyCycle |
| `ex05_PWM_Frequency_Sweep.c` | เปลี่ยนความถี่ runtime (PWM_SetFrequency + Buzzer) |
| **`ex06_PWM_Remap.c`** | 🆕 PWM Remap — PWM_SetRemap + analogWrite auto-init |
| **`ex07_PWM_SOP8_Guard.c`** | 🆕 ตรวจสอบแพ็กเกจ — IS_PWM_VALID_PACKAGE |

## PWM Channels

| Channel | Pin | Timer | SOP-8 |
|:---:|:---:|:---:|:---:|
| PWM1_CH1 | PD2 | TIM1 | ✅ |
| PWM1_CH2 | PA1 | TIM1 | ❌ |
| PWM1_CH3 | PC3 | TIM1 | ❌ |
| PWM1_CH4 | PC4 | TIM1 | ❌ |
| PWM2_CH1 | PD4 | TIM2 | ✅ |
| PWM2_CH2 | PD3 | TIM2 | ❌ |
| PWM2_CH3 | PC0 | TIM2 | ❌ |
| PWM2_CH4 | PD7 | TIM2 | ❌ |

## Remap ใช้ยังไง?

```c
// วิธีง่าย — ตั้งรีแมปล่วงหน้า แล้ว analogWrite จัดการ auto-init เอง
PWM_SetRemap(PWM1_CH1, PWM_REMAP_PARTIAL1);
PWM_Write(PWM1_CH1, 128);  // 50% @ 1kHz + PARTIAL1 remap

// หรือ manual
PWM_InitRemap(PWM1_CH1, 1000, PWM_REMAP_PARTIAL1);
PWM_Start(PWM1_CH1);
```

⚠️ PWM_REMAP_FULL ถูกลบแล้ว (ใช้พอร์ท PE/PB ที่ไม่มีใน CH32V003)

## ไอเดีย

```c
#if CH32V003_IS_SOP8
  #define LED_PWM_CH  PWM1_CH1    // PD2 — ใช้ได้บน SOP-8
#else
  #define LED_PWM_CH  PWM1_CH2    // PA1 — ใช้ได้บนแพ็กเกจใหญ่
#endif
```
