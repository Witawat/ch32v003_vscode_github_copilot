# SimpleDMA — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `SimpleDMA.h / SimpleDMA.c`

---

## ภาพรวม

SimpleDMA ให้ใช้งาน DMA (Direct Memory Access) controller ของ CH32V003 โดยไม่ต้องจัดการ register โดยตรง รองรับ memory-to-memory copy (เร็วกว่า memcpy), peripheral-to-memory (เช่น ADC → RAM) และ memory-to-peripheral (เช่น RAM → USART)

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
| 7 | `DMA_CH7` | USART1 TX / TIM2 CH2 |

---

## Enums

### Direction

```c
DMA_DIR_PERIPH_TO_MEM   // ADC, USART, SPI → RAM
DMA_DIR_MEM_TO_PERIPH   // RAM → USART, SPI
DMA_DIR_MEM_TO_MEM      // RAM → RAM (memcpy)
```

### Priority

```c
DMA_PRIORITY_LOW
DMA_PRIORITY_MEDIUM
DMA_PRIORITY_HIGH
DMA_PRIORITY_VERY_HIGH
```

### Data Size

```c
DMA_SIZE_BYTE       // 8-bit
DMA_SIZE_HALFWORD   // 16-bit
DMA_SIZE_WORD       // 32-bit
```

### Mode

```c
DMA_MODE_NORMAL     // ทำครั้งเดียวแล้วหยุด
DMA_MODE_CIRCULAR   // วนซ้ำต่อเนื่อง
```

---

## API Reference

### Simple Copy

#### `void DMA_MemCopy(void* dst, const void* src, uint32_t len)`

คัดลอกข้อมูล M→M แบบรวดเร็ว (เทียบเท่า fast memcpy)

```c
uint8_t src[64] = { /* ... */ };
uint8_t dst[64];
DMA_MemCopy(dst, src, 64);  // copy 64 bytes
```

---

### Configuration

#### `DMA_Config_t` struct

```c
typedef struct {
    uint8_t  channel;          // DMA_CH1 - DMA_CH7
    uint8_t  direction;        // DMA_DIR_xxx
    uint8_t  priority;         // DMA_PRIORITY_xxx
    uint8_t  data_size;        // DMA_SIZE_xxx
    uint8_t  mode;             // DMA_MODE_NORMAL / CIRCULAR
    uint8_t  mem_increment;    // 1 = เลื่อน pointer ไปข้างหน้า
    uint8_t  periph_increment; // 1 = เลื่อน peripheral address
} DMA_Config_t;
```

---

### Callbacks (Weak)

Override ใน user code เพื่อรับแจ้งเมื่อ transfer เสร็จ

```c
void DMA_TransferCompleteCallback(uint8_t channel) {
    // เรียกเมื่อ DMA เสร็จ
}

void DMA_ErrorCallback(uint8_t channel) {
    // เรียกเมื่อเกิด error
}
```

---

### Peripheral Helpers

```c
DMA_PERIPH_ADC1
DMA_PERIPH_USART1
DMA_PERIPH_SPI1
DMA_PERIPH_I2C1
DMA_PERIPH_TIM1
DMA_PERIPH_TIM2
```

---

## ตัวอย่างการใช้งาน

### ขั้นต้น — DMA MemCopy (เร็วกว่า memcpy)

```c
#include "SimpleHAL.h"
#include <string.h>

uint8_t src_buf[128];
uint8_t dst_buf[128];

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    // เติม src
    for (uint16_t i = 0; i < 128; i++) src_buf[i] = (uint8_t)i;

    // DMA copy
    DMA_MemCopy(dst_buf, src_buf, 128);

    // ตรวจสอบ
    uint8_t ok = (memcmp(src_buf, dst_buf, 128) == 0);
    USART_Print(ok ? "Copy OK\r\n" : "Copy FAIL\r\n");
}
```

### ขั้นกลาง — DMA USART TX (ส่ง string ไม่บล็อก CPU)

```c
#include "SimpleHAL.h"

volatile uint8_t dma_tx_done = 1;

void DMA_TransferCompleteCallback(uint8_t channel) {
    if (channel == DMA_CH2) {
        dma_tx_done = 1;
    }
}

// ส่งข้อมูลผ่าน USART DMA (non-blocking)
void usart_dma_send(uint8_t* data, uint16_t len) {
    // รอจน TX เสร็จก่อน
    while (!dma_tx_done);
    dma_tx_done = 0;

    DMA_Config_t cfg = {
        .channel          = DMA_CH2,
        .direction        = DMA_DIR_MEM_TO_PERIPH,
        .priority         = DMA_PRIORITY_MEDIUM,
        .data_size        = DMA_SIZE_BYTE,
        .mode             = DMA_MODE_NORMAL,
        .mem_increment    = 1,
        .periph_increment = 0
    };

    // ตั้งค่า DMA ส่งไปยัง USART1->DATAR
    // (implementation depends on HAL internals)
    (void)cfg; (void)data; (void)len;
    // ดู ch32v00x_dma.c สำหรับ DMA_Init full API
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    uint8_t msg[] = "Hello via DMA!\r\n";
    usart_dma_send(msg, sizeof(msg) - 1);
    Delay_Ms(1000);
}
```

### ขั้นสูง — DMA ADC Circular (continuous sampling)

```c
#include "SimpleHAL.h"

#define ADC_BUF_SIZE  16
volatile uint16_t adc_buf[ADC_BUF_SIZE];
volatile uint8_t  adc_ready = 0;

void DMA_TransferCompleteCallback(uint8_t channel) {
    if (channel == DMA_CH1) {
        adc_ready = 1;
    }
}

// คำนวณค่าเฉลี่ยจาก circular buffer
uint16_t adc_average_dma(void) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < ADC_BUF_SIZE; i++) {
        sum += adc_buf[i];
    }
    return (uint16_t)(sum / ADC_BUF_SIZE);
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    // ADC_SimpleInit() + DMA CH1 circular setup
    // (ต้องตั้งค่า ADC ให้ใช้ DMA ผ่าน ADC1->CTLR2 |= ADC_DMAReq_Enable)
    ADC_SimpleInit();

    USART_Print("DMA ADC running...\r\n");
    while (1) {
        if (adc_ready) {
            adc_ready = 0;
            uint16_t avg = adc_average_dma();
            USART_Print("avg="); USART_PrintNum(avg); USART_Print("\r\n");
        }
    }
}
```

---

## ข้อควรระวัง

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| DMA conflict | 2 peripheral ต้องการ channel เดียวกัน | ดู channel map, เลือก peripheral ที่ไม่ชน |
| ข้อมูลเสีย | buffer ถูก overwrite ก่อนอ่าน | ใช้ double buffer หรือรอ `TransferComplete` |
| DMA ไม่เริ่ม | ลืม enable DMA clock | SimpleDMA จัดการให้ใน init แล้ว |
| `DMA_MemCopy` ยังไม่เสร็จ | ใช้ผลลัพธ์เร็วเกินไป | รอ `TransferCompleteCallback` ก่อน |
| `CIRCULAR` mode ไม่หยุด | ออกแบบมาให้วนตลอด | `DMA_MODE_NORMAL` ถ้าต้องการครั้งเดียว |
