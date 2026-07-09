# TJC T1 Series Design Guide

คู่มือการออกแบบ UI และเชื่อมต่อ CH32V003 กับ TJC T1 series

---

## สารบัญ

- [ภาพรวม TJC T1 Series](#ภาพรวม-tjc-t1-series)
- [การเชื่อมต่อฮาร์ดแวร์](#การเชื่อมต่อฮาร์ดแวร์)
- [การออกแบบ UI ใน TJC Editor](#การออกแบบ-ui-ใน-tjc-editor)
- [Component ที่ใช้บ่อย](#component-ที่ใช้บ่อย)
- [การตั้งค่า Event](#การตั้งค่า-event)
- [ตัวอย่างโปรเจกต์ TJC Editor](#ตัวอย่างโปรเจกต์-tjc-editor)
- [Tips สำหรับ T1](#tips-สำหรับ-t1)
- [ข้อแตกต่าง T1 vs รุ่นอื่น](#ข้อแตกต่าง-t1-vs-รุ่นอื่น)

---

## ภาพรวม TJC T1 Series

TJC T1 series เป็นจอ HMI แบบ **resistive touch** มีหลายขนาด ใช้งานผ่าน TJC Editor (ฟรี)

| รุ่น | ขนาด | ความละเอียด | Flash | RAM |
|------|------|-------------|-------|-----|
| T1 2.4" | 2.4" | 320x240 | 16MB | 2MB |
| T1 2.8" | 2.8" | 320x240 | 16MB | 2MB |
| T1 3.2" | 3.2" | 320x240 | 16MB | 2MB |
| T1 3.5" | 3.5" | 480x320 | 16MB | 4MB |
| T1 4.3" | 4.3" | 480x272 | 16MB | 4MB |
| T1 5.0" | 5.0" | 800x480 | 16MB | 4MB |
| T1 7.0" | 7.0" | 800x480 | 16MB | 4MB |

---

## การเชื่อมต่อฮาร์ดแวร์

### UART Connection

```
CH32V003          TJC T1
────────          ──────
PD5 (TX)  ──────  RX (Blue)
PD6 (RX)  ──────  TX (Green)
GND       ──────  GND (Black)
3.3V/5V   ──────  VCC (Red) *ตามสเปคจอ*
```

### Pin Configuration

| Config | TX | RX | หมายเหตุ |
|--------|----|----|----------|
| `USART_PINS_DEFAULT` | PD5 | PD6 | Default |
| `USART_PINS_REMAP1` | PD0 | PD1 | TSSOP-20/QFN-20 only |
| `USART_PINS_REMAP2` | PD6 | PD5 | สลับ TX/RX |

### Baud Rate

| ขนาดจอ | Baud Rate แนะนำ | เหตุผล |
|--------|-----------------|--------|
| 2.4" - 3.5" | 9600 - 115200 | จอเล็ก ข้อมูลน้อย |
| 4.3" - 7.0" | 115200 | จอใหญ่ ต้องการ bandwidth สูง |

---

## การออกแบบ UI ใน TJC Editor

### โครงสร้างโปรเจกต์

```
Project
├── Global Event
│   └── Startup Event: bkcmd=3, sendme
├── Page 0 (Main)
│   ├── Components
│   └── Events
├── Page 1 (Settings)
│   ├── Components
│   └── Events
└── Page 2 (About)
    └── ...
```

### ตัวอย่าง Layout สำหรับ T1 3.5"

```
┌─────────────────────────────────┐
│  t_title: "Sensor Monitor"      │  ← Text (24px, bold)
├─────────────────────────────────┤
│                                 │
│   Temperature: 28.5°C           │  ← t_temp (text)
│   Humidity:    65%              │  ← t_hum (text)
│   Pressure: 1013.2 hPa          │  ← t_press (text)
│                                 │
│   ████████████░░░░  75%         │  ← j_progress
│                                 │
├─────────────────────────────────┤
│  [ LED ON ]    [ Settings ]     │  ← b_led, b_settings (40x40)
│  [ Refresh  ]    [ Calibrate ]  │  ← b_refresh, b_cal
└─────────────────────────────────┘
```

### ขนาด Component แนะนำ

| Component | ขนาดต่ำสุด | หมายเหตุ |
|-----------|-----------|----------|
| Button | 40x40 pixels | นิ้วกดง่าย |
| Slider | 200x40 pixels | เลื่อนง่าย |
| Text | auto | Font 24+ |
| Number | auto | Font 24+ |
| Progress | 200x20 pixels | อ่านค่าง่าย |

---

## Component ที่ใช้บ่อย

### Button

ใช้ส่งคำสั่งไป MCU เมื่อกด

**TJC Editor — Touch Press Event:**
```
prints "led|1;"
```

**MCU:**
```c
void OnTJCCommand(TJC_ReceivedCommand_t *cmd) {
    if (strcmp(cmd->command, "led") == 0) {
        // เปิด LED
    }
}
```

### Slider

ใช้ส่งค่า analog ไป MCU

**TJC Editor — Value Change Event:**
```
prints "pwm|"
prints h0.val,0
prints ";"
```

**MCU:**
```c
void OnTJCCommand(TJC_ReceivedCommand_t *cmd) {
    if (strcmp(cmd->command, "pwm") == 0) {
        uint16_t duty = atoi(cmd->params[0]);
        PWM_SetDutyCycle(PWM1_CH1, duty);
    }
}
```

### Dual-state Button

ปุ่มสลับ ON/OFF

**TJC Editor — Touch Release Event:**
```
if(b0.val==1)
{
  prints "led|1;"
}
else
{
  prints "led|0;"
}
```

### Number (แสดงค่าจาก MCU)

**MCU ส่งค่าไป:**
```c
TJC_SendCommand("n0.val=42");
```

**MCU ขอค่ากลับ:**
```c
TJC_SendCommand("get n0.val");  // → OnNumeric callback
```

### Text (แสดงข้อความจาก MCU)

**MCU ส่งค่าไป:**
```c
TJC_SendCommand("t0.txt=\"Hello TJC\"");
```

**MCU ขอค่ากลับ:**
```c
TJC_SendCommand("get t0.txt");  // → OnString callback
```

### Progress Bar

**MCU ส่งค่าไป:**
```c
TJC_SendCommand("j0.val=75");  // 0-100
```

---

## การตั้งค่า Event

### Global Startup Event (สำคัญ!)

ทุกโปรเจกต์ต้องตั้งค่าใน **Page 0 → Global Event → Startup Event**:

```
bkcmd=3
sendme
```

| คำสั่ง | คำอธิบาย |
|--------|----------|
| `bkcmd=0` | ปิด response (production) |
| `bkcmd=1` | ส่งเฉพาะ success |
| `bkcmd=3` | ส่งทุกคำสั่ง (debug) |
| `sendme` | ส่ง page ID ปัจจุบันตอน startup |

### Touch Event (ปุ่ม)

**Touch Press Event:**
```
prints "cmd|param;"
```

**Touch Release Event:**
```
// ใช้กับ dual-state button
if(b0.val==1)
{
  prints "led|1;"
}
else
{
  prints "led|0;"
}
```

### Page Navigation

**ไปหน้าอื่น:**
```
page 1
```

**กลับหน้าหลัก:**
```
page 0
```

### Timer Event

**ทุก 1 วินาที:**
```
tm0.en=1
tm0.tim=1000
```

**Timer Event:**
```
prints "tick;"
```

---

## ตัวอย่างโปรเจกต์ TJC Editor

### Sensor Monitor (3 Pages)

```
Page 0: Main Dashboard
├── t_title: "Sensor Monitor"
├── t_temp: "28.5°C"
├── t_hum: "65%"
├── t_press: "1013 hPa"
├── j_progress: 75
├── b_refresh: [Refresh] → prints "refresh;"
└── b_settings: [Settings] → page 1

Page 1: Settings
├── t_title: "Settings"
├── h_brightness: Slider → prints "dim|" + h_brightness.val + ";"
├── b_led: [LED ON/OFF] → prints "led|toggle;"
├── b_back: [Back] → page 0
└── b_cal: [Calibrate] → prints "cal;"

Page 2: About
├── t_title: "About"
├── t_info: "TJC T1 + CH32V003"
├── t_version: "v1.0"
└── b_back: [Back] → page 0
```

### MCU Code

```c
void OnTJCCommand(TJC_ReceivedCommand_t *cmd) {
    if (strcmp(cmd->command, "refresh") == 0) {
        // อ่าน sensor แล้วส่งไปจอ
        TJC_SendCommand("t_temp.txt=\"28.5C\"");
        TJC_SendCommand("t_hum.txt=\"65%\"");
        TJC_SendCommand("j0.val=75");
    }
    else if (strcmp(cmd->command, "led") == 0) {
        if (strcmp(cmd->params[0], "toggle") == 0) {
            led_state = !led_state;
            digitalWrite(LED_PIN, led_state);
        }
    }
    else if (strcmp(cmd->command, "dim") == 0) {
        uint8_t brightness = atoi(cmd->params[0]);
        char buf[32];
        snprintf(buf, sizeof(buf), "dim=%d", brightness);
        TJC_SendCommand(buf);
    }
}
```

---

## Tips สำหรับ T1

### การออกแบบ UI

1. **ขนาดปุ่ม** — อย่างน้อย 40x40 pixels (นิ้วกดง่าย)
2. **ระยะห่าง** — เว้นอย่างน้อย 10 pixels ระหว่างปุ่ม
3. **สี** — ใช้สีตัดกันชัดเจน (T1 สีไม่สวยเท่า capacitive)
4. **Font** — ใช้ font ขนาด 24+ สำหรับข้อความที่อ่านง่าย
5. **Background** — ใช้สีทึบ ไม่ใช้ gradient (T1 render ช้า)

### Performance

1. **หลีกเลี่ยง** — ภาพใหญ่, animation ซับซ้อน
2. **ใช้** — ภาพ PNG ที่ optimize แล้ว
3. **จำกัด** — จำนวน component ต่อหน้า (ไม่เกิน 20)
4. **ใช้** — page navigation แทน scroll

### Touch Calibration

T1 เป็น resistive touch อาจต้อง calibrate:

```
// ใน TJC Editor → Tools → Touch Calibration
// หรือส่งคำสั่งจาก MCU
TJC_SendCommand("calibrate");
```

### Debug

1. เปิด `bkcmd=3` ใน Global Startup Event
2. ใช้ Serial Monitor ดู response จาก TJC
3. ใช้ `TJC_GetErrorString()` แปลง error code

---

## ข้อแตกต่าง T1 vs รุ่นอื่น

| คุณสมบัติ | T1 | X1 | K1 |
|-----------|-----|-----|-----|
| **Touch Type** | Resistive | Capacitive | Capacitive |
| **Multi-touch** | ไม่รองรับ | รองรับ | รองรับ |
| **ความไว** | ปานกลาง | สูง | สูง |
| **ราคา** | ถูก | แพง | ปานกลาง |
| **ขนาด** | 2.4"-7" | 3.5"-10.1" | 2.4"-7" |
| **การใช้งาน** | นิ้ว/stylus | นิ้ว | นิ้ว |
| **ความทนทาน** | ปานกลาง | สูง | สูง |
| **เหมาะสำหรับ** | โปรเจกต์ทั่วไป | Consumer product | โปรเจกต์ประหยัด |

### MCU Code

**ใช้ library เดียวกัน** — ไม่ต่างกันที่โค้ด MCU ต่างกันที่ TJC Editor project file

```c
// ใช้ได้กับทุกตระกูล TJC
TJC_Init(BAUD_115200, USART_PINS_DEFAULT);
TJC_SendCommand("page 0");
TJC_RegisterCommandCallback(OnTJCCommand);
```

---

## ไฟล์ตัวอย่างที่เกี่ยวข้อง

| ไฟล์ | คำอธิบาย |
|------|----------|
| `ex01_Basic_Command.c` | ส่งคำสั่งพื้นฐาน |
| `ex02_Touch_Event.c` | รับ touch event จากจอสัมผัส |
| `ex06_Custom_Command.c` | TJC สั่ง MCU (ปุ่มบนจอ) |
| `ex08_Physical_Button_Control.c` | ใช้ปุ่มกดภายนอก (ไม่ใช้จอสัมผัส) |

---

## แหล่งข้อมูลเพิ่มเติม

- [TJC Editor Download](https://tjc.com.cn) — ซอฟต์แวร์ออกแบบ UI
- [TJC Instruction Set](https://tjc.com.cn/document) — เอกสารคำสั่งทั้งหมด
- [TJC T1 Datasheet](https://tjc.com.cn) — สเปคฮาร์ดแวร์
- `TJC.h` — API reference ทั้งหมด
- `README.md` — ตัวอย่างโค้ด MCU ทุกรูปแบบ
