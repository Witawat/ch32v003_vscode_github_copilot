# 14_Integrated_Applications — แอปพลิเคชันรวม

## ไฟล์ในโฟลเดอร์

| ไฟล์ | คำอธิบาย |
|------|----------|
| `ex01_Weather_Station.c` | DS18B20 + BMP280 Sensor Hub (I2C + 1Wire + USART) |
| `ex02_LED_Controller_USART.c` | LED Controller ผ่าน USART (PWM + GPIO + USART) |
| `ex03_Data_Logger_Flash.c` | ADC Logger ลง Flash (ADC + Flash + Stopwatch) |
| `ex04_Smart_Countdown.c` | Smart Countdown (I2C OLED + PWM Buzzer + Interrupt) |
| `ex05_Sensor_Hub_DMA.c` | Sensor Hub DMA (BMP280 + DS18B20 + W25Qxx + DMA) |

## แนวคิด

แต่ละไฟล์รวมหลาย module เข้าด้วยกัน — เป็นตัวอย่างการสร้างแอปพลิเคชันจริง

```c
#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include "SimpleHAL.h"
// ใช้ GPIO + ADC + USART + I2C + Flash + ...
```

💡 **Tip:** ถ้าบน SOP-8 — เปลี่ยน `CH32V003_PACKAGE` และ rebuild โค้ดจะกรองฟีเจอร์ที่ใช้ไม่ได้ให้อัตโนมัติ
