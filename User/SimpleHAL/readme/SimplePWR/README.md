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

## Entry Methods

```c
PWR_ENTRY_WFI   // Wait For Interrupt
PWR_ENTRY_WFE   // Wait For Event
```

---

## API Reference

### Sleep Mode

#### `void PWR_Sleep(void)`

CPU หยุด — peripherals (DMA, TIM, ADC, USART) ทำงานต่อ  
ตื่นเมื่อ **มี interrupt ใดก็ได้**

```c
PWR_Sleep();
// โปรแกรมดำเนินต่อจากบรรทัดนี้หลัง wakeup
```

---

### Standby Mode

#### `void PWR_Standby(uint32_t timeout_ms)`

เข้า Standby และตั้ง AWU timer — ตื่นหลัง `timeout_ms` → system resets  
RAM และ register ทุกตัว**หายหมด** หลัง wakeup

```c
PWR_Standby(5000);   // นอน 5 วินาที แล้ว reset
```

#### `void PWR_StandbyUntilInterrupt(void)`

เข้า Standby และรอ EXTI interrupt (ปุ่มกด, sensor)

```c
PWR_StandbyUntilInterrupt();
// หลัง wakeup → MCU reset → main() รันใหม่ตั้งแต่ต้น
```

---

### PVD (Power Voltage Detector)

ตรวจสอบแรงดัน VDD ว่าต่ำกว่า threshold หรือไม่

```c
typedef enum {
    PWR_PVD_2V9,
    PWR_PVD_3V1,
    PWR_PVD_3V3,
    PWR_PVD_3V5,
    PWR_PVD_3V7,
    PWR_PVD_3V9,
    PWR_PVD_4V1,
    PWR_PVD_4V4
} PWR_PVD_Level;
```

---

### AWU (Auto Wake-Up)

```c
// AWU ใช้ LSI 128kHz
// Prescaler options: PWR_AWU_PRESCALER_1 ถึง PWR_AWU_PRESCALER_61440
// max window: 0x3F (6-bit)
```

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
