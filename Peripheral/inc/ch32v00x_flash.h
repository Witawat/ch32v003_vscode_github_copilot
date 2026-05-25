/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_flash.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/25
 * Description        : This file contains all the functions prototypes for the FLASH
 *                      firmware library.
 *                      ไฟล์นี้มีต้นแบบฟังก์ชันทั้งหมดสำหรับไลบรารีเฟิร์มแวร์ FLASH
 *                      ใช้สำหรับการอ่าน/เขียนโปรแกรมและข้อมูลในหน่วยความจำ Flash
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __CH32V00x_FLASH_H
#define __CH32V00x_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>

/* FLASH Status */
/* สถานะการทำงานของ FLASH */
typedef enum
{
    FLASH_BUSY = 1,              /* กำลังทำงาน */
    FLASH_ERROR_PG,              /* ข้อผิดพลาดในการโปรแกรม */
    FLASH_ERROR_WRP,             /* ข้อผิดพลาดการป้องกันเขียน */
    FLASH_COMPLETE,              /* เสร็จสมบูรณ์ */
    FLASH_TIMEOUT,               /* หมดเวลา */
    FLASH_OP_RANGE_ERROR = 0xFD, /* ข้อผิดพลาดช่วงการทำงาน */
    FLASH_ALIGN_ERROR = 0xFE,    /* ข้อผิดพลาดการจัดตำแหน่ง */
    FLASH_ADR_RANGE_ERROR = 0xFF,/* ข้อผิดพลาดช่วงที่อยู่ */
} FLASH_Status;

/* Flash_Latency */
/* ค่าความล่าช้าของ Flash (รอบนาฬิกา) */
#define FLASH_Latency_0                  ((uint32_t)0x00000000) /* ไม่มีความล่าช้า */
#define FLASH_Latency_1                  ((uint32_t)0x00000001) /* ล่าช้า 1 รอบ */
#define FLASH_Latency_2                  ((uint32_t)0x00000002) /* ล่าช้า 2 รอบ */

/* Values to be used with CH32V00x devices (1page = 64Byte) */
/* การป้องกันเขียน Flash (1 หน้า = 64 ไบต์) */
#define FLASH_WRProt_Pages0to15          ((uint32_t)0x00000001) /* ป้องกันการเขียนหน้า 0-15 */
#define FLASH_WRProt_Pages16to31         ((uint32_t)0x00000002) /* ป้องกันการเขียนหน้า 16-31 */
#define FLASH_WRProt_Pages32to47         ((uint32_t)0x00000004) /* ป้องกันการเขียนหน้า 32-47 */
#define FLASH_WRProt_Pages48to63         ((uint32_t)0x00000008) /* ป้องกันการเขียนหน้า 48-63 */
#define FLASH_WRProt_Pages64to79         ((uint32_t)0x00000010) /* ป้องกันการเขียนหน้า 64-79 */
#define FLASH_WRProt_Pages80to95         ((uint32_t)0x00000020) /* ป้องกันการเขียนหน้า 80-95 */
#define FLASH_WRProt_Pages96to111        ((uint32_t)0x00000040) /* ป้องกันการเขียนหน้า 96-111 */
#define FLASH_WRProt_Pages112to127       ((uint32_t)0x00000080) /* ป้องกันการเขียนหน้า 112-127 */
#define FLASH_WRProt_Pages128to143       ((uint32_t)0x00000100) /* ป้องกันการเขียนหน้า 128-143 */
#define FLASH_WRProt_Pages144to159       ((uint32_t)0x00000200) /* ป้องกันการเขียนหน้า 144-159 */
#define FLASH_WRProt_Pages160to175       ((uint32_t)0x00000400) /* ป้องกันการเขียนหน้า 160-175 */
#define FLASH_WRProt_Pages176to191       ((uint32_t)0x00000800) /* ป้องกันการเขียนหน้า 176-191 */
#define FLASH_WRProt_Pages192to207       ((uint32_t)0x00001000) /* ป้องกันการเขียนหน้า 192-207 */
#define FLASH_WRProt_Pages208to223       ((uint32_t)0x00002000) /* ป้องกันการเขียนหน้า 208-223 */
#define FLASH_WRProt_Pages224to239       ((uint32_t)0x00004000) /* ป้องกันการเขียนหน้า 224-239 */
#define FLASH_WRProt_Pages240to255       ((uint32_t)0x00008000) /* ป้องกันการเขียนหน้า 240-255 */

#define FLASH_WRProt_AllPages            ((uint32_t)0x0000FFFF) /* ป้องกันการเขียนทุกหน้า */

/* Option_Bytes_IWatchdog */
/* ตัวเลือก Independent Watchdog */
#define OB_IWDG_SW                       ((uint16_t)0x0001) /* ใช้ซอฟต์แวร์ IWDG */
#define OB_IWDG_HW                       ((uint16_t)0x0000) /* ใช้ฮาร์ดแวร์ IWDG */

/* Option_Bytes_nRST_STOP */
/* ตัวเลือกการรีเซ็ตในโหมด STOP */
#define OB_STOP_NoRST                    ((uint16_t)0x0002) /* ไม่รีเซ็ตเมื่อเข้าโหมด STOP */
#define OB_STOP_RST                      ((uint16_t)0x0000) /* รีเซ็ตเมื่อเข้าโหมด STOP */

/* Option_Bytes_nRST_STDBY */
/* ตัวเลือกการรีเซ็ตในโหมด STANDBY */
#define OB_STDBY_NoRST                   ((uint16_t)0x0004) /* ไม่รีเซ็ตเมื่อเข้าโหมด STANDBY */
#define OB_STDBY_RST                     ((uint16_t)0x0000) /* รีเซ็ตเมื่อเข้าโหมด STANDBY */

/* Option_Bytes_RST_ENandDT */
/* ตัวเลือกการเปิดใช้งานขา RESET และเวลาล่าช้า */
#define OB_RST_NoEN                      ((uint16_t)0x0018) /* ปิดใช้งานขา RESET (PD7) */
#define OB_RST_EN_DT12ms                 ((uint16_t)0x0010) /* เปิดใช้งานขา RESET ล่าช้า 12ms */
#define OB_RST_EN_DT1ms                  ((uint16_t)0x0008) /* เปิดใช้งานขา RESET ล่าช้า 1ms */
#define OB_RST_EN_DT128us                ((uint16_t)0x0000) /* เปิดใช้งานขา RESET ล่าช้า 128us */

/* Option_Bytes_Power_ON_Start_Mode */
/* โหมดเริ่มต้นหลังจากเปิดไฟ (Power On) */
#define OB_PowerON_Start_Mode_BOOT       ((uint16_t)0x0020) /* เริ่มจาก Bootloader */
#define OB_PowerON_Start_Mode_USER       ((uint16_t)0x0000) /* เริ่มจากโปรแกรมผู้ใช้ */

/* FLASH_Interrupts */
#define FLASH_IT_ERROR                   ((uint32_t)0x00000400) /* FPEC error interrupt source */
#define FLASH_IT_EOP                     ((uint32_t)0x00001000) /* End of FLASH Operation Interrupt source */
#define FLASH_IT_BANK1_ERROR             FLASH_IT_ERROR         /* FPEC BANK1 error interrupt source */
#define FLASH_IT_BANK1_EOP               FLASH_IT_EOP           /* End of FLASH BANK1 Operation Interrupt source */

/* FLASH_Flags */
#define FLASH_FLAG_BSY                   ((uint32_t)0x00000001) /* FLASH Busy flag */
#define FLASH_FLAG_EOP                   ((uint32_t)0x00000020) /* FLASH End of Operation flag */
#define FLASH_FLAG_WRPRTERR              ((uint32_t)0x00000010) /* FLASH Write protected error flag */
#define FLASH_FLAG_OPTERR                ((uint32_t)0x00000001) /* FLASH Option Byte error flag */

#define FLASH_FLAG_BANK1_BSY             FLASH_FLAG_BSY       /* FLASH BANK1 Busy flag*/
#define FLASH_FLAG_BANK1_EOP             FLASH_FLAG_EOP       /* FLASH BANK1 End of Operation flag */
#define FLASH_FLAG_BANK1_WRPRTERR        FLASH_FLAG_WRPRTERR  /* FLASH BANK1 Write protected error flag */

/* System_Reset_Start_Mode */
#define Start_Mode_USER                  ((uint32_t)0x00000000)
#define Start_Mode_BOOT                  ((uint32_t)0x00004000)


/*Functions used for all CH32V00x devices*/
void         FLASH_SetLatency(uint32_t FLASH_Latency);
void         FLASH_Unlock(void);
void         FLASH_Lock(void);
FLASH_Status FLASH_ErasePage(uint32_t Page_Address);
FLASH_Status FLASH_EraseAllPages(void);
FLASH_Status FLASH_EraseOptionBytes(void);
FLASH_Status FLASH_ProgramWord(uint32_t Address, uint32_t Data);
FLASH_Status FLASH_ProgramHalfWord(uint32_t Address, uint16_t Data);
FLASH_Status FLASH_ProgramOptionByteData(uint32_t Address, uint8_t Data);
FLASH_Status FLASH_EnableWriteProtection(uint32_t FLASH_Pages);
FLASH_Status FLASH_ReadOutProtection(FunctionalState NewState);
FLASH_Status FLASH_UserOptionByteConfig(uint16_t OB_IWDG, uint16_t OB_STDBY, uint16_t OB_RST, uint16_t OB_PowerON_Start_Mode);
uint32_t     FLASH_GetUserOptionByte(void);
uint32_t     FLASH_GetWriteProtectionOptionByte(void);
FlagStatus   FLASH_GetReadOutProtectionStatus(void);
void         FLASH_ITConfig(uint32_t FLASH_IT, FunctionalState NewState);
FlagStatus   FLASH_GetFlagStatus(uint32_t FLASH_FLAG);
void         FLASH_ClearFlag(uint32_t FLASH_FLAG);
FLASH_Status FLASH_GetStatus(void);
FLASH_Status FLASH_WaitForLastOperation(uint32_t Timeout);
void         FLASH_Unlock_Fast(void);
void         FLASH_Lock_Fast(void);
void         FLASH_BufReset(void);
void         FLASH_BufLoad(uint32_t Address, uint32_t Data0);
void         FLASH_ErasePage_Fast(uint32_t Page_Address);
void         FLASH_ProgramPage_Fast(uint32_t Page_Address);
void         SystemReset_StartMode(uint32_t Mode);
FLASH_Status FLASH_ROM_ERASE(uint32_t StartAddr, uint32_t Length);
FLASH_Status FLASH_ROM_WRITE(uint32_t StartAddr, uint32_t *pbuf, uint32_t Length);

#ifdef __cplusplus
}
#endif

#endif /* __CH32V00x_FLASH_H */
