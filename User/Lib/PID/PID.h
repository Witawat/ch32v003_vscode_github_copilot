/**
 * @file PID.h
 * @brief PID Controller Library สำหรับ CH32V003
 * @version 1.0
 * @date 2026-06-22
 *
 * @details
 * Library สำหรับ PID (Proportional-Integral-Derivative) controller
 * ใช้สำหรับควบคุมระบบป้อนกลับ เช่น ควบคุมอุณหภูมิ, ความเร็วรอบ, ตำแหน่ง
 *
 * **คุณสมบัติ:**
 * - รองรับทั้ง P, PI, PD, PID mode (ตั้ง Ki/Kd = 0 เพื่อปิด)
 * - Output limiting (anti-windup)
 * - Direct/Reverse action
 * - Manual/Auto mode
 * - คำนวณด้วย float แบบ IEEE 754
 *
 * @example
 * #include "PID.h"
 *
 * PID_Controller pid;
 * PID_Init(&pid, 2.0f, 0.5f, 0.1f, 0.01f);  // Kp=2, Ki=0.5, Kd=0.1, dt=10ms
 * PID_SetSetpoint(&pid, 100.0f);
 * PID_SetLimits(&pid, 0.0f, 255.0f);
 * PID_SetMode(&pid, PID_MODE_AUTO);
 *
 * while (1) {
 *     float input = read_sensor();     // ค่าปัจจุบัน
 *     float output = PID_Compute(&pid, input);  // ค่าควบคุม
 *     apply_output(output);
 *     Delay_Ms(10);
 * }
 *
 * @note เป็น pure software library ไม่ใช้ทรัพยากรฮาร์ดแวร์
 */

#ifndef __PID_H
#define __PID_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ========== Modes ========== */

#define PID_MODE_MANUAL 0
#define PID_MODE_AUTO   1

#define PID_DIRECT  0
#define PID_REVERSE 1

/* ========== Status ========== */

typedef enum {
    PID_OK    = 0,
    PID_ERROR = 1
} PID_Status;

/* ========== Instance ========== */

typedef struct {
    float kp;
    float ki;
    float kd;
    float setpoint;
    float input;
    float output;
    float integral;
    float prev_error;
    float dt;
    float out_min;
    float out_max;
    uint8_t mode;
    uint8_t direction;
} PID_Controller;

/* ========== Function Prototypes ========== */

PID_Status PID_Init(PID_Controller* pid, float kp, float ki, float kd, float dt);

PID_Status PID_SetTunings(PID_Controller* pid, float kp, float ki, float kd);

PID_Status PID_SetLimits(PID_Controller* pid, float min, float max);

PID_Status PID_SetSetpoint(PID_Controller* pid, float setpoint);

PID_Status PID_SetMode(PID_Controller* pid, uint8_t mode);

PID_Status PID_SetDirection(PID_Controller* pid, uint8_t direction);

PID_Status PID_Reset(PID_Controller* pid);

float PID_Compute(PID_Controller* pid, float input);

float PID_GetOutput(PID_Controller* pid);

#ifdef __cplusplus
}
#endif

#endif /* __PID_H */
