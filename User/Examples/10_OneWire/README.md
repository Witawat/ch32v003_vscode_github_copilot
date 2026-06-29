# 10_OneWire — ตัวอย่าง 1-Wire

## ไฟล์ในโฟลเดอร์

| ไฟล์ | คำอธิบาย |
|------|----------|
| `ex01_DS18B20_Temperature.c` | DS18B20 อ่านอุณหภูมิ (OneWire_Init, SkipROM, ReadByte) |
| `ex02_ROM_Search.c` | ค้นหา ROM address (OneWire_Search, GetAddress) |
| `ex03_Multi_Device_Select.c` | เลือกอุปกรณ์หลายตัว (OneWire_Select, Match ROM) |

## Quick Start

```c
#include "SimpleHAL.h"
#include "Lib/DS18B20/DS18B20.h"

OneWire_Bus* bus = OneWire_Init(PD2);
DS18B20_Device* sensor = DS18B20_Init(bus);  // Auto-detect
float temp = DS18B20_ReadTemperature(sensor);  // °C
```

## ต่อวงจร

```
VCC (3.3V)
  |
[4.7kΩ]
  |
  +--- GPIO --- DS18B20 DATA
```
