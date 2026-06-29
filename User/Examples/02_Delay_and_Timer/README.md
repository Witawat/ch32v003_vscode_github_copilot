# 02_Delay_and_Timer — ตัวอย่าง Delay & Timer

## ไฟล์ในโฟลเดอร์

| ไฟล์ | คำอธิบาย |
|------|----------|
| `ex01_NonBlocking_Timer.c` | Timer แบบ non-blocking (Timer_t, Start_Timer, Is_Timer_Expired) |
| `ex02_Blocking_Delay.c` | Delay แบบ blocking (Delay_Ms, Delay_Us) |
| `ex03_Time_Reading.c` | อ่านเวลาปัจจุบัน (Get_CurrentMs, Get_CurrentUs) |
| `ex04_Timer_Interrupt.c` | Timer interrupt (TIM_SimpleInit, TIM_AttachInterrupt) |
| `ex05_Timer_Advanced.c` | Timer ขั้นสูง (TIM_AdvancedInit, TIM_SetPrescaler) |
| `ex06_Stopwatch.c` | Stopwatch จับเวลา (Stopwatch_Init/Start/GetTimeString) |
| `ex07_Countdown.c` | Countdown นับถอยหลัง (Countdown_Init/IsFinished/AlarmCallback) |

## ต้องทำเสมอ

```c
SystemCoreClockUpdate();  // อันดับแรก
Timer_Init();             // ต้องเรียกเองก่อนใช้ Delay/Timer
```

## Timer Resource Map

| Resource | ใช้โดย | หมายเหตุ |
|----------|--------|---------|
| SysTick | SimpleDelay | Timer_Init() |
| TIM1 | SimpleTIM หรือ SimplePWM | เลือกอย่างใดอย่างหนึ่ง |
| TIM2 | SimpleTIM, SimpleTIM_Ext หรือ SimplePWM | เลือกอย่างใดอย่างหนึ่ง |
