# 16_TJC_HMI_Display — TJC HMI Display Examples

ตัวอย่างการใช้งาน TJC HMI Display Library กับ CH32V003

> ✅ **v2.1 — แก้ conflict กับ SimpleUSART แล้ว ไม่ต้องเพิ่ม IRQ handler เองอีกต่อไป:**
> `SimpleUSART.c` เป็นเจ้าของ `USART1_IRQHandler()` แต่ต่อนี้จะเรียก weak hook
> `USART_RxByteHook()` ให้ทุกครั้งที่ได้รับ byte ใหม่ — `TJC.c` override hook นี้เพื่อรับข้อมูล
> เข้า `rx_buffer` ของตัวเองโดยอัตโนมัติ ไม่ต้องเขียน `USART1_IRQHandler()` เองอีกแล้ว
>
> ⛔ **ห้ามเพิ่ม `USART1_IRQHandler()` ของตัวเองที่เรียก `TJC_UART_IRQHandler()`** (ตามที่เอกสาร
> รุ่นเก่าเคยแนะนำ) — จะได้ linker error "multiple definition of USART1_IRQHandler" ทันที
> เพราะ `SimpleUSART.c` define ไว้แล้ว `TJC_UART_IRQHandler()` ถูกเก็บไว้เพื่อ backward
> compatibility เท่านั้น (deprecated) ตัวอย่างทั้ง 8 ไฟล์ในโฟลเดอร์นี้ใช้งานได้ปกติแล้วโดยไม่ต้อง
> แก้ไขอะไรเพิ่ม

## เอกสารเพิ่มเติม

| ไฟล์ | คำอธิบาย |
|------|----------|
| `TJC_T1_Design_Guide.md` | คู่มือออกแบบ UI สำหรับ TJC T1 series (resistive touch) |

## ไฟล์ในโฟลเดอร์

| ไฟล์ | คำอธิบาย | Callback ที่ใช้ |
|------|----------|-----------------|
| `ex01_Basic_Command.c` | ส่งคำสั่งพื้นฐาน (page, text, number, progress bar, visibility, click, dim) | Error |
| `ex02_Touch_Event.c` | รับ touch event (Press/Release) ควบคุม LED | Touch Event |
| `ex03_Touch_Coordinate.c` | รับพิกัด XY การสัมผัส | Touch Coordinate |
| `ex04_Numeric_String_Callback.c` | รับ numeric/string data จาก `get` command + Page ID | Numeric, String, Page ID |
| `ex05_System_Event.c` | รับ startup/sleep/wake event + error handling | System Event, Error |
| `ex06_Custom_Command.c` | TJC สั่ง MCU (LED, Relay, PWM, goto, status) | Command |
| `ex07_Full_Features.c` | ตัวอย่างเต็มใช้ทุก callback + sensor (DHT + BMP280) | All callbacks |
| `ex08_Physical_Button_Control.c` | ใช้ปุ่มกดภายนอกควบคุม TJC (ไม่ใช้จอสัมผัส) | System Event, Error |

## การติดตั้ง

### 1. เพิ่มไฟล์เข้า build

```makefile
# ใน obj/sources.mk หรือ makefile
User/Lib/TJC/TJC.c
```

### 2. ตั้งค่า TJC Editor

```
// Global Initialization Event
bkcmd=3    // ส่ง response ทุกคำสั่ง
sendxy=1   // ส่งพิกัดเมื่อสัมผัส (ถ้าต้องการ)
thsp=30    // Sleep หลัง 30 วินาทีไม่มีการสัมผัส
```

## Pin Configuration

| Enum | TX | RX | หมายเหตุ |
|------|----|----|----------|
| `USART_PINS_DEFAULT` | PD5 | PD6 | Default |
| `USART_PINS_REMAP1` | PD0 | PD1 | TSSOP-20/QFN-20 only |
| `USART_PINS_REMAP2` | PD6 | PD5 | สลับ TX/RX |

## Callback ทั้ง 7 ประเภท

| Callback | Trigger | ข้อมูลที่ได้รับ | ตัวอย่าง |
|----------|---------|-----------------|----------|
| `TJC_RegisterErrorCallback` | Response ทุกคำสั่ง | error_code | ex01, ex05, ex07, ex08 |
| `TJC_RegisterTouchEventCallback` | กด/ปล่อย component | page_id, component_id, event_type | ex02, ex07 |
| `TJC_RegisterTouchCoordCallback` | สัมผัสหน้าจอ (sendxy=1) | x, y, event_type | ex03 |
| `TJC_RegisterNumericCallback` | `get n0.val` | uint32_t value | ex04, ex07 |
| `TJC_RegisterStringCallback` | `get t0.txt` | string, length | ex04, ex07 |
| `TJC_RegisterPageIdCallback` | เปลี่ยนหน้า / `sendme` | page_id | ex04 |
| `TJC_RegisterSystemEventCallback` | startup/sleep/wake | event_type | ex05, ex07, ex08 |
| `TJC_RegisterCommandCallback` | TJC ส่ง `prints "cmd\|param;"` | TJC_ReceivedCommand_t* | ex06, ex07 |

## ข้อควรจำ

- **ต้องเรียก `TJC_ProcessResponse()` ใน `while(1)` เสมอ**
- ทุกคำสั่งต้องจบด้วย `0xFF 0xFF 0xFF` — library จัดการให้อัตโนมัติ
- `TJC_Init()` ใช้ `USART_BaudRate` enum (เช่น `BAUD_115200`) ไม่ใช่ค่าตัวเลขตรง
- `TJC_Init()` ใช้ `USART_PinConfig` enum (เช่น `USART_PINS_DEFAULT`) ไม่ใช่ `TJC_PINS_*`

## ตัวอย่าง: Physical Button Control (ex08)

สำหรับโปรเจกต์ที่ไม่ใช้จอสัมผัส หรือต้องการ backup control ด้วยปุ่มกดภายนอก:

```c
#define BTN_UP    PC0
#define BTN_DOWN  PC1
#define BTN_LEFT  PC2
#define BTN_RIGHT PC3

pinMode(BTN_UP, PIN_MODE_INPUT_PULLUP);
pinMode(BTN_DOWN, PIN_MODE_INPUT_PULLUP);
pinMode(BTN_LEFT, PIN_MODE_INPUT_PULLUP);
pinMode(BTN_RIGHT, PIN_MODE_INPUT_PULLUP);

while (1) {
    TJC_ProcessResponse();

    if (digitalRead(BTN_UP) == 0) {
        counter++;
        char buf[32];
        snprintf(buf, sizeof(buf), "n0.val=%d", counter);
        TJC_SendCommand(buf);
        Delay_Ms(200);  // debounce
    }

    if (digitalRead(BTN_DOWN) == 0) {
        counter--;
        char buf[32];
        snprintf(buf, sizeof(buf), "n0.val=%d", counter);
        TJC_SendCommand(buf);
        Delay_Ms(200);
    }

    if (digitalRead(BTN_LEFT) == 0) {
        TJC_SendCommand("page 0");
        Delay_Ms(200);
    }

    if (digitalRead(BTN_RIGHT) == 0) {
        TJC_SendCommand("page 1");
        Delay_Ms(200);
    }
}
```

ดู `ex08_Physical_Button_Control.c` สำหรับตัวอย่างเต็มที่มี debounce และ repeat function
