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

#### `void DMA_SimpleInit(DMA_Config_t* config)`

เริ่มต้น DMA channel — เปิด RCC clock + ตั้งค่าตาม config

| พารามิเตอร์ | ชนิด | คำอธิบาย |
|------------|------|----------|
| `config` | `DMA_Config_t*` | โครงสร้างตั้งค่า: channel, direction, priority, data_size, mode, addresses, buffer_size |

```c
DMA_Config_t cfg = {
    .channel = DMA_CH1, .direction = DMA_DIR_MEM_TO_MEM,
    .data_size = DMA_SIZE_BYTE, .mode = DMA_MODE_NORMAL,
    .mem_increment = 1, .periph_increment = 1,
    .periph_addr = (uint32_t)src, .mem_addr = (uint32_t)dst,
    .buffer_size = 100
};
DMA_SimpleInit(&cfg);
```

#### `void DMA_Start(DMA_Channel channel)` / `void DMA_Stop(DMA_Channel channel)`

เริ่ม/หยุด transfer

#### `DMA_Status DMA_GetStatus(DMA_Channel channel)`

| **คืนค่า** | `DMA_Status` — `IDLE` (0), `BUSY` (1), `COMPLETE` (2), `ERROR` (3) |

#### `uint8_t DMA_WaitComplete(DMA_Channel channel, uint32_t timeout_ms)`

รอ transfer เสร็จ (blocking) พร้อม timeout

| **คืนค่า** | `1` = เสร็จ, `0` = timeout |
|---|---|

```c
DMA_Start(DMA_CH1);
if (!DMA_WaitComplete(DMA_CH1, 1000)) {
    USART_Print("DMA timeout!\r\n");
}
```

#### `void DMA_Reset(DMA_Channel channel)`

รีเซ็ต DMA channel — หยุด transfer ทั้งหมด, เคลียร์ flags

#### `uint16_t DMA_GetRemainingCount(DMA_Channel channel)`

จำนวนข้อมูลที่ยังไม่ได้ถ่ายโอน

#### `void DMA_EnableInterrupt(DMA_Channel channel, uint8_t enable)`

เปิด/ปิด DMA interrupt — `1` = เปิด, `0` = ปิด

#### `DMA_Channel_TypeDef* DMA_GetChannelBase(DMA_Channel channel)`

แปลง enum `DMA_Channel` → pointer ไปยัง `DMA_Channel_TypeDef` register

#### `IRQn_Type DMA_GetChannelIRQn(DMA_Channel channel)`

แปลง `DMA_Channel` → `IRQn` สำหรับ `NVIC_Init()`

### Callbacks

#### `void DMA_SetTransferCompleteCallback(DMA_Channel ch, DMA_TransferCompleteCallback cb)`

ตั้ง callback เมื่อ transfer เสร็จ — `cb(DMA_Channel ch)` ถูกเรียกจาก DMA ISR

| ⚠️ | callback ทำงานใน interrupt context — ห้ามใช้ `Delay_Ms`, `USART_Print` |
|---|---|

```c
void on_complete(DMA_Channel ch) { dma_done = 1; }
DMA_SetTransferCompleteCallback(DMA_CH1, on_complete);
```

#### `void DMA_SetErrorCallback(DMA_Channel ch, DMA_ErrorCallback cb)`

ตั้ง callback เมื่อ DMA error

#### `void DMA_SetHalfTransferCallback(DMA_Channel ch, DMA_HalfTransferCallback cb)`

ตั้ง callback เมื่อ transfer ถึงครึ่งทาง — ใช้กับ ping-pong buffer

### Memory Transfer

#### `void DMA_MemCopy(void* dst, const void* src, uint16_t size)`

คัดลอกหน่วยความจำแบบ blocking (รอให้เสร็จก่อน return)

```c
DMA_MemCopy(dst_buf, src_buf, 128);
```

#### `void DMA_MemCopyAsync(DMA_Channel ch, void* dst, const void* src, uint16_t size)`

คัดลอกแบบ non-blocking — ใช้ `DMA_GetStatus()` หรือ callback ตรวจสอบ

#### `void DMA_MemSet(void* dst, uint8_t value, uint16_t size)`

กำหนดค่าหน่วยความจำเป็นค่าเดียว (เหมือน memset)

```c
DMA_MemSet(buf, 0, 256);  // เคลียร์ 256 bytes เป็น 0
```

### ADC Integration

#### `void DMA_ADC_Init(DMA_Channel ch, uint16_t* buf, uint16_t size, uint8_t circular)`

ตั้งค่า DMA สำหรับ ADC continuous conversion

| ⚠️ | ต้องเรียก `ADC_SimpleInit()` ก่อน |
|---|---|
| `circular` | `1` = วนซ้ำ (ring buffer), `0` = ครั้งเดียว |

#### `void DMA_ADC_InitMultiChannel(DMA_Channel ch, uint16_t* buf, uint8_t n_ch, uint16_t samples)`

ADC multi-channel — เก็บ n_ch × samples ค่าใน buf

#### `void DMA_analogReadStart(uint8_t pin, uint16_t* buf, uint16_t size, uint8_t continuous)`

เริ่ม ADC DMA แบบง่าย — ระบุ GPIO pin ได้เลย

| `pin` | ต้องเป็น pin ที่รองรับ ADC (ใช้ `IS_ADC_PIN` ตรวจสอบ) |
|---|---|

```c
uint16_t buf[100];
DMA_analogReadStart(PD2, buf, 100, 1);  // circular, PD2
```

#### `uint16_t DMA_analogReadAverage(uint16_t* buf, uint16_t size)`

อ่านค่าเฉลี่ยจาก DMA buffer

#### `void DMA_analogReadStop(void)` / `uint8_t DMA_analogReadBusy(void)`

หยุด/ตรวจสอบสถานะ DMA ADC

#### `void DMA_SetAnalogReadChannel(DMA_Channel ch)`

เปลี่ยน DMA channel สำหรับ `DMA_analogReadStart()` — default `DMA_CH1`

### USART Integration

#### `void DMA_USART_InitTx(DMA_Channel ch, uint8_t* buf, uint16_t size)`

ตั้งค่า DMA สำหรับ USART transmit

| ⚠️ | ต้องเรียก `USART_SimpleInit()` ก่อน |
|---|---|

#### `void DMA_USART_InitRx(DMA_Channel ch, uint8_t* buf, uint16_t size, uint8_t circular)`

ตั้งค่า DMA สำหรับ USART receive — แนะนำ circular mode สำหรับ RX

#### `void DMA_USART_Transmit(DMA_Channel ch, const uint8_t* data, uint16_t len)`

ส่งข้อมูลผ่าน USART DMA — non-blocking

#### `uint16_t DMA_USART_GetReceivedCount(DMA_Channel ch, uint16_t buf_size)`

จำนวนข้อมูลที่ DMA USART รับได้

#### `void DMA_USART_Send(DMA_Channel ch, const uint8_t* data, uint16_t len)`

ส่งข้อมูลแบบ one-shot blocking — init + transmit + wait ครบในฟังก์ชันเดียว

```c
uint8_t msg[] = "Hello DMA!\r\n";
DMA_USART_Send(DMA_CH2, msg, sizeof(msg)-1);  // บล็อกจนส่งเสร็จ
```

### SPI Integration

#### `void DMA_SPI_Init(DMA_Channel tx_ch, DMA_Channel rx_ch)`

ตั้งค่า DMA TX+RX สำหรับ SPI

| ⚠️ | ต้องเรียก `SPI_SimpleInit()` ก่อน |
|---|---|

#### `void DMA_SPI_TransferBuffer(DMA_Channel tx_ch, DMA_Channel rx_ch, const uint8_t* tx, uint8_t* rx, uint16_t len)`

ส่ง+รับข้อมูล SPI ผ่าน DMA

### I2C Integration

#### `void DMA_I2C_InitTx(DMA_Channel ch)` / `void DMA_I2C_InitRx(DMA_Channel ch)`

ตั้งค่า DMA สำหรับ I2C transmit/receive — **ต้องใช้ DMA_CH4 (TX), DMA_CH5 (RX)** เท่านั้น

| ⚠️ | ต้องเรียก `I2C_SimpleInit()` ก่อน |
|---|---|

#### `void DMA_I2C_Transfer(DMA_Channel tx_ch, DMA_Channel rx_ch, const uint8_t* tx, uint8_t* rx, uint16_t len)`

ส่ง+รับ I2C ผ่าน DMA

| ⚠️ | ต้องเรียก `I2C_WriteReg()` ส่ง slave+register address ก่อน ถึงจะใช้ DMA transfer ได้ |
|---|---|

### TIM Integration

#### `void DMA_TIM_InitCapture(DMA_Channel ch, TIM_TypeDef* TIMx, uint16_t* buf, uint16_t len, uint32_t ccx_addr)`

จับค่า TIM capture → buffer ด้วย DMA

| ⚠️ | ต้องตั้งค่า TIM input capture ก่อน |
|---|---|

| พารามิเตอร์ | ชนิด | คำอธิบาย |
|------------|------|----------|
| `ccx_addr` | `uint32_t` | Address ของ CCR register เช่น `(uint32_t)&TIM1->CH4CVR` |

#### `void DMA_TIM_UpdatePWM(DMA_Channel ch, TIM_TypeDef* TIMx, uint32_t ccr_addr, const uint16_t* waveform, uint16_t len, uint8_t circular)`

อัปเดต PWM duty cycle อัตโนมัติจาก lookup table — ใช้สร้าง sine wave, sawtooth

| ⚠️ | ต้องเรียก `TIM_DMACmd(TIMx, TIM_DMA_Update, ENABLE)` ก่อน |
|---|---|

```c
// สร้าง sine wave 64 samples ที่ PWM output
uint16_t sine_table[64];  // คำนวณล่วงหน้า
DMA_TIM_UpdatePWM(DMA_CH5, TIM1, DMA_TIM_GetCCRAddress(TIM1, 1),
                  sine_table, 64, 1);  // circular
TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);
DMA_Start(DMA_CH5);
```

#### `uint32_t DMA_TIM_GetCCRAddress(TIM_TypeDef* TIMx, uint8_t channel)`

คืนค่า address ของ TIMx CCR register สำหรับ PWM channel 1-4

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
