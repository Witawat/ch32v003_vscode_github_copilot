/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_spi.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/06/01
 * Description        : This file contains all the functions prototypes for the
 *                      SPI firmware library.
 *                      ไฟล์นี้มีต้นแบบฟังก์ชันทั้งหมดสำหรับไลบรารีเฟิร์มแวร์ SPI
 *                      SPI = Serial Peripheral Interface - อินเทอร์เฟซอุปกรณ์อนุกรม
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __CH32V00x_SPI_H
#define __CH32V00x_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>

/* SPI Init structure definition */
/* โครงสร้างสำหรับการตั้งค่า SPI */
typedef struct
{
    uint16_t SPI_Direction; /* Specifies the SPI unidirectional or bidirectional data mode.
                               This parameter can be a value of @ref SPI_data_direction 
                               ระบุโหมดข้อมูลแบบทิศทางเดียวหรือสองทิศทาง */

    uint16_t SPI_Mode; /* Specifies the SPI operating mode.
                          This parameter can be a value of @ref SPI_mode 
                          ระบุโหมดการทำงาน (Master หรือ Slave) */

    uint16_t SPI_DataSize; /* Specifies the SPI data size.
                              This parameter can be a value of @ref SPI_data_size 
                              ระบุขนาดข้อมูล (8 หรือ 16 บิต) */

    uint16_t SPI_CPOL; /* Specifies the serial clock steady state.
                          This parameter can be a value of @ref SPI_Clock_Polarity 
                          When using SPI slave mode to send data, the CPOL bit should be set to 1 
                          ระบุสถานะคงที่ของนาฬิกาสื่อสาร */

    uint16_t SPI_CPHA; /* Specifies the clock active edge for the bit capture.
                          This parameter can be a value of @ref SPI_Clock_Phase 
                          ระบุขอบนาฬิกาที่ใช้จับบิตข้อมูล */

    uint16_t SPI_NSS; /* Specifies whether the NSS signal is managed by
                         hardware (NSS pin) or by software using the SSI bit.
                         This parameter can be a value of @ref SPI_Slave_Select_management 
                         ระบุว่าสัญญาณ NSS จัดการโดยฮาร์ดแวร์หรือซอฟต์แวร์ */

    uint16_t SPI_BaudRatePrescaler; /* Specifies the Baud Rate prescaler value which will be
                                       used to configure the transmit and receive SCK clock.
                                       This parameter can be a value of @ref SPI_BaudRate_Prescaler.
                                       @note The communication clock is derived from the master
                                             clock. The slave clock does not need to be set. 
                                       ระบุตัวหารอัตราการส่งข้อมูล */

    uint16_t SPI_FirstBit; /* Specifies whether data transfers start from MSB bit. 
                              ระบุว่าส่งข้อมูลจากบิตสำคัญสูงสุดหรือต่ำสุดก่อน */

    uint16_t SPI_CRCPolynomial; /* Specifies the polynomial used for the CRC calculation. 
                                   ระบุพหุนามสำหรับการคำนวณ CRC */
} SPI_InitTypeDef;

/* SPI_data_direction */
/* ทิศทางข้อมูล SPI */
#define SPI_Direction_2Lines_FullDuplex    ((uint16_t)0x0000)  /* สองสาย แบบเต็มดูเพล็กซ์ */
#define SPI_Direction_2Lines_RxOnly        ((uint16_t)0x0400)  /* สองสาย รับอย่างเดียว */
#define SPI_Direction_1Line_Rx             ((uint16_t)0x8000)  /* สายเดียว รับข้อมูล */
#define SPI_Direction_1Line_Tx             ((uint16_t)0xC000)  /* สายเดียว ส่งข้อมูล */

/* SPI_mode */
/* โหมด SPI */
#define SPI_Mode_Master                    ((uint16_t)0x0104)  /* โหมด Master - ตัวควบคุมหลัก */
#define SPI_Mode_Slave                     ((uint16_t)0x0000)  /* โหมด Slave - ตัวตาม */

/* SPI_data_size */
/* ขนาดข้อมูล */
#define SPI_DataSize_16b                   ((uint16_t)0x0800)  /* 16 บิต */
#define SPI_DataSize_8b                    ((uint16_t)0x0000)  /* 8 บิต */

/* SPI_Clock_Polarity */
/* ขั้วของนาฬิกา */
#define SPI_CPOL_Low                       ((uint16_t)0x0000)  /* นาฬิกระดับต่ำ */
#define SPI_CPOL_High                      ((uint16_t)0x0002)  /* นาฬิกระดับสูง - เมื่อใช้ Slave mode ต้องตั้งเป็น 1 */

/* SPI_Clock_Phase */
/* เฟสของนาฬิกา */
#define SPI_CPHA_1Edge                     ((uint16_t)0x0000)  /* ขอบแรก */
#define SPI_CPHA_2Edge                     ((uint16_t)0x0001)  /* ขอบที่สอง */

/* SPI_Slave_Select_management */
/* การจัดการ Slave Select */
#define SPI_NSS_Soft                       ((uint16_t)0x0200)  /* จัดการด้วยซอฟต์แวร์ */
#define SPI_NSS_Hard                       ((uint16_t)0x0000)  /* จัดการด้วยฮาร์ดแวร์ (ขา NSS) */

/* SPI_BaudRate_Prescaler */
/* ตัวหารอัตราการส่งข้อมูล */
#define SPI_BaudRatePrescaler_2            ((uint16_t)0x0000)  /* หารด้วย 2 */
#define SPI_BaudRatePrescaler_4            ((uint16_t)0x0008)  /* หารด้วย 4 */
#define SPI_BaudRatePrescaler_8            ((uint16_t)0x0010)  /* หารด้วย 8 */
#define SPI_BaudRatePrescaler_16           ((uint16_t)0x0018)  /* หารด้วย 16 */
#define SPI_BaudRatePrescaler_32           ((uint16_t)0x0020)  /* หารด้วย 32 */
#define SPI_BaudRatePrescaler_64           ((uint16_t)0x0028)  /* หารด้วย 64 */
#define SPI_BaudRatePrescaler_128          ((uint16_t)0x0030)  /* หารด้วย 128 */
#define SPI_BaudRatePrescaler_256          ((uint16_t)0x0038)  /* หารด้วย 256 */

/* SPI_MSB_LSB transmission */
/* ลำดับการส่งบิต */
#define SPI_FirstBit_MSB                   ((uint16_t)0x0000)  /* ส่ง MSB ก่อน (บิตสำคัญสูงสุด) */
#define SPI_FirstBit_LSB                   ((uint16_t)0x0080)  /* ส่ง LSB ก่อน (ไม่รองรับ Slave mode) */

/* SPI_I2S_DMA_transfer_requests */
/* คำขอ DMA สำหรับ SPI/I2S */
#define SPI_I2S_DMAReq_Tx                  ((uint16_t)0x0002)  /* ขอ DMA สำหรับส่ง */
#define SPI_I2S_DMAReq_Rx                  ((uint16_t)0x0001)  /* ขอ DMA สำหรับรับ */

/* SPI_NSS_internal_software_management */
/* การจัดการ NSS ภายในด้วยซอฟต์แวร์ */
#define SPI_NSSInternalSoft_Set            ((uint16_t)0x0100)  /* ตั้งค่า NSS */
#define SPI_NSSInternalSoft_Reset          ((uint16_t)0xFEFF)  /* รีเซ็ต NSS */

/* SPI_CRC_Transmit_Receive */
/* CRC สำหรับส่งและรับ */
#define SPI_CRC_Tx                         ((uint8_t)0x00)  /* CRC ของการส่ง */
#define SPI_CRC_Rx                         ((uint8_t)0x01)  /* CRC ของการรับ */

/* SPI_direction_transmit_receive */
/* ทิศทางการส่งและรับ */
#define SPI_Direction_Rx                   ((uint16_t)0xBFFF)  /* รับข้อมูล */
#define SPI_Direction_Tx                   ((uint16_t)0x4000)  /* ส่งข้อมูล */

/* SPI_I2S_interrupts_definition */
/* การขัดจังหวะ SPI/I2S */
#define SPI_I2S_IT_TXE                     ((uint8_t)0x71)  /* Transmit buffer empty */
#define SPI_I2S_IT_RXNE                    ((uint8_t)0x60)  /* Receive buffer not empty */
#define SPI_I2S_IT_ERR                     ((uint8_t)0x50)  /* Error interrupt */
#define SPI_I2S_IT_OVR                     ((uint8_t)0x56)  /* Overrun interrupt */
#define SPI_IT_MODF                        ((uint8_t)0x55)  /* Mode fault interrupt */
#define SPI_IT_CRCERR                      ((uint8_t)0x54)  /* CRC error interrupt */
#define I2S_IT_UDR                         ((uint8_t)0x53)  /* Underrun interrupt (I2S) */

/* SPI_I2S_flags_definition */
/* แฟล็กสถานะ SPI/I2S */
#define SPI_I2S_FLAG_RXNE                  ((uint16_t)0x0001)  /* Receive buffer not empty */
#define SPI_I2S_FLAG_TXE                   ((uint16_t)0x0002)  /* Transmit buffer empty */
#define I2S_FLAG_CHSIDE                    ((uint16_t)0x0004)  /* Channel side (I2S) */
#define I2S_FLAG_UDR                       ((uint16_t)0x0008)  /* Underrun flag (I2S) */
#define SPI_FLAG_CRCERR                    ((uint16_t)0x0010)  /* CRC error flag */
#define SPI_FLAG_MODF                      ((uint16_t)0x0020)  /* Mode fault flag */
#define SPI_I2S_FLAG_OVR                   ((uint16_t)0x0040)  /* Overrun flag */
#define SPI_I2S_FLAG_BSY                   ((uint16_t)0x0080)  /* Busy flag - บัสกำลังทำงาน */

void       SPI_I2S_DeInit(SPI_TypeDef *SPIx);
void       SPI_Init(SPI_TypeDef *SPIx, SPI_InitTypeDef *SPI_InitStruct);
void       SPI_StructInit(SPI_InitTypeDef *SPI_InitStruct);
void       SPI_Cmd(SPI_TypeDef *SPIx, FunctionalState NewState);
void       SPI_I2S_ITConfig(SPI_TypeDef *SPIx, uint8_t SPI_I2S_IT, FunctionalState NewState);
void       SPI_I2S_DMACmd(SPI_TypeDef *SPIx, uint16_t SPI_I2S_DMAReq, FunctionalState NewState);
void       SPI_I2S_SendData(SPI_TypeDef *SPIx, uint16_t Data);
uint16_t   SPI_I2S_ReceiveData(SPI_TypeDef *SPIx);
void       SPI_NSSInternalSoftwareConfig(SPI_TypeDef *SPIx, uint16_t SPI_NSSInternalSoft);
void       SPI_SSOutputCmd(SPI_TypeDef *SPIx, FunctionalState NewState);
void       SPI_DataSizeConfig(SPI_TypeDef *SPIx, uint16_t SPI_DataSize);
void       SPI_TransmitCRC(SPI_TypeDef *SPIx);
void       SPI_CalculateCRC(SPI_TypeDef *SPIx, FunctionalState NewState);
uint16_t   SPI_GetCRC(SPI_TypeDef *SPIx, uint8_t SPI_CRC);
uint16_t   SPI_GetCRCPolynomial(SPI_TypeDef *SPIx);
void       SPI_BiDirectionalLineConfig(SPI_TypeDef *SPIx, uint16_t SPI_Direction);
FlagStatus SPI_I2S_GetFlagStatus(SPI_TypeDef *SPIx, uint16_t SPI_I2S_FLAG);
void       SPI_I2S_ClearFlag(SPI_TypeDef *SPIx, uint16_t SPI_I2S_FLAG);
ITStatus   SPI_I2S_GetITStatus(SPI_TypeDef *SPIx, uint8_t SPI_I2S_IT);
void       SPI_I2S_ClearITPendingBit(SPI_TypeDef *SPIx, uint8_t SPI_I2S_IT);

#ifdef __cplusplus
}
#endif

#endif /*__CH32V00x_SPI_H */
