/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v00x_conf.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2020/08/08
 * Description        : Library configuration file.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __CH32V00x_CONF_H
#define __CH32V00x_CONF_H

#include <ch32v00x_adc.h>
#include <ch32v00x_dbgmcu.h>
#include <ch32v00x_dma.h>
#include <ch32v00x_exti.h>
#include <ch32v00x_flash.h>
#include <ch32v00x_gpio.h>
#include <ch32v00x_i2c.h>
#include <ch32v00x_it.h>
#include <ch32v00x_iwdg.h>
#include <ch32v00x_misc.h>
#include <ch32v00x_pwr.h>
#include <ch32v00x_rcc.h>
#include <ch32v00x_spi.h>
#include <ch32v00x_tim.h>
#include <ch32v00x_usart.h>
#include <ch32v00x_wwdg.h>
#include <ch32v00x_opa.h>


// เปิดการใช้งาน printf
#define PRINTF_ON      1
#define PRINTF_OFF     0
#define DISABLE_PRINTF PRINTF_ON                        // ถ้าใช้งานจริงให้ comment บรรทัดนี้
// #define DISABLE_PRINTF PRINTF_OFF

#if DISABLE_PRINTF == 0
    #define printf(...)
#endif

// Macro สำหรับแปลง GPIO Port เป็น RCC Peripheral
#define GPIO_TO_RCC_PERIPH(GPIOx) (           \
    (GPIOx) == GPIOA   ? RCC_APB2Periph_GPIOA \
    : (GPIOx) == GPIOC ? RCC_APB2Periph_GPIOC \
    : (GPIOx) == GPIOD ? RCC_APB2Periph_GPIOD \
                       : 0)

#define ABS(x)                                   ((x) < 0 ? -(x) : (x))
#define ARRAY_SIZE(a)                            (sizeof (a) / sizeof ((a)[0]))
#define CONSTRAIN(x, low, high)                  ((x) < (low) ? (low) : ((x) > (high) ? (high) : (x)))
#define MAX(a, b)                                ((a) > (b) ? (a) : (b))
#define MIN(a, b)                                ((a) < (b) ? (a) : (b))
#define MAP(x, in_min, in_max, out_min, out_max) (((x) - (in_min)) * ((out_max) - (out_min)) / ((in_max) - (in_min)) + (out_min))
#define ROUND(x)                                 (((x) >= 0) ? (long)((x) + 0.5f) : (long)((x)-0.5f))

/* 2) RADIANS/DEGREES/SQ/ROUND: คำนวณเรเดียน/องศา, กำลังสอง, ปัดเศษ */
#define RADIANS(deg)                             ((deg) * (M_PI / 180.0f))
#define DEGREES(rad)                             ((rad) * (180.0f / M_PI))
#define SQ(x)                                    ((x) * (x))
#define ROUND(x)                                 (((x) >= 0) ? (long)((x) + 0.5f) : (long)((x)-0.5f))

/* 3) Bit-manipulation แบบ Arduino */
#define _BV(bit)                                 (1UL << (bit))
#define BIT_READ(value, bit)                     (((value) >> (bit)) & 0x01)
#define BIT_SET(value, bit)                      ((value) |= _BV (bit))
#define BIT_CLEAR(value, bit)                    ((value) &= ~_BV (bit))
#define BIT_TOGGLE(value, bit)                   ((value) ^= _BV (bit))
#define BIT_WRITE(value, bit, b)                 ((b) ? BIT_SET (value, bit) : BIT_CLEAR (value, bit))

/* 4) แยก–รวมไบต์ (Low/High byte) */
#define LOW_BYTE(w)                              ((uint8_t)((w)&0xFF))
#define HIGH_BYTE(w)                             ((uint8_t)(((w) >> 8) & 0xFF))
#define MAKE_WORD(high, low)                     (((uint16_t)(high) << 8) | (uint16_t)(low))

#endif /* __CH32V00x_CONF_H */
