# 11_DMA — ตัวอย่าง DMA

## ไฟล์ในโฟลเดอร์

| ไฟล์ | คำอธิบาย |
|------|----------|
| `ex01_MemCopy_Sync.c` | Memory copy (DMA_MemCopy, DMA_MemSet) |
| `ex02_MemCopy_Async.c` | Async memory copy + callback |
| `ex03_DMA_USART_Transmit.c` | USART send via DMA |
| `ex04_DMA_SPI_Transfer.c` | SPI transfer via DMA |
| `ex05_DMA_PWM_Waveform.c` | PWM sine wave via DMA (TIM DMA) |
| `ex06_DMA_I2C_Transfer.c` | I2C transfer via DMA (BH1750) |

## DMA Channels

| Channel | ใช้กับ |
|:---:|------|
| CH1 | General / ADC analogRead |
| CH2 | USART TX |
| CH3 | USART RX |
| CH4 | I2C1 TX / SPI TX / TIM1_CH4 |
| CH5 | I2C1 RX / SPI RX / TIM1_Update |
| CH6 | TIM1_CH3 |
| CH7 | TIM2_CH2 / USART TX |

## Quick ADC with DMA

```c
uint16_t buf[100];
DMA_analogReadStart(PD2, buf, 100, 1);  // circular
while (1) {
    uint16_t avg = DMA_analogReadAverage(buf, 100);
    Delay_Ms(100);
}
```
