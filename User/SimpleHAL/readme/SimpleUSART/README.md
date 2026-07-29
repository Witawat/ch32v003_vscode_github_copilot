# SimpleUSART — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `SimpleUSART.h / SimpleUSART.c`

---

## ภาพรวม

SimpleUSART ห่อหุ้ม USART1 hardware ให้ใช้งานง่ายแบบ Arduino Serial รองรับ TX/RX, print helpers, non-blocking receive check และ pin remapping 3 แบบ

> **v2.1:** `USART_SimpleInit()` เปิด RX interrupt + ring buffer อัตโนมัติแล้ว (ขนาดเริ่มต้น
> 64 bytes, ปรับได้ด้วย `#define USART_RX_BUFFER_SIZE <n>` ก่อน `#include "SimpleUSART.h"`)
> — เดิม hardware buffer มีแค่ 1 byte ถ้าโปรแกรมอ่านไม่ทันข้อมูลจะหาย ตอนนี้ไม่มีปัญหานี้แล้ว
>
> ⚠️ **ห้าม define `USART1_IRQHandler()` ซ้ำในโค้ดของคุณเอง** — `SimpleUSART.c` เป็นเจ้าของ
> ISR นี้แล้ว (มี ISR ได้แค่ตัวเดียวต่อ interrupt vector) แต่ถ้า library อื่นต้องการรับ byte
> ที่เข้ามาแบบ real-time (เช่น TJC HMI) ให้ override `USART_RxByteHook()` แทน — ดูด้านล่าง

---

## Pin Configuration

| Config | TX | RX | SOP-8/16 |
|--------|----|----|:---:|
| `USART_PINS_DEFAULT` | PD5 | PD6 | ✅ |
| `USART_PINS_REMAP1`  | PD0 | PD1 | ❌ ไม่มี PD0 |
| `USART_PINS_REMAP2`  | PD6 | PD5 | ✅ |
| `USART_PINS_FULL_REMAP` 🆕 | PD6 | PD5 | ✅ |

> ⚠️ `USART_PINS_REMAP1` ใช้ PD0 (มีเฉพาะ TSSOP-20/QFN-20) — บน SOP-8/SOP-16 ค่านี้ไม่มีใน enum → compile error

---

## Baud Rates

```c
BAUD_9600    BAUD_19200    BAUD_38400
BAUD_57600   BAUD_115200   BAUD_230400   BAUD_460800
```

---

## API Reference

### Initialization

#### `void USART_SimpleInit(USART_BaudRate baud, USART_PinConfig pin_config)`

```c
USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);  // PD5=TX, PD6=RX
USART_SimpleInit(BAUD_9600,   USART_PINS_REMAP1);   // PD0=TX, PD1=RX
```

Format: **8N1** (8 data bits, No parity, 1 stop bit), no flow control

> ต้องเรียก `SystemCoreClockUpdate()` ก่อนเสมอ เพราะใช้ system clock คำนวณ baud rate

---

### Transmit

#### `void USART_Print(const char* str)`

ส่ง null-terminated string

```c
USART_Print("Hello World!\r\n");
USART_Print("Value: ");
```

#### `void USART_WriteByte(uint8_t data)`

ส่ง 1 byte

```c
USART_WriteByte(0x55);
USART_WriteByte('\n');
```

#### `void USART_PrintNum(int32_t num)`

ส่งตัวเลขทศนิยม signed

```c
USART_PrintNum(12345);   // "12345"
USART_PrintNum(-999);    // "-999"
USART_PrintNum(0);       // "0"
```

#### `void USART_PrintHex(uint32_t num, uint8_t uppercase)`

ส่งเลขฐาน 16

```c
USART_PrintHex(0xFF, 1);    // "0xFF"
USART_PrintHex(255, 0);     // "0xff"
USART_PrintHex(0x1234, 1);  // "0x1234"
```

#### `void USART_Flush(void)`

ล้างข้อมูลค้างใน receive buffer (อ่านทิ้งจนหมด)

```c
while (USART_Available()) { (void)USART_Read(); }  // manual flush
USART_Flush();  // อ่านข้อมูล RX ค้างใน hardware buffer ทิ้ง
```

> ⚠️ ต่างจาก Arduino `Serial.flush()` (ที่รอ TX เสร็จ) — CH32V003 `USART_Flush()` ล้าง RX buffer แทน ให้ใช้ `USART_Print` ตามด้วย `Delay_Ms(1)` แทนการรอ TX

---

### RX Byte Hook (สำหรับ library อื่นที่ต้องการรับ byte แบบ real-time)

#### `void USART_RxByteHook(uint8_t data)` 🆕 (v2.1)

Weak function — เรียกจาก `USART1_IRQHandler()` ทุกครั้งที่ได้รับ byte ใหม่ (หลังเติมลง ring
buffer ของ SimpleUSART แล้ว) default ไม่ทำอะไร override ได้โดย define ฟังก์ชันชื่อเดียวกัน
แบบไม่ใส่ `weak` ในไฟล์ของคุณเอง

```c
// ในไฟล์ของคุณเอง (ไม่ต้องใส่ __attribute__((weak)))
void USART_RxByteHook(uint8_t data) {
    my_protocol_parser_feed(data);  // แอบดู byte ที่เข้ามาแบบ real-time
}
```

> ใช้แทนการเขียน `USART1_IRQHandler()` เอง (ซึ่งจะชนกับของ SimpleUSART) — `User/Lib/TJC/TJC.c`
> ใช้กลไกนี้เพื่อรับข้อมูลจาก TJC HMI display แบบ interrupt-driven

### Receive

#### `uint8_t USART_Available(void)`

ตรวจว่ามีข้อมูลรอรับใน **RX ring buffer** (เติมโดย interrupt อัตโนมัติ) — คืน `1` ถ้ามี

```c
if (USART_Available()) {
    uint8_t b = USART_Read();
}
```

#### `uint8_t USART_Read(void)`

อ่าน 1 byte จาก ring buffer **(blocking — รอจนกว่า ISR จะเติมข้อมูล)**

```c
uint8_t b = USART_Read();
```

#### `uint16_t USART_ReadBytes(uint8_t* buffer, uint16_t length)`

อ่านหลาย bytes จาก ring buffer (non-blocking — อ่านเฉพาะข้อมูลที่มีอยู่ ไม่รอ)

```c
uint8_t buf[64];
uint16_t count = USART_ReadBytes(buf, sizeof(buf));
// count = จำนวน byte ที่อ่านได้จริง
```

> **v2.1:** RX buffer เป็น ring buffer ขนาด `USART_RX_BUFFER_SIZE` (default 64) เติมโดย
> `USART1_IRQHandler()` อัตโนมัติ — ถ้า buffer เต็ม byte ใหม่ที่เข้ามาจะถูกทิ้ง (ไม่เขียนทับ
> ข้อมูลเก่าที่ยังไม่ได้อ่าน) ถ้าข้อมูลมาเร็ว/เยอะกว่าที่โปรแกรมอ่านทัน ให้เพิ่มขนาด buffer:
> ```c
> #define USART_RX_BUFFER_SIZE 256
> #include "SimpleHAL.h"
> ```

---

## ตัวอย่างการใช้งาน

### ขั้นต้น — Debug Print

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    USART_Print("System started\r\n");

    uint16_t cnt = 0;
    while (1) {
        USART_Print("cnt=");
        USART_PrintNum(cnt++);
        USART_Print("\r\n");
        Delay_Ms(1000);
    }
}
```

### ขั้นกลาง — Command Line Receiver

```c
#include "SimpleHAL.h"

#define CMD_BUF_SIZE 32
char cmd_buf[CMD_BUF_SIZE];
uint8_t cmd_len = 0;

void process_command(const char* cmd) {
    if (cmd[0] == 'L' && cmd[1] == 'O' && cmd[2] == 'N') {
        digitalWrite(PC0, HIGH);
        USART_Print("LED ON\r\n");
    } else if (cmd[0] == 'L' && cmd[1] == 'O' && cmd[2] == 'F') {
        digitalWrite(PC0, LOW);
        USART_Print("LED OFF\r\n");
    } else {
        USART_Print("Unknown command\r\n");
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(PC0, PIN_MODE_OUTPUT);
    USART_Print("Ready. Commands: LON, LOF\r\n");

    while (1) {
        if (USART_Available()) {
            char c = (char)USART_Read();
            if (c == '\n' || c == '\r') {
                if (cmd_len > 0) {
                    cmd_buf[cmd_len] = '\0';
                    process_command(cmd_buf);
                    cmd_len = 0;
                }
            } else if (cmd_len < CMD_BUF_SIZE - 1) {
                cmd_buf[cmd_len++] = c;
            }
        }
    }
}
```

### ขั้นสูง — Binary Protocol พร้อม Checksum

```c
#include "SimpleHAL.h"

// Frame: [0xAA][0x55][LEN][DATA...][XOR_CHK]
void send_frame(uint8_t* data, uint8_t len) {
    USART_WriteByte(0xAA);
    USART_WriteByte(0x55);
    USART_WriteByte(len);
    uint8_t chk = 0;
    for (uint8_t i = 0; i < len; i++) {
        USART_WriteByte(data[i]);
        chk ^= data[i];
    }
    USART_WriteByte(chk);
}

// รับ frame (blocking)
uint8_t recv_frame(uint8_t* buf, uint8_t* len_out) {
    if (USART_Read() != 0xAA) return 0;
    if (USART_Read() != 0x55) return 0;
    uint8_t len = USART_Read();
    uint8_t chk = 0;
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = USART_Read();
        chk ^= buf[i];
    }
    uint8_t recv_chk = USART_Read();
    *len_out = len;
    return (chk == recv_chk) ? 1 : 0;
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    uint8_t payload[] = {0x01, 0x02, 0x03};
    while (1) {
        send_frame(payload, sizeof(payload));
        Delay_Ms(500);
    }
}
```

### ขั้นสูง — RX Buffer ขนาดใหญ่ขึ้นสำหรับ Data Rate สูง

> ตั้งแต่ v2.1 ไม่ต้องเขียน ring buffer เองแล้ว — `USART_SimpleInit()` มี RX interrupt +
> ring buffer ในตัว (`USART_Available()`/`USART_Read()` อ่านจาก buffer นี้โดยตรง) ถ้า data
> rate สูงหรือโปรแกรมอ่านไม่ทันบ่อยๆ แค่เพิ่มขนาด buffer ก่อน include:

```c
#define USART_RX_BUFFER_SIZE 256   // default คือ 64
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_460800, USART_PINS_DEFAULT);

    while (1) {
        if (USART_Available()) {
            uint8_t b = USART_Read();  // ไม่มี byte หายแม้ main loop ช้า
        }
    }
}
```

---

## ข้อควรระวัง

> **⚡ v2.0:** Pin remapping เปิด AFIO clock อัตโนมัติ — USART/I2C/SPI remap ทุกแบบทำงานถูกต้อง  
> `Int32ToString` จัดการ INT32_MIN ได้ (ใช้ unsigned arithmetic)

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| Baud rate ผิด, ข้อมูลเสีย | ลืม `SystemCoreClockUpdate()` | เรียกเป็นบรรทัดแรกของ main |
| `USART_Read()` ค้าง | ไม่มีข้อมูลแต่เรียก Read | เช็ค `USART_Available()` ก่อน |
| ส่งข้อมูลไม่ครบ | ไม่รอ flush | เรียก `USART_Flush()` ถ้าสำคัญ |
| ใช้ USART2/USART3 | CH32V003 มีแค่ USART1 | ใช้ USART_PINS_REMAP เปลี่ยน pin แทน |
