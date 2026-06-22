/**
 * @file SimpleScheduler.h
 * @brief Cooperative Task Scheduler Library สำหรับ CH32V003
 * @version 1.0
 * @date 2026-06-22
 *
 * @details
 * Library สำหรับจัดการ task แบบ cooperative (non-preemptive) round-robin
 * ใช้สำหรับทำงานหลายๆ งานบน single-core โดยไม่ต้องใช้ RTOS
 *
 * **หลักการทำงาน:**
 * - แต่ละ task มี interval (ms) ของตัวเอง
 * - Scheduler จะเรียก task เมื่อถึงเวลาที่กำหนด
 * - Task ต้องไม่บล็อกนานเกินไป (ต้อง return โดยเร็ว)
 * - ใช้ Get_CurrentMs() เป็น time base
 *
 * **คุณสมบัติ:**
 * - สูงสุด 8 tasks (ปรับได้)
 * - แต่ละ task มี interval เป็นอิสระ
 * - Enable/Disable task ได้
 * - เปลี่ยน interval ขณะรันได้
 *
 * @example
 * #include "SimpleScheduler.h"
 *
 * void led_task(void) { digitalToggle(PC0); }
 * void sensor_task(void) { read_sensor(); }
 *
 * int main(void) {
 *     Timer_Init();  // ต้อง init timer ก่อน
 *     Scheduler_Init();
 *
 *     Scheduler_AddTask(led_task, 500);     // 500ms
 *     Scheduler_AddTask(sensor_task, 1000); // 1000ms
 *
 *     while (1) {
 *         Scheduler_Run();
 *     }
 * }
 *
 * @note ต้องเรียก Timer_Init() ก่อนใช้งาน
 */

#ifndef __SIMPLE_SCHEDULER_H
#define __SIMPLE_SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ========== Configuration ========== */

#ifndef SCHEDULER_MAX_TASKS
#define SCHEDULER_MAX_TASKS  8
#endif

/* ========== Status ========== */

typedef enum {
    SCHEDULER_OK           = 0,
    SCHEDULER_ERROR_FULL   = 1,
    SCHEDULER_ERROR_PARAM  = 2,
    SCHEDULER_ERROR_NOTASK = 3
} Scheduler_Status;

/* ========== Types ========== */

typedef void (*Scheduler_TaskFunc)(void);

typedef struct {
    Scheduler_TaskFunc function;
    uint32_t           interval_ms;
    uint32_t           last_run_ms;
    bool               enabled;
} Scheduler_Task;

/* ========== Function Prototypes ========== */

void Scheduler_Init(void);

Scheduler_Status Scheduler_AddTask(Scheduler_TaskFunc func, uint32_t interval_ms);

Scheduler_Status Scheduler_RemoveTask(uint8_t id);

void Scheduler_Run(void);

Scheduler_Status Scheduler_SetInterval(uint8_t id, uint32_t interval_ms);

Scheduler_Status Scheduler_EnableTask(uint8_t id);

Scheduler_Status Scheduler_DisableTask(uint8_t id);

uint8_t Scheduler_GetTaskCount(void);

#ifdef __cplusplus
}
#endif

#endif /* __SIMPLE_SCHEDULER_H */
