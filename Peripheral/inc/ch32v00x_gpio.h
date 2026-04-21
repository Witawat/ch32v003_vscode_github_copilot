/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_gpio.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/02/27
 * Description        : This file contains all the functions prototypes for the
 *                      GPIO firmware library.
 *                      ไฟล์นี้มีต้นแบบฟังก์ชันทั้งหมดสำหรับไลบรารีเฟิร์มแวร์ GPIO
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __CH32V00x_GPIO_H
#define __CH32V00x_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>

/* Output Maximum frequency selection */
/* การเลือกความถี่สูงสุดของเอาต์พุต */
typedef enum
{
    GPIO_Speed_10MHz = 1,  /* ความเร็ว 10 MHz */
    GPIO_Speed_2MHz,       /* ความเร็ว 2 MHz */
    GPIO_Speed_30MHz       /* ความเร็ว 30 MHz */
} GPIOSpeed_TypeDef;

#define GPIO_Speed_50MHz GPIO_Speed_30MHz  /* กำหนดให้ 50MHz เท่ากับ 30MHz */

/* Configuration Mode enumeration */
/* การกำหนดโหมดการทำงาน */
typedef enum
{
    GPIO_Mode_AIN = 0x0,        /* โหมด Analog Input - ใช้สำหรับ ADC */
    GPIO_Mode_IN_FLOATING = 0x04,  /* โหมด Input Floating - ลอยตัว ไม่มี Pull-up/Pull-down */
    GPIO_Mode_IPD = 0x28,       /* โหมด Input Pull-Down - มีตัวต้านทานดึงลงกราวด์ */
    GPIO_Mode_IPU = 0x48,       /* โหมด Input Pull-Up - มีตัวต้านทานดึงขึ้น VCC */
    GPIO_Mode_Out_OD = 0x14,    /* โหมด Output Open Drain - เอาต์พุตแบบเปิดคอลเลกเตอร์ */
    GPIO_Mode_Out_PP = 0x10,    /* โหมด Output Push-Pull - เอาต์พุตแบบพุช-พูล */
    GPIO_Mode_AF_OD = 0x1C,     /* โหมด Alternate Function Open Drain - ฟังก์ชันสำรองแบบเปิด */
    GPIO_Mode_AF_PP = 0x18      /* โหมด Alternate Function Push-Pull - ฟังก์ชันสำรองแบบพุช-พูล */
} GPIOMode_TypeDef;

/* GPIO Init structure definition */
/* โครงสร้างสำหรับการตั้งค่า GPIO */
typedef struct
{
    uint16_t GPIO_Pin; /* Specifies the GPIO pins to be configured.
                          This parameter can be any value of @ref GPIO_pins_define 
                          ระบุขา GPIO ที่ต้องการตั้งค่า สามารถใช้ค่าจาก GPIO_Pin_x */

    GPIOSpeed_TypeDef GPIO_Speed; /* Specifies the speed for the selected pins.
                                     This parameter can be a value of @ref GPIOSpeed_TypeDef 
                                     ระบุความเร็วสำหรับขาที่เลือก */

    GPIOMode_TypeDef GPIO_Mode; /* Specifies the operating mode for the selected pins.
                                   This parameter can be a value of @ref GPIOMode_TypeDef 
                                   ระบุโหมดการทำงานสำหรับขาที่เลือก */
} GPIO_InitTypeDef;

/* Bit_SET and Bit_RESET enumeration */
/* การกำหนดสถานะบิต - ตั้งค่า หรือ รีเซ็ต */
typedef enum
{
    Bit_RESET = 0,  /* รีเซ็ตบิต เป็น 0 */
    Bit_SET         /* ตั้งค่าบิต เป็น 1 */
} BitAction;

/* GPIO_pins_define */
/* กำหนดขา GPIO */
#define GPIO_Pin_0                     ((uint16_t)0x0001) /* Pin 0 selected - เลือกขาที่ 0 */
#define GPIO_Pin_1                     ((uint16_t)0x0002) /* Pin 1 selected - เลือกขาที่ 1 */
#define GPIO_Pin_2                     ((uint16_t)0x0004) /* Pin 2 selected - เลือกขาที่ 2 */
#define GPIO_Pin_3                     ((uint16_t)0x0008) /* Pin 3 selected - เลือกขาที่ 3 */
#define GPIO_Pin_4                     ((uint16_t)0x0010) /* Pin 4 selected - เลือกขาที่ 4 */
#define GPIO_Pin_5                     ((uint16_t)0x0020) /* Pin 5 selected - เลือกขาที่ 5 */
#define GPIO_Pin_6                     ((uint16_t)0x0040) /* Pin 6 selected - เลือกขาที่ 6 */
#define GPIO_Pin_7                     ((uint16_t)0x0080) /* Pin 7 selected - เลือกขาที่ 7 */
#define GPIO_Pin_All                   ((uint16_t)0xFFFF) /* All pins selected - เลือกทุกขา */

/* GPIO_Remap_define */
/* การกำหนดการแมปฟังก์ชันสำรองของ GPIO */
#define GPIO_Remap_SPI1                ((uint32_t)0x00000001) /* SPI1 Alternate Function mapping - แมปฟังก์ชันสำรอง SPI1 */
#define GPIO_PartialRemap_I2C1         ((uint32_t)0x10000002) /* I2C1 Partial Alternate Function mapping - แมปฟังก์ชันสำรอง I2C1 บางส่วน */
#define GPIO_FullRemap_I2C1            ((uint32_t)0x10400002) /* I2C1 Full Alternate Function mapping - แมปฟังก์ชันสำรอง I2C1 ทั้งหมด */
#define GPIO_PartialRemap1_USART1      ((uint32_t)0x80000004) /* USART1 Partial1 Alternate Function mapping - แมปฟังก์ชันสำรอง USART1 แบบที่ 1 */
#define GPIO_PartialRemap2_USART1      ((uint32_t)0x80200000) /* USART1 Partial2 Alternate Function mapping - แมปฟังก์ชันสำรอง USART1 แบบที่ 2 */
#define GPIO_FullRemap_USART1          ((uint32_t)0x80200004) /* USART1 Full Alternate Function mapping - แมปฟังก์ชันสำรอง USART1 ทั้งหมด */
#define GPIO_PartialRemap1_TIM1        ((uint32_t)0x00160040) /* TIM1 Partial1 Alternate Function mapping - แมปฟังก์ชันสำรอง TIM1 แบบที่ 1 */
#define GPIO_PartialRemap2_TIM1        ((uint32_t)0x00160080) /* TIM1 Partial2 Alternate Function mapping - แมปฟังก์ชันสำรอง TIM1 แบบที่ 2 */
#define GPIO_FullRemap_TIM1            ((uint32_t)0x001600C0) /* TIM1 Full Alternate Function mapping - แมปฟังก์ชันสำรอง TIM1 ทั้งหมด */
#define GPIO_PartialRemap1_TIM2        ((uint32_t)0x00180100) /* TIM2 Partial1 Alternate Function mapping - แมปฟังก์ชันสำรอง TIM2 แบบที่ 1 */
#define GPIO_PartialRemap2_TIM2        ((uint32_t)0x00180200) /* TIM2 Partial2 Alternate Function mapping - แมปฟังก์ชันสำรอง TIM2 แบบที่ 2 */
#define GPIO_FullRemap_TIM2            ((uint32_t)0x00180300) /* TIM2 Full Alternate Function mapping - แมปฟังก์ชันสำรอง TIM2 ทั้งหมด */
#define GPIO_Remap_PA1PA2              ((uint32_t)0x00008000) /* PA1 and PA2 Alternate Function mapping - แมปฟังก์ชันสำรอง PA1 และ PA2 */
#define GPIO_Remap_ADC1_ETRGINJ        ((uint32_t)0x00200002) /* ADC1 External Trigger Injected Conversion remapping - แมปทริกเกอร์ภายนอก ADC1 แบบ Injected */
#define GPIO_Remap_ADC1_ETRGREG        ((uint32_t)0x00200004) /* ADC1 External Trigger Regular Conversion remapping - แมปทริกเกอร์ภายนอก ADC1 แบบ Regular */
#define GPIO_Remap_LSI_CAL             ((uint32_t)0x00200080) /* LSI calibration Alternate Function mapping - แมปฟังก์ชันสอบเทียบ LSI */
#define GPIO_Remap_SDI_Disable         ((uint32_t)0x00300400) /* SDI Disabled - ปิดใช้งาน SDI */
#define GPIO_Remap_PA1_2               GPIO_Remap_PA1PA2      /* ชื่อเดิมของ GPIO_Remap_PA1PA2 */

/* GPIO_Port_Sources */
/* แหล่งที่มาของพอร์ต GPIO */
#define GPIO_PortSourceGPIOA           ((uint8_t)0x00)  /* พอร์ต A */
#define GPIO_PortSourceGPIOC           ((uint8_t)0x02)  /* พอร์ต C */
#define GPIO_PortSourceGPIOD           ((uint8_t)0x03)  /* พอร์ต D */

/* GPIO_Pin_sources */
/* แหล่งที่มาของขา GPIO */
#define GPIO_PinSource0                ((uint8_t)0x00)  /* ขาที่ 0 */
#define GPIO_PinSource1                ((uint8_t)0x01)  /* ขาที่ 1 */
#define GPIO_PinSource2                ((uint8_t)0x02)  /* ขาที่ 2 */
#define GPIO_PinSource3                ((uint8_t)0x03)  /* ขาที่ 3 */
#define GPIO_PinSource4                ((uint8_t)0x04)  /* ขาที่ 4 */
#define GPIO_PinSource5                ((uint8_t)0x05)  /* ขาที่ 5 */
#define GPIO_PinSource6                ((uint8_t)0x06)  /* ขาที่ 6 */
#define GPIO_PinSource7                ((uint8_t)0x07)  /* ขาที่ 7 */

void     GPIO_DeInit(GPIO_TypeDef *GPIOx);
void     GPIO_AFIODeInit(void);
void     GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_InitStruct);
void     GPIO_StructInit(GPIO_InitTypeDef *GPIO_InitStruct);
uint8_t  GPIO_ReadInputDataBit(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
uint16_t GPIO_ReadInputData(GPIO_TypeDef *GPIOx);
uint8_t  GPIO_ReadOutputDataBit(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
uint16_t GPIO_ReadOutputData(GPIO_TypeDef *GPIOx);
void     GPIO_SetBits(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void     GPIO_ResetBits(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void     GPIO_WriteBit(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, BitAction BitVal);
void     GPIO_Write(GPIO_TypeDef *GPIOx, uint16_t PortVal);
void     GPIO_PinLockConfig(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void     GPIO_EventOutputConfig(uint8_t GPIO_PortSource, uint8_t GPIO_PinSource);
void     GPIO_EventOutputCmd(FunctionalState NewState);
void     GPIO_PinRemapConfig(uint32_t GPIO_Remap, FunctionalState NewState);
void     GPIO_EXTILineConfig(uint8_t GPIO_PortSource, uint8_t GPIO_PinSource);

#ifdef __cplusplus
}
#endif

#endif /* __CH32V00x_GPIO_H */
