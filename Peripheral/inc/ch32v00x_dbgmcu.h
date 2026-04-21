/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_dbgmcu.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : This file contains all the functions prototypes for the
 *                      DBGMCU firmware library.
 *                      ไฟล์นี้มีต้นแบบฟังก์ชันทั้งหมดสำหรับไลบรารีเฟิร์มแวร์ DBGMCU
 *                      DBGMCU = Debug MCU - การดีบักและระบุตัวตนของไมโครคอนโทรลเลอร์
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __CH32V00x_DBGMCU_H
#define __CH32V00x_DBGMCU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>

/* CFGR0 Register */
/* การตั้งค่าการดีบัก - หยุดเพอริเฟรัลเมื่อ CPU หยุด */
#define DBGMCU_IWDG_STOP             ((uint32_t)0x00000001)  /* หยุด IWDG เมื่อดีบัก */
#define DBGMCU_WWDG_STOP             ((uint32_t)0x00000002)  /* หยุด WWDG เมื่อดีบัก */
#define DBGMCU_TIM1_STOP             ((uint32_t)0x00000010)  /* หยุด TIM1 เมื่อดีบัก */
#define DBGMCU_TIM2_STOP             ((uint32_t)0x00000020)  /* หยุด TIM2 เมื่อดีบัก */

uint32_t DBGMCU_GetREVID(void);
uint32_t DBGMCU_GetDEVID(void);
uint32_t __get_DEBUG_CR(void);
void __set_DEBUG_CR(uint32_t value);
void DBGMCU_Config(uint32_t DBGMCU_Periph, FunctionalState NewState);
uint32_t DBGMCU_GetCHIPID( void );
#ifdef __cplusplus
}
#endif

#endif /* __CH32V00x_DBGMCU_H */
