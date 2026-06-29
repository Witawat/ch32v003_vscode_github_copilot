# SimpleGPIO — คู่มือการใช้งาน

> **Version:** 1.3 | **MCU:** CH32V003 | **File:** `SimpleGPIO.h / SimpleGPIO.c`

---

## ภาพรวม

SimpleGPIO ให้ API แบบ Arduino-style สำหรับควบคุม GPIO ของ CH32V003 ครอบคลุมตั้งแต่การเขียน/อ่าน digital ไปจนถึง External Interrupt และการควบคุม Port ทั้ง byte พร้อมกัน

---

## GPIO Pins ของ CH32V003

| Pin | Enum Value | คุณสมบัติพิเศษ | SOP-8 |
|-----|-----------|----------------|:---:|
| PA1 | 0  | PWM1_CH2, ADC_CH1 | ❌ |
| PA2 | 1  | ADC_CH0 | ⚠️ |
| PC0 | 10 | PWM2_CH3 | ❌ |
| PC1 | 11 | I2C SDA (default) | ⚠️ |
| PC2 | 12 | I2C SCL (default) | ⚠️ |
| PC3 | 13 | PWM1_CH3 | ❌ |
| PC4 | 14 | PWM1_CH4, ADC_CH2 | ❌ |
| PC5 | 15 | SPI SCK (default) | ❌ |
| PC6 | 16 | SPI MOSI (default) | ❌ |
| PC7 | 17 | SPI MISO (default) | ❌ |
| PD0 | 18 | SPI NSS remap, USART TX remap, I2C SCL remap | ❌ |
| PD1 | 19 | SWDIO, SPI SCK remap, USART RX remap, I2C SDA remap | ✅ |
| PD2 | 20 | PWM1_CH1, ADC_CH3 | ❌ |
| PD3 | 21 | PWM2_CH2, ADC_CH4 | ❌ |
| PD4 | 22 | PWM2_CH1, ADC_CH7 | ✅ |
| PD5 | 23 | ADC_CH5, USART TX (default) | ✅ |
| PD6 | 24 | ADC_CH6, USART RX (default) | ✅ |
| PD7 | 25 | PWM2_CH4 (ไม่รองรับ ADC!) | ❌ |

> ⚠️ **PD0** ไม่มีใน SOP-8/SOP-16 — `pinMode()` จะคืนค่าเงียบ  
> ⚠️ **PD7** ไม่รองรับ ADC — ใช้ `IS_ADC_PIN()` ตรวจสอบก่อน  
> ⚠️ **SOP-8** มี 6 user pins: PD1, PD4, PD5, PD6, PC1, PC2

### การตรวจสอบ Pin ตามแพ็กเกจ

```c
#define CH32V003_PACKAGE  PACKAGE_SOP8  // หรือ SOP16, TSSOP20, QFN20
#include "SimpleHAL.h"

// Compile-time check — pin ที่ไม่มีในแพ็กเกจจะคืน false
if (IS_VALID_PIN(PD0)) {
    pinMode(PD0, PIN_MODE_OUTPUT);  // SOP-8/SOP-16: ข้ามบรรทัดนี้
}
```

---

## Pin Modes

```c
typedef enum {
    PIN_MODE_INPUT,           // Input floating
    PIN_MODE_OUTPUT,          // Output push-pull
    PIN_MODE_INPUT_PULLUP,    // Input + pull-up (ปุ่ม active-low)
    PIN_MODE_INPUT_PULLDOWN,  // Input + pull-down (ปุ่ม active-high)
    PIN_MODE_OUTPUT_OD        // Output open-drain (I2C-style, buzzer)
} GPIO_PinMode;
```

---

## API Reference

### ฟังก์ชันพื้นฐาน

#### `void pinMode(uint8_t pin, GPIO_PinMode mode)`

ตั้งค่าโหมดของ pin — **ต้องเรียกก่อนใช้งานทุกครั้ง**

```c
pinMode(PC0, PIN_MODE_OUTPUT);         // LED output push-pull
pinMode(PC1, PIN_MODE_INPUT_PULLUP);   // ปุ่ม active-low
pinMode(PD2, PIN_MODE_INPUT);          // Input floating
pinMode(PD3, PIN_MODE_OUTPUT_OD);      // Open-drain (buzzer / I2C-style)
pinMode(PD4, PIN_MODE_INPUT_PULLDOWN); // Input พร้อม pull-down
```

#### `void digitalWrite(uint8_t pin, uint8_t value)`

เขียนค่า HIGH/LOW

```c
digitalWrite(PC0, HIGH);  // เปิด LED
digitalWrite(PC0, LOW);   // ปิด LED
```

#### `uint8_t digitalRead(uint8_t pin)`

อ่านสถานะ digital

```c
uint8_t btn = digitalRead(PC1);
if (btn == LOW) {
    // ปุ่มถูกกด (active-low)
}
```

#### `void digitalToggle(uint8_t pin)`

สลับสถานะ HIGH ↔ LOW

```c
while (1) {
    digitalToggle(PC0);
    Delay_Ms(500);
}
```

---

### ฟังก์ชัน Multi-Pin

#### `pinModeMultiple(array, mode)` — Macro

ตั้งโหมดหลาย pin พร้อมกัน ไม่ต้องระบุจำนวน

```c
const uint8_t leds[] = {PC0, PC3, PC4};
pinModeMultiple(leds, PIN_MODE_OUTPUT);

const uint8_t btns[] = {PD2, PD3, PD4};
pinModeMultiple(btns, PIN_MODE_INPUT_PULLUP);
```

> **หมายเหตุ:** ต้องประกาศ array เป็น `const uint8_t arr[] = {...}` เพื่อให้ macro คำนวณ size ได้ถูกต้อง

#### `digitalWriteMultiple(array, values)` — Macro

เขียนค่าหลาย pin พร้อมกัน ไม่ต้องระบุจำนวน

```c
const uint8_t leds[] = {PC0, PC1, PC2};
const uint8_t states[] = {HIGH, LOW, HIGH};
digitalWriteMultiple(leds, states);
```

---

### Port-level I/O (ขั้นสูง)

#### `void portWrite(GPIO_TypeDef* port, uint8_t value)`

เขียน 8-bit ทั้ง port พร้อมกัน — เร็วกว่าเขียนทีละ pin มาก

```c
portWrite(GPIOC, 0xFF);             // PC0-PC7 = HIGH ทุกตัว
portWrite(GPIOC, 0x00);             // PC0-PC7 = LOW ทุกตัว
portWrite(GPIOC, 0b00001010);       // เฉพาะ PC1 และ PC3 = HIGH
portWrite(GPIOD, 1 << (PD5 - 20)); // เปิด PD5 เพียงตัวเดียว
```

#### `uint8_t portRead(GPIO_TypeDef* port)`

อ่านค่าทั้ง port ในครั้งเดียว

```c
uint8_t val  = portRead(GPIOD);     // อ่าน PD0-PD7 พร้อมกัน
uint8_t bit3 = (val >> 3) & 1;      // ตรวจ PD3 เฉพาะ
```

---

### External Interrupt

#### `void attachInterrupt(uint8_t pin, void (*callback)(void), GPIO_InterruptMode mode)`

| Mode | ความหมาย |
|------|----------|
| `RISING`  | ขอบขาขึ้น LOW → HIGH |
| `FALLING` | ขอบขาลง HIGH → LOW |
| `CHANGE`  | ทั้งสองขอบ |

```c
volatile uint8_t flag = 0;

void button_isr(void) {
    flag = 1;  // ISR ต้องสั้น อย่าทำงานหนักใน ISR
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    pinMode(PC1, PIN_MODE_INPUT_PULLUP);
    attachInterrupt(PC1, button_isr, FALLING);

    while (1) {
        if (flag) {
            flag = 0;
            digitalToggle(PC0);
        }
    }
}
```

#### `void detachInterrupt(uint8_t pin)`

ปิด interrupt

```c
detachInterrupt(PC1);
```

---

## ตัวอย่างการใช้งาน

### ขั้นต้น — LED Blink

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    pinMode(PC0, PIN_MODE_OUTPUT);
    while (1) {
        digitalToggle(PC0);
        Delay_Ms(500);
    }
}
```

### ขั้นกลาง — ปุ่มควบคุม LED (polling)

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    pinMode(PC0, PIN_MODE_OUTPUT);
    pinMode(PC1, PIN_MODE_INPUT_PULLUP);

    while (1) {
        if (digitalRead(PC1) == LOW) {
            digitalWrite(PC0, HIGH);
        } else {
            digitalWrite(PC0, LOW);
        }
    }
}
```

### ขั้นสูง — Interrupt-driven พร้อม Software Debounce

```c
#include "SimpleHAL.h"

volatile uint8_t btn_flag = 0;
uint32_t last_press_ms = 0;

void btn_isr(void) {
    uint32_t now = Get_CurrentMs();
    if (now - last_press_ms > 50) {   // debounce 50ms
        btn_flag = 1;
        last_press_ms = now;
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    pinMode(PC0, PIN_MODE_OUTPUT);
    pinMode(PC1, PIN_MODE_INPUT_PULLUP);
    attachInterrupt(PC1, btn_isr, FALLING);

    while (1) {
        if (btn_flag) {
            btn_flag = 0;
            digitalToggle(PC0);
        }
    }
}
```

### ขั้นสูง — Bit-bang Shift Register ด้วย port write

```c
#include "SimpleHAL.h"

#define PIN_LATCH  PC3
#define PIN_CLK    PC4
#define PIN_DATA   PC5

void shift_out_byte(uint8_t val) {
    for (int8_t i = 7; i >= 0; i--) {
        digitalWrite(PIN_DATA, (val >> i) & 1);
        digitalWrite(PIN_CLK, HIGH);
        Delay_Us(1);
        digitalWrite(PIN_CLK, LOW);
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    const uint8_t pins[] = {PIN_LATCH, PIN_CLK, PIN_DATA};
    pinModeMultiple(pins, PIN_MODE_OUTPUT);

    const uint8_t patterns[] = {0x01, 0x03, 0x07, 0x0F,
                                  0x1F, 0x3F, 0x7F, 0xFF};
    uint8_t idx = 0;
    while (1) {
        digitalWrite(PIN_LATCH, LOW);
        shift_out_byte(patterns[idx]);
        digitalWrite(PIN_LATCH, HIGH);
        idx = (idx + 1) % 8;
        Delay_Ms(200);
    }
}
```

---

### Analog I/O (Arduino-style)

#### `uint16_t analogRead(uint8_t pin)` — Macro

อ่านค่า ADC (0-1023) จาก pin ที่รองรับ ADC
**Compile-time check** ด้วย `_Static_assert` — ใช้ pin ผิด → compile error

```c
uint16_t val = analogRead(PD2);   // OK — PD2 รองรับ ADC
// analogRead(PD7);                // ✗ Compile Error! PD7 ไม่ใช่ ADC
// analogRead(PC0);                // ✗ Compile Error! PC0 ไม่ใช่ ADC

float voltage = val * 3.3f / 1023.f;
```

**ADC Pins:** PA1, PA2, PC4, PD2, PD3, PD4, PD5, PD6

#### `void analogWrite(uint8_t pin, uint8_t value)` — Macro

สร้าง PWM output (0-255) บน pin ที่รองรับ PWM
**Compile-time check** เช่นกัน — ใช้ pin ผิด → compile error
Auto-init @1kHz ถ้ายังไม่เคย init

```c
analogWrite(PA1, 128);  // 50% duty @ 1kHz — OK, PA1 รองรับ PWM
// analogWrite(PA2, 128);  // ✗ Compile Error! PA2 ไม่ใช่ PWM
```

**PWM Pins:** PA1, PC0, PC3, PC4, PD2, PD3, PD4, PD7

#### `uint32_t pulseIn(uint8_t pin, uint8_t state, uint32_t timeout)`

วัดความกว้าง pulse (µs) — ใช้กับ Ultrasonic, IR receiver

```c
// HC-SR04 Ultrasonic
digitalWrite(TRIG, LOW); Delay_Us(2);
digitalWrite(TRIG, HIGH); Delay_Us(10);
digitalWrite(TRIG, LOW);
uint32_t duration = pulseIn(ECHO, HIGH, 30000);
float distance = duration * 0.034f / 2.f;
```

#### `void shiftOut(dataPin, clkPin, bitOrder, value)`

ส่ง 1 byte ผ่าน software SPI — ใช้กับ 74HC595, LED matrix

```c
#define DATA PC0
#define CLK  PC1
shiftOut(DATA, CLK, MSBFIRST, 0xFF);  // ส่ง 0b11111111
```

#### `uint8_t shiftIn(dataPin, clkPin, bitOrder)`

รับ 1 byte ผ่าน software SPI — ใช้กับ 74HC165, shift register

```c
uint8_t data = shiftIn(DATA, CLK, MSBFIRST);
```

### Validation Macros

| Macro | ใช้ตรวจสอบ |
|-------|-----------|
| `IS_VALID_PIN(pin)` | Pin มีจริงในแพ็กเกจไหม |
| `IS_ADC_PIN(pin)` | Pin รองรับ ADC ไหม |
| `IS_PWM_PIN(pin)` | Pin รองรับ PWM ไหม |

---

## ข้อควรระวัง

| ปัญหา | สาเหตุ | วิธีแก้ |
|-------|--------|---------|
| Pin ไม่ทำงาน | ลืมเรียก `pinMode()` | เรียก `pinMode()` ก่อนใช้ทุกครั้ง |
| Delay_Ms ไม่ทำงาน | ลืม `Timer_Init()` | เรียกต้น `main()` |
| ISR ทำงานช้า / crash | มี `Delay_Ms` ใน ISR | ใช้ flag แทน ห้ามเรียก delay ใน ISR |
| อ่านค่าผิดพลาดใน ISR | ลืม `volatile` | ประกาศตัวแปรที่แชร์กับ ISR เป็น `volatile` |
| ใช้ PA0 / PB* | ไม่มีใน CH32V003 | ใช้เฉพาะ PA1-PA2, PC0-PC7, PD2-PD7 |
