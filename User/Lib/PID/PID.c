/**
 * @file PID.c
 * @brief PID Controller Implementation
 */

#include "PID.h"
#include <stddef.h>

PID_Status PID_Init(PID_Controller* pid, float kp, float ki, float kd, float dt) {
    if (pid == NULL || dt <= 0.0f) return PID_ERROR;

    pid->kp        = kp;
    pid->ki        = ki;
    pid->kd        = kd;
    pid->dt        = dt;
    pid->setpoint  = 0.0f;
    pid->input     = 0.0f;
    pid->output    = 0.0f;
    pid->integral  = 0.0f;
    pid->prev_error = 0.0f;
    pid->out_min   = 0.0f;
    pid->out_max   = 0.0f;
    pid->mode      = PID_MODE_MANUAL;
    pid->direction = PID_DIRECT;

    return PID_OK;
}

PID_Status PID_SetTunings(PID_Controller* pid, float kp, float ki, float kd) {
    if (pid == NULL) return PID_ERROR;

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    if (pid->direction == PID_REVERSE) {
        pid->kp = -kp;
        pid->ki = -ki;
        pid->kd = -kd;
    }

    return PID_OK;
}

PID_Status PID_SetLimits(PID_Controller* pid, float min, float max) {
    if (pid == NULL || min >= max) return PID_ERROR;

    pid->out_min = min;
    pid->out_max = max;

    if (pid->output > max) pid->output = max;
    if (pid->output < min) pid->output = min;

    return PID_OK;
}

PID_Status PID_SetSetpoint(PID_Controller* pid, float setpoint) {
    if (pid == NULL) return PID_ERROR;
    pid->setpoint = setpoint;
    return PID_OK;
}

PID_Status PID_SetMode(PID_Controller* pid, uint8_t mode) {
    if (pid == NULL || mode > 1) return PID_ERROR;

    if (pid->mode == PID_MODE_MANUAL && mode == PID_MODE_AUTO) {
        pid->integral = 0.0f;
        pid->prev_error = 0.0f;
    }

    pid->mode = mode;
    return PID_OK;
}

PID_Status PID_SetDirection(PID_Controller* pid, uint8_t direction) {
    if (pid == NULL || direction > 1) return PID_ERROR;

    pid->direction = direction;
    PID_SetTunings(pid, pid->kp, pid->ki, pid->kd);

    return PID_OK;
}

PID_Status PID_Reset(PID_Controller* pid) {
    if (pid == NULL) return PID_ERROR;

    pid->integral   = 0.0f;
    pid->prev_error = 0.0f;
    pid->output     = 0.0f;

    return PID_OK;
}

static float _clamp(float value, float min, float max) {
    /* NaN compares false against min/max, so a NaN value would otherwise
     * pass through unclamped (see LIB_AUDIT.md #20) */
    if (value != value) return min;
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float PID_Compute(PID_Controller* pid, float input) {
    if (pid == NULL) return 0.0f;

    pid->input = input;

    if (pid->mode == PID_MODE_MANUAL) {
        return pid->output;
    }

    float error = pid->setpoint - input;

    pid->integral += error * pid->dt;
    if (pid->out_min != pid->out_max) {
        pid->integral = _clamp(pid->integral, pid->out_min, pid->out_max);
    }

    float derivative = (error - pid->prev_error) / pid->dt;
    pid->prev_error = error;

    float output = (pid->kp * error) + (pid->ki * pid->integral) + (pid->kd * derivative);

    if (pid->out_min != pid->out_max) {
        output = _clamp(output, pid->out_min, pid->out_max);
    }

    pid->output = output;
    return output;
}

float PID_GetOutput(PID_Controller* pid) {
    if (pid == NULL) return 0.0f;
    return pid->output;
}
