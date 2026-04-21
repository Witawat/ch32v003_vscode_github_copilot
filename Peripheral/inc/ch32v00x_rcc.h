/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_rcc.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : This file provides all the RCC firmware functions.
 *                      ไฟล์นี้มีฟังก์ชันเฟิร์มแวร์ RCC ทั้งหมด
 *                      RCC = Reset and Clock Control - ควบคุมการรีเซ็ตและนาฬิกา
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __CH32V00x_RCC_H
#define __CH32V00x_RCC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>

/* RCC_Exported_Types */
/* ประเภทข้อมูลที่ส่งออกของ RCC */
typedef struct
{
    uint32_t SYSCLK_Frequency; /* returns SYSCLK clock frequency expressed in Hz - ความถี่นาฬิการะบบ */
    uint32_t HCLK_Frequency;   /* returns HCLK clock frequency expressed in Hz - ความถี่นาฬิกา AHB */
    uint32_t PCLK1_Frequency;  /* returns PCLK1 clock frequency expressed in Hz - ความถี่นาฬิกา APB1 */
    uint32_t PCLK2_Frequency;  /* returns PCLK2 clock frequency expressed in Hz - ความถี่นาฬิกา APB2 */
    uint32_t ADCCLK_Frequency; /* returns ADCCLK clock frequency expressed in Hz - ความถี่นาฬิกา ADC */
} RCC_ClocksTypeDef;

/* HSE_configuration */
/* การกำหนดค่า HSE (High Speed External oscillator) - ออสซิลเลเตอร์ความเร็วสูงภายนอก */
#define RCC_HSE_OFF                      ((uint32_t)0x00000000)  /* ปิด HSE */
#define RCC_HSE_ON                       ((uint32_t)0x00010000)  /* เปิด HSE */
#define RCC_HSE_Bypass                   ((uint32_t)0x00040000)  /* ข้าม HSE ใช้สัญญาณจากภายนอก */

/* PLL_entry_clock_source */
/* แหล่งที่มาของนาฬิกาเข้า PLL (Phase Locked Loop) */
#define RCC_PLLSource_HSI_MUL2           ((uint32_t)0x00000000)  /* ใช้ HSI คูณ 2 เป็นแหล่ง PLL */
#define RCC_PLLSource_HSE_MUL2           ((uint32_t)0x00010000)  /* ใช้ HSE คูณ 2 เป็นแหล่ง PLL */

/* System_clock_source */
/* แหล่งที่มาของนาฬิการะบบ */
#define RCC_SYSCLKSource_HSI             ((uint32_t)0x00000000)  /* ใช้ HSI เป็นนาฬิการะบบ */
#define RCC_SYSCLKSource_HSE             ((uint32_t)0x00000001)  /* ใช้ HSE เป็นนาฬิการะบบ */
#define RCC_SYSCLKSource_PLLCLK          ((uint32_t)0x00000002)  /* ใช้ PLL เป็นนาฬิการะบบ */

/* AHB_clock_source */
/* ตัวหารนาฬิกา AHB (Advanced High-performance Bus) */
#define RCC_SYSCLK_Div1                  ((uint32_t)0x00000000)  /* หารด้วย 1 */
#define RCC_SYSCLK_Div2                  ((uint32_t)0x00000010)  /* หารด้วย 2 */
#define RCC_SYSCLK_Div3                  ((uint32_t)0x00000020)  /* หารด้วย 3 */
#define RCC_SYSCLK_Div4                  ((uint32_t)0x00000030)  /* หารด้วย 4 */
#define RCC_SYSCLK_Div5                  ((uint32_t)0x00000040)  /* หารด้วย 5 */
#define RCC_SYSCLK_Div6                  ((uint32_t)0x00000050)  /* หารด้วย 6 */
#define RCC_SYSCLK_Div7                  ((uint32_t)0x00000060)  /* หารด้วย 7 */
#define RCC_SYSCLK_Div8                  ((uint32_t)0x00000070)  /* หารด้วย 8 */
#define RCC_SYSCLK_Div16                 ((uint32_t)0x000000B0)  /* หารด้วย 16 */
#define RCC_SYSCLK_Div32                 ((uint32_t)0x000000C0)  /* หารด้วย 32 */
#define RCC_SYSCLK_Div64                 ((uint32_t)0x000000D0)  /* หารด้วย 64 */
#define RCC_SYSCLK_Div128                ((uint32_t)0x000000E0)  /* หารด้วย 128 */
#define RCC_SYSCLK_Div256                ((uint32_t)0x000000F0)  /* หารด้วย 256 */

/* RCC_Interrupt_source */
/* แหล่งที่มาของการขัดจังหวะ RCC */
#define RCC_IT_LSIRDY                    ((uint8_t)0x01)  /* LSI พร้อมใช้งาน */
#define RCC_IT_HSIRDY                    ((uint8_t)0x04)  /* HSI พร้อมใช้งาน */
#define RCC_IT_HSERDY                    ((uint8_t)0x08)  /* HSE พร้อมใช้งาน */
#define RCC_IT_PLLRDY                    ((uint8_t)0x10)  /* PLL ล็อกแล้ว */
#define RCC_IT_CSS                       ((uint8_t)0x80)  /* ระบบความปลอดภัยนาฬิกา */

/* ADC_clock_source */
/* แหล่งที่มาของนาฬิกา ADC */
#define RCC_PCLK2_Div2                   ((uint32_t)0x00000000)  /* PCLK2 หาร 2 */
#define RCC_PCLK2_Div4                   ((uint32_t)0x00004000)  /* PCLK2 หาร 4 */
#define RCC_PCLK2_Div6                   ((uint32_t)0x00008000)  /* PCLK2 หาร 6 */
#define RCC_PCLK2_Div8                   ((uint32_t)0x0000C000)  /* PCLK2 หาร 8 */
#define RCC_PCLK2_Div12                  ((uint32_t)0x0000A000)  /* PCLK2 หาร 12 */
#define RCC_PCLK2_Div16                  ((uint32_t)0x0000E000)  /* PCLK2 หาร 16 */
#define RCC_PCLK2_Div24                  ((uint32_t)0x0000A800)  /* PCLK2 หาร 24 */
#define RCC_PCLK2_Div32                  ((uint32_t)0x0000E800)  /* PCLK2 หาร 32 */
#define RCC_PCLK2_Div48                  ((uint32_t)0x0000B000)  /* PCLK2 หาร 48 */
#define RCC_PCLK2_Div64                  ((uint32_t)0x0000F000)  /* PCLK2 หาร 64 */
#define RCC_PCLK2_Div96                  ((uint32_t)0x0000B800)  /* PCLK2 หาร 96 */
#define RCC_PCLK2_Div128                 ((uint32_t)0x0000F800)  /* PCLK2 หาร 128 */

/* AHB_peripheral */
/* อุปกรณ์ต่อพ่วงบนบัส AHB */
#define RCC_AHBPeriph_DMA1               ((uint32_t)0x00000001)  /* DMA1 - Direct Memory Access */
#define RCC_AHBPeriph_SRAM               ((uint32_t)0x00000004)  /* SRAM - Static RAM */

/* APB2_peripheral */
/* อุปกรณ์ต่อพ่วงบนบัส APB2 (ความเร็วสูง) */
#define RCC_APB2Periph_AFIO              ((uint32_t)0x00000001)  /* AFIO - Alternate Function I/O */
#define RCC_APB2Periph_GPIOA             ((uint32_t)0x00000004)  /* GPIO พอร์ต A */
#define RCC_APB2Periph_GPIOC             ((uint32_t)0x00000010)  /* GPIO พอร์ต C */
#define RCC_APB2Periph_GPIOD             ((uint32_t)0x00000020)  /* GPIO พอร์ต D */
#define RCC_APB2Periph_ADC1              ((uint32_t)0x00000200)  /* ADC1 - Analog to Digital Converter */
#define RCC_APB2Periph_TIM1              ((uint32_t)0x00000800)  /* TIM1 - Timer 1 */
#define RCC_APB2Periph_SPI1              ((uint32_t)0x00001000)  /* SPI1 - Serial Peripheral Interface */
#define RCC_APB2Periph_USART1            ((uint32_t)0x00004000)  /* USART1 - Universal Sync/Async Receiver Transmitter */

/* APB1_peripheral */
/* อุปกรณ์ต่อพ่วงบนบัส APB1 (ความเร็วต่ำ) */
#define RCC_APB1Periph_TIM2              ((uint32_t)0x00000001)  /* TIM2 - Timer 2 */
#define RCC_APB1Periph_WWDG              ((uint32_t)0x00000800)  /* WWDG - Window Watchdog */
#define RCC_APB1Periph_I2C1              ((uint32_t)0x00200000)  /* I2C1 - Inter-Integrated Circuit */
#define RCC_APB1Periph_PWR               ((uint32_t)0x10000000)  /* PWR - Power management */

/* Clock_source_to_output_on_MCO_pin */
/* แหล่งนาฬิกาที่จะส่งออกทางขา MCO (Microcontroller Clock Output) */
#define RCC_MCO_NoClock                  ((uint8_t)0x00)  /* ไม่ส่งออกนาฬิกา */
#define RCC_MCO_SYSCLK                   ((uint8_t)0x04)  /* ส่งออก SYSCLK */
#define RCC_MCO_HSI                      ((uint8_t)0x05)  /* ส่งออก HSI */
#define RCC_MCO_HSE                      ((uint8_t)0x06)  /* ส่งออก HSE */
#define RCC_MCO_PLLCLK                   ((uint8_t)0x07)  /* ส่งออก PLLCLK */

/* RCC_Flag */
/* แฟล็กสถานะ RCC */
#define RCC_FLAG_HSIRDY                  ((uint8_t)0x21)  /* HSI พร้อมใช้งาน */
#define RCC_FLAG_HSERDY                  ((uint8_t)0x31)  /* HSE พร้อมใช้งาน */
#define RCC_FLAG_PLLRDY                  ((uint8_t)0x39)  /* PLL ล็อกแล้ว */
#define RCC_FLAG_LSIRDY                  ((uint8_t)0x61)  /* LSI พร้อมใช้งาน */
#define RCC_FLAG_PINRST                  ((uint8_t)0x7A)  /* รีเซ็ตจากขา RESET */
#define RCC_FLAG_PORRST                  ((uint8_t)0x7B)  /* รีเซ็ตจาก Power-On */
#define RCC_FLAG_SFTRST                  ((uint8_t)0x7C)  /* รีเซ็ตจากซอฟต์แวร์ */
#define RCC_FLAG_IWDGRST                 ((uint8_t)0x7D)  /* รีเซ็ตจาก Independent Watchdog */
#define RCC_FLAG_WWDGRST                 ((uint8_t)0x7E)  /* รีเซ็ตจาก Window Watchdog */
#define RCC_FLAG_LPWRRST                 ((uint8_t)0x7F)  /* รีเซ็ตจากโหมดประหยัดพลังงาน */

/* SysTick_clock_source */
/* แหล่งนาฬิกาสำหรับ SysTick */
#define SysTick_CLKSource_HCLK_Div8      ((uint32_t)0xFFFFFFFB)  /* ใช้ HCLK หาร 8 */
#define SysTick_CLKSource_HCLK           ((uint32_t)0x00000004)  /* ใช้ HCLK โดยตรง */

void        RCC_DeInit(void);
void        RCC_HSEConfig(uint32_t RCC_HSE);
ErrorStatus RCC_WaitForHSEStartUp(void);
void        RCC_AdjustHSICalibrationValue(uint8_t HSICalibrationValue);
void        RCC_HSICmd(FunctionalState NewState);
void        RCC_PLLConfig(uint32_t RCC_PLLSource);
void        RCC_PLLCmd(FunctionalState NewState);
void        RCC_SYSCLKConfig(uint32_t RCC_SYSCLKSource);
uint8_t     RCC_GetSYSCLKSource(void);
void        RCC_HCLKConfig(uint32_t RCC_SYSCLK);
void        RCC_ITConfig(uint8_t RCC_IT, FunctionalState NewState);
void        RCC_ADCCLKConfig(uint32_t RCC_PCLK2);
void        RCC_LSICmd(FunctionalState NewState);
void        RCC_GetClocksFreq(RCC_ClocksTypeDef *RCC_Clocks);
void        RCC_AHBPeriphClockCmd(uint32_t RCC_AHBPeriph, FunctionalState NewState);
void        RCC_APB2PeriphClockCmd(uint32_t RCC_APB2Periph, FunctionalState NewState);
void        RCC_APB1PeriphClockCmd(uint32_t RCC_APB1Periph, FunctionalState NewState);
void        RCC_APB2PeriphResetCmd(uint32_t RCC_APB2Periph, FunctionalState NewState);
void        RCC_APB1PeriphResetCmd(uint32_t RCC_APB1Periph, FunctionalState NewState);
void        RCC_ClockSecuritySystemCmd(FunctionalState NewState);
void        RCC_MCOConfig(uint8_t RCC_MCO);
FlagStatus  RCC_GetFlagStatus(uint8_t RCC_FLAG);
void        RCC_ClearFlag(void);
ITStatus    RCC_GetITStatus(uint8_t RCC_IT);
void        RCC_ClearITPendingBit(uint8_t RCC_IT);

#ifdef __cplusplus
}
#endif

#endif /* __CH32V00x_RCC_H */
