/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_pwr.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : This file contains all the functions prototypes for the PWR
 *                      firmware library.
 *                      ไฟล์นี้มีต้นแบบฟังก์ชันทั้งหมดสำหรับไลบรารีเฟิร์มแวร์ PWR
 *                      PWR = Power Management - การจัดการพลังงานและโหมดประหยัดไฟ
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __CH32V00x_PWR_H
#define __CH32V00x_PWR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>

/* PVD_detection_level  */
/* ระดับการตรวจจับแรงดันไฟฟ้าต่ำ (PVD - Programmable Voltage Detector) */
#define PWR_PVDLevel_MODE0          ((uint32_t)0x00000000)  /* โหมด 0 */
#define PWR_PVDLevel_MODE1          ((uint32_t)0x00000020)  /* โหมด 1 */
#define PWR_PVDLevel_MODE2          ((uint32_t)0x00000040)  /* โหมด 2 */
#define PWR_PVDLevel_MODE3          ((uint32_t)0x00000060)  /* โหมด 3 */
#define PWR_PVDLevel_MODE4          ((uint32_t)0x00000080)  /* โหมด 4 */
#define PWR_PVDLevel_MODE5          ((uint32_t)0x000000A0)  /* โหมด 5 */
#define PWR_PVDLevel_MODE6          ((uint32_t)0x000000C0)  /* โหมด 6 */
#define PWR_PVDLevel_MODE7          ((uint32_t)0x000000E0)  /* โหมด 7 */

#define PWR_PVDLevel_2V9            PWR_PVDLevel_MODE0  /* ตรวจจับที่ 2.9V */
#define PWR_PVDLevel_3V1            PWR_PVDLevel_MODE1  /* ตรวจจับที่ 3.1V */
#define PWR_PVDLevel_3V3            PWR_PVDLevel_MODE2  /* ตรวจจับที่ 3.3V */
#define PWR_PVDLevel_3V5            PWR_PVDLevel_MODE3  /* ตรวจจับที่ 3.5V */
#define PWR_PVDLevel_3V7            PWR_PVDLevel_MODE4  /* ตรวจจับที่ 3.7V */
#define PWR_PVDLevel_3V9            PWR_PVDLevel_MODE5  /* ตรวจจับที่ 3.9V */
#define PWR_PVDLevel_4V1            PWR_PVDLevel_MODE6  /* ตรวจจับที่ 4.1V */
#define PWR_PVDLevel_4V4            PWR_PVDLevel_MODE7  /* ตรวจจับที่ 4.4V */

/* PWR_AWU_Prescaler */
/* ตัวหารนาฬิกา Auto Wake-up (ปลุกอัตโนมัติจากโหมดประหยัดพลังงาน) */
#define PWR_AWU_Prescaler_1       ((uint32_t)0x00000000)  /* หารด้วย 1 */
#define PWR_AWU_Prescaler_2       ((uint32_t)0x00000002)  /* หารด้วย 2 */
#define PWR_AWU_Prescaler_4       ((uint32_t)0x00000003)  /* หารด้วย 4 */
#define PWR_AWU_Prescaler_8       ((uint32_t)0x00000004)  /* หารด้วย 8 */
#define PWR_AWU_Prescaler_16      ((uint32_t)0x00000005)  /* หารด้วย 16 */
#define PWR_AWU_Prescaler_32      ((uint32_t)0x00000006)  /* หารด้วย 32 */
#define PWR_AWU_Prescaler_64      ((uint32_t)0x00000007)  /* หารด้วย 64 */
#define PWR_AWU_Prescaler_128     ((uint32_t)0x00000008)  /* หารด้วย 128 */
#define PWR_AWU_Prescaler_256     ((uint32_t)0x00000009)  /* หารด้วย 256 */
#define PWR_AWU_Prescaler_512     ((uint32_t)0x0000000A)  /* หารด้วย 512 */
#define PWR_AWU_Prescaler_1024    ((uint32_t)0x0000000B)  /* หารด้วย 1024 */
#define PWR_AWU_Prescaler_2048    ((uint32_t)0x0000000C)  /* หารด้วย 2048 */
#define PWR_AWU_Prescaler_4096    ((uint32_t)0x0000000D)  /* หารด้วย 4096 */
#define PWR_AWU_Prescaler_10240   ((uint32_t)0x0000000E)  /* หารด้วย 10240 */
#define PWR_AWU_Prescaler_61440   ((uint32_t)0x0000000F)  /* หารด้วย 61440 */

/* STOP_mode_entry */
/* วิธีการเข้าโหมด STANDBY */
#define PWR_STANDBYEntry_WFI      ((uint8_t)0x01)  /* ใช้คำสั่ง WFI (Wait For Interrupt) */
#define PWR_STANDBYEntry_WFE      ((uint8_t)0x02)  /* ใช้คำสั่ง WFE (Wait For Event) */

/* PWR_Flag */
#define PWR_FLAG_PVDO             ((uint32_t)0x00000004)

void       PWR_DeInit(void);
void       PWR_PVDCmd(FunctionalState NewState);
void       PWR_PVDLevelConfig(uint32_t PWR_PVDLevel);
void       PWR_AutoWakeUpCmd(FunctionalState NewState);
void       PWR_AWU_SetPrescaler(uint32_t AWU_Prescaler);
void       PWR_AWU_SetWindowValue(uint8_t WindowValue);
void       PWR_EnterSTANDBYMode(uint8_t PWR_STANDBYEntry);
FlagStatus PWR_GetFlagStatus(uint32_t PWR_FLAG);

#ifdef __cplusplus
}
#endif

#endif /* __CH32V00x_PWR_H */
