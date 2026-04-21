/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_usart.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : This file contains all the functions prototypes for the
 *                      USART firmware library.
 *                      ไฟล์นี้มีต้นแบบฟังก์ชันทั้งหมดสำหรับไลบรารีเฟิร์มแวร์ USART
 *                      USART = Universal Synchronous/Asynchronous Receiver Transmitter
 *                      ใช้สำหรับการสื่อสารอนุกรม (Serial Communication)
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __CH32V00x_USART_H
#define __CH32V00x_USART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>

/* USART Init Structure definition */
/* โครงสร้างสำหรับการตั้งค่า USART */
typedef struct
{
    uint32_t USART_BaudRate; /* This member configures the USART communication baud rate.
                                The baud rate is computed using the following formula:
                                 - IntegerDivider = ((PCLKx) / (16 * (USART_InitStruct->USART_BaudRate)))
                                 - FractionalDivider = ((IntegerDivider - ((u32) IntegerDivider)) * 16) + 0.5 
                                กำหนดอัตราการส่งข้อมูล (Baud Rate) ของ USART */

    uint16_t USART_WordLength; /* Specifies the number of data bits transmitted or received in a frame.
                                  This parameter can be a value of @ref USART_Word_Length 
                                  ระบุจำนวนบิตข้อมูลในหนึ่งเฟรม */

    uint16_t USART_StopBits; /* Specifies the number of stop bits transmitted.
                                This parameter can be a value of @ref USART_Stop_Bits 
                                ระบุจำนวนบิตหยุด */

    uint16_t USART_Parity; /* Specifies the parity mode.
                              This parameter can be a value of @ref USART_Parity
                              @note When parity is enabled, the computed parity is inserted
                                    at the MSB position of the transmitted data (9th bit when
                                    the word length is set to 9 data bits; 8th bit when the
                                    word length is set to 8 data bits). 
                              ระบุโหมดพาริตี (ตรวจสอบข้อผิดพลาด) */

    uint16_t USART_Mode; /* Specifies wether the Receive or Transmit mode is enabled or disabled.
                            This parameter can be a value of @ref USART_Mode 
                            ระบุโหมดรับหรือส่งข้อมูล */

    uint16_t USART_HardwareFlowControl; /* Specifies wether the hardware flow control mode is enabled
                                           or disabled.
                                           This parameter can be a value of @ref USART_Hardware_Flow_Control 
                                           ระบุการควบคุมการไหลของข้อมูลด้วยฮาร์ดแวร์ */
} USART_InitTypeDef;

/* USART Clock Init Structure definition */
/* โครงสร้างสำหรับการตั้งค่านาฬิกา USART (สำหรับโหมดซิงโครนัส) */
typedef struct
{
    uint16_t USART_Clock; /* Specifies whether the USART clock is enabled or disabled.
                             This parameter can be a value of @ref USART_Clock 
                             ระบุว่าเปิดหรือปิดนาฬิกา USART */

    uint16_t USART_CPOL; /* Specifies the steady state value of the serial clock.
                            This parameter can be a value of @ref USART_Clock_Polarity 
                            ระบุสถานะคงที่ของนาฬิกาสื่อสารอนุกรม */

    uint16_t USART_CPHA; /* Specifies the clock transition on which the bit capture is made.
                            This parameter can be a value of @ref USART_Clock_Phase 
                            ระบุขอบนาฬิกาที่ใช้ในการจับบิตข้อมูล */

    uint16_t USART_LastBit; /* Specifies whether the clock pulse corresponding to the last transmitted
                               data bit (MSB) has to be output on the SCLK pin in synchronous mode.
                               This parameter can be a value of @ref USART_Last_Bit 
                               ระบุว่าส่งออกพัลส์นาฬิกาของบิตสุดท้ายหรือไม่ */
} USART_ClockInitTypeDef;

/* USART_Word_Length */
/* ความยาวบิตข้อมูล */
#define USART_WordLength_8b                  ((uint16_t)0x0000)  /* 8 บิต */
#define USART_WordLength_9b                  ((uint16_t)0x1000)  /* 9 บิต */

/* USART_Stop_Bits */
/* จำนวนบิตหยุด */
#define USART_StopBits_1                     ((uint16_t)0x0000)  /* 1 บิตหยุด */
#define USART_StopBits_0_5                   ((uint16_t)0x1000)  /* 0.5 บิตหยุด */
#define USART_StopBits_2                     ((uint16_t)0x2000)  /* 2 บิตหยุด */
#define USART_StopBits_1_5                   ((uint16_t)0x3000)  /* 1.5 บิตหยุด */

/* USART_Parity */
/* โหมดพาริตี */
#define USART_Parity_No                      ((uint16_t)0x0000)  /* ไม่มีพาริตี */
#define USART_Parity_Even                    ((uint16_t)0x0400)  /* พาริตีคู่ */
#define USART_Parity_Odd                     ((uint16_t)0x0600)  /* พาริตีคี่ */

/* USART_Mode */
/* โหมดการทำงาน */
#define USART_Mode_Rx                        ((uint16_t)0x0004)  /* โหมดรับข้อมูล */
#define USART_Mode_Tx                        ((uint16_t)0x0008)  /* โหมดส่งข้อมูล */

/* USART_Hardware_Flow_Control */
/* การควบคุมการไหลด้วยฮาร์ดแวร์ */
#define USART_HardwareFlowControl_None       ((uint16_t)0x0000)  /* ไม่มีการควบคุม */
#define USART_HardwareFlowControl_RTS        ((uint16_t)0x0100)  /* RTS - Request To Send */
#define USART_HardwareFlowControl_CTS        ((uint16_t)0x0200)  /* CTS - Clear To Send */
#define USART_HardwareFlowControl_RTS_CTS    ((uint16_t)0x0300)  /* ทั้ง RTS และ CTS */

/* USART_Clock */
/* นาฬิกา USART */
#define USART_Clock_Disable                  ((uint16_t)0x0000)  /* ปิดนาฬิกา */
#define USART_Clock_Enable                   ((uint16_t)0x0800)  /* เปิดนาฬิกา */

/* USART_Clock_Polarity */
/* ขั้วของนาฬิกา */
#define USART_CPOL_Low                       ((uint16_t)0x0000)  /* นาฬิกระดับต่ำ */
#define USART_CPOL_High                      ((uint16_t)0x0400)  /* นาฬิกระดับสูง */

/* USART_Clock_Phase */
/* เฟสของนาฬิกา */
#define USART_CPHA_1Edge                     ((uint16_t)0x0000)  /* ขอบแรก */
#define USART_CPHA_2Edge                     ((uint16_t)0x0200)  /* ขอบที่สอง */

/* USART_Last_Bit */
/* บิตสุดท้าย */
#define USART_LastBit_Disable                ((uint16_t)0x0000)  /* ปิด */
#define USART_LastBit_Enable                 ((uint16_t)0x0100)  /* เปิด */

/* USART_Interrupt_definition */
/* การขัดจังหวะของ USART */
#define USART_IT_PE                          ((uint16_t)0x0028)  /* Parity Error - ข้อผิดพลาดพาริตี */
#define USART_IT_TXE                         ((uint16_t)0x0727)  /* Transmit Data Register Empty - เรจิสเตอร์ส่งว่าง */
#define USART_IT_TC                          ((uint16_t)0x0626)  /* Transmission Complete - ส่งเสร็จสมบูรณ์ */
#define USART_IT_RXNE                        ((uint16_t)0x0525)  /* Read Data Register Not Empty - มีข้อมูลให้อ่าน */
#define USART_IT_ORE_RX                      ((uint16_t)0x0325)  /* Overrun Error - ข้อมูลล้น */
#define USART_IT_IDLE                        ((uint16_t)0x0424)  /* IDLE line detected - ตรวจพบเส้นไอดีล */
#define USART_IT_LBD                         ((uint16_t)0x0846)  /* LIN Break Detection - ตรวจพบ LIN Break */
#define USART_IT_CTS                         ((uint16_t)0x096A)  /* CTS flag change - สถานะ CTS เปลี่ยน */
#define USART_IT_ERR                         ((uint16_t)0x0060)  /* Error interrupt - การขัดจังหวะข้อผิดพลาด */
#define USART_IT_ORE_ER                      ((uint16_t)0x0360)  /* Overrun Error - ข้อมูลล้น */
#define USART_IT_NE                          ((uint16_t)0x0260)  /* Noise Error - ข้อผิดพลาดสัญญาณรบกวน */
#define USART_IT_FE                          ((uint16_t)0x0160)  /* Framing Error - ข้อผิดพลาดเฟรม */

#define USART_IT_ORE                         USART_IT_ORE_ER     /* ชื่อเดิมของ USART_IT_ORE_ER */

/* USART_DMA_Requests */
/* คำขอ DMA ของ USART */
#define USART_DMAReq_Tx                      ((uint16_t)0x0080)  /* ขอ DMA สำหรับส่งข้อมูล */
#define USART_DMAReq_Rx                      ((uint16_t)0x0040)  /* ขอ DMA สำหรับรับข้อมูล */

/* USART_WakeUp_methods */
/* วิธีการปลุก USART จากโหมดประหยัดพลังงาน */
#define USART_WakeUp_IdleLine                ((uint16_t)0x0000)  /* ปลุกด้วยเส้นไอดีล */
#define USART_WakeUp_AddressMark             ((uint16_t)0x0800)  /* ปลุกด้วยเครื่องหมายที่อยู่ */

/* USART_LIN_Break_Detection_Length */
/* ความยาวของการตรวจจับ LIN Break */
#define USART_LINBreakDetectLength_10b       ((uint16_t)0x0000)  /* 10 บิต */
#define USART_LINBreakDetectLength_11b       ((uint16_t)0x0020)  /* 11 บิต */

/* USART_IrDA_Low_Power */
/* โหมดพลังงานต่ำ IrDA */
#define USART_IrDAMode_LowPower              ((uint16_t)0x0004)  /* โหมดพลังงานต่ำ */
#define USART_IrDAMode_Normal                ((uint16_t)0x0000)  /* โหมดปกติ */

/* USART_Flags */
/* แฟล็กสถานะ USART */
#define USART_FLAG_CTS                       ((uint16_t)0x0200)  /* CTS flag - สถานะ CTS */
#define USART_FLAG_LBD                       ((uint16_t)0x0100)  /* LIN Break Detection flag */
#define USART_FLAG_TXE                       ((uint16_t)0x0080)  /* Transmit data register empty */
#define USART_FLAG_TC                        ((uint16_t)0x0040)  /* Transmission complete */
#define USART_FLAG_RXNE                      ((uint16_t)0x0020)  /* Read data register not empty */
#define USART_FLAG_IDLE                      ((uint16_t)0x0010)  /* IDLE line detected */
#define USART_FLAG_ORE                       ((uint16_t)0x0008)  /* Overrun error */
#define USART_FLAG_NE                        ((uint16_t)0x0004)  /* Noise error */
#define USART_FLAG_FE                        ((uint16_t)0x0002)  /* Framing error */
#define USART_FLAG_PE                        ((uint16_t)0x0001)  /* Parity error */

void       USART_DeInit(USART_TypeDef *USARTx);
void       USART_Init(USART_TypeDef *USARTx, USART_InitTypeDef *USART_InitStruct);
void       USART_StructInit(USART_InitTypeDef *USART_InitStruct);
void       USART_ClockInit(USART_TypeDef *USARTx, USART_ClockInitTypeDef *USART_ClockInitStruct);
void       USART_ClockStructInit(USART_ClockInitTypeDef *USART_ClockInitStruct);
void       USART_Cmd(USART_TypeDef *USARTx, FunctionalState NewState);
void       USART_ITConfig(USART_TypeDef *USARTx, uint16_t USART_IT, FunctionalState NewState);
void       USART_DMACmd(USART_TypeDef *USARTx, uint16_t USART_DMAReq, FunctionalState NewState);
void       USART_SetAddress(USART_TypeDef *USARTx, uint8_t USART_Address);
void       USART_WakeUpConfig(USART_TypeDef *USARTx, uint16_t USART_WakeUp);
void       USART_ReceiverWakeUpCmd(USART_TypeDef *USARTx, FunctionalState NewState);
void       USART_LINBreakDetectLengthConfig(USART_TypeDef *USARTx, uint16_t USART_LINBreakDetectLength);
void       USART_LINCmd(USART_TypeDef *USARTx, FunctionalState NewState);
void       USART_SendData(USART_TypeDef *USARTx, uint16_t Data);
uint16_t   USART_ReceiveData(USART_TypeDef *USARTx);
void       USART_SendBreak(USART_TypeDef *USARTx);
void       USART_SetGuardTime(USART_TypeDef *USARTx, uint8_t USART_GuardTime);
void       USART_SetPrescaler(USART_TypeDef *USARTx, uint8_t USART_Prescaler);
void       USART_SmartCardCmd(USART_TypeDef *USARTx, FunctionalState NewState);
void       USART_SmartCardNACKCmd(USART_TypeDef *USARTx, FunctionalState NewState);
void       USART_HalfDuplexCmd(USART_TypeDef *USARTx, FunctionalState NewState);
void       USART_OverSampling8Cmd(USART_TypeDef *USARTx, FunctionalState NewState);
void       USART_OneBitMethodCmd(USART_TypeDef *USARTx, FunctionalState NewState);
void       USART_IrDAConfig(USART_TypeDef *USARTx, uint16_t USART_IrDAMode);
void       USART_IrDACmd(USART_TypeDef *USARTx, FunctionalState NewState);
FlagStatus USART_GetFlagStatus(USART_TypeDef *USARTx, uint16_t USART_FLAG);
void       USART_ClearFlag(USART_TypeDef *USARTx, uint16_t USART_FLAG);
ITStatus   USART_GetITStatus(USART_TypeDef *USARTx, uint16_t USART_IT);
void       USART_ClearITPendingBit(USART_TypeDef *USARTx, uint16_t USART_IT);

#ifdef __cplusplus
}
#endif

#endif /* __CH32V00x_USART_H */
