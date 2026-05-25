# DMA Examples Guide — CH32V003 + SimpleDMA

> **Version:** 1.0 | **MCU:** CH32V003 | **Library:** SimpleDMA (SimpleDMA.h/.c)

---

## สารบัญ

1. [Memory-to-Memory (M2M)](#1-memory-to-memory-m2m)
2. [ADC + DMA](#2-adc--dma)
3. [USART + DMA](#3-usart--dma)
4. [SPI + DMA](#4-spi--dma)
5. [I2C + DMA](#5-i2c--dma)
6. [TIM + DMA](#6-tim--dma)
7. [Advanced Patterns](#7-advanced-patterns)

---

## DMA Channel Reference

ก่อนเริ่มตัวอย่าง ควรรู้ fixed-map DMA channels ของ CH32V003 ก่อน:

| Channel | Peripheral Mapping                   |
|---------|--------------------------------------|
| CH1     | ADC1                                 |
| CH2     | SPI1_RX / USART1_TX                  |
| CH3     | SPI1_TX / USART1_RX                  |
| CH4     | I2C1_TX / TIM1_CH4                   |
| CH5     | I2C1_RX / TIM1_Update                |
| CH6     | USART1_RX / TIM1_CH3                 |
| CH7     | USART1_TX / TIM2_CH2                 |

**ข้อควรรู้:**
- Channel น้อย = priority สูงกว่า (ถ้าตั้ง priority เท่ากัน)
- แต่ละ channel ใช้กับ peripheral ได้ทีละตัวเท่านั้น
- Memory-to-Memory ใช้ channel ไหนก็ได้ (ไม่จำเป็นต้อง fixed-map)

---

## 1. Memory-to-Memory (M2M)

### 1.1 Fast memcpy (Blocking)

ก็อปปี้ข้อมูลจาก buffer หนึ่งไปอีก buffer หนึ่ง รอจนเสร็จ

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    
    uint8_t src[128];
    uint8_t dst[128];
    
    // เติมข้อมูล src
    for (int i = 0; i < 128; i++) src[i] = i;
    
    // DMA copy — รอจนเสร็จ (blocking)
    DMA_MemCopy(dst, src, 128);
    
    // ตอนนี้ dst มีข้อมูลเหมือน src แล้ว
    // เร็วกว่า memcpy() เพราะ DMA ทำงานขนานกับ CPU bus
    
    while (1);
}
```

### 1.2 Fast memset (Blocking)

เติมค่าให้ buffer ขนาดใหญ่ด้วย DMA:

```c
uint8_t buffer[1024];

// เติม 0 ทั้ง buffer — เร็วกว่า memset()
DMA_MemSet(buffer, 0x00, 1024);

// เติม pattern
DMA_MemSet(buffer, 0xA5, 512);
```

### 1.3 Non-blocking memcpy

เริ่ม copy แล้วกลับมาทำงานอื่นได้:

```c
uint8_t src[256];
uint8_t dst[256];

// เริ่ม copy แบบไม่รอ
DMA_MemCopyAsync(DMA_CH1, dst, src, 256);

// ทำอย่างอื่นระหว่างรอ DMA ทำงาน
some_processing();

// ตรวจสอบว่ายังทำงานอยู่หรือไม่
if (DMA_GetStatus(DMA_CH1) == DMA_STATUS_COMPLETE) {
    // copy เสร็จแล้ว
}

// หรือรอแบบมี timeout (หน่วย ms)
if (DMA_WaitComplete(DMA_CH1, 5000)) {
    // เสร็จภายใน 5 วินาที
} else {
    // timeout หรือ error
}
```

### 1.4 Chained Copy (หลาย buffer เรียงกัน)

ใช้ callback เพื่อ copy buffer ถัดไปทันทีที่ buffer ก่อนเสร็จ:

```c
#include "SimpleHAL.h"

#define BUF_SIZE 64

uint8_t src1[BUF_SIZE], src2[BUF_SIZE], src3[BUF_SIZE];
uint8_t dst1[BUF_SIZE], dst2[BUF_SIZE], dst3[BUF_SIZE];

volatile uint8_t chain_step = 0;

void DMA_TransferCompleteCallback(DMA_Channel ch) {
    if (ch == DMA_CH1) {
        chain_step++;
        switch (chain_step) {
            case 1:
                DMA_MemCopyAsync(DMA_CH1, dst2, src2, BUF_SIZE);
                break;
            case 2:
                DMA_MemCopyAsync(DMA_CH1, dst3, src3, BUF_SIZE);
                break;
            case 3:
                // ทั้งหมดเสร็จ
                break;
        }
    }
}

void start_chained_copy(void) {
    chain_step = 0;
    DMA_MemCopyAsync(DMA_CH1, dst1, src1, BUF_SIZE);
}

int main(void) {
    SystemCoreClockUpdate();
    Delay_Init();  // ถ้าต้องการ
    
    // เติม src buffers
    for (int i = 0; i < BUF_SIZE; i++) {
        src1[i] = i;
        src2[i] = i + 100;
        src3[i] = i + 200;
    }
    
    // Set callback (ใช้ weak callback function หรือ set โดยตรง)
    DMA_SetTransferCompleteCallback(DMA_CH1, DMA_TransferCompleteCallback);
    
    start_chained_copy();
    
    // ทำอย่างอื่นระหว่างรอ chained copy ทำงาน
    while (chain_step < 3) {
        // รอ
    }
    
    while (1);
}
```

---

## 2. ADC + DMA

### 2.1 Single Channel Circular (อ่านต่อเนื่อง)

อ่านค่า ADC จาก PD2 ต่อเนื่อง 100 ตัวอย่าง DMA จะเขียนลง buffer เรื่อยๆ:

```c
#include "SimpleHAL.h"

#define ADC_SAMPLES 100

uint16_t adc_buffer[ADC_SAMPLES];

int main(void) {
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // เริ่มอ่าน ADC แบบต่อเนื่อง (circular) ผ่าน DMA
    DMA_analogReadStart(PD2, adc_buffer, ADC_SAMPLES, 1);
    
    while (1) {
        Delay_Ms(100);
        
        // อ่านค่าล่าสุด (ตำแหน่งสุดท้ายใน buffer)
        uint16_t latest = adc_buffer[ADC_SAMPLES - 1];
        
        // หรือค่าเฉลี่ยทั้ง buffer
        uint16_t avg = DMA_analogReadAverage(adc_buffer, ADC_SAMPLES);
        
        // แปลงเป็นแรงดัน
        float voltage = ADC_ToVoltage(avg, 3.3);
        
        USART_Print("ADC=");
        USART_PrintNum(avg);
        USART_Print(" Voltage=");
        // แสดง voltage (ต้องใช้ sprintf หรือ custom float print)
        USART_Print("\r\n");
    }
}
```

### 2.2 Multi-Channel Scan

อ่าน 3 channels (PA2, PA1, PD2) พร้อมกัน:

```c
#include "SimpleHAL.h"

#define NUM_CHANNELS  3
#define SAMPLES_PER_CH  10

// buffer ขนาด NUM_CHANNELS * SAMPLES_PER_CH
uint16_t adc_buffer[NUM_CHANNELS * SAMPLES_PER_CH];

int main(void) {
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // เปิด ADC channels ที่ต้องการ
    ADC_SimpleInitChannels(3, (ADC_Channel[]){
        ADC_Channel_0,   // PA2
        ADC_Channel_1,   // PA1
        ADC_Channel_3    // PD2
    });
    
    // ตั้งค่า DMA + ADC multi-channel
    DMA_ADC_InitMultiChannel(DMA_CH1, adc_buffer, NUM_CHANNELS, SAMPLES_PER_CH);
    DMA_Start(DMA_CH1);
    
    // เริ่ม ADC continuous conversion
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    while (1) {
        Delay_Ms(200);
        
        // buffer layout: [ch0_0, ch1_0, ch2_0, ch0_1, ch1_1, ch2_1, ...]
        // อ่านค่าเฉลี่ยของแต่ละ channel
        for (int ch = 0; ch < NUM_CHANNELS; ch++) {
            uint32_t sum = 0;
            for (int s = 0; s < SAMPLES_PER_CH; s++) {
                sum += adc_buffer[s * NUM_CHANNELS + ch];
            }
            uint16_t avg = sum / SAMPLES_PER_CH;
            
            USART_Print("CH");
            USART_PrintNum(ch);
            USART_Print("=");
            USART_PrintNum(avg);
            USART_Print(" ");
        }
        USART_Print("\r\n");
    }
}
```

### 2.3 One-Shot ADC Read (อ่านค่าเดียว)

ใช้ DMA อ่าน N ตัวอย่างแล้วคืนค่าเฉลี่ย (จบในฟังก์ชันเดียว):

```c
#include "SimpleHAL.h"

// ฟังก์ชัน wrapper: อ่าน ADC ผ่าน DMA แล้วคืนค่าเฉลี่ย
uint16_t ADC_DMA_Read(uint8_t pin, uint8_t samples) {
    uint16_t buf[64];
    if (samples > 64) samples = 64;
    
    DMA_analogReadStart(pin, buf, samples, 0);  // normal mode
    while (DMA_analogReadBusy());
    DMA_analogReadStop();
    
    return DMA_analogReadAverage(buf, samples);
}

int main(void) {
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    while (1) {
        // อ่าน PD2 16 ตัวอย่าง ค่าเฉลี่ย
        uint16_t val = ADC_DMA_Read(PD2, 16);
        
        USART_Print("Value=");
        USART_PrintNum(val);
        USART_Print("\r\n");
        
        Delay_Ms(500);
    }
}
```

### 2.4 Dual Channel Simultaneous (2 channels พร้อมกัน)

อ่าน 2 ADC channels สลับกันด้วย DMA multi-channel:

```c
#include "SimpleHAL.h"

#define BUF_LEN 20

uint16_t adc_buf[2 * BUF_LEN];  // 2 channels × 20 samples

int main(void) {
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // เปิด 2 channels
    ADC_EnableChannel(ADC_Channel_0);  // PA2
    ADC_EnableChannel(ADC_Channel_3);  // PD2
    
    // ตั้ง scan mode + DMA
    DMA_ADC_InitMultiChannel(DMA_CH1, adc_buf, 2, BUF_LEN);
    DMA_Start(DMA_CH1);
    
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    while (1) {
        Delay_Ms(100);
        
        uint16_t ch0 = adc_buf[0];  // ค่าล่าสุดของ channel 0
        uint16_t ch1 = adc_buf[1];  // ค่าล่าสุดของ channel 1
        
        USART_Print("CH0=");
        USART_PrintNum(ch0);
        USART_Print(" CH1=");
        USART_PrintNum(ch1);
        USART_Print("\r\n");
    }
}
```

---

## 3. USART + DMA

### 3.1 Non-blocking Transmit

ส่งข้อมูลผ่าน USART โดยไม่ต้องรอ (ใช้ DMA ช่วย):

```c
#include "SimpleHAL.h"

volatile uint8_t tx_busy = 0;

void DMA_TransferCompleteCallback(DMA_Channel ch) {
    if (ch == DMA_CH2) {
        tx_busy = 0;
    }
}

void send_nonblocking(const uint8_t* data, uint16_t len) {
    while (tx_busy);  // รอรอบก่อนหน้าถ้ายังไม่เสร็จ
    tx_busy = 1;
    DMA_USART_Transmit(DMA_CH2, data, len);
}

int main(void) {
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // ตั้งค่า DMA USART TX (init ครั้งเดียว)
    uint8_t dummy = 0;
    DMA_USART_InitTx(DMA_CH2, &dummy, 1);
    
    // ตั้ง callback
    DMA_SetTransferCompleteCallback(DMA_CH2, DMA_TransferCompleteCallback);
    
    const uint8_t msg[] = "Hello via DMA!\r\n";
    
    while (1) {
        send_nonblocking(msg, sizeof(msg) - 1);
        Delay_Ms(1000);
    }
}
```

### 3.2 Circular RX Buffer (Ring Buffer)

รับข้อมูลแบบต่อเนื่องผ่าน DMA เข้า ring buffer:

```c
#include "SimpleHAL.h"

#define RX_BUF_SIZE 256

uint8_t rx_buffer[RX_BUF_SIZE];

// ตรวจสอบว่ามีข้อมูลใหม่เข้ามาหรือไม่
uint16_t get_rx_count(void) {
    return DMA_USART_GetReceivedCount(DMA_CH3, RX_BUF_SIZE);
}

void process_received_data(void) {
    static uint16_t last_pos = 0;
    uint16_t current_pos = get_rx_count();
    
    if (current_pos != last_pos) {
        // แปลงเป็น linear index
        if (current_pos > last_pos) {
            uint16_t n = current_pos - last_pos;
            for (uint16_t i = 0; i < n; i++) {
                uint8_t byte = rx_buffer[(last_pos + i) % RX_BUF_SIZE];
                // process byte...
                (void)byte;
            }
        } else {
            // wrap-around
            uint16_t n1 = RX_BUF_SIZE - last_pos;
            uint16_t n2 = current_pos;
            for (uint16_t i = 0; i < n1; i++) {
                uint8_t byte = rx_buffer[(last_pos + i) % RX_BUF_SIZE];
                (void)byte;
            }
            for (uint16_t i = 0; i < n2; i++) {
                uint8_t byte = rx_buffer[i];
                (void)byte;
            }
        }
        last_pos = current_pos;
    }
}

int main(void) {
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // ตั้งค่า DMA USART RX แบบ circular
    DMA_USART_InitRx(DMA_CH3, rx_buffer, RX_BUF_SIZE, 1);
    DMA_Start(DMA_CH3);
    
    while (1) {
        process_received_data();
        // ทำอย่างอื่น...
    }
}
```

### 3.3 One-Shot USART Send (จบในฟังก์ชันเดียว)

ส่ง string ผ่าน USART DMA ด้วยฟังก์ชันเดียว (new API):

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    const uint8_t msg1[] = "Hello World!\r\n";
    const uint8_t msg2[] = "DMA is fast!\r\n";
    
    while (1) {
        // ครบจบในบรรทัดเดียว — init + send + wait
        DMA_USART_Send(DMA_CH2, msg1, sizeof(msg1) - 1);
        Delay_Ms(500);
        DMA_USART_Send(DMA_CH2, msg2, sizeof(msg2) - 1);
        Delay_Ms(500);
    }
}
```

### 3.4 Ping-Pong Buffer สำหรับ RX

ใช้ half-transfer callback เพื่อสลับ buffer ครึ่งหนึ่ง:

```c
#include "SimpleHAL.h"

#define BUF_SIZE 128
uint8_t rx_buffer[BUF_SIZE];

volatile uint8_t half_ready = 0;
volatile uint8_t full_ready = 0;

void DMA_HalfTransferCallback(DMA_Channel ch) {
    if (ch == DMA_CH3) {
        half_ready = 1;  // buffer ครึ่งแรก (0..63) พร้อมอ่าน
    }
}

void DMA_TransferCompleteCallback(DMA_Channel ch) {
    if (ch == DMA_CH3) {
        full_ready = 1;  // buffer ครึ่งหลัง (64..127) พร้อมอ่าน
    }
}

void process_pingpong(void) {
    if (half_ready) {
        half_ready = 0;
        // อ่านข้อมูลจาก rx_buffer[0..63]
        for (int i = 0; i < BUF_SIZE/2; i++) {
            uint8_t d = rx_buffer[i];
            // process...
            (void)d;
        }
    }
    if (full_ready) {
        full_ready = 0;
        // อ่านข้อมูลจาก rx_buffer[64..127]
        for (int i = BUF_SIZE/2; i < BUF_SIZE; i++) {
            uint8_t d = rx_buffer[i];
            // process...
            (void)d;
        }
    }
}

int main(void) {
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    DMA_USART_InitRx(DMA_CH3, rx_buffer, BUF_SIZE, 1);
    
    // ตั้ง callback ทั้ง half-transfer และ complete
    DMA_SetHalfTransferCallback(DMA_CH3, DMA_HalfTransferCallback);
    DMA_SetTransferCompleteCallback(DMA_CH3, DMA_TransferCompleteCallback);
    
    DMA_Start(DMA_CH3);
    
    while (1) {
        process_pingpong();
        // CPU ทำงานอื่นได้โดยไม่เสียข้อมูล
    }
}
```

---

## 4. SPI + DMA

### 4.1 Full-Duplex Transfer

ส่งและรับข้อมูลพร้อมกันผ่าน SPI:

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // ตั้งค่า SPI
    SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);
    
    uint8_t tx_data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t rx_data[8] = {0};
    
    // ตั้งค่า DMA สำหรับ SPI (TX + RX)
    DMA_SPI_Init(DMA_CH3, DMA_CH2);
    
    // ส่ง-รับพร้อมกัน
    DMA_SPI_TransferBuffer(DMA_CH3, DMA_CH2, tx_data, rx_data, 8);
    
    // rx_data มีค่าที่รับมา
    
    USART_Print("SPI DMA done\r\n");
    
    while (1);
}
```

### 4.2 SPI Read Flash (TX only + RX ignore)

ส่งคำสั่ง SPI โดยไม่สน RX:

```c
// ส่งคำสั่งไปยัง SPI Flash
void spi_flash_send_cmd(uint8_t cmd) {
    uint8_t tx = cmd;
    uint8_t rx;
    
    DMA_SPI_Init(DMA_CH3, DMA_CH2);
    DMA_SPI_TransferBuffer(DMA_CH3, DMA_CH2, &tx, &rx, 1);
}

// อ่านข้อมูลจาก SPI Flash
void spi_flash_read(uint8_t cmd, uint32_t addr, uint8_t* data, uint16_t len) {
    uint8_t tx_buf[4 + len];
    uint8_t rx_buf[4 + len];
    
    tx_buf[0] = cmd;         // read command
    tx_buf[1] = (addr >> 16) & 0xFF;  // address high
    tx_buf[2] = (addr >> 8) & 0xFF;   // address mid
    tx_buf[3] = addr & 0xFF;          // address low
    // remaining bytes = 0 (dummy for output)
    
    DMA_SPI_Init(DMA_CH3, DMA_CH2);
    DMA_SPI_TransferBuffer(DMA_CH3, DMA_CH2, tx_buf, rx_buf, 4 + len);
    
    // คัดลอกข้อมูลที่อ่านได้
    for (uint16_t i = 0; i < len; i++) {
        data[i] = rx_buf[4 + i];
    }
}
```

### 4.3 SPI Update Display Buffer

ส่ง buffer ทั้ง buffer ไปยัง OLED/LCD ผ่าน SPI DMA:

```c
#include "SimpleHAL.h"

#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT  64
#define DISPLAY_BUFFER  (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8)

uint8_t display_buffer[DISPLAY_BUFFER];

// ส่ง display buffer ทั้งหมดผ่าน SPI DMA
void display_update(void) {
    DMA_SPI_Init(DMA_CH3, DMA_CH2);
    DMA_SPI_TransferBuffer(DMA_CH3, DMA_CH2, 
                          display_buffer, NULL, DISPLAY_BUFFER);
}
```

### 4.4 SPI with Chip Select

ควบคุม CS pin รอบๆ DMA transfer:

```c
// กำหนด CS pin
#define SPI_CS_PIN  PC1

void spi_dma_transfer_cs(const uint8_t* tx, uint8_t* rx, uint16_t len) {
    // CS low
    digitalWrite(SPI_CS_PIN, LOW);
    
    Delay_Us(1);  // เล็กน้อย
    
    // DMA transfer
    DMA_SPI_Init(DMA_CH3, DMA_CH2);
    DMA_SPI_TransferBuffer(DMA_CH3, DMA_CH2, tx, rx, len);
    
    // CS high
    digitalWrite(SPI_CS_PIN, HIGH);
}

int main(void) {
    SystemCoreClockUpdate();
    
    // ตั้ง CS pin เป็น output
    pinMode(SPI_CS_PIN, OUTPUT);
    digitalWrite(SPI_CS_PIN, HIGH);
    
    // ตั้ง SPI
    SPI_SimpleInit(SPI_MODE0, SPI_4MHZ, SPI_PINS_DEFAULT);
    
    uint8_t cmd[] = {0x90, 0x00, 0x00, 0x00};
    uint8_t resp[4];
    
    spi_dma_transfer_cs(cmd, resp, 4);
    
    while (1);
}
```

---

## 5. I2C + DMA

### 5.1 I2C Write via DMA (ส่งข้อมูลไปยัง I2C slave)

```c
#include "SimpleHAL.h"

#define I2C_DEV_ADDR  0x3C  // ตัวอย่าง: OLED SSD1306

int main(void) {
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // ตั้งค่า I2C
    I2C_SimpleInit(I2C_400KHZ, I2C_PINS_DEFAULT);
    
    // ตั้งค่า DMA TX (I2C1_TX fixed ที่ DMA_CH4)
    DMA_I2C_InitTx(DMA_CH4);
    
    // ตัวอย่าง: ส่งข้อมูลไปยัง I2C device
    // ขั้นตอน: เขียน register address ก่อน แล้วจึงใช้ DMA ส่ง data
    
    uint8_t data_block[64];
    for (int i = 0; i < 64; i++) data_block[i] = i;
    
    // วิธีใช้ DMA: ต้องตั้งค่า I2C ให้ส่ง data block ผ่าน DMA
    // 1. ส่ง slave address + register address ด้วย I2C_WriteReg
    // 2. ต่อจากนั้นใช้ DMA ส่งข้อมูลจำนวนมาก
    
    // I2C_WriteRegMulti(I2C_DEV_ADDR, 0x00, data_block, 64);
    
    USART_Print("I2C DMA ready\r\n");
    
    while (1);
}
```

### 5.2 I2C Read EEPROM via DMA

อ่านข้อมูลจาก EEPROM 24LCxx ผ่าน DMA:

```c
#include "SimpleHAL.h"

#define EEPROM_ADDR  0x50  // 24LC32

// อ่าน EEPROM ผ่าน DMA (blocking)
uint8_t eeprom_read_dma(uint16_t mem_addr) {
    uint8_t addr_buf[2] = {(mem_addr >> 8) & 0xFF, mem_addr & 0xFF};
    uint8_t data = 0;
    
    // I2C DMA TX (CH4) + RX (CH5)
    DMA_I2C_InitTx(DMA_CH4);
    DMA_I2C_InitRx(DMA_CH5);
    
    // ขั้นตอน I2C read:
    // 1. ส่ง memory address (แบบ blocking ปกติ)
    I2C_WriteReg(EEPROM_ADDR, addr_buf[0], addr_buf[1]);
    
    // 2. เริ่ม I2C read + เปิด DMA
    I2C_DMACmd(I2C1, ENABLE);
    
    // 3. ส่ง start + slave addr + read bit (ใช้ I2C_Read)
    // DMA จะรับข้อมูลอัตโนมัติ
    // I2C_Read(EEPROM_ADDR, &data, 1);
    
    return data;
}
```

**หมายเหตุ:** I2C + DMA บน CH32V003 มีข้อจำกัด — ต้องใช้ I2C interrupt ช่วยในการจัดการ start/stop condition เพราะ DMA ไม่สามารถสร้าง I2C protocol ได้เอง แนะนำให้ใช้ DMA สำหรับ bulk data transfer (>16 bytes) หลังจาก I2C address/reister ถูกส่งด้วย CPU แล้ว

---

## 6. TIM + DMA

### 6.1 TIM Capture → RAM (วัดความถี่สัญญาณ)

จับค่าความถี่สัญญาณขาเข้าโดย TIM1_CH4 capture event trigger DMA:

```c
#include "SimpleHAL.h"

#define CAPTURE_COUNT 100

// buffer สำหรับเก็บค่าที่ TIM capture ได้
volatile uint16_t cap_buffer[CAPTURE_COUNT];

int main(void) {
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // ตั้งค่า TIM1 สำหรับ input capture บน CH4 (PC4)
    // 1. เปิด clock TIM1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    
    // 2. ตั้งค่า GPIO PC4 เป็น input (TIM1_CH4)
    pinMode(PC4, INPUT_FLATING);
    
    // 3. ตั้งค่า TIM1 input capture
    TIM_ICInitTypeDef ic_config;
    TIM_ICStructInit(&ic_config);
    ic_config.TIM_Channel = TIM_Channel_4;
    ic_config.TIM_ICPolarity = TIM_ICPolarity_Rising;  // ขอบขาขึ้น
    ic_config.TIM_ICSelection = TIM_ICSelection_DirectTI;
    ic_config.TIM_ICPrescaler = TIM_ICPSC_DIV1;        // no prescaler
    ic_config.TIM_ICFilter = 0x0;
    TIM_ICInit(TIM1, &ic_config);
    
    // 4. ตั้งค่า DMA capture (TIM1_CH4 capture → cap_buffer)
    DMA_TIM_InitCapture(DMA_CH4, TIM1, (uint16_t*)cap_buffer, 
                        CAPTURE_COUNT, (uint32_t)&TIM1->CH4CVR);
    
    // 5. เปิด TIM capture DMA
    TIM_DMACmd(TIM1, TIM_DMA_CC4, ENABLE);
    
    // 6. เริ่ม TIM
    TIM_Cmd(TIM1, ENABLE);
    
    // 7. เริ่ม DMA
    DMA_Start(DMA_CH4);
    
    USART_Print("TIM capture started\r\n");
    
    while (1) {
        if (DMA_GetStatus(DMA_CH4) == DMA_STATUS_COMPLETE) {
            // buffer มีค่าที่ capture ได้
            // คำนวณความถี่: frequency = TIM1 clock / (cap_buffer[1] - cap_buffer[0])
            USART_Print("Capture done!\r\n");
            DMA_Reset(DMA_CH4);
            break;
        }
    }
    
    while (1);
}
```

### 6.2 DMA PWM Waveform Generator (สร้าง waveform ด้วย DMA)

ใช้ DMA อัปเดต duty cycle ของ PWM ทุกครั้งที่ TIM update เพื่อสร้าง waveform:

```c
#include "SimpleHAL.h"

// 64 samples sine wave (12-bit resolution, 0-4095)
const uint16_t sine_wave_64[] = {
    2048, 2248, 2447, 2643, 2831, 3012, 3183, 3343,
    3490, 3624, 3743, 3847, 3936, 4008, 4064, 4103,
    4126, 4132, 4122, 4096, 4055, 3999, 3929, 3846,
    3751, 3645, 3529, 3405, 3275, 3140, 3002, 2863,
    2725, 2590, 2460, 2338, 2225, 2123, 2035, 1961,
    1903, 1862, 1838, 1832, 1844, 1874, 1922, 1987,
    2069, 2166, 2277, 2399, 2531, 2670, 2814, 2959,
    3104, 3245, 3379, 3505, 3620, 3722, 3811, 3885
};

int main(void) {
    SystemCoreClockUpdate();
    
    // 1. ตั้งค่า PWM ที่ความถี่สูง (เช่น 10kHz)
    PWM_Init(PWM1_CH1, 10000);  // TIM1_CH1 พร้อม PWM
    PWM_SetDutyCycle(PWM1_CH1, 50);
    PWM_Start(PWM1_CH1);
    
    // 2. ตั้งค่า DMA — ให้ DMA เขียนค่า sine_wave_64 ไปยัง TIM1->CH1CVR
    //    ทุกครั้งที่ TIM1 update event เกิดขึ้น
    //    ใช้ DMA_CH5 (TIM1_Update) เพื่อ trigger DMA transfer
    DMA_TIM_UpdatePWM(DMA_CH5, TIM1, 
                      DMA_TIM_GetCCRAddress(TIM1, 1),
                      sine_wave_64, 64, 1);  // circular mode
    
    // 3. เปิด TIM update DMA
    TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);
    
    // 4. เริ่ม DMA
    DMA_Start(DMA_CH5);
    
    // ตอนนี้ PWM output จะเป็น sine wave!
    // ความถี่ sine wave = PWM_freq / 64
    // เช่น 10kHz / 64 = 156.25 Hz
    
    while (1);
}
```

### 6.3 สร้าง Sawtooth / Triangle Wave

```c
// Sawtooth wave table (64 samples)
const uint16_t sawtooth_64[] = {
    0, 64, 128, 192, 256, 320, 384, 448,
    512, 576, 640, 704, 768, 832, 896, 960,
    1024, 1088, 1152, 1216, 1280, 1344, 1408, 1472,
    1536, 1600, 1664, 1728, 1792, 1856, 1920, 1984,
    2048, 2112, 2176, 2240, 2304, 2368, 2432, 2496,
    2560, 2624, 2688, 2752, 2816, 2880, 2944, 3008,
    3072, 3136, 3200, 3264, 3328, 3392, 3456, 3520,
    3584, 3648, 3712, 3776, 3840, 3904, 3968, 4095
};

// Triangle wave table (32 samples)
const uint16_t triangle_32[] = {
    0, 256, 512, 768, 1024, 1280, 1536, 1792,
    2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840,
    4095, 3840, 3584, 3328, 3072, 2816, 2560, 2304,
    2048, 1792, 1536, 1280, 1024, 768, 512, 256
};
```

### 6.4 TIM Update Trigger + DMA (buffer size auto-reload)

ใช้ TIM update event trigger DMA เพื่อย้ายข้อมูลจาก RAM → peripheral แบบอัตโนมัติ:

```c
// ตัวอย่าง: TIM1 update → DMA → USART1 send byte (rate generator)
// ใช้ DMA_CH5 (TIM1_Update) เป็น trigger

uint8_t pattern[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

void tim_dma_to_usart(void) {
    // ตั้งค่า DMA — MEM → USART1_DATAR โดยใช้ TIM1_Update trigger
    DMA_USART_InitTx(DMA_CH5, pattern, sizeof(pattern));
    
    // เปิด TIM1 update DMA
    TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);
    
    // เริ่ม DMA (จะรอ trigger จาก TIM1)
    DMA_Start(DMA_CH5);
    
    // ทุกครั้งที่ TIM1 update, DMA จะส่ง 1 byte ไปยัง USART
    // ยิงข้อมูลต่อเนื่องด้วยความถี่ TIM1
}
```

---

## 7. Advanced Patterns

### 7.1 Ping-Pong Double Buffer

ใช้ half-transfer interrupt เพื่อสลับ buffer สำหรับ continuous ADC:

```c
#include "SimpleHAL.h"

#define BUF_HALF 64
#define BUF_SIZE (BUF_HALF * 2)

uint16_t adc_buf[BUF_SIZE];
volatile uint8_t buf_sel = 0;  // 0 = ครึ่งแรก, 1 = ครึ่งหลัง

void DMA_HalfTransferCallback(DMA_Channel ch) {
    if (ch == DMA_CH1) {
        buf_sel = 0;  // buffer ครึ่งแรกพร้อม
    }
}

void DMA_TransferCompleteCallback(DMA_Channel ch) {
    if (ch == DMA_CH1) {
        buf_sel = 1;  // buffer ครึ่งหลังพร้อม
    }
}

int main(void) {
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // ตั้ง ADC DMA circular buffer
    DMA_ADC_Init(DMA_CH1, adc_buf, BUF_SIZE, 1);
    DMA_SetHalfTransferCallback(DMA_CH1, DMA_HalfTransferCallback);
    DMA_SetTransferCompleteCallback(DMA_CH1, DMA_TransferCompleteCallback);
    DMA_Start(DMA_CH1);
    
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    uint32_t sum = 0;
    uint16_t count = 0;
    
    while (1) {
        if (buf_sel == 0) {
            buf_sel = 0xFF;  // reset flag
            // อ่าน buffer ครึ่งแรก (0..BUF_HALF-1)
            for (int i = 0; i < BUF_HALF; i++) {
                sum += adc_buf[i];
                count++;
            }
        }
        if (buf_sel == 1) {
            buf_sel = 0xFF;
            // อ่าน buffer ครึ่งหลัง (BUF_HALF..BUF_SIZE-1)
            for (int i = BUF_HALF; i < BUF_SIZE; i++) {
                sum += adc_buf[i];
                count++;
            }
        }
        
        // ทุก 1000 ตัวอย่าง แสดงค่าเฉลี่ย
        if (count >= 1000) {
            uint16_t avg = sum / count;
            USART_Print("Avg=");
            USART_PrintNum(avg);
            USART_Print("\r\n");
            sum = 0;
            count = 0;
        }
    }
}
```

### 7.2 DMA Chaining (Transfer → Callback → Next Transfer)

ใช้งาน DMA หลายรอบต่อเนื่องผ่าน callback:

```c
#include "SimpleHAL.h"

typedef struct {
    void* dst;
    const void* src;
    uint16_t len;
} DMA_Job_t;

DMA_Job_t job_queue[4];
volatile uint8_t job_index = 0;
volatile uint8_t job_count = 0;

void DMA_TransferCompleteCallback(DMA_Channel ch) {
    if (ch == DMA_CH1) {
        job_index++;
        if (job_index < job_count) {
            DMA_MemCopyAsync(DMA_CH1, 
                           job_queue[job_index].dst,
                           job_queue[job_index].src,
                           job_queue[job_index].len);
        }
    }
}

void dma_chain_start(DMA_Job_t* jobs, uint8_t count) {
    job_index = 0;
    job_count = count;
    for (uint8_t i = 0; i < count; i++) {
        job_queue[i] = jobs[i];
    }
    
    DMA_SetTransferCompleteCallback(DMA_CH1, DMA_TransferCompleteCallback);
    DMA_MemCopyAsync(DMA_CH1, jobs[0].dst, jobs[0].src, jobs[0].len);
}

// ตัวอย่างการใช้งาน
int main(void) {
    SystemCoreClockUpdate();
    
    uint8_t a[100], b[100], c[100], d[100];
    uint8_t src_a[100], src_b[100], src_c[100], src_d[100];
    
    DMA_Job_t tasks[] = {
        {a, src_a, 100},
        {b, src_b, 100},
        {c, src_c, 100},
        {d, src_d, 100}
    };
    
    dma_chain_start(tasks, 4);
    
    // ทำอย่างอื่นระหว่างรอ 4 transfers ทำงานต่อเนื่อง
    while (job_index < job_count) {
        // CPU ทำงานอิสระ
    }
    
    // ทั้ง 4 transfers เสร็จแล้ว
    
    while (1);
}
```

### 7.3 DMA + Sleep Mode (ประหยัดพลังงาน)

CPU เข้า sleep ในขณะที่ DMA ทำงาน:

```c
#include "SimpleHAL.h"

volatile uint8_t dma_done = 0;

void DMA_TransferCompleteCallback(DMA_Channel ch) {
    if (ch == DMA_CH1) {
        dma_done = 1;
    }
}

int main(void) {
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    uint8_t src[256];
    uint8_t dst[256];
    for (int i = 0; i < 256; i++) src[i] = i;
    
    DMA_SetTransferCompleteCallback(DMA_CH1, DMA_TransferCompleteCallback);
    DMA_MemCopyAsync(DMA_CH1, dst, src, 256);
    
    // เข้า sleep ขณะ DMA ทำงาน
    // DMA ยังทำงานได้เพราะ DMA อยู่บน bus matrix แยกจาก CPU
    while (!dma_done) {
        __WFI();  // Sleep until interrupt (DMA complete)
    }
    
    // DMA เสร็จ CPU ถูกปลุกด้วย DMA interrupt
    
    while (1);
}
```

### 7.4 Performance Benchmark: DMA vs CPU

เปรียบเทียบความเร็ว DMA copy กับ CPU copy:

```c
#include "SimpleHAL.h"

#define TEST_SIZE 1024

uint8_t src[TEST_SIZE];
uint8_t dst_dma[TEST_SIZE];
uint8_t dst_cpu[TEST_SIZE];

void benchmark(void) {
    uint32_t start, time_dma, time_cpu;
    
    // เติมข้อมูล
    for (int i = 0; i < TEST_SIZE; i++) src[i] = (uint8_t)i;
    
    // --- CPU copy ---
    start = Get_CurrentUs();
    for (int i = 0; i < TEST_SIZE; i++) {
        dst_cpu[i] = src[i];
    }
    time_cpu = Get_CurrentUs() - start;
    
    // --- DMA copy ---
    start = Get_CurrentUs();
    DMA_MemCopy(dst_dma, src, TEST_SIZE);
    time_dma = Get_CurrentUs() - start;
    
    // แสดงผล
    USART_Print("CPU copy: ");
    USART_PrintNum(time_cpu);
    USART_Print(" us\r\n");
    
    USART_Print("DMA copy: ");
    USART_PrintNum(time_dma);
    USART_Print(" us\r\n");
    
    if (time_dma < time_cpu) {
        USART_Print("DMA is faster!\r\n");
    } else {
        USART_Print("CPU is faster for small size\r\n");
    }
}

int main(void) {
    SystemCoreClockUpdate();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    benchmark();
    
    while (1);
}
```

> **ข้อสังเกต:** DMA เหมาะกับ buffer ขนาดใหญ่ (>64 bytes) และต้องการให้ CPU ทำงานอื่นพร้อมกัน สำหรับขนาดเล็ก overhead การตั้งค่า DMA อาจทำให้ช้ากว่า CPU copy

---

## Appendix: API Quick Reference (New Functions)

ฟังก์ชันใหม่ที่เพิ่มใน SimpleDMA v1.1:

| ฟังก์ชัน | คำอธิบาย |
|----------|----------|
| `DMA_SetHalfTransferCallback(ch, cb)` | ตั้ง callback เมื่อ transfer มาครึ่งทาง |
| `DMA_USART_Send(ch, data, len)` | USART send แบบครบจบ (init+send+wait) |
| `DMA_I2C_InitTx(ch)` | ตั้งค่า I2C1 TX DMA |
| `DMA_I2C_InitRx(ch)` | ตั้งค่า I2C1 RX DMA |
| `DMA_I2C_Transfer(tx_ch, rx_ch, tx, rx, len)` | I2C ส่ง-รับด้วย DMA |
| `DMA_TIM_InitCapture(ch, tim, buf, len, ccx_addr)` | TIM capture → RAM |
| `DMA_TIM_UpdatePWM(ch, tim, ccr_addr, wave, len, circ)` | RAM → TIM CCR (waveform gen) |
| `DMA_TIM_GetCCRAddress(tim, ch)` | แปลง TIM channel → CCR address |

### Macro ช่วยจำ DMA Channel Map

```c
// สำหรับอ้างอิงเวลาเลือก channel
#define DMA_CH_ADC1      DMA_CH1
#define DMA_CH_USART1_TX DMA_CH2
#define DMA_CH_SPI1_RX   DMA_CH2
#define DMA_CH_SPI1_TX   DMA_CH3
#define DMA_CH_USART1_RX DMA_CH3
#define DMA_CH_I2C1_TX   DMA_CH4
#define DMA_CH_TIM1_CH4  DMA_CH4
#define DMA_CH_I2C1_RX   DMA_CH5
#define DMA_CH_TIM1_UPD  DMA_CH5
#define DMA_CH_USART1_RX_ALT DMA_CH6
#define DMA_CH_TIM1_CH3  DMA_CH6
#define DMA_CH_USART1_TX_ALT DMA_CH7
#define DMA_CH_TIM2_CH2  DMA_CH7
```
