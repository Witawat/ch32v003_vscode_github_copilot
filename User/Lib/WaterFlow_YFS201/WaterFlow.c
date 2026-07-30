/**
 * @file WaterFlow.c
 * @brief YF-S201 Water Flow Sensor Library Implementation
 * @version 1.0
 * @date 2026-05-01
 */

#include "WaterFlow.h"

/* ========== Private Globals ========== */

static WaterFlow_Instance* g_flow_instances[WATERFLOW_MAX_INSTANCES] = {NULL};
static uint8_t g_flow_count = 0;

/* ========== ISR Handlers ==========
 * attachInterrupt() dispatches per-EXTI-line, so each registered callback
 * already only fires for its own pin's edge. The previous single shared
 * ISR ignored that and incremented every instance whose pin happened to
 * read HIGH at the time, corrupting counts with 2+ sensors attached (see
 * LIB_AUDIT.md #11). A per-slot trampoline fixes this by only ever
 * touching the one instance registered to that pin. */
static void WaterFlow_Trampoline0(void) { if (g_flow_instances[0]) g_flow_instances[0]->pulse_count++; }
static void WaterFlow_Trampoline1(void) { if (g_flow_instances[1]) g_flow_instances[1]->pulse_count++; }
static void WaterFlow_Trampoline2(void) { if (g_flow_instances[2]) g_flow_instances[2]->pulse_count++; }
static void WaterFlow_Trampoline3(void) { if (g_flow_instances[3]) g_flow_instances[3]->pulse_count++; }

static void (*const g_flow_trampolines[WATERFLOW_MAX_INSTANCES])(void) = {
    WaterFlow_Trampoline0, WaterFlow_Trampoline1, WaterFlow_Trampoline2, WaterFlow_Trampoline3
};

/* ========== Public Functions ========== */

WaterFlow_Status WaterFlow_Init(WaterFlow_Instance* flow, uint8_t pin, float k_factor) {

    if (flow == NULL)     return WATERFLOW_ERROR_PARAM;
    if (k_factor <= 0.0f) return WATERFLOW_ERROR_PARAM;

    /* check max instances */
    if (g_flow_count >= WATERFLOW_MAX_INSTANCES) {
        return WATERFLOW_ERROR_FULL;
    }

    flow->pin          = pin;
    flow->k_factor     = k_factor;
    flow->pulse_count  = 0;
    flow->last_pulse   = 0;
    flow->last_time_ms = 0;
    flow->initialized  = 1;

    /* setup GPIO as input */
    pinMode(pin, PIN_MODE_INPUT_PULLUP);

    /* register instance, then attach its dedicated trampoline */
    uint8_t slot = g_flow_count++;
    g_flow_instances[slot] = flow;
    attachInterrupt(pin, g_flow_trampolines[slot], RISING);

    return WATERFLOW_OK;
}

uint32_t WaterFlow_GetPulseCount(WaterFlow_Instance* flow) {
    uint32_t count;

    if (flow == NULL || !flow->initialized) return 0;

    __disable_irq();
    count = flow->pulse_count;
    __enable_irq();

    return count;
}

float WaterFlow_GetFlowRate(WaterFlow_Instance* flow) {
    uint32_t current_pulses;
    uint32_t current_time_ms;
    uint32_t delta_pulses;
    uint32_t delta_ms;
    float flow_rate;

    if (flow == NULL || !flow->initialized) return 0.0f;

    current_time_ms = Get_CurrentMs();

    __disable_irq();
    current_pulses = flow->pulse_count;
    __enable_irq();

    /* first call — store baseline */
    if (flow->last_time_ms == 0) {
        flow->last_pulse   = current_pulses;
        flow->last_time_ms = current_time_ms;
        return 0.0f;
    }

    delta_pulses = current_pulses - flow->last_pulse;
    delta_ms     = current_time_ms - flow->last_time_ms;

    /* update baseline for next call */
    flow->last_pulse   = current_pulses;
    flow->last_time_ms = current_time_ms;

    if (delta_ms == 0 || delta_pulses == 0) return 0.0f;

    /* Calculate flow rate: (pulses / k_factor) / (ms / 60000) = L/min */
    flow_rate = ((float)delta_pulses / flow->k_factor) / ((float)delta_ms / 60000.0f);

    return flow_rate;
}

float WaterFlow_GetTotalVolume(WaterFlow_Instance* flow) {
    uint32_t pulses;

    if (flow == NULL || !flow->initialized) return 0.0f;

    __disable_irq();
    pulses = flow->pulse_count;
    __enable_irq();

    if (flow->k_factor <= 0.0f) return 0.0f;

    return (float)pulses / flow->k_factor;
}

void WaterFlow_Reset(WaterFlow_Instance* flow) {
    if (flow == NULL || !flow->initialized) return;

    __disable_irq();
    flow->pulse_count  = 0;
    __enable_irq();

    flow->last_pulse   = 0;
    flow->last_time_ms = 0;
}

void WaterFlow_SetKFactor(WaterFlow_Instance* flow, float k_factor) {
    if (flow == NULL || !flow->initialized) return;
    if (k_factor <= 0.0f) return;
    flow->k_factor = k_factor;
}
