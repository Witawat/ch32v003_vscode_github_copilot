# 15_Arduino_API — ตัวอย่าง Arduino API

## ไฟล์ในโฟลเดอร์

| ไฟล์ | คำอธิบาย |
|------|----------|
| `ex01_Basic_Aliases.c` | millis(), micros(), delay(), delayMicroseconds() |
| `ex02_Random_Numbers.c` | randomSeed(), _randomMax(), _randomRange() |
| `ex03_Interrupt_yield.c` | digitalPinToInterrupt(), attachInterrupt(), yield() |
| `ex04_Float_Print.c` | dtostrf(), USART_Println(), USART_PrintFloat() |
| `ex05_Advanced_Scheduler.c` | 3-task non-blocking scheduler (millis + yield) |

## Arduino Aliases

| Arduino | SimpleHAL |
|---------|-----------|
| `millis()` | `Get_CurrentMs()` |
| `micros()` | `Get_CurrentUs()` |
| `delay(ms)` | `Delay_Ms(ms)` |
| `delayMicroseconds(us)` | `Delay_Us(us)` |
| `interrupts()` | `__enable_irq()` |
| `noInterrupts()` | `__disable_irq()` |
| `analogRead(pin)` | `_analogRead_impl(pin)` |
| `analogWrite(pin, val)` | `_analogWrite_impl(pin, val)` |
