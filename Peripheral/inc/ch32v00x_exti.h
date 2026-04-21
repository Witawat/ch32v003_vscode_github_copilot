/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_exti.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : This file contains all the functions prototypes for the
 *                      EXTI firmware library.
 *                      ไฟล์นี้มีต้นแบบฟังก์ชันทั้งหมดสำหรับไลบรารีเฟิร์มแวร์ EXTI
 *                      EXTI = External Interrupts and Events - การขัดจังหวะและเหตุการณ์ภายนอก
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __CH32V00x_EXTI_H
#define __CH32V00x_EXTI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>

/* EXTI mode enumeration */
/* โหมด EXTI */
typedef enum
{
    EXTI_Mode_Interrupt = 0x00,  /* โหมดการขัดจังหวะ */
    EXTI_Mode_Event = 0x04       /* โหมดเหตุการณ์ */
} EXTIMode_TypeDef;

/* EXTI Trigger enumeration */
/* ตัวกระตุ้น EXTI */
typedef enum
{
    EXTI_Trigger_Rising = 0x08,              /* กระตุ้นที่ขอบขาขึ้น */
    EXTI_Trigger_Falling = 0x0C,             /* กระตุ้นที่ขอบขาลง */
    EXTI_Trigger_Rising_Falling = 0x10       /* กระตุ้นทั้งสองขอบ */
} EXTITrigger_TypeDef;

/* EXTI Init Structure definition */
/* โครงสร้างสำหรับการตั้งค่า EXTI */
typedef struct
{
    uint32_t EXTI_Line; /* Specifies the EXTI lines to be enabled or disabled.
                           This parameter can be any combination of @ref EXTI_Lines 
                           ระบุเส้น EXTI ที่จะเปิดหรือปิดใช้งาน */

    EXTIMode_TypeDef EXTI_Mode; /* Specifies the mode for the EXTI lines.
                                   This parameter can be a value of @ref EXTIMode_TypeDef 
                                   ระบุโหมดสำหรับเส้น EXTI */

    EXTITrigger_TypeDef EXTI_Trigger; /* Specifies the trigger signal active edge for the EXTI lines.
                                         This parameter can be a value of @ref EXTIMode_TypeDef 
                                         ระบุขอบสัญญาณที่ใช้กระตุ้น */

    FunctionalState EXTI_LineCmd; /* Specifies the new state of the selected EXTI lines.
                                     This parameter can be set either to ENABLE or DISABLE 
                                     ระบุสถานะใหม่ของเส้น EXTI ที่เลือก */
} EXTI_InitTypeDef;

/* EXTI_Lines */
/* เส้น EXTI */
#define EXTI_Line0     ((uint32_t)0x00001) /* External interrupt line 0 - เส้นขัดจังหวะภายนอก 0 */
#define EXTI_Line1     ((uint32_t)0x00002) /* External interrupt line 1 - เส้นขัดจังหวะภายนอก 1 */
#define EXTI_Line2     ((uint32_t)0x00004) /* External interrupt line 2 - เส้นขัดจังหวะภายนอก 2 */
#define EXTI_Line3     ((uint32_t)0x00008) /* External interrupt line 3 - เส้นขัดจังหวะภายนอก 3 */
#define EXTI_Line4     ((uint32_t)0x00010) /* External interrupt line 4 - เส้นขัดจังหวะภายนอก 4 */
#define EXTI_Line5     ((uint32_t)0x00020) /* External interrupt line 5 - เส้นขัดจังหวะภายนอก 5 */
#define EXTI_Line6     ((uint32_t)0x00040) /* External interrupt line 6 - เส้นขัดจังหวะภายนอก 6 */
#define EXTI_Line7     ((uint32_t)0x00080) /* External interrupt line 7 - เส้นขัดจังหวะภายนอก 7 */
#define EXTI_Line8     ((uint32_t)0x00100) /* External interrupt line 8 Connected to the PVD Output - เชื่อมต่อกับเอาต์พุต PVD */
#define EXTI_Line9     ((uint32_t)0x00200) /* External interrupt line 9 Connected to the PWR Auto Wake-up event - เชื่อมต่อกับเหตุการณ์ปลุกอัตโนมัติ */

void       EXTI_DeInit(void);
void       EXTI_Init(EXTI_InitTypeDef *EXTI_InitStruct);
void       EXTI_StructInit(EXTI_InitTypeDef *EXTI_InitStruct);
void       EXTI_GenerateSWInterrupt(uint32_t EXTI_Line);
FlagStatus EXTI_GetFlagStatus(uint32_t EXTI_Line);
void       EXTI_ClearFlag(uint32_t EXTI_Line);
ITStatus   EXTI_GetITStatus(uint32_t EXTI_Line);
void       EXTI_ClearITPendingBit(uint32_t EXTI_Line);

#ifdef __cplusplus
}
#endif

#endif /* __CH32V00x_EXTI_H */
