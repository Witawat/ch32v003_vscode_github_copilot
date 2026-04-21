/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_adc.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/05/20
 * Description        : This file contains all the functions prototypes for the
 *                      ADC firmware library.
 *                      ไฟล์นี้มีต้นแบบฟังก์ชันทั้งหมดสำหรับไลบรารีเฟิร์มแวร์ ADC
 *                      ADC = Analog to Digital Converter - ตัวแปลงสัญญาณอนาล็อกเป็นดิจิทัล
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __CH32V00x_ADC_H
#define __CH32V00x_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>

/* ADC Init structure definition */
/* โครงสร้างสำหรับการตั้งค่า ADC */
typedef struct
{
    uint32_t ADC_Mode; /* Configures the ADC to operate in independent or
                          dual mode.
                          This parameter can be a value of @ref ADC_mode 
                          กำหนดโหมดการทำงานของ ADC (อิสระหรือคู่) */

    FunctionalState ADC_ScanConvMode; /* Specifies whether the conversion is performed in
                                         Scan (multichannels) or Single (one channel) mode.
                                         This parameter can be set to ENABLE or DISABLE 
                                         ระบุว่าการแปลงทำในโหมดสแกน (หลายช่อง) หรือโหมดเดี่ยว (หนึ่งช่อง) */

    FunctionalState ADC_ContinuousConvMode; /* Specifies whether the conversion is performed in
                                               Continuous or Single mode.
                                               This parameter can be set to ENABLE or DISABLE. 
                                               ระบุว่าการแปลงทำต่อเนื่องหรือครั้งเดียว */

    uint32_t ADC_ExternalTrigConv; /* Defines the external trigger used to start the analog
                                      to digital conversion of regular channels. This parameter
                                      can be a value of @ref ADC_external_trigger_sources_for_regular_channels_conversion 
                                      กำหนดทริกเกอร์ภายนอกที่ใช้เริ่มการแปลง ADC ของช่องปกติ */

    uint32_t ADC_DataAlign; /* Specifies whether the ADC data alignment is left or right.
                               This parameter can be a value of @ref ADC_data_align 
                               ระบุการจัดเรียงข้อมูล ADC ซ้ายหรือขวา */

    uint8_t ADC_NbrOfChannel; /* Specifies the number of ADC channels that will be converted
                                   using the sequencer for regular channel group.
                                   This parameter must range from 1 to 16. 
                                   ระบุจำนวนช่อง ADC ที่จะถูกแปลง */
} ADC_InitTypeDef;

/* ADC_mode */
/* โหมด ADC */
#define ADC_Mode_Independent                           ((uint32_t)0x00000000)  /* โหมดอิสระ */

/* ADC_external_trigger_sources_for_regular_channels_conversion */
/* แหล่งทริกเกอร์ภายนอกสำหรับการแปลงช่องปกติ */
#define ADC_ExternalTrigConv_T1_TRGO                   ((uint32_t)0x00000000)  /* ทริกเกอร์จาก Timer 1 TRGO */
#define ADC_ExternalTrigConv_T1_CC1                    ((uint32_t)0x00020000)  /* ทริกเกอร์จาก Timer 1 CC1 */
#define ADC_ExternalTrigConv_T1_CC2                    ((uint32_t)0x00040000)  /* ทริกเกอร์จาก Timer 1 CC2 */
#define ADC_ExternalTrigConv_T2_TRGO                   ((uint32_t)0x00060000)  /* ทริกเกอร์จาก Timer 2 TRGO */
#define ADC_ExternalTrigConv_T2_CC1                    ((uint32_t)0x00080000)  /* ทริกเกอร์จาก Timer 2 CC1 */
#define ADC_ExternalTrigConv_T2_CC2                    ((uint32_t)0x000A0000)  /* ทริกเกอร์จาก Timer 2 CC2 */
#define ADC_ExternalTrigConv_Ext_PD3_PC2               ((uint32_t)0x000C0000)  /* ทริกเกอร์จากขา PD3 หรือ PC2 */
#define ADC_ExternalTrigConv_None                      ((uint32_t)0x000E0000)  /* ไม่มีทริกเกอร์ภายนอก */

/* ADC_data_align */
/* การจัดเรียงข้อมูล ADC */
#define ADC_DataAlign_Right                            ((uint32_t)0x00000000)  /* ชิดขวา */
#define ADC_DataAlign_Left                             ((uint32_t)0x00000800)  /* ชิดซ้าย */

/* ADC_channels */
/* ช่องสัญญาณ ADC */
#define ADC_Channel_0                                  ((uint8_t)0x00)  /* ช่องที่ 0 */
#define ADC_Channel_1                                  ((uint8_t)0x01)  /* ช่องที่ 1 */
#define ADC_Channel_2                                  ((uint8_t)0x02)  /* ช่องที่ 2 */
#define ADC_Channel_3                                  ((uint8_t)0x03)  /* ช่องที่ 3 */
#define ADC_Channel_4                                  ((uint8_t)0x04)  /* ช่องที่ 4 */
#define ADC_Channel_5                                  ((uint8_t)0x05)  /* ช่องที่ 5 */
#define ADC_Channel_6                                  ((uint8_t)0x06)  /* ช่องที่ 6 */
#define ADC_Channel_7                                  ((uint8_t)0x07)  /* ช่องที่ 7 */
#define ADC_Channel_8                                  ((uint8_t)0x08)  /* ช่องที่ 8 - อ้างอิงแรงดันภายใน */
#define ADC_Channel_9                                  ((uint8_t)0x09)  /* ช่องที่ 9 - สอบเทียบแรงดันภายใน */

#define ADC_Channel_Vrefint                            ((uint8_t)ADC_Channel_8)  /* ช่องอ้างอิงแรงดันภายใน */
#define ADC_Channel_Vcalint                            ((uint8_t)ADC_Channel_9)  /* ช่องสอบเทียบแรงดันภายใน */

/* ADC_sampling_time */
/* เวลาสุ่มตัวอย่าง ADC (จำนวนรอบนาฬิกา) */
#define ADC_SampleTime_3Cycles                         ((uint8_t)0x00)  /* 3 รอบนาฬิกา */
#define ADC_SampleTime_9Cycles                         ((uint8_t)0x01)  /* 9 รอบนาฬิกา */
#define ADC_SampleTime_15Cycles                        ((uint8_t)0x02)  /* 15 รอบนาฬิกา */
#define ADC_SampleTime_30Cycles                        ((uint8_t)0x03)  /* 30 รอบนาฬิกา */
#define ADC_SampleTime_43Cycles                        ((uint8_t)0x04)  /* 43 รอบนาฬิกา */
#define ADC_SampleTime_57Cycles                        ((uint8_t)0x05)  /* 57 รอบนาฬิกา */
#define ADC_SampleTime_73Cycles                        ((uint8_t)0x06)  /* 73 รอบนาฬิกา */
#define ADC_SampleTime_241Cycles                       ((uint8_t)0x07)  /* 241 รอบนาฬิกา */

/* ADC_external_trigger_sources_for_injected_channels_conversion */
/* แหล่งทริกเกอร์ภายนอกสำหรับการแปลงช่อง Injected */
#define ADC_ExternalTrigInjecConv_T1_CC3               ((uint32_t)0x00000000)  /* ทริกเกอร์จาก Timer 1 CC3 */
#define ADC_ExternalTrigInjecConv_T1_CC4               ((uint32_t)0x00001000)  /* ทริกเกอร์จาก Timer 1 CC4 */
#define ADC_ExternalTrigInjecConv_T2_CC3               ((uint32_t)0x00002000)  /* ทริกเกอร์จาก Timer 2 CC3 */
#define ADC_ExternalTrigInjecConv_T2_CC4               ((uint32_t)0x00003000)  /* ทริกเกอร์จาก Timer 2 CC4 */
#define ADC_ExternalTrigInjecConv_Ext_PD1_PA2          ((uint32_t)0x00006000)  /* ทริกเกอร์จากขา PD1 หรือ PA2 */
#define ADC_ExternalTrigInjecConv_None                 ((uint32_t)0x00007000)  /* ไม่มีทริกเกอร์ภายนอก */

/* ADC_injected_channel_selection */
/* การเลือกช่อง Injected */
#define ADC_InjectedChannel_1                          ((uint8_t)0x14)  /* ช่อง Injected ที่ 1 */
#define ADC_InjectedChannel_2                          ((uint8_t)0x18)  /* ช่อง Injected ที่ 2 */
#define ADC_InjectedChannel_3                          ((uint8_t)0x1C)  /* ช่อง Injected ที่ 3 */
#define ADC_InjectedChannel_4                          ((uint8_t)0x20)  /* ช่อง Injected ที่ 4 */

/* ADC_analog_watchdog_selection */
/* การเลือก Watchdog อนาล็อก (ตรวจสอบช่วงแรงดัน) */
#define ADC_AnalogWatchdog_SingleRegEnable             ((uint32_t)0x00800200)  /* เปิด Watchdog สำหรับช่อง Regular เดียว */
#define ADC_AnalogWatchdog_SingleInjecEnable           ((uint32_t)0x00400200)  /* เปิด Watchdog สำหรับช่อง Injected เดียว */
#define ADC_AnalogWatchdog_SingleRegOrInjecEnable      ((uint32_t)0x00C00200)  /* เปิด Watchdog สำหรับช่อง Regular หรือ Injected เดียว */
#define ADC_AnalogWatchdog_AllRegEnable                ((uint32_t)0x00800000)  /* เปิด Watchdog สำหรับทุกช่อง Regular */
#define ADC_AnalogWatchdog_AllInjecEnable              ((uint32_t)0x00400000)  /* เปิด Watchdog สำหรับทุกช่อง Injected */
#define ADC_AnalogWatchdog_AllRegAllInjecEnable        ((uint32_t)0x00C00000)  /* เปิด Watchdog สำหรับทุกช่อง */
#define ADC_AnalogWatchdog_None                        ((uint32_t)0x00000000)  /* ปิด Watchdog */

/* ADC_interrupts_definition */
/* การขัดจังหวะของ ADC */
#define ADC_IT_EOC                                     ((uint16_t)0x0220)  /* End of Conversion - แปลงเสร็จสิ้น */
#define ADC_IT_AWD                                     ((uint16_t)0x0140)  /* Analog Watchdog - Watchdog อนาล็อก */
#define ADC_IT_JEOC                                    ((uint16_t)0x0480)  /* Injected End of Conversion - แปลง Injected เสร็จสิ้น */

/* ADC_flags_definition */
/* แฟล็กสถานะ ADC */
#define ADC_FLAG_AWD                                   ((uint8_t)0x01)  /* Analog Watchdog flag */
#define ADC_FLAG_EOC                                   ((uint8_t)0x02)  /* End of Conversion flag */
#define ADC_FLAG_JEOC                                  ((uint8_t)0x04)  /* Injected End of Conversion flag */
#define ADC_FLAG_JSTRT                                 ((uint8_t)0x08)  /* Injected Channel Start flag */
#define ADC_FLAG_STRT                                  ((uint8_t)0x10)  /* Regular Channel Start flag */

/* ADC_calibration_voltage_definition */
/* การตั้งค่าแรงดันสำหรับการสอบเทียบ */
#define ADC_CALVOL_50PERCENT                           ((uint32_t)0x02000000)  /* 50% ของแรงดันอ้างอิง */
#define ADC_CALVOL_75PERCENT                           ((uint32_t)0x04000000)  /* 75% ของแรงดันอ้างอิง */

/* ADC_external_trigger_sources_delay_channels_definition */
/* แหล่งทริกเกอร์ภายนอกและการเลือกช่องสำหรับการเลื่อนเวลา */
#define ADC_ExternalTrigRegul_DLY                      ((uint32_t)0x00000000)  /* ทริกเกอร์จากแหล่งภายนอกสำหรับช่องปกติ */
#define ADC_ExternalTrigInjec_DLY                      ((uint32_t)0x00000200)  /* ทริกเกอร์จากแหล่งภายนอกสำหรับช่อง Injected */

void       ADC_DeInit(ADC_TypeDef *ADCx);
void       ADC_Init(ADC_TypeDef *ADCx, ADC_InitTypeDef *ADC_InitStruct);
void       ADC_StructInit(ADC_InitTypeDef *ADC_InitStruct);
void       ADC_Cmd(ADC_TypeDef *ADCx, FunctionalState NewState);
void       ADC_DMACmd(ADC_TypeDef *ADCx, FunctionalState NewState);
void       ADC_ITConfig(ADC_TypeDef *ADCx, uint16_t ADC_IT, FunctionalState NewState);
void       ADC_ResetCalibration(ADC_TypeDef *ADCx);
FlagStatus ADC_GetResetCalibrationStatus(ADC_TypeDef *ADCx);
void       ADC_StartCalibration(ADC_TypeDef *ADCx);
FlagStatus ADC_GetCalibrationStatus(ADC_TypeDef *ADCx);
void       ADC_SoftwareStartConvCmd(ADC_TypeDef *ADCx, FunctionalState NewState);
FlagStatus ADC_GetSoftwareStartConvStatus(ADC_TypeDef *ADCx);
void       ADC_DiscModeChannelCountConfig(ADC_TypeDef *ADCx, uint8_t Number);
void       ADC_DiscModeCmd(ADC_TypeDef *ADCx, FunctionalState NewState);
void       ADC_RegularChannelConfig(ADC_TypeDef *ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime);
void       ADC_ExternalTrigConvCmd(ADC_TypeDef *ADCx, FunctionalState NewState);
uint16_t   ADC_GetConversionValue(ADC_TypeDef *ADCx);
void       ADC_AutoInjectedConvCmd(ADC_TypeDef *ADCx, FunctionalState NewState);
void       ADC_InjectedDiscModeCmd(ADC_TypeDef *ADCx, FunctionalState NewState);
void       ADC_ExternalTrigInjectedConvConfig(ADC_TypeDef *ADCx, uint32_t ADC_ExternalTrigInjecConv);
void       ADC_ExternalTrigInjectedConvCmd(ADC_TypeDef *ADCx, FunctionalState NewState);
void       ADC_SoftwareStartInjectedConvCmd(ADC_TypeDef *ADCx, FunctionalState NewState);
FlagStatus ADC_GetSoftwareStartInjectedConvCmdStatus(ADC_TypeDef *ADCx);
void       ADC_InjectedChannelConfig(ADC_TypeDef *ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime);
void       ADC_InjectedSequencerLengthConfig(ADC_TypeDef *ADCx, uint8_t Length);
void       ADC_SetInjectedOffset(ADC_TypeDef *ADCx, uint8_t ADC_InjectedChannel, uint16_t Offset);
uint16_t   ADC_GetInjectedConversionValue(ADC_TypeDef *ADCx, uint8_t ADC_InjectedChannel);
void       ADC_AnalogWatchdogCmd(ADC_TypeDef *ADCx, uint32_t ADC_AnalogWatchdog);
void       ADC_AnalogWatchdogThresholdsConfig(ADC_TypeDef *ADCx, uint16_t HighThreshold, uint16_t LowThreshold);
void       ADC_AnalogWatchdogSingleChannelConfig(ADC_TypeDef *ADCx, uint8_t ADC_Channel);
FlagStatus ADC_GetFlagStatus(ADC_TypeDef *ADCx, uint8_t ADC_FLAG);
void       ADC_ClearFlag(ADC_TypeDef *ADCx, uint8_t ADC_FLAG);
ITStatus   ADC_GetITStatus(ADC_TypeDef *ADCx, uint16_t ADC_IT);
void       ADC_ClearITPendingBit(ADC_TypeDef *ADCx, uint16_t ADC_IT);
void       ADC_Calibration_Vol(ADC_TypeDef *ADCx, uint32_t ADC_CALVOL);
void       ADC_ExternalTrig_DLY(ADC_TypeDef *ADCx, uint32_t channel, uint16_t DelayTim);

#ifdef __cplusplus
}
#endif

#endif /*__CH32V00x_ADC_H */
