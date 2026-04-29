# CH32V003 — GitHub Copilot Instructions

> ไฟล์นี้โหลดโดย GitHub Copilot อัตโนมัติสำหรับ project นี้  
> สำหรับรายละเอียดครบถ้วนให้ดูที่ `.vscode/guidelines.md`

---

## Project Context

- **MCU**: CH32V003 (WCH RISC-V RV32EC, 48MHz, 16KB Flash, 2KB RAM, 3.3V)
- **OS**: Windows, **Toolchain**: RISC-V GCC12 (`riscv-wch-elf-`), make-based build
- **Programmer**: WCH-Link via `upload.bat`
- **Flags**: `-march=rv32ecxw -mabi=ilp32e -Os`

---

## ไฟล์ที่ห้ามแก้ไข

`Core/`, `Debug/`, `Peripheral/`, `Startup/`, `Ld/` — ห้ามแก้ไขเด็ดขาด  
เขียนโค้ดใหม่ทั้งหมดใน `User/` เท่านั้น

---

## GPIO Pin Numbering (enum `GPIO_Pin` ใน SimpleGPIO.h)

```
PA1=0, PA2=1
PC0=10, PC1=11, PC2=12, PC3=13, PC4=14, PC5=15, PC6=16, PC7=17
PD2=20, PD3=21, PD4=22, PD5=23, PD6=24, PD7=25
```

**PWM channels**: PD2=PWM1_CH1, PA1=PWM1_CH2, PC3=PWM1_CH3, PC4=PWM1_CH4, PD4=PWM2_CH1, PD3=PWM2_CH2, PC0=PWM2_CH3, PD7=PWM2_CH4  
**ADC channels**: PA2=Ch0, PA1=Ch1, PC4=Ch2, PD2=Ch3, PD3=Ch4, PD5=Ch5, PD6=Ch6, PD4=Ch7  
**I2C**: default SCL=PC2, SDA=PC1 | remap SCL=PD0, SDA=PD1  
**USART**: default TX=PD5, RX=PD6 | remap1 TX=PD0/RX=PD1 | remap2 TX=PD6/RX=PD5

---

## SimpleHAL API Quick Reference

```c
// GPIO
pinMode(PC0, PIN_MODE_OUTPUT);
digitalWrite(PC0, HIGH);  digitalToggle(PC0);
uint8_t v = digitalRead(PC1);
attachInterrupt(PC1, cb, FALLING);  // RISING, FALLING, CHANGE
uint16_t a = analogRead(PD5);       // 0-1023 (10-bit)
analogWrite(PC0, 50);               // PWM duty % (uses SimplePWM)

// Timing — ต้องเรียก Timer_Init() ก่อน
Timer_Init();
Delay_Ms(500);  Delay_Us(100);
uint32_t ms = Get_CurrentMs();  uint32_t us = Get_CurrentUs();
Timer_t t;
Start_Timer(&t, 1000, 0);      // 1000ms, repeat=0
if (Is_Timer_Expired(&t)) { }

// USART
USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
USART_Print("text");  USART_PrintNum(42);
if (USART_Available()) { uint8_t c = USART_Read(); }

// I2C — ต้องต่อ pull-up 4.7kΩ
I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
I2C_WriteReg(0x50, reg, val);
uint8_t v = I2C_ReadReg(0x50, reg);

// PWM
PWM_Init(PWM1_CH1, 1000);   // channel, freq_hz
PWM_Start(PWM1_CH1);
PWM_SetDutyCycle(PWM1_CH1, 50);  // 0-100%
PWM_Write(PWM1_CH1, 128);        // 0-255 (Arduino-style)

// ADC
ADC_SimpleInit();
uint16_t val = ADC_Read(ADC_CH_PD5);  // 0-1023

// SPI
SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);
uint8_t rx = SPI_Transfer(0xAA);
```

---

## main() โครงสร้างมาตรฐาน

```c
#include <main.h>   // includes "SimpleHAL/SimpleHAL.h"

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();

#if (SDI_PRINT == SDI_PR_OPEN && DISABLE_PRINTF == PRINTF_ON)
    SDI_Printf_Enable();
#endif

    Timer_Init();
    // I2C_SimpleInit / USART_SimpleInit / etc.

    Delay_Ms(100);
    printf("SystemClk:%d\r\n", SystemCoreClock);

    while(1) {
        // main loop
    }
}
```

---

## กฎการเขียนโค้ด

1. **Naming**: ตัวแปร `snake_case` | constants `UPPER_SNAKE_CASE` | functions `Module_ActionName` | types `PascalCase_t`
2. **ISR**: สั้น เร็ว — แค่ set flag, `__disable_irq()` / `__enable_irq()` รอบ timing-critical code
3. **ห้ามใน ISR**: `printf`, `Delay_Ms`, `I2C_Write`, `USART_Print`
4. **volatile**: ตัวแปรที่แก้ใน ISR ต้องประกาศ `volatile`
5. **ห้าม busy-wait ไม่มีกำหนด**: ใช้ timeout กับ `Get_CurrentMs()` หรือ counter
6. **เปิด Clock**: `RCC_APB2PeriphClockCmd(...)` ก่อนใช้ peripheral เสมอ
7. **printf**: ปิดด้วย `DISABLE_PRINTF PRINTF_OFF` ใน production เพื่อประหยัด Flash

---

## การเขียน Lib ใหม่ (ดูรายละเอียดใน .vscode/guidelines.md#13)

**ไฟล์ที่ต้องสร้างเสมอ**: `User/Lib/<Name>/<Name>.h`, `<Name>.c`, `README.md`

**Header template สำคัญ**:
```c
/**
 * @file <Name>.h
 * @brief <คำอธิบาย> Library สำหรับ CH32V003
 * @version 1.0
 * @date YYYY-MM-DD
 * @author CH32V003 Library Team
 * @copyright MIT License
 */
#ifndef __<NAME>_H
#define __<NAME>_H
#ifdef __cplusplus
extern "C" {
#endif
#include "../../SimpleHAL/SimpleHAL.h"
// ...
#ifdef __cplusplus
}
#endif
#endif
```

**Struct ต้องมี**: `initialized` flag ทุก struct  
**Function ต้องมี**: null check + initialized check ทุก public function  
**Private**: ตัวแปรและฟังก์ชัน internal ทั้งหมดต้อง `static`  
**Config macros**: ใช้ `#ifndef` เสมอเพื่อให้ override ได้  
**Error enum**: `LIBNAME_OK = 0` เสมอ

---

## README ของ Lib (ดูรายละเอียดใน .vscode/guidelines.md#14)

- เขียน**ภาษาไทยเป็นหลัก** ทุกส่วน (ยกเว้น code)
- บังคับมี: ASCII วงจร, timing diagram (ถ้ามี protocol), complete code example
- หัวข้อมาตรฐาน: ภาพรวม → คุณสมบัติ → Hardware Setup → การใช้งานพื้นฐาน → ขั้นสูง → Troubleshooting → API Reference
- Troubleshooting ใช้ตาราง "สาเหตุ | วิธีแก้"
- API Reference: ทุก function มีตาราง parameter

---

## Resource Conflicts

| TIM1 | PWM PA1/PC3/PC4/PD2 — ห้ามใช้ SimpleTIM TIM_1 ร่วมด้วย |
| TIM2 | PWM PC0/PD3/PD4/PD7 — ห้ามใช้ SimpleTIM TIM_2 ร่วมด้วย |
| SysTick | SimpleDelay — ห้ามตั้ง SysTick เอง |
| I2C1 | SimpleI2C — ห้ามใช้ SimpleI2C_Soft บน pin เดียวกัน |

---

## Libs ที่พร้อมใช้แล้ว

SimpleHAL: GPIO, TIM, PWM, USART, I2C, I2C_Soft, SPI, ADC, Delay, Flash, IWDG, WWDG, DMA, 1Wire, OPAMP, PWR  
Lib: Buzzer, DS18B20, I2CScan, IR, LCD1602_I2C, MAX7219, NeoPixel, NTC10K, OLED, PIR, RotaryEncoder, TJC, TM1637, WS2815Matrix, **DHT, HCSR04, Servo, Button**
