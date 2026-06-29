# SimplePWR — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `SimplePWR.h / SimplePWR.c`

---

## ภาพรวม

SimplePWR ควบคุม power management ของ CH32V003 รองรับ **Sleep mode** (CPU หยุด peripherals ทำงานต่อ) และ **Standby mode** (กินไฟต่ำสุด RAM หาย wakeup = system reset) เหมาะสำหรับ battery-powered device

---

## Power Modes

| Mode | กินไฟ | CPU | RAM | Wake up | หมายเหตุ |
|------|:-----:|:---:|:---:|---------|---------|
| Normal | ~5-10mA | ทำงาน | ✅ | — | ทำงานปกติ |
| Sleep | ~1-2mA | หยุด | ✅ | Any interrupt | เร็ว, RAM ยังอยู่ |
| Standby | ~2-5µA | หยุด | ❌ | AWU / EXTI / NRST | ประหยัดสูงสุด, RAM หาย |

---

## Wake-up Sources

```c
PWR_WAKEUP_INTERRUPT  // EXTI interrupt
PWR_WAKEUP_AWU        // Auto Wake-Up Unit (timer)
PWR_WAKEUP_RESET      // NRST pin
```

> **⚠️ หมายเหตุ:** `PWR_EnableWakeupPin()` / `PWR_DisableWakeupPin()` ใช้ WKUP pin = PA0 แต่ CH32V003 **ทุกแพ็กเกจไม่มีขา PA0** (ไม่ได้ bond ออกมาจาก die) — ใช้ AWU หรือ EXTI interrupt แทน

#### `void PWR_EnableWakeupPin(void)`
#### `void PWR_DisableWakeupPin(void)`

เปิด/ปิด WKUP pin (PA0) สำหรับ wakeup จาก Standby

> ⚠️ ไม่มีผลบน CH32V003 (ไม่มีขา PA0) — ฟังก์ชันนี้เก็บไว้เพื่อความสมบูรณ์ของ API เท่านั้น

## Entry Methods

```c
PWR_ENTRY_WFI   // Wait For Interrupt
PWR_ENTRY_WFE   // Wait For Event
```

---

## API Reference

### Sleep Mode

#### `void PWR_Sleep(void)`

CPU หยุด — peripherals ทำงานต่อ ตื่นเมื่อมี interrupt

```c
PWR_Sleep();
// ทำงานต่อจากบรรทัดนี้หลัง wakeup
```

#### `void PWR_EnterSleepMode(uint8_t entry_method)`

เลือก WFI/WFE

```c
PWR_EnterSleepMode(PWR_ENTRY_WFI);  // Wait For Interrupt
PWR_EnterSleepMode(PWR_ENTRY_WFE);  // Wait For Event
```

---

### Standby Mode

#### `void PWR_Standby(uint32_t timeout_ms)`

เข้า Standby + AWU timer — ตื่นหลัง timeout → system reset

```c
PWR_Standby(5000);   // นอน 5 วิ → reset
```

#### `void PWR_StandbyUntilInterrupt(void)`

เข้า Standby รอ EXTI — หลัง wakeup → reset

```c
PWR_StandbyUntilInterrupt();
```

#### `void PWR_EnterStandbyMode(uint8_t entry_method)`

เลือก WFI/WFE สำหรับ Standby

```c
PWR_ConfigureAWU(PWR_AWU_PRESCALER_1024, 31);  // ~1s
PWR_EnterStandbyMode(PWR_ENTRY_WFI);
```

#### `uint8_t PWR_WasStandbyWakeup(void)`

ตรวจสอบว่า wakeup มาจาก Standby หรือไม่ — คืน 1 ถ้าใช่

```c
if (PWR_WasStandbyWakeup()) { /* ตื่นจาก standby */ }
```

#### `void PWR_ClearStandbyFlag(void)`

เคลียร์ standby wakeup flag

---

### AWU (Auto Wake-Up)

#### `void PWR_ConfigureAWU(uint32_t prescaler, uint8_t window)`

ตั้งค่า AWU แบบ manual

```c
// ~500ms: prescaler=1024 (~8ms/tick), window=63
PWR_ConfigureAWU(PWR_AWU_PRESCALER_1024, PWR_AWU_CALC_WINDOW(1024, 500));
```

#### `uint32_t PWR_GetAWUTimeout(uint32_t prescaler, uint8_t window)`

คำนวณ timeout จริง (ms)

#### AWU Timeout Reference

| Prescaler | ~Time/Count | Max (window=63) |
|:---:|------|------|
| 1 | ~7.8 us | ~0.49 ms |
| 1024 | ~8 ms | ~504 ms |
| 4096 | ~32 ms | ~2.0 s |
| 10240 | ~80 ms | ~5.0 s |

ตัวเต็มทั้งหมด: `PWR_AWU_PRESCALER_1` ถึง `_61440` (15 ค่า)

#### Helper Macros

```c
PWR_AWU_CALC_WINDOW(prescaler, timeout_ms)  // คำนวณ window
PWR_AWU_TIMEOUT_MS(prescaler, window)       // คำนวณ ms
PWR_AWU_MAX_WINDOW                           // = 0x3F (63)
```

---

### PVD (Power Voltage Detector)

#### `void PWR_EnablePVD(uint32_t voltage_level)`

เปิด PVD — ตรวจจับแรงดันตก

```c
PWR_EnablePVD(PWR_PVD_3V3);  // trigger เมื่อ VDD < 3.3V
```

#### `void PWR_DisablePVD(void)`

ปิด PVD

#### `uint8_t PWR_GetPVDStatus(void)`

ตรวจสถานะ — คืน 1 ถ้า VDD ต่ำกว่า threshold

```c
if (PWR_GetPVDStatus()) { /* แบตต่ำ! */ }
```

**PVD Levels:** `PWR_PVD_2V9`, `_3V1`, `_3V3`, `_3V5`, `_3V7`, `_3V9`, `_4V1`, `_4V4`

---

### Battery & Power Estimation

#### `uint32_t PWR_EstimateStandbyCurrent(pvd, awu)`
ประมาณกระแสตอน standby (µA)

```c
uint32_t cur = PWR_EstimateStandbyCurrent(0, 1);  // PVD off, AWU on → ~5µA
```

#### `uint32_t PWR_CalculateBatteryLife(mAh, active%, active_mA, standby_uA)`
คำนวณอายุแบตเตอรี่ (ชั่วโมง)

```c
// 1000mAh, 1% active @20mA, 99% standby @5uA
uint32_t hours = PWR_CalculateBatteryLife(1000, 1, 20, 5);
// → ~4878 hours (~203 days)
```

### Power Consumption Reference

| Mode | Typical |
|------|--------|
| Active (Run) | ~3-5 mA |
| Sleep | ~1-2 mA |
| Standby (no AWU) | ~2 µA |
| Standby (AWU on) | ~5 µA |

---

## ตัวอย่างการใช้งาน

### ขั้นต้น — Sleep เมื่อไม่มีงาน

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    // เปิด TIM interrupt เพื่อ wakeup ทุก 1 วินาที
    TIM_SimpleInit(TIM_1, 1);    // 1Hz
    TIM_AttachInterrupt(TIM_1, NULL);  // interrupt เปล่า (แค่ wakeup)
    TIM_Start(TIM_1);

    while (1) {
        USART_Print("Awake, doing work...\r\n");
        USART_Flush();

        // ทำงาน
        ADC_SimpleInit();
        uint16_t raw = ADC_Read(ADC_CH_PA2);
        USART_PrintNum(raw); USART_Print("\r\n");
        USART_Flush();

        // นอน (CPU หยุด ประหยัดไฟ จนกว่า TIM1 interrupt จะมา)
        PWR_Sleep();
        // ตื่นแล้ว → วนซ้ำ
    }
}
```

### ขั้นกลาง — Standby + AWU (deep sleep ประหยัดไฟสูงสุด)

```c
#include "SimpleHAL.h"

// ใช้ Flash เก็บข้อมูลก่อน Standby เพราะ RAM หายหลัง wakeup
typedef struct {
    uint16_t sample_count;
    uint16_t last_adc;
} PersistData_t;

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    Flash_Init();
    ADC_SimpleInit();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    // โหลด state จาก Flash
    PersistData_t data;
    if (Flash_LoadConfig(&data, sizeof(data)) != FLASH_OK) {
        data.sample_count = 0;
        data.last_adc = 0;
    }

    // ทำงาน: อ่าน ADC
    data.last_adc = ADC_Read(ADC_CH_PA2);
    data.sample_count++;

    USART_Print("Sample #"); USART_PrintNum(data.sample_count);
    USART_Print(" ADC="); USART_PrintNum(data.last_adc);
    USART_Print("\r\n");
    USART_Flush();

    // บันทึกก่อน sleep
    Flash_SaveConfig(&data, sizeof(data));

    // นอน 10 วินาที แล้ว system reset (ตื่น → main() รันใหม่)
    PWR_Standby(10000);
    // ไม่มีโค้ดหลังนี้ (MCU reset)
}
```

### ขั้นสูง — Smart Sleep (Sleep ถ้าไม่มี event, Standby ถ้านานเกินไป)

```c
#include "SimpleHAL.h"

#define IDLE_TIMEOUT_MS    30000   // 30 วินาที ไม่มี event → Standby
#define SLEEP_INTERVAL_MS  1000    // ตรวจทุก 1 วินาที

volatile uint8_t event_flag = 0;   // เซ็ตจาก EXTI / USART

void button_isr(void) {
    event_flag = 1;
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    Flash_Init();

    pinMode(PD2, PIN_MODE_INPUT_PULLUP);
    attachInterrupt(PD2, button_isr, INTERRUPT_FALLING);

    TIM_SimpleInit(TIM_1, 1);   // wakeup sleep ทุก 1s
    TIM_AttachInterrupt(TIM_1, NULL);
    TIM_Start(TIM_1);

    uint32_t last_event_ms = Get_CurrentMs();

    while (1) {
        if (event_flag) {
            event_flag = 0;
            last_event_ms = Get_CurrentMs();
            USART_Print("Event!\r\n");
            USART_Flush();
            // ประมวลผล event...
        }

        uint32_t idle_ms = Get_CurrentMs() - last_event_ms;

        if (idle_ms >= IDLE_TIMEOUT_MS) {
            // นานเกินไป → Standby (ประหยัดสูงสุด)
            USART_Print("Entering Standby...\r\n");
            USART_Flush();
            PWR_StandbyUntilInterrupt();  // ตื่นเมื่อกดปุ่ม PD2
        } else {
            // นอนสั้นๆ รอ interrupt
            PWR_Sleep();
        }
    }
}
```

---

## ข้อควรระวัง

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| ตื่นจาก Standby แล้วค้าง | ไม่เคลียร์ Wakeup flag | SimplePWR จัดการให้ แต่ตรวจสอบ PWR->CSR ด้วย |
| RAM หายหลัง Standby | Standby ออกแบบมาแบบนั้น | ใช้ SimpleFlash เก็บ state ก่อน standby |
| Sleep ไม่ประหยัดไฟ | Peripheral ยังทำงาน | ปิด ADC, TIM ที่ไม่ใช้ก่อน sleep |
| AWU timeout ไม่แม่นยำ | LSI ±25% variation | เผื่อ margin หรือ trim ด้วย HSI calibration |
| `USART_Print` ไม่ออก | ส่งข้อมูลค้างใน buffer | เรียก `USART_Flush()` ก่อนเข้า sleep เสมอ |
