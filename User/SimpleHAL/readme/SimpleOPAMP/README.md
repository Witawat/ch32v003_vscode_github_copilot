# SimpleOPAMP — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `SimpleOPAMP.h / SimpleOPAMP.c`

---

## ภาพรวม

SimpleOPAMP ควบคุม OPA (Operational Amplifier) ในตัวของ CH32V003 OPA นี้เป็น hardware อยู่ในชิป สามารถใช้เป็น voltage follower, non-inverting amplifier, inverting amplifier หรือ comparator ได้โดยไม่ต้องใช้ IC ภายนอก

> OPA ของ CH32V003 เชื่อมต่อผ่าน multiplexer ภายในชิป — ค่าอ่านต้องผ่าน ADC ต่อ

---

## Channel Map

| Channel | Direction | Pin |
|---------|-----------|-----|
| `OPAMP_CHP0` | Positive input | PA2 |
| `OPAMP_CHP1` | Positive input | PC4 |
| `OPAMP_CHN0` | Negative input | PA1 |
| `OPAMP_CHN1` | Negative input | PC4 |

---

## Modes

| Mode | ชื่อ | การใช้งาน |
|------|------|---------|
| `OPAMP_MODE_VOLTAGE_FOLLOWER` | Unity gain buffer | ป้องกัน loading, impedance matching |
| `OPAMP_MODE_NON_INVERTING`    | Non-inverting amp | ขยายสัญญาณ +(Gain ตั้งด้วย R ภายนอก) |
| `OPAMP_MODE_INVERTING`        | Inverting amp     | ขยายสัญญาณ −(Gain ตั้งด้วย R ภายนอก) |
| `OPAMP_MODE_COMPARATOR`       | Comparator        | เปรียบเทียบ 2 แรงดัน |

---

## Gain Presets

```c
OPAMP_GAIN_1X    // 1x  (สำหรับคำนวณ R ภายนอก)
OPAMP_GAIN_2X    // 2x
OPAMP_GAIN_4X    // 4x
OPAMP_GAIN_8X    // 8x
OPAMP_GAIN_16X   // 16x
```

> Gain จริงขึ้นอยู่กับ resistor ภายนอก ค่าเหล่านี้ใช้เป็น reference สำหรับคำนวณ R เท่านั้น

---

## API Reference

### Initialization

#### `void OPAMP_SimpleInit(OPAMP_Mode mode)`

```c
OPAMP_SimpleInit(OPAMP_MODE_VOLTAGE_FOLLOWER);
OPAMP_SimpleInit(OPAMP_MODE_NON_INVERTING);
OPAMP_SimpleInit(OPAMP_MODE_COMPARATOR);
```

---

### Enable / Disable

#### `void OPAMP_Enable(void)`
#### `void OPAMP_Disable(void)`

```c
OPAMP_SimpleInit(OPAMP_MODE_VOLTAGE_FOLLOWER);
OPAMP_Enable();
// ... ทำงาน
OPAMP_Disable();  // ประหยัดพลังงาน
```

---

## วงจรพื้นฐาน

### Voltage Follower (Unity Buffer)

```
Input ──→ CHP0 (PA2) ──→ OPA ──→ Output (ผ่าน ADC)
                           ↑
               CHN0 (PA1) ←─ (feedback loop ภายใน)
```

ใช้สำหรับ: อ่านสัญญาณจาก sensor ที่มี high source impedance เช่น pH probe

### Non-Inverting Amplifier

```
Input → CHP0 (PA2) → OPA → Output
                      |
          R1 ──── PA1 (CHN0)
          |
         GND + R2
```

Gain = 1 + R1/R2

---

## ตัวอย่างการใช้งาน

### ขั้นต้น — Voltage Follower (Buffer)

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    ADC_SimpleInit();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    // PA2 = input, output อ่านจาก ADC หลัง OPA
    OPAMP_SimpleInit(OPAMP_MODE_VOLTAGE_FOLLOWER);
    OPAMP_Enable();

    while (1) {
        uint16_t raw = ADC_Read(ADC_CH_PA2);
        float volt = ADC_ToVoltage(raw, 3.3f);
        USART_Print("V=");
        USART_PrintNum((int32_t)(volt * 1000));
        USART_Print(" mV\r\n");
        Delay_Ms(200);
    }
}
```

### ขั้นกลาง — Comparator

```c
#include "SimpleHAL.h"

// เปรียบเทียบ: PA2 (CHP0) vs PA1 (CHN0)
// ถ้า PA2 > PA1 → output HIGH
// ถ้า PA2 < PA1 → output LOW

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    ADC_SimpleInit();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(PC0, PIN_MODE_OUTPUT);

    OPAMP_SimpleInit(OPAMP_MODE_COMPARATOR);
    OPAMP_Enable();

    while (1) {
        uint16_t vp = ADC_Read(ADC_CH_PA2);  // positive
        uint16_t vn = ADC_Read(ADC_CH_PA1);  // negative (threshold)

        if (vp > vn) {
            digitalWrite(PC0, HIGH);
            USART_Print("HIGH\r\n");
        } else {
            digitalWrite(PC0, LOW);
            USART_Print("LOW\r\n");
        }
        Delay_Ms(100);
    }
}
```

### ขั้นสูง — Non-Inverting Amplifier (Gain=2x)

```c
// วงจร: R1=10k (PA1 → GND), R2=10k (Output → PA1)
// Gain = 1 + R2/R1 = 1 + 10k/10k = 2x
// Input เข้า PA2, อ่าน output ที่ ADC หลัง OPA

#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    ADC_SimpleInit();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    OPAMP_SimpleInit(OPAMP_MODE_NON_INVERTING);
    OPAMP_Enable();

    while (1) {
        uint16_t raw = ADC_Read(ADC_CH_PA2);
        float vin  = ADC_ToVoltage(raw, 3.3f);
        float vout = vin * 2.0f;   // Gain = 2 จาก R1=R2=10k

        USART_Print("Vin=");
        USART_PrintNum((int32_t)(vin * 1000));
        USART_Print("mV Vout_est=");
        USART_PrintNum((int32_t)(vout * 1000));
        USART_Print("mV\r\n");
        Delay_Ms(200);
    }
}
```

---

## ข้อควรระวัง

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| Input voltage เกิน VDD | OPA ทำงานในช่วง 0–VDD | ต่อ voltage divider ถ้า input เกิน 3.3V |
| Gain ไม่ถูกต้อง | R ภายนอกค่าผิด | คำนวณ Gain = 1 + Rf/Rin ให้ถูกต้อง |
| Output oscillate | Op-amp feedback loop ไม่เสถียร | เพิ่ม capacitor 10–100pF ที่ feedback |
| ค่า ADC หลัง OPA ไม่ตรง | Rail-to-rail limitation | OPA อาจไม่ถึง VDD เต็มที่ เหลือ headroom ~100-300mV |
