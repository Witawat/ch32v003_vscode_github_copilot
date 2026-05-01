# Relay Library

ไลบรารีควบคุมรีเลย์บน CH32V003 รองรับทั้งโมดูลแบบ Active High และ Active Low
เหมาะกับงานเปิดปิดโหลด เช่น ปั๊มน้ำ หลอดไฟ พัดลม หรือโซลินอยด์

## สารบัญ

1. [ภาพรวม](#ภาพรวม)
2. [คุณสมบัติ](#คุณสมบัติ)
3. [Hardware Setup](#hardware-setup)
4. [หลักการทำงาน](#หลักการทำงาน)
5. [การใช้งานพื้นฐาน](#การใช้งานพื้นฐาน)
6. [การใช้งานขั้นสูง](#การใช้งานขั้นสูง)
7. [Troubleshooting](#troubleshooting)
8. [API Reference](#api-reference)

---

## ภาพรวม

Relay module ส่วนใหญ่มีอินพุตตรรกะ 1 ขา และใช้ทรานซิสเตอร์/ออปโตคัปเปลอร์บนบอร์ด
แต่ตรรกะเปิดใช้งานอาจไม่เหมือนกัน

- Active Low: เขียน LOW แล้วรีเลย์ติด (พบบ่อยที่สุด)
- Active High: เขียน HIGH แล้วรีเลย์ติด

ไลบรารีนี้จึงมีการระบุ active level ตั้งแต่ตอน init เพื่อให้ API ใช้งานตรงความหมาย

- Relay_On = เปิดรีเลย์จริง
- Relay_Off = ปิดรีเลย์จริง

---

## คุณสมบัติ

- รองรับ Active High และ Active Low
- จัดการสถานะภายใน instance
- มี Toggle และ Set แบบตรงสถานะ
- มีฟังก์ชัน Relay_IsOn สำหรับเช็คสถานะล่าสุด

---

## Hardware Setup

### Wiring พื้นฐาน

| CH32V003 | Relay Module |
|---|---|
| GPIO pin (เช่น PC0) | IN |
| 3.3V หรือ 5V | VCC |
| GND | GND |

ข้อควรระวัง

- ตรวจว่าโมดูลรับอินพุต 3.3V ได้หรือไม่
- โหลด AC mains ต้องต่อผ่าน terminal relay อย่างถูกต้องและปลอดภัย
- งานไฟบ้านควรมีฟิวส์และฉนวนตามมาตรฐาน

---

## หลักการทำงาน

### Active level mapping

| โหมด | Relay_On เขียนค่า | Relay_Off เขียนค่า |
|---|---|---|
| RELAY_ACTIVE_HIGH | HIGH | LOW |
| RELAY_ACTIVE_LOW | LOW | HIGH |

ดังนั้นโค้ดชั้นบนไม่ต้องจำว่าโมดูลกลับตรรกะหรือไม่

---

## การใช้งานพื้นฐาน

### 1) Init

```c
#include "SimpleHAL.h"
#include "Relay.h"

Relay_Instance relay;

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();

    // โมดูลส่วนใหญ่เป็น active low
    Relay_Init(&relay, PC0, RELAY_ACTIVE_LOW);

    Relay_On(&relay);
    Delay_Ms(1000);
    Relay_Off(&relay);

    while (1) {
    }
}
```

### 2) ควบคุมแบบตรงสถานะ

```c
Relay_Set(&relay, 1); // ON
Delay_Ms(500);
Relay_Set(&relay, 0); // OFF
```

---

## การใช้งานขั้นสูง

### Toggle เป็นจังหวะทุก 1 วินาที

```c
while (1) {
    Relay_Toggle(&relay);
    Delay_Ms(1000);
}
```

### ใช้ร่วมกับเซนเซอร์

```c
if (temperature > 40.0f) {
    Relay_On(&relay);   // เปิดพัดลม
} else {
    Relay_Off(&relay);
}
```

### ตรวจสถานะก่อนสั่ง

```c
if (!Relay_IsOn(&relay)) {
    Relay_On(&relay);
}
```

---

## Troubleshooting

### รีเลย์ติดตลอดหรือดับตลอด

| สาเหตุ | วิธีแก้ |
|---|---|
| เลือก active level ผิด | เปลี่ยน RELAY_ACTIVE_HIGH/LOW ให้ตรงโมดูล |
| ไฟเลี้ยงโมดูลไม่พอ | ตรวจ VCC และกระแส |

### สั่งติดแล้วมีเสียงแต่โหลดไม่ทำงาน

| สาเหตุ | วิธีแก้ |
|---|---|
| ต่อขา COM/NO/NC ผิด | ตรวจ wiring หน้าโหลด |
| โหลดเกินสเปกหน้าสัมผัส | ใช้ relay/contactor ที่รองรับ |

### MCU รีเซ็ตตอนสวิตช์โหลด

| สาเหตุ | วิธีแก้ |
|---|---|
| มีสัญญาณรบกวนจากโหลดเหนี่ยวนำ | ใส่ snubber/varistor/flyback ตามชนิดโหลด |
| กราวด์หรือไฟเลี้ยงตก | ปรับ power routing และ decoupling |

---

## API Reference

### Types

- Relay_ActiveLevel
  - RELAY_ACTIVE_HIGH
  - RELAY_ACTIVE_LOW
- Relay_Status
  - RELAY_OK
  - RELAY_ERROR_PARAM
- Relay_Instance

### Functions

- Relay_Status Relay_Init(Relay_Instance* relay, uint8_t pin, Relay_ActiveLevel level)
  - เริ่มต้น relay instance และกำหนด active level

- void Relay_On(Relay_Instance* relay)
  - เปิด relay ตามตรรกะจริงของโมดูล

- void Relay_Off(Relay_Instance* relay)
  - ปิด relay ตามตรรกะจริงของโมดูล

- void Relay_Toggle(Relay_Instance* relay)
  - สลับสถานะ ON/OFF

- void Relay_Set(Relay_Instance* relay, uint8_t state)
  - กำหนดสถานะโดยตรง (1=ON, 0=OFF)

- uint8_t Relay_IsOn(Relay_Instance* relay)
  - คืนค่าสถานะล่าสุดของ relay (1=ON, 0=OFF)
