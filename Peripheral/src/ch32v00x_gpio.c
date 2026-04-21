/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_gpio.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/01/09
 * Description        : This file provides all the GPIO firmware functions.
 *                      ไฟล์นี้มีฟังก์ชันเฟิร์มแวร์ GPIO ทั้งหมด
 *                      ใช้สำหรับควบคุมขาอินพุต/เอาต์พุตอเนกประสงค์
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include <ch32v00x_gpio.h>
#include <ch32v00x_rcc.h>

/* MASK */
/* มาสก์สำหรับจัดการบิต */
#define LSB_MASK                  ((uint16_t)0xFFFF)
#define DBGAFR_POSITION_MASK      ((uint32_t)0x000F0000)
#define DBGAFR_SDI_MASK           ((uint32_t)0xF8FFFFFF)
#define DBGAFR_LOCATION_MASK      ((uint32_t)0x00200000)
#define DBGAFR_NUMBITS_MASK       ((uint32_t)0x00100000)

/*********************************************************************
 * @fn      GPIO_DeInit
 *
 * @brief   Deinitializes the GPIOx peripheral registers to their default
 *        reset values.
 *        รีเซ็ตเรจิสเตอร์ของพอร์ต GPIO กลับสู่ค่าเริ่มต้น
 *
 * @param   GPIOx - where x can be (A..D) to select the GPIO peripheral.
 *                  พอร์ต GPIO ที่ต้องการรีเซ็ต (A, C หรือ D)
 *
 * @return  none
 */
void GPIO_DeInit(GPIO_TypeDef *GPIOx)
{
    if(GPIOx == GPIOA)
    {
        RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA, DISABLE);
    }
    else if(GPIOx == GPIOC)
    {
        RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOC, ENABLE);
        RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOC, DISABLE);
    }
    else if(GPIOx == GPIOD)
    {
        RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOD, ENABLE);
        RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOD, DISABLE);
    }
}

/*********************************************************************
 * @fn      GPIO_AFIODeInit
 *
 * @brief   Deinitializes the Alternate Functions (remap, event control
 *        and EXTI configuration) registers to their default reset values.
 *        รีเซ็ตเรจิสเตอร์ฟังก์ชันสำรองกลับสู่ค่าเริ่มต้น
 *
 * @return  none
 */
void GPIO_AFIODeInit(void)
{
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_AFIO, DISABLE);
}

/*********************************************************************
 * @fn      GPIO_Init
 *
 * @brief   Initializes the GPIOx peripheral according to the specified
 *        parameters in the GPIO_InitStruct.
 *        ตั้งค่าพอร์ต GPIO ตามพารามิเตอร์ที่กำหนด
 *
 * @param   GPIOx - where x can be (A..D) to select the GPIO peripheral.
 *                  พอร์ต GPIO ที่ต้องการตั้งค่า
 * @param   GPIO_InitStruct - pointer to a GPIO_InitTypeDef structure that
 *        contains the configuration information for the specified GPIO peripheral.
 *        โครงสร้างที่มีข้อมูลการตั้งค่า GPIO
 *
 * @return  none
 */
void GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_InitStruct)
{
    uint32_t currentmode = 0x00, currentpin = 0x00, pinpos = 0x00, pos = 0x00;
    uint32_t tmpreg = 0x00, pinmask = 0x00;

    currentmode = ((uint32_t)GPIO_InitStruct->GPIO_Mode) & ((uint32_t)0x0F);

    if((((uint32_t)GPIO_InitStruct->GPIO_Mode) & ((uint32_t)0x10)) != 0x00)
    {
        currentmode |= (uint32_t)GPIO_InitStruct->GPIO_Speed;
    }

    if(((uint32_t)GPIO_InitStruct->GPIO_Pin & ((uint32_t)0x00FF)) != 0x00)
    {
        tmpreg = GPIOx->CFGLR;

        for(pinpos = 0x00; pinpos < 0x08; pinpos++)
        {
            pos = ((uint32_t)0x01) << pinpos;
            currentpin = (GPIO_InitStruct->GPIO_Pin) & pos;

            if(currentpin == pos)
            {
                pos = pinpos << 2;
                pinmask = ((uint32_t)0x0F) << pos;
                tmpreg &= ~pinmask;
                tmpreg |= (currentmode << pos);

                if(GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IPD)
                {
                    GPIOx->BCR = (((uint32_t)0x01) << pinpos);
                }
                else
                {
                    if(GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IPU)
                    {
                        GPIOx->BSHR = (((uint32_t)0x01) << pinpos);
                    }
                }
            }
        }
        GPIOx->CFGLR = tmpreg;
    }
}

/*********************************************************************
 * @fn      GPIO_StructInit
 *
 * @brief   Fills each GPIO_InitStruct member with its default
 *        value.
 *        กำหนดค่าเริ่มต้นให้กับแต่ละสมาชิกของ GPIO_InitStruct
 *
 * @param   GPIO_InitStruct - pointer to a GPIO_InitTypeDef structure
 *      which will be initialized.
 *      โครงสร้างที่จะถูกกำหนดค่าเริ่มต้น
 *
 * @return  none
 */
void GPIO_StructInit(GPIO_InitTypeDef *GPIO_InitStruct)
{
    GPIO_InitStruct->GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStruct->GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct->GPIO_Mode = GPIO_Mode_IN_FLOATING;
}

/*********************************************************************
 * @fn      GPIO_ReadInputDataBit
 *
 * @brief   Reads the specified input port pin.
 *        อ่านค่าของขาอินพุตที่ระบุ
 *
 * @param   GPIOx - where x can be (A..D) to select the GPIO peripheral.
 *                  พอร์ต GPIO ที่ต้องการอ่าน
 * @param    GPIO_Pin - specifies the port bit to read.
 *             This parameter can be GPIO_Pin_x where x can be (0..7).
 *             ขาที่ต้องการอ่าน (0 ถึง 7)
 *
 * @return  The input port pin value.
 *          ค่าของขาอินพุตที่อ่านได้
 */
uint8_t GPIO_ReadInputDataBit(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    uint8_t bitstatus = 0x00;

    if((GPIOx->INDR & GPIO_Pin) != (uint32_t)Bit_RESET)
    {
        bitstatus = (uint8_t)Bit_SET;
    }
    else
    {
        bitstatus = (uint8_t)Bit_RESET;
    }

    return bitstatus;
}

/*********************************************************************
 * @fn      GPIO_ReadInputData
 *
 * @brief   Reads the specified GPIO input data port.
 *        อ่านค่าของพอร์ตอินพุตที่ระบุ
 *
 * @param   GPIOx: where x can be (A..D) to select the GPIO peripheral.
 *                  พอร์ต GPIO ที่ต้องการอ่าน
 *
 * @return  The input port pin value.
 *          ค่าของพอร์ตอินพุตที่อ่านได้
 */
uint16_t GPIO_ReadInputData(GPIO_TypeDef *GPIOx)
{
    return ((uint16_t)GPIOx->INDR);
}

/*********************************************************************
 * @fn      GPIO_ReadOutputDataBit
 *
 * @brief   Reads the specified output data port bit.
 *        อ่านค่าของขาเอาต์พุตที่ระบุ
 *
 * @param   GPIOx - where x can be (A..D) to select the GPIO peripheral.
 *          GPIO_Pin - specifies the port bit to read.
 *            This parameter can be GPIO_Pin_x where x can be (0..7).
 *            ขาที่ต้องการอ่าน (0 ถึง 7)
 *
 * @return  none
 */
uint8_t GPIO_ReadOutputDataBit(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    uint8_t bitstatus = 0x00;

    if((GPIOx->OUTDR & GPIO_Pin) != (uint32_t)Bit_RESET)
    {
        bitstatus = (uint8_t)Bit_SET;
    }
    else
    {
        bitstatus = (uint8_t)Bit_RESET;
    }

    return bitstatus;
}

/*********************************************************************
 * @fn      GPIO_ReadOutputData
 *
 * @brief   Reads the specified GPIO output data port.
 *        อ่านค่าของพอร์ตเอาต์พุตที่ระบุ
 *
 * @param   GPIOx - where x can be (A..D) to select the GPIO peripheral.
 *                  พอร์ต GPIO ที่ต้องการอ่าน
 *
 * @return  GPIO output port pin value.
 *          ค่าของพอร์ตเอาต์พุตที่อ่านได้
 */
uint16_t GPIO_ReadOutputData(GPIO_TypeDef *GPIOx)
{
    return ((uint16_t)GPIOx->OUTDR);
}

/*********************************************************************
 * @fn      GPIO_SetBits
 *
 * @brief   Sets the selected data port bits.
 *        ตั้งค่าบิตที่เลือกของพอร์ต
 *
 * @param   GPIOx - where x can be (A..D) to select the GPIO peripheral.
 *          GPIO_Pin - specifies the port bits to be written.
 *            This parameter can be any combination of GPIO_Pin_x where x can be (0..7).
 *            บิตที่ต้องการตั้งค่า (0 ถึง 7)
 *
 * @return  none
 */
void GPIO_SetBits(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    GPIOx->BSHR = GPIO_Pin;
}

/*********************************************************************
 * @fn      GPIO_ResetBits
 *
 * @brief   Clears the selected data port bits.
 *        ล้างค่าบิตที่เลือกของพอร์ต
 *
 * @param   GPIOx - where x can be (A..D) to select the GPIO peripheral.
 *          GPIO_Pin - specifies the port bits to be written.
 *            This parameter can be any combination of GPIO_Pin_x where x can be (0..7).
 *            บิตที่ต้องการล้างค่า (0 ถึง 7)
 *
 * @return  none
 */
void GPIO_ResetBits(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    GPIOx->BCR = GPIO_Pin;
}

/*********************************************************************
 * @fn      GPIO_WriteBit
 *
 * @brief   Sets or clears the selected data port bit.
 *        ตั้งค่าหรือล้างค่าบิตที่เลือกของพอร์ต
 *
 * @param   GPIO_Pin - specifies the port bit to be written.
 *            This parameter can be one of GPIO_Pin_x where x can be (0..7).
 *            ขาที่ต้องการตั้งค่าหรือล้างค่า (0 ถึง 7)
 *          BitVal - specifies the value to be written to the selected bit.
 *            Bit_RESET - to clear the port pin.
 *            Bit_SET - to set the port pin.
 *            ค่าที่ต้องการตั้งค่าหรือล้างค่า
 *
 * @return  none
 */
void GPIO_WriteBit(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, BitAction BitVal)
{
    if(BitVal != Bit_RESET)
    {
        GPIOx->BSHR = GPIO_Pin;
    }
    else
    {
        GPIOx->BCR = GPIO_Pin;
    }
}

/*********************************************************************
 * @fn      GPIO_Write
 *
 * @brief   Writes data to the specified GPIO data port.
 *        เขียนค่าลงในพอร์ต GPIO
 *
 * @param   GPIOx - where x can be (A..D) to select the GPIO peripheral.
 *          PortVal - specifies the value to be written to the port output data register.
 *          ค่าที่ต้องการเขียนลงในพอร์ต
 *
 * @return  none
 */
void GPIO_Write(GPIO_TypeDef *GPIOx, uint16_t PortVal)
{
    GPIOx->OUTDR = PortVal;
}

/*********************************************************************
 * @fn      GPIO_PinLockConfig
 *
 * @brief   Locks GPIO Pins configuration registers.
 *        ล็อกการตั้งค่าของขา GPIO
 *
 * @param   GPIOx - where x can be (A..D) to select the GPIO peripheral.
 *          GPIO_Pin - specifies the port bit to be written.
 *            This parameter can be any combination of GPIO_Pin_x where x can be (0..7).
 *            ขาที่ต้องการล็อก (0 ถึง 7)
 *
 * @return  none
 */
void GPIO_PinLockConfig(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    uint32_t tmp = 0x00010000;

    tmp |= GPIO_Pin;
    GPIOx->LCKR = tmp;
    GPIOx->LCKR = GPIO_Pin;
    GPIOx->LCKR = tmp;
    tmp = GPIOx->LCKR;
    tmp = GPIOx->LCKR;
}

/*********************************************************************
 * @fn      GPIO_PinRemapConfig
 *
 * @brief   Changes the mapping of the specified pin.
 *        เปลี่ยนการแมปปิ้งของขาที่ระบุ
 *
 * @param   GPIO_Remap - selects the pin to remap.
 *            GPIO_Remap_SPI1 - SPI1 Alternate Function mapping
 *            GPIO_PartialRemap_I2C1 - I2C1 Partial Alternate Function mapping
 *            GPIO_FullRemap_I2C1 - I2C1 Full Alternate Function mapping
 *            GPIO_PartialRemap1_USART1 - USART1 Partial1 Alternate Function mapping
 *            GPIO_PartialRemap2_USART1 - USART1 Partial2 Alternate Function mapping
 *            GPIO_FullRemap_USART1 - USART1 Full Alternate Function mapping
 *            GPIO_PartialRemap1_TIM1 - TIM1 Partial1 Alternate Function mapping
 *            GPIO_PartialRemap2_TIM1 - TIM1 Partial2 Alternate Function mapping
 *            GPIO_FullRemap_TIM1 - TIM1 Full Alternate Function mapping
 *            GPIO_PartialRemap1_TIM2 - TIM2 Partial1 Alternate Function mapping
 *            GPIO_PartialRemap2_TIM2 - TIM2 Partial2 Alternate Function mapping
 *            GPIO_FullRemap_TIM2 - TIM2 Full Alternate Function mapping
 *            GPIO_Remap_PA1PA2 - PA1_PA2 Alternate Function mapping
 *            GPIO_Remap_ADC1_ETRGINJ - ADC1 External Trigger Injected Conversion remapping
 *            GPIO_Remap_ADC1_ETRGREG - ADC1 External Trigger Regular Conversion remapping
 *            GPIO_Remap_LSI_CAL - LSI calibration Alternate Function mapping
 *            GPIO_Remap_SDI_Disable - SDI Disabled
 *          NewState - ENABLE or DISABLE.
 *          สถานะการเปิด/ปิด
 *
 * @return  none
 */
void GPIO_PinRemapConfig(uint32_t GPIO_Remap, FunctionalState NewState)
{
    uint32_t tmp = 0x00, tmp1 = 0x00, tmpreg = 0x00, tmpmask = 0x00;

    tmpreg = AFIO->PCFR1;

    tmpmask = (GPIO_Remap & DBGAFR_POSITION_MASK) >> 0x10;
    tmp = GPIO_Remap & LSB_MASK;

    if((GPIO_Remap & 0x10000000) == 0x10000000)
    {
        tmpreg &= ~((1<<1) | (1<<22));
        tmpreg |= ~DBGAFR_SDI_MASK;
        if(NewState != DISABLE)
        {
            tmpreg |= (GPIO_Remap & 0xEFFFFFFF);
        }

    }
    else if((GPIO_Remap & 0x80000000) == 0x80000000)
    {
        tmpreg &= ~((1<<2) | (1<<21));
        tmpreg |= ~DBGAFR_SDI_MASK;
        if(NewState != DISABLE)
        {
            tmpreg |= (GPIO_Remap & 0x7FFFFFFF);
        }

    }
    else if((GPIO_Remap & (DBGAFR_LOCATION_MASK | DBGAFR_NUMBITS_MASK)) == (DBGAFR_LOCATION_MASK | DBGAFR_NUMBITS_MASK))/* SDI */
    {
        tmpreg &= DBGAFR_SDI_MASK;
        AFIO->PCFR1 &= DBGAFR_SDI_MASK;

        if(NewState != DISABLE)
        {
            tmpreg |= (tmp << ((GPIO_Remap >> 0x15) * 0x10));
        }
    }
    else if((GPIO_Remap & DBGAFR_NUMBITS_MASK) == DBGAFR_NUMBITS_MASK)/* [15:0] 2bit */
    {
        tmp1 = ((uint32_t)0x03) << tmpmask;
        tmpreg &= ~tmp1;
        tmpreg |= ~DBGAFR_SDI_MASK;

        if(NewState != DISABLE)
        {
            tmpreg |= (tmp << ((GPIO_Remap >> 0x15) * 0x10));
        }
    }
    else/* [31:0] 1bit */
    {
        tmpreg &= ~(tmp << ((GPIO_Remap >> 0x15) * 0x10));
        tmpreg |= ~DBGAFR_SDI_MASK;

        if(NewState != DISABLE)
        {
            tmpreg |= (tmp << ((GPIO_Remap >> 0x15) * 0x10));
        }
    }


     AFIO->PCFR1 = tmpreg;
}

/*********************************************************************
 * @fn      GPIO_EXTILineConfig
 *
 * @brief   Selects the GPIO pin used as EXTI Line.
 *        เลือกขา GPIO ที่ใช้เป็นเส้น EXTI
 *
 * @param   GPIO_PortSource - selects the GPIO port to be used as source for EXTI lines.
 *            This parameter can be GPIO_PortSourceGPIOx where x can be (A..D).
 *            พอร์ต GPIO ที่จะใช้เป็นแหล่งของเส้น EXTI
 *          GPIO_PinSource - specifies the EXTI line to be configured.
 *            This parameter can be GPIO_PinSourcex where x can be (0..7).
 *            เส้น EXTI ที่ต้องการตั้งค่า (0 ถึง 7)
 *
 * @return  none
 */
void GPIO_EXTILineConfig(uint8_t GPIO_PortSource, uint8_t GPIO_PinSource)
{
    uint32_t tmp = 0x00;

    tmp = ((uint32_t)(3<<(GPIO_PinSource<<1)));
    AFIO->EXTICR &= ~tmp;
    AFIO->EXTICR |= ((uint32_t)(GPIO_PortSource<<(GPIO_PinSource<<1)));
}

/*********************************************************************
 * @fn      GPIO_IPD_Unused
 *
 * @brief   Configure unused GPIO as input pull-up.
 *        กำหนดขา GPIO ที่ไม่ได้ใช้เป็นอินพุตดึงขึ้น
 *
 * @param   none
 *
 * @return  none
 */
void GPIO_IPD_Unused(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    uint32_t chip = 0;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC, ENABLE);
    chip =  *( uint32_t * )0x1FFFF7C4 & (~0x000000F0);
    switch(chip)
    {
        case 0x00320500:     //CH32V003A4M6
        {
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_2\
                                         |GPIO_Pin_3;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
            GPIO_Init(GPIOC, &GPIO_InitStructure);
            break;
        }
        case 0x00330500:     //CH32V003J4M6
        {
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_2|GPIO_Pin_3\
                                          |GPIO_Pin_7;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_3\
                                          |GPIO_Pin_5|GPIO_Pin_6\
                                          |GPIO_Pin_7;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
            GPIO_Init(GPIOC, &GPIO_InitStructure);
            break;
        }
        default:
        {
            break;
        }

    }

}



