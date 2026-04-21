/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_dma.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : This file contains all the functions prototypes for the
 *                      DMA firmware library.
 *                      ไฟล์นี้มีต้นแบบฟังก์ชันทั้งหมดสำหรับไลบรารีเฟิร์มแวร์ DMA
 *                      DMA = Direct Memory Access - การเข้าถึงหน่วยความจำโดยตรง (ไม่ต้องใช้ CPU)
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __CH32V00x_DMA_H
#define __CH32V00x_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>

/* DMA Init structure definition */
/* โครงสร้างสำหรับการตั้งค่า DMA */
typedef struct
{
    uint32_t DMA_PeripheralBaseAddr; /* Specifies the peripheral base address for DMAy Channelx. 
                                        ระบุที่อยู่ฐานของอุปกรณ์ต่อพ่วง */

    uint32_t DMA_MemoryBaseAddr; /* Specifies the memory base address for DMAy Channelx. 
                                    ระบุที่อยู่ฐานของหน่วยความจำ */

    uint32_t DMA_DIR; /* Specifies if the peripheral is the source or destination.
                         This parameter can be a value of @ref DMA_data_transfer_direction 
                         ระบุว่าอุปกรณ์เป็นแหล่งที่มาหรือปลายทาง */

    uint32_t DMA_BufferSize; /* Specifies the buffer size, in data unit, of the specified Channel.
                                The data unit is equal to the configuration set in DMA_PeripheralDataSize
                                or DMA_MemoryDataSize members depending in the transfer direction. 
                                ระบุขนาดบัฟเฟอร์ */

    uint32_t DMA_PeripheralInc; /* Specifies whether the Peripheral address register is incremented or not.
                                   This parameter can be a value of @ref DMA_peripheral_incremented_mode 
                                   ระบุว่าเพิ่มที่อยู่ของอุปกรณ์อัตโนมัติหรือไม่ */

    uint32_t DMA_MemoryInc; /* Specifies whether the memory address register is incremented or not.
                               This parameter can be a value of @ref DMA_memory_incremented_mode 
                               ระบุว่าเพิ่มที่อยู่หน่วยความจำอัตโนมัติหรือไม่ */

    uint32_t DMA_PeripheralDataSize; /* Specifies the Peripheral data width.
                                        This parameter can be a value of @ref DMA_peripheral_data_size 
                                        ระบุความกว้างข้อมูลของอุปกรณ์ */

    uint32_t DMA_MemoryDataSize; /* Specifies the Memory data width.
                                    This parameter can be a value of @ref DMA_memory_data_size 
                                    ระบุความกว้างข้อมูลของหน่วยความจำ */

    uint32_t DMA_Mode; /* Specifies the operation mode of the DMAy Channelx.
                          This parameter can be a value of @ref DMA_circular_normal_mode.
                          @note: The circular buffer mode cannot be used if the memory-to-memory
                                data transfer is configured on the selected Channel 
                          ระบุโหมดการทำงาน (ปกติหรือวนซ้ำ) */

    uint32_t DMA_Priority; /* Specifies the software priority for the DMAy Channelx.
                              This parameter can be a value of @ref DMA_priority_level 
                              ระบุลำดับความสำคัญ */

    uint32_t DMA_M2M; /* Specifies if the DMAy Channelx will be used in memory-to-memory transfer.
                         This parameter can be a value of @ref DMA_memory_to_memory 
                         ระบุว่าใช้สำหรับการถ่ายโอนหน่วยความจำสู่หน่วยความจำหรือไม่ */
} DMA_InitTypeDef;

/* DMA_data_transfer_direction */
/* ทิศทางการถ่ายโอนข้อมูล DMA */
#define DMA_DIR_PeripheralDST              ((uint32_t)0x00000010)  /* อุปกรณ์เป็นปลายทาง (ส่งไปยังอุปกรณ์) */
#define DMA_DIR_PeripheralSRC              ((uint32_t)0x00000000)  /* อุปกรณ์เป็นแหล่งที่มา (รับจากอุปกรณ์) */

/* DMA_peripheral_incremented_mode */
/* โหมดการเพิ่มที่อยู่ของอุปกรณ์ */
#define DMA_PeripheralInc_Enable           ((uint32_t)0x00000040)  /* เปิดใช้งาน */
#define DMA_PeripheralInc_Disable          ((uint32_t)0x00000000)  /* ปิดใช้งาน */

/* DMA_memory_incremented_mode */
/* โหมดการเพิ่มที่อยู่หน่วยความจำ */
#define DMA_MemoryInc_Enable               ((uint32_t)0x00000080)  /* เปิดใช้งาน */
#define DMA_MemoryInc_Disable              ((uint32_t)0x00000000)  /* ปิดใช้งาน */

/* DMA_peripheral_data_size */
/* ความกว้างข้อมูลของอุปกรณ์ */
#define DMA_PeripheralDataSize_Byte        ((uint32_t)0x00000000)  /* 1 ไบต์ */
#define DMA_PeripheralDataSize_HalfWord    ((uint32_t)0x00000100)  /* 2 ไบต์ (Half Word) */
#define DMA_PeripheralDataSize_Word        ((uint32_t)0x00000200)  /* 4 ไบต์ (Word) */

/* DMA_memory_data_size */
/* ความกว้างข้อมูลของหน่วยความจำ */
#define DMA_MemoryDataSize_Byte            ((uint32_t)0x00000000)  /* 1 ไบต์ */
#define DMA_MemoryDataSize_HalfWord        ((uint32_t)0x00000400)  /* 2 ไบต์ (Half Word) */
#define DMA_MemoryDataSize_Word            ((uint32_t)0x00000800)  /* 4 ไบต์ (Word) */

/* DMA_circular_normal_mode */
/* โหมดการทำงาน */
#define DMA_Mode_Circular                  ((uint32_t)0x00000020)  /* โหมดวนซ้ำ (Circular) */
#define DMA_Mode_Normal                    ((uint32_t)0x00000000)  /* โหมดปกติ */

/* DMA_priority_level */
/* ระดับความสำคัญ */
#define DMA_Priority_VeryHigh              ((uint32_t)0x00003000)  /* สูงมาก */
#define DMA_Priority_High                  ((uint32_t)0x00002000)  /* สูง */
#define DMA_Priority_Medium                ((uint32_t)0x00001000)  /* ปานกลาง */
#define DMA_Priority_Low                   ((uint32_t)0x00000000)  /* ต่ำ */

/* DMA_memory_to_memory */
/* การถ่ายโอนหน่วยความจำสู่หน่วยความจำ */
#define DMA_M2M_Enable                     ((uint32_t)0x00004000)  /* เปิดใช้งาน */
#define DMA_M2M_Disable                    ((uint32_t)0x00000000)  /* ปิดใช้งาน */

/* DMA_interrupts_definition */
/* การขัดจังหวะ DMA */
#define DMA_IT_TC                          ((uint32_t)0x00000002)  /* Transfer Complete - ถ่ายโอนเสร็จสมบูรณ์ */
#define DMA_IT_HT                          ((uint32_t)0x00000004)  /* Half Transfer - ถ่ายโอนครึ่งหนึ่ง */
#define DMA_IT_TE                          ((uint32_t)0x00000008)  /* Transfer Error - ข้อผิดพลาดในการถ่ายโอน */

#define DMA1_IT_GL1                        ((uint32_t)0x00000001)
#define DMA1_IT_TC1                        ((uint32_t)0x00000002)
#define DMA1_IT_HT1                        ((uint32_t)0x00000004)
#define DMA1_IT_TE1                        ((uint32_t)0x00000008)
#define DMA1_IT_GL2                        ((uint32_t)0x00000010)
#define DMA1_IT_TC2                        ((uint32_t)0x00000020)
#define DMA1_IT_HT2                        ((uint32_t)0x00000040)
#define DMA1_IT_TE2                        ((uint32_t)0x00000080)
#define DMA1_IT_GL3                        ((uint32_t)0x00000100)
#define DMA1_IT_TC3                        ((uint32_t)0x00000200)
#define DMA1_IT_HT3                        ((uint32_t)0x00000400)
#define DMA1_IT_TE3                        ((uint32_t)0x00000800)
#define DMA1_IT_GL4                        ((uint32_t)0x00001000)
#define DMA1_IT_TC4                        ((uint32_t)0x00002000)
#define DMA1_IT_HT4                        ((uint32_t)0x00004000)
#define DMA1_IT_TE4                        ((uint32_t)0x00008000)
#define DMA1_IT_GL5                        ((uint32_t)0x00010000)
#define DMA1_IT_TC5                        ((uint32_t)0x00020000)
#define DMA1_IT_HT5                        ((uint32_t)0x00040000)
#define DMA1_IT_TE5                        ((uint32_t)0x00080000)
#define DMA1_IT_GL6                        ((uint32_t)0x00100000)
#define DMA1_IT_TC6                        ((uint32_t)0x00200000)
#define DMA1_IT_HT6                        ((uint32_t)0x00400000)
#define DMA1_IT_TE6                        ((uint32_t)0x00800000)
#define DMA1_IT_GL7                        ((uint32_t)0x01000000)
#define DMA1_IT_TC7                        ((uint32_t)0x02000000)
#define DMA1_IT_HT7                        ((uint32_t)0x04000000)
#define DMA1_IT_TE7                        ((uint32_t)0x08000000)

/* DMA_flags_definition */
#define DMA1_FLAG_GL1                      ((uint32_t)0x00000001)
#define DMA1_FLAG_TC1                      ((uint32_t)0x00000002)
#define DMA1_FLAG_HT1                      ((uint32_t)0x00000004)
#define DMA1_FLAG_TE1                      ((uint32_t)0x00000008)
#define DMA1_FLAG_GL2                      ((uint32_t)0x00000010)
#define DMA1_FLAG_TC2                      ((uint32_t)0x00000020)
#define DMA1_FLAG_HT2                      ((uint32_t)0x00000040)
#define DMA1_FLAG_TE2                      ((uint32_t)0x00000080)
#define DMA1_FLAG_GL3                      ((uint32_t)0x00000100)
#define DMA1_FLAG_TC3                      ((uint32_t)0x00000200)
#define DMA1_FLAG_HT3                      ((uint32_t)0x00000400)
#define DMA1_FLAG_TE3                      ((uint32_t)0x00000800)
#define DMA1_FLAG_GL4                      ((uint32_t)0x00001000)
#define DMA1_FLAG_TC4                      ((uint32_t)0x00002000)
#define DMA1_FLAG_HT4                      ((uint32_t)0x00004000)
#define DMA1_FLAG_TE4                      ((uint32_t)0x00008000)
#define DMA1_FLAG_GL5                      ((uint32_t)0x00010000)
#define DMA1_FLAG_TC5                      ((uint32_t)0x00020000)
#define DMA1_FLAG_HT5                      ((uint32_t)0x00040000)
#define DMA1_FLAG_TE5                      ((uint32_t)0x00080000)
#define DMA1_FLAG_GL6                      ((uint32_t)0x00100000)
#define DMA1_FLAG_TC6                      ((uint32_t)0x00200000)
#define DMA1_FLAG_HT6                      ((uint32_t)0x00400000)
#define DMA1_FLAG_TE6                      ((uint32_t)0x00800000)
#define DMA1_FLAG_GL7                      ((uint32_t)0x01000000)
#define DMA1_FLAG_TC7                      ((uint32_t)0x02000000)
#define DMA1_FLAG_HT7                      ((uint32_t)0x04000000)
#define DMA1_FLAG_TE7                      ((uint32_t)0x08000000)


void       DMA_DeInit(DMA_Channel_TypeDef *DMAy_Channelx);
void       DMA_Init(DMA_Channel_TypeDef *DMAy_Channelx, DMA_InitTypeDef *DMA_InitStruct);
void       DMA_StructInit(DMA_InitTypeDef *DMA_InitStruct);
void       DMA_Cmd(DMA_Channel_TypeDef *DMAy_Channelx, FunctionalState NewState);
void       DMA_ITConfig(DMA_Channel_TypeDef *DMAy_Channelx, uint32_t DMA_IT, FunctionalState NewState);
void       DMA_SetCurrDataCounter(DMA_Channel_TypeDef *DMAy_Channelx, uint16_t DataNumber);
uint16_t   DMA_GetCurrDataCounter(DMA_Channel_TypeDef *DMAy_Channelx);
FlagStatus DMA_GetFlagStatus(uint32_t DMAy_FLAG);
void       DMA_ClearFlag(uint32_t DMAy_FLAG);
ITStatus   DMA_GetITStatus(uint32_t DMAy_IT);
void       DMA_ClearITPendingBit(uint32_t DMAy_IT);

#ifdef __cplusplus
}
#endif

#endif /*__CH32V00x_DMA_H */
