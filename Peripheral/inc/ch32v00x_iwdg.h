/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_iwdg.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : This file contains all the functions prototypes for the
 *                      IWDG firmware library.
 *                      ไฟล์นี้มีต้นแบบฟังก์ชันทั้งหมดสำหรับไลบรารีเฟิร์มแวร์ IWDG
 *                      IWDG = Independent Watchdog - วอทช์ด็อกอิสระ (ใช้นาฬิกา LSI แยกต่างหาก)
 *                      ใช้สำหรับรีเซ็ตระบบเมื่อโปรแกรมค้างหรือทำงานผิดปกติ
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __CH32V00x_IWDG_H
#define __CH32V00x_IWDG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>

/* IWDG_WriteAccess */
/* การอนุญาตเขียนเรจิสเตอร์ IWDG */
#define IWDG_WriteAccess_Enable     ((uint16_t)0x5555)  /* เปิดการเขียน */
#define IWDG_WriteAccess_Disable    ((uint16_t)0x0000)  /* ปิดการเขียน */

/* IWDG_prescaler */
/* ตัวหารนาฬิกา IWDG */
#define IWDG_Prescaler_4            ((uint8_t)0x00)  /* หารด้วย 4 */
#define IWDG_Prescaler_8            ((uint8_t)0x01)  /* หารด้วย 8 */
#define IWDG_Prescaler_16           ((uint8_t)0x02)  /* หารด้วย 16 */
#define IWDG_Prescaler_32           ((uint8_t)0x03)  /* หารด้วย 32 */
#define IWDG_Prescaler_64           ((uint8_t)0x04)  /* หารด้วย 64 */
#define IWDG_Prescaler_128          ((uint8_t)0x05)  /* หารด้วย 128 */
#define IWDG_Prescaler_256          ((uint8_t)0x06)  /* หารด้วย 256 */

/* IWDG_Flag */
/* แฟล็กสถานะ IWDG */
#define IWDG_FLAG_PVU               ((uint16_t)0x0001)  /* Prescaler Value Update - อัปเดตค่าตัวหารแล้ว */
#define IWDG_FLAG_RVU               ((uint16_t)0x0002)  /* Reload Value Update - อัปเดตค่าโหลดใหม่แล้ว */

void       IWDG_WriteAccessCmd(uint16_t IWDG_WriteAccess);
void       IWDG_SetPrescaler(uint8_t IWDG_Prescaler);
void       IWDG_SetReload(uint16_t Reload);
void       IWDG_ReloadCounter(void);
void       IWDG_Enable(void);
FlagStatus IWDG_GetFlagStatus(uint16_t IWDG_FLAG);

#ifdef __cplusplus
}
#endif

#endif /* __CH32V00x_IWDG_H */
