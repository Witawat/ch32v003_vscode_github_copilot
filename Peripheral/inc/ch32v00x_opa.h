/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_opa.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : This file contains all the functions prototypes for the
 *                      OPA firmware library.
 *                      ไฟล์นี้มีต้นแบบฟังก์ชันทั้งหมดสำหรับไลบรารีเฟิร์มแวร์ OPA
 *                      OPA = Operational Amplifier - ออปแอมป์ภายในชิป
 *                      ใช้สำหรับขยายสัญญาณอนาล็อก
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __CH32V00x_OPA_H
#define __CH32V00x_OPA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v00x.h"

/* OPA PSEL enumeration */
/* ขาอินพุตบวกของ OPA */
typedef enum
{
    CHP0 = 0,  /* ขาบวกช่อง 0 */
    CHP1       /* ขาบวกช่อง 1 */
} OPA_PSEL_TypeDef;

/* OPA NSEL enumeration */
/* ขาอินพุตลบของ OPA */
typedef enum
{
    CHN0 = 0,  /* ขาลบช่อง 0 */
    CHN1       /* ขาลบช่อง 1 */
} OPA_NSEL_TypeDef;


/* OPA Init Structure definition */
/* โครงสร้างสำหรับการตั้งค่า OPA */
typedef struct
{
    OPA_PSEL_TypeDef PSEL;    /* Specifies the positive channel of OPA - เลือกขาอินพุตบวก */
    OPA_NSEL_TypeDef NSEL;    /* Specifies the negative channel of OPA - เลือกขาอินพุตลบ */
} OPA_InitTypeDef;

void OPA_DeInit(void);
void OPA_Init(OPA_InitTypeDef *OPA_InitStruct);
void OPA_StructInit(OPA_InitTypeDef *OPA_InitStruct);
void OPA_Cmd(FunctionalState NewState);

#ifdef __cplusplus
}
#endif

#endif
