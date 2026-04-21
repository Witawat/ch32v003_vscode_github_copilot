/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_misc.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/26
 * Description        : This file contains all the functions prototypes for the 
 *                      miscellaneous firmware library functions.
 *                      ไฟล์นี้มีต้นแบบฟังก์ชันสำหรับฟังก์ชันเบ็ดเตล็ดต่างๆ
 *                      รวมถึงการตั้งค่า NVIC (Nested Vectored Interrupt Controller)
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/   
#ifndef __CH32V00X_MISC_H
#define __CH32V00X_MISC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <ch32v00x.h>

/* CSR_INTSYSCR_INEST_definition */
/* การกำหนดค่าการซ้อนทับของการขัดจังหวะ (Interrupt Nesting) */
#define INTSYSCR_INEST_NoEN   0x00   /* ปิดใช้งานการซ้อนทับของการขัดจังหวะ (CSR-0x804 bit1 = 0) */
#define INTSYSCR_INEST_EN     0x01   /* เปิดใช้งานการซ้อนทับของการขัดจังหวะ (CSR-0x804 bit1 = 1) */

/* Check the configuration of CSR(0x804) in the startup file(.S)
*   interrupt nesting enable(CSR-0x804 bit1 = 1)
*     priority - bit[7] - Preemption Priority
*                bit[6] - Sub priority
*                bit[5:0] - Reserve
*   interrupt nesting disable(CSR-0x804 bit1 = 0)
*     priority - bit[7:6] - Sub priority
*                bit[5:0] - Reserve
* ตรวจสอบการตั้งค่า CSR(0x804) ในไฟล์ startup (.S)
*   ถ้าเปิดใช้งานการซ้อนทับ: บิต[7] = ลำดับความสำคัญก่อน, บิต[6] = ลำดับความสำคัญรอง
*   ถ้าปิดใช้งานการซ้อนทับ: บิต[7:6] = ลำดับความสำคัญรอง
*/

#ifndef INTSYSCR_INEST
#define INTSYSCR_INEST   INTSYSCR_INEST_EN
#endif

/* NVIC Init Structure definition
 * โครงสร้างสำหรับการตั้งค่า NVIC
 *   interrupt nesting enable(CSR-0x804 bit1 = 1)
 *     NVIC_IRQChannelPreemptionPriority - ช่วงจาก 0 ถึง 1
 *     NVIC_IRQChannelSubPriority - ช่วงจาก 0 ถึง 1
 *
 *   interrupt nesting disable(CSR-0x804 bit1 = 0)
 *     NVIC_IRQChannelPreemptionPriority - ค่าเป็น 0 เท่านั้น
 *     NVIC_IRQChannelSubPriority - ช่วงจาก 0 ถึง 3
 */
typedef struct
{
  uint8_t NVIC_IRQChannel;                        /* ช่องทางการขัดจังหวะ */
  uint8_t NVIC_IRQChannelPreemptionPriority;      /* ลำดับความสำคัญก่อน (Preemption) */
  uint8_t NVIC_IRQChannelSubPriority;             /* ลำดับความสำคัญรอง (Sub) */
  FunctionalState NVIC_IRQChannelCmd;             /* เปิดหรือปิดช่องทางการขัดจังหวะ */
} NVIC_InitTypeDef;

/* Preemption_Priority_Group */
/* กลุ่มลำดับความสำคัญก่อน */
#if (INTSYSCR_INEST == INTSYSCR_INEST_NoEN)
#define NVIC_PriorityGroup_0           ((uint32_t)0x00) /* ปิดใช้งานการซ้อนทับ */
#else
#define NVIC_PriorityGroup_1           ((uint32_t)0x01) /* เปิดใช้งานการซ้อนทับ */
#endif


void NVIC_PriorityGroupConfig(uint32_t NVIC_PriorityGroup);
void NVIC_Init(NVIC_InitTypeDef* NVIC_InitStruct);

#ifdef __cplusplus
}
#endif

#endif /* __CH32V00x_MISC_H */

