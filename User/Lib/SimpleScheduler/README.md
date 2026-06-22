# SimpleScheduler Library

> **Cooperative Task Scheduler สำหรับการทำงานหลาย Task แบบไม่ต้องใช้ RTOS บน CH32V003**

## 📋 สารบัญ

1. [ภาพรวม](#ภาพรวม)
2. [คุณสมบัติ](#คุณสมบัติ)
3. [การใช้งานพื้นฐาน](#การใช้งานพื้นฐาน)
4. [API Reference](#api-reference)

---

## ภาพรวม

SimpleScheduler เป็นตัวจัดตารางงานแบบ cooperative (non-preemptive) ให้ทำงานหลายๆ งานบน single-core ได้โดยไม่ต้องใช้ RTOS แต่ละ task มี interval เป็นอิสระต่อกัน

**ข้อควรระวัง:** Task ต้องไม่บล็อกนานเกิน interval ของตัวเอง เพราะจะทำให้ task อื่นช้าไปด้วย

---

## คุณสมบัติ

- ✅ สูงสุด 8 tasks (ปรับได้ที่ `SCHEDULER_MAX_TASKS`)
- ✅ แต่ละ task มี interval เป็นอิสระ
- ✅ Enable / Disable task ได้
- ✅ เปลี่ยน interval ขณะรันได้
- ✅ Cooperative multitasking

---

## การใช้งานพื้นฐาน

```c
#include "main.h"
#include "Lib/SimpleScheduler/SimpleScheduler.h"

void led_task(void) {
    digitalToggle(PC0);
}

void sensor_task(void) {
    printf("Sensor read...\r\n");
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    Scheduler_Init();
    Scheduler_AddTask(led_task, 500);
    Scheduler_AddTask(sensor_task, 1000);

    while (1) {
        Scheduler_Run();
    }
}
```

---

## API Reference

- `Scheduler_Init()` : เริ่มต้น scheduler
- `Scheduler_AddTask(func, interval_ms)` : เพิ่ม task
- `Scheduler_RemoveTask(id)` : ลบ task
- `Scheduler_Run()` : เรียกใน main loop เพื่อ dispatch tasks
- `Scheduler_SetInterval(id, interval_ms)` : เปลี่ยน interval
- `Scheduler_EnableTask(id)` : เปิดใช้งาน task
- `Scheduler_DisableTask(id)` : ปิดใช้งาน task
- `Scheduler_GetTaskCount()` : จำนวน task ปัจจุบัน

---
**พัฒนาโดย:** CH32V003 Library Team
**รองรับบอร์ด:** CH32V003 Development Board
