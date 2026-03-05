# SimpleADC Library - คู่มือการใช้งาน

## ภาพรวม

SimpleADC เป็น library สำหรับใช้งาน ADC (Analog-to-Digital Converter) บน CH32V003 แบบง่ายๆ คล้าย Arduino โดยรองรับ:
- **8 External Channels** (PA1, PA2, PC4, PD2-PD6)
- **2 Internal Channels** (Vrefint, Vcalint)
- **Battery Monitoring** โดยไม่ต้องใช้วงจรภายนอก
- **VDD Compensation** เพื่อความแม่นยำสูง

## คุณสมบัติเด่น

✨ **ใช้งานง่าย** - API แบบ Arduino-style  
🔋 **วัดแบตเตอรี่** - วัดแรงดันและแสดงเปอร์เซ็นต์  
📊 **แม่นยำสูง** - ชดเชย VDD อัตโนมัติ  
⚡ **รวดเร็ว** - 10-bit resolution (0-1023)

---

## ADC Channels

### External Channels (0-7)

| Channel | GPIO Pin | ชื่อ Alias |
|---------|----------|------------|
| 0       | PA2      | `ADC_CH_PA2` |
| 1       | PA1      | `ADC_CH_PA1` |
| 2       | PC4      | `ADC_CH_PC4` |
| 3       | PD2      | `ADC_CH_PD2` |
| 4       | PD3      | `ADC_CH_PD3` |
| 5       | PD5      | `ADC_CH_PD5` |
| 6       | PD6      | `ADC_CH_PD6` |
| 7       | PD4      | `ADC_CH_PD4` |

### Internal Channels (8-9)

| Channel | ชื่อ | ค่าโดยประมาณ | การใช้งาน |
|---------|------|---------------|-----------|
| 8       | Vrefint | ~1.2V | วัด VDD จริง |
| 9       | Vcalint | 50-75% VDD | Calibration |

---

## การใช้งานพื้นฐาน

### 1. เริ่มต้น ADC

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เปิดใช้งาน ADC ทุก channels
    ADC_SimpleInit();
    
    // หรือเปิดเฉพาะบางช่อง
    ADC_Channel channels[] = {ADC_CH_PD2, ADC_CH_PD3};
    ADC_SimpleInitChannels(channels, 2);
}
```

### 2. อ่านค่า ADC

```c
// อ่านค่า ADC (0-1023)
uint16_t value = ADC_Read(ADC_CH_PD2);

// อ่านและแปลงเป็นแรงดัน
float voltage = ADC_ReadVoltage(ADC_CH_PD2, 3.3);

// อ่านแบบ average เพื่อลด noise
uint16_t avg = ADC_ReadAverage(ADC_CH_PD2, 10);
```

---

## Battery Monitoring (ฟีเจอร์ใหม่!)

### ทำไมต้องใช้ VDD Compensation?

เมื่อแบตเตอรี่หมด แรงดัน VDD จะลดลง ทำให้การวัด ADC ไม่แม่นยำ  
SimpleADC ใช้ **Internal Vref** เพื่อวัด VDD จริงและชดเชยอัตโนมัติ!

### วิธีการทำงาน

```
1. อ่าน Vrefint (Internal Reference ~1.2V)
2. คำนวณ VDD จริง: VDD = 1.2V × 1023 / Vrefint_ADC
3. ใช้ VDD ที่คำนวณได้ในการแปลงค่า ADC
```

### ตัวอย่างการใช้งาน

#### วัดแรงดัน VDD (แบตเตอรี่)

```c
// วัด VDD จริง
float vdd = ADC_GetVDD();
// ผลลัพธ์: 3.7V (ถ้าแบต Li-ion กำลังใช้งาน)
```

#### คำนวณเปอร์เซ็นต์แบตเตอรี่

```c
// สำหรับ Li-ion (4.2V เต็ม, 3.0V หมด)
float vdd = ADC_GetVDD();
float percent = ADC_GetBatteryPercent(vdd, 3.0, 4.2);

if (percent < 20) {
    // แบตต่ำ! ทำอะไรสักอย่าง
    LED_Blink();
}
```

#### ตัวอย่างสมบูรณ์

```c
void BatteryMonitor_Example(void) {
    ADC_SimpleInit();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // กำหนดช่วงแรงดันตามประเภทแบต
    const float V_MAX = 4.2;  // Li-ion เต็ม
    const float V_MIN = 3.0;  // Li-ion หมด
    
    while(1) {
        float vdd = ADC_GetVDD();
        float percent = ADC_GetBatteryPercent(vdd, V_MIN, V_MAX);
        
        printf("Battery: %.2fV (%d%%)\r\n", vdd, (int)percent);
        
        if (percent < 20) {
            printf("WARNING: Low battery!\r\n");
        }
        
        Delay_Ms(1000);
    }
}
```

---

## ตารางแรงดันมาตรฐาน

| ประเภทแบตเตอรี่ | แรงดันเต็ม | แรงดันหมด | ตัวอย่างโค้ด |
|----------------|-----------|-----------|-------------|
| **Li-ion/Li-Po** | 4.2V | 3.0V | `ADC_GetBatteryPercent(vdd, 3.0, 4.2)` |
| **Alkaline 2xAA** | 3.2V | 2.0V | `ADC_GetBatteryPercent(vdd, 2.0, 3.2)` |
| **CR2032** | 3.0V | 2.0V | `ADC_GetBatteryPercent(vdd, 2.0, 3.0)` |
| **NiMH 2xAA** | 2.8V | 2.0V | `ADC_GetBatteryPercent(vdd, 2.0, 2.8)` |

---

## API Reference

### Basic Functions

#### `ADC_SimpleInit()`
เปิดใช้งาน ADC ทุก channels (0-7)

#### `ADC_SimpleInitChannels(channels, count)`
เปิดใช้งานเฉพาะ channels ที่ระบุ

#### `ADC_Read(channel)`
อ่านค่า ADC (0-1023)

#### `ADC_ReadVoltage(channel, vref)`
อ่านและแปลงเป็นแรงดัน (V)

#### `ADC_ReadAverage(channel, samples)`
อ่านค่าเฉลี่ยหลายครั้ง เพื่อลด noise

#### `ADC_ToPercent(adc_value)`
แปลงค่า ADC เป็นเปอร์เซ็นต์ (0-100%)

### Battery Monitoring Functions

#### `ADC_ReadVrefInt()`
อ่านค่า Internal Reference Voltage (Vrefint)
- **Return**: ค่า ADC ของ Vrefint (0-1023)
- **ใช้สำหรับ**: คำนวณ VDD จริง

#### `ADC_GetVDD()`
คำนวณแรงดัน VDD จริงจาก Vrefint
- **Return**: แรงดัน VDD (V)
- **หมายเหตุ**: อ่านค่าเฉลี่ย 10 ครั้งเพื่อความแม่นยำ

#### `ADC_ReadVoltageCompensated(channel)`
อ่านค่า ADC พร้อมชดเชย VDD อัตโนมัติ
- **Parameters**: `channel` - ADC channel ที่ต้องการอ่าน
- **Return**: แรงดันที่ชดเชยแล้ว (V)
- **ข้อดี**: แม่นยำกว่า `ADC_ReadVoltage()` เมื่อ VDD เปลี่ยนแปลง

#### `ADC_GetBatteryPercent(vdd, v_min, v_max)`
คำนวณเปอร์เซ็นต์แบตเตอรี่
- **Parameters**:
  - `vdd` - แรงดัน VDD ปัจจุบัน (V)
  - `v_min` - แรงดันต่ำสุดของแบต (V)
  - `v_max` - แรงดันสูงสุดของแบต (V)
- **Return**: เปอร์เซ็นต์ (0-100)
- **หมายเหตุ**: ผลลัพธ์จะถูก clamp ให้อยู่ในช่วง 0-100%

---

## ตัวอย่างการใช้งาน

ดูตัวอย่างเพิ่มเติมใน `Examples/SimpleADC_Examples.c`:

1. **Example_ADC_Single()** - อ่าน ADC ช่องเดียว
2. **Example_ADC_Multiple()** - อ่านหลายช่องพร้อมกัน
3. **Example_ADC_Average()** - อ่านแบบ average
4. **Example_ADC_VoltageMonitor()** - ตรวจสอบแรงดัน
5. **Example_ADC_Potentiometer()** - อ่านค่า potentiometer
6. **Example_ADC_Temperature()** - อ่านอุณหภูมิจาก LM35
7. **Example_ADC_LightSensor()** - อ่านความสว่างจาก LDR
8. **Example_ADC_InternalVref()** - อ่าน Vref และคำนวณ VDD ⭐ ใหม่!
9. **Example_ADC_BatteryMonitor()** - ตรวจสอบแบตเตอรี่ ⭐ ใหม่!

---

## Tips & Tricks

### 1. ลด Noise
```c
// ใช้ average แทนการอ่านครั้งเดียว
uint16_t stable_value = ADC_ReadAverage(ADC_CH_PD2, 10);
```

### 2. ประหยัดพลังงาน
```c
// ตรวจสอบแบตและลด clock เมื่อแบตต่ำ
float percent = ADC_GetBatteryPercent(ADC_GetVDD(), 3.0, 4.2);
if (percent < 20) {
    // ลด clock speed
    SystemCoreClockUpdate();
}
```

### 3. Calibration
```c
// ถ้าต้องการความแม่นยำสูงสุด ให้วัด Vrefint ที่ VDD = 3.3V
// แล้วเปลี่ยนค่า ADC_VREFINT_CAL ใน SimpleADC.h
```

---

## เวอร์ชัน

**v1.1** (2025-12-22)
- ✨ เพิ่มฟีเจอร์ Battery Monitoring
- ✨ เพิ่มการอ่าน Internal Vref
- ✨ เพิ่มฟังก์ชัน VDD Compensation
- 📝 เพิ่ม Example 8 และ 9

**v1.0** (2025-12-12)
- 🎉 เวอร์ชันแรก
- รองรับ 8 external channels
- API แบบ Arduino-style

---

## License

MIT License - ใช้งานได้อย่างอิสระ

## ผู้พัฒนา

SimpleHAL Team - CH32V003 Community
