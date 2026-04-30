# SimpleUSART — คู่มือการใช้งาน

> **Version:** 1.0 | **MCU:** CH32V003 | **File:** `SimpleUSART.h / SimpleUSART.c`

---

## ภาพรวม

SimpleUSART ห่อหุ้ม USART1 hardware ให้ใช้งานง่ายแบบ Arduino Serial รองรับ TX/RX, print helpers, non-blocking receive check และ pin remapping 3 แบบ

---

## Pin Configuration

| Config | TX | RX |
|--------|----|----|
| `USART_PINS_DEFAULT` | PD5 | PD6 |
| `USART_PINS_REMAP1`  | PD0 | PD1 |
| `USART_PINS_REMAP2`  | PD6 | PD5 |

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

รอจนส่งข้อมูลทั้งหมดเสร็จ

```c
USART_Print("Goodbye\r\n");
USART_Flush();  // รอก่อนปิดระบบ / เข้า sleep
```

---

### Receive

#### `uint8_t USART_Available(void)`

ตรวจว่ามีข้อมูลรอรับ — คืน `1` ถ้ามี

```c
if (USART_Available()) {
    uint8_t b = USART_Read();
}
```

#### `uint8_t USART_Read(void)`

อ่าน 1 byte **(blocking — รอจนมีข้อมูล)**

```c
uint8_t b = USART_Read();
```

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

### ขั้นสูง — Circular RX Buffer (Non-blocking)

```c
#include "SimpleHAL.h"

#define RX_BUF  64
static uint8_t rx_buf[RX_BUF];
static uint8_t rx_head = 0, rx_tail = 0;

// เรียกใน main loop ทุก iteration
void rx_poll(void) {
    while (USART_Available()) {
        uint8_t next = (rx_head + 1) % RX_BUF;
        if (next != rx_tail) {
            rx_buf[rx_head] = USART_Read();
            rx_head = next;
        }
    }
}

uint8_t rx_has_data(void)   { return rx_head != rx_tail; }
uint8_t rx_get_byte(void) {
    uint8_t b = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1) % RX_BUF;
    return b;
}
```

---

## ข้อควรระวัง

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| Baud rate ผิด, ข้อมูลเสีย | ลืม `SystemCoreClockUpdate()` | เรียกเป็นบรรทัดแรกของ main |
| `USART_Read()` ค้าง | ไม่มีข้อมูลแต่เรียก Read | เช็ค `USART_Available()` ก่อน |
| ส่งข้อมูลไม่ครบ | ไม่รอ flush | เรียก `USART_Flush()` ถ้าสำคัญ |
| ใช้ USART2/USART3 | CH32V003 มีแค่ USART1 | ใช้ USART_PINS_REMAP เปลี่ยน pin แทน |
