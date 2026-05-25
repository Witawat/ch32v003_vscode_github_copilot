# SimpleDMA — คู่มือการใช้งาน

> **Version:** 1.1 | **MCU:** CH32V003 | **File:** `SimpleDMA.h / SimpleDMA.c`

---

## ภาพรวม

SimpleDMA ให้ใช้งาน DMA (Direct Memory Access) controller ของ CH32V003 โดยไม่ต้องจัดการ register โดยตรง รองรับ memory-to-memory copy, peripheral-to-memory, memory-to-peripheral, และ helper functions สำหรับ USART, I2C, TIM, ADC

---

## DMA Channels

CH32V003 มี DMA 7 channels:

| Channel | Enum | Peripheral ที่ fixed-map |
|:-------:|------|--------------------------|
| 1 | `DMA_CH1` | ADC1 |
| 2 | `DMA_CH2` | SPI1 RX / USART1 TX |
| 3 | `DMA_CH3` | SPI1 TX / USART1 RX |
| 4 | `DMA_CH4` | I2C1 TX / TIM1 CH4 |
| 5 | `DMA_CH5` | I2C1 RX / TIM1 Update |
| 6 | `DMA_CH6` | USART1 RX / TIM1 CH3 |
| 7 | `DMA_CH7` | USART1 TX / TIM2 CH2 / TIM1 CH2 |

---

## API Reference

### Core Functions

| ฟังก์ชั่น | คำอธิบาย |
|-----------|----------|
| `DMA_Init(ch, periph, mem, size, dir, mode)` | กำหนดค่า DMA channel เต็มรูปแบบ |
| `DMA_Start(ch)` | เริ่ม transfer |
| `DMA_Stop(ch)` | หยุด transfer |
| `DMA_SetCallback(ch, cb)` | ตั้ง callback เมื่อ transfer เสร็จ |
| `DMA_SetHalfTransferCallback(ch, cb)` | ตั้ง callback เมื่อ transfer ถึงครึ่งทาง |
| `DMA_WaitComplete(ch, timeout_ms)` | รอ transfer เสร็จ (มี timeout ป้องกันค้าง) |
| `DMA_MemCopy(dst, src, size)` | คัดลอกหน่วยความจำแบบบล็อกกิ้ง (เร็วกว่า memcpy) |
| `DMA_SetAnalogReadChannel(adc_ch)` | ระบุ ADC channel สำหรับ DMA-triggered conversion |

### USART Helper

| ฟังก์ชั่น | คำอธิบาย |
|-----------|----------|
| `DMA_USART_Send(data, len)` | ส่งข้อมูลผ่าน USART1 TX แบบ one-shot (non-blocking แต่มี callback รอเสร็จ) |

### I2C Helper

| ฟังก์ชั่น | คำอธิบาย |
|-----------|----------|
| `DMA_I2C_InitTx(data, len)` | ตั้งค่า DMA สำหรับ I2C TX (ใช้ DMA_CH4) |
| `DMA_I2C_InitRx(data, len)` | ตั้งค่า DMA สำหรับ I2C RX (ใช้ DMA_CH5) |
| `DMA_I2C_Transfer(tx_data, rx_data, len)` | ตั้งค่า DMA สำหรับ I2C TX + RX พร้อมกัน |

### TIM Helper

| ฟังก์ชั่น | คำอธิบาย |
|-----------|----------|
| `DMA_TIM_InitCapture(timer, ch)` | เปิดใช้งาน TIM → DMA เมื่อมี Capture/Compare event |
| `DMA_TIM_UpdatePWM(timer, ch, duty_buf)` | ตั้งค่า DMA อัปเดต duty cycle PWM อัตโนมัติจาก buffer |
| `DMA_TIM_GetCCRAddress(timer, pwm_ch)` | คืนค่า address ของ CCR register สำหรับ PWM channel |

---

## ตัวอย่างการใช้งาน

### DMA MemCopy

```c
#include "SimpleHAL.h"
#include <string.h>

uint8_t src_buf[128];
uint8_t dst_buf[128];

int main(void) {
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    for (uint16_t i = 0; i < 128; i++) src_buf[i] = (uint8_t)i;

    DMA_MemCopy(dst_buf, src_buf, 128);

    uint8_t ok = (memcmp(src_buf, dst_buf, 128) == 0);
    USART_Print(ok ? "Copy OK\r\n" : "Copy FAIL\r\n");
}
```

### DMA USART TX (ส่ง string ไม่บล็อก CPU)

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    uint8_t msg[] = "Hello via DMA!\r\n";
    DMA_USART_Send(msg, sizeof(msg) - 1);
    Delay_Ms(1000);
}
```

### DMA I2C Transfer

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);

    uint8_t tx_buf[] = { 0x3C, 0x00, 0x01, 0x02 };
    uint8_t rx_buf[4];

    DMA_I2C_Transfer(tx_buf, rx_buf, 4);
    // rx_buf ได้ข้อมูลแล้ว
}
```

---

## ข้อควรระวัง

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| DMA conflict | 2 peripheral ต้องการ channel เดียวกัน | ดู channel map, เลือก peripheral ที่ไม่ชน |
| ข้อมูลเสีย | buffer ถูก overwrite ก่อนอ่าน | ใช้ double buffer หรือรอ transfer complete |
| DMA ไม่เริ่ม | ลืม enable DMA clock | SimpleDMA จัดการให้ใน init แล้ว |
| `DMA_MemCopy` ยังไม่เสร็จ | ใช้ผลลัพธ์เร็วเกินไป | รอ callback หรือ `DMA_WaitComplete()` ก่อน |
| `CIRCULAR` mode ไม่หยุด | ออกแบบมาให้วนตลอด | ใช้ `DMA_MODE_NORMAL` ถ้าต้องการครั้งเดียว |
| Callback ไม่ทำงาน | ลืม `volatile` หรือไม่ได้ตั้ง NVIC priority | ดูคำแนะนำใน `KNOWLEDGE_BASE.md §8.8–8.10` |
