/**
 * @file SimpleScheduler.c
 * @brief Cooperative Task Scheduler Implementation
 */

#include "SimpleScheduler.h"
#include "../../SimpleHAL/SimpleHAL.h"

static Scheduler_Task _tasks[SCHEDULER_MAX_TASKS];
static uint8_t        _task_count = 0;

void Scheduler_Init(void) {
    for (uint8_t i = 0; i < SCHEDULER_MAX_TASKS; i++) {
        _tasks[i].function     = NULL;
        _tasks[i].interval_ms  = 0;
        _tasks[i].last_run_ms  = 0;
        _tasks[i].enabled      = false;
    }
    _task_count = 0;
}

Scheduler_Status Scheduler_AddTask(Scheduler_TaskFunc func, uint32_t interval_ms) {
    if (func == NULL || interval_ms == 0) return SCHEDULER_ERROR_PARAM;
    if (_task_count >= SCHEDULER_MAX_TASKS) return SCHEDULER_ERROR_FULL;

    _tasks[_task_count].function    = func;
    _tasks[_task_count].interval_ms = interval_ms;
    _tasks[_task_count].last_run_ms = Get_CurrentMs();
    _tasks[_task_count].enabled     = true;
    _task_count++;

    return SCHEDULER_OK;
}

Scheduler_Status Scheduler_RemoveTask(uint8_t id) {
    if (id >= _task_count) return SCHEDULER_ERROR_PARAM;

    for (uint8_t i = id; i < _task_count - 1; i++) {
        _tasks[i] = _tasks[i + 1];
    }
    _task_count--;

    return SCHEDULER_OK;
}

void Scheduler_Run(void) {
    uint32_t now = Get_CurrentMs();

    for (uint8_t i = 0; i < _task_count; i++) {
        if (!_tasks[i].enabled || _tasks[i].function == NULL) continue;

        uint32_t elapsed = now - _tasks[i].last_run_ms;

        if (elapsed >= _tasks[i].interval_ms) {
            _tasks[i].last_run_ms = now;
            _tasks[i].function();
        }
    }
}

Scheduler_Status Scheduler_SetInterval(uint8_t id, uint32_t interval_ms) {
    if (id >= _task_count) return SCHEDULER_ERROR_PARAM;

    _tasks[id].interval_ms = interval_ms;
    return SCHEDULER_OK;
}

Scheduler_Status Scheduler_EnableTask(uint8_t id) {
    if (id >= _task_count) return SCHEDULER_ERROR_PARAM;

    _tasks[id].enabled = true;
    _tasks[id].last_run_ms = Get_CurrentMs();
    return SCHEDULER_OK;
}

Scheduler_Status Scheduler_DisableTask(uint8_t id) {
    if (id >= _task_count) return SCHEDULER_ERROR_PARAM;

    _tasks[id].enabled = false;
    return SCHEDULER_OK;
}

uint8_t Scheduler_GetTaskCount(void) {
    return _task_count;
}
