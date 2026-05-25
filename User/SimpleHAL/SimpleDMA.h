/**
 * @file SimpleDMA.h
 * @brief Simple DMA Library สำหรับ CH32V003 แบบ Arduino-style
 * @version 1.0
 * @date 2025-12-22
 * 
 * @details
 * Library นี้ห่อหุ้ม Hardware DMA ให้ใช้งานง่ายแบบ Arduino
 * รองรับการถ่ายโอนข้อมูลแบบ Memory-to-Memory, Peripheral-to-Memory, Memory-to-Peripheral
 * 
 * **คุณสมบัติ:**
 * - รองรับ 7 DMA channels
 * - 3 โหมดการถ่ายโอน (M2M, P2M, M2P)
 * - Circular buffer mode
 * - Priority management
 * - Callback functions สำหรับ Transfer Complete และ Error
 * - Integration กับ SimpleADC, SimpleUSART, SimpleSPI
 * 
 * **DMA Channels ของ CH32V003:**
 * - Channel 1-7: General purpose DMA channels
 * - แต่ละ channel มี priority ที่ตั้งค่าได้
 * - รองรับ peripheral: ADC, USART, SPI, I2C, TIMx
 * 
 * @example
 * // ตัวอย่างการใช้งาน Memory-to-Memory
 * #include "SimpleDMA.h"
 * 
 * uint8_t src[100], dst[100];
 * 
 * int main(void) {
 *     SystemCoreClockUpdate();
 *     
 *     // Copy memory ด้วย DMA
 *     DMA_MemCopy(dst, src, 100);
 * }
 * 
 * @note CH32V003 มี DMA controller 1 ตัว พร้อม 7 channels
 */

#ifndef __SIMPLE_DMA_H
#define __SIMPLE_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>
#include <stdint.h>

/* ========== Enumerations ========== */

/**
 * @brief DMA Channel definitions
 * 
 * @details CH32V003 มี 7 DMA channels
 * @note Channel number ต่ำกว่ามี priority สูงกว่า (ถ้าตั้ง priority เท่ากัน)
 */
typedef enum {
    DMA_CH1 = 1,  /**< DMA Channel 1 */
    DMA_CH2 = 2,  /**< DMA Channel 2 */
    DMA_CH3 = 3,  /**< DMA Channel 3 */
    DMA_CH4 = 4,  /**< DMA Channel 4 */
    DMA_CH5 = 5,  /**< DMA Channel 5 */
    DMA_CH6 = 6,  /**< DMA Channel 6 */
    DMA_CH7 = 7   /**< DMA Channel 7 */
} DMA_Channel;

/**
 * @brief ทิศทางการถ่ายโอนข้อมูล
 */
typedef enum {
    DMA_DIR_PERIPH_TO_MEM = 0,  /**< Peripheral to Memory */
    DMA_DIR_MEM_TO_PERIPH = 1,  /**< Memory to Peripheral */
    DMA_DIR_MEM_TO_MEM    = 2   /**< Memory to Memory */
} DMA_Direction;

/**
 * @brief ระดับ Priority ของ DMA channel
 */
typedef enum {
    DMA_PRIORITY_LOW       = 0,  /**< Priority ต่ำ */
    DMA_PRIORITY_MEDIUM    = 1,  /**< Priority ปานกลาง */
    DMA_PRIORITY_HIGH      = 2,  /**< Priority สูง */
    DMA_PRIORITY_VERY_HIGH = 3   /**< Priority สูงมาก */
} DMA_Priority;

/**
 * @brief ขนาดของข้อมูลที่ถ่ายโอน
 */
typedef enum {
    DMA_SIZE_BYTE     = 0,  /**< 8-bit (1 byte) */
    DMA_SIZE_HALFWORD = 1,  /**< 16-bit (2 bytes) */
    DMA_SIZE_WORD     = 2   /**< 32-bit (4 bytes) */
} DMA_DataSize;

/**
 * @brief โหมดการทำงานของ DMA
 */
typedef enum {
    DMA_MODE_NORMAL   = 0,  /**< Normal mode - ถ่ายโอนครั้งเดียวแล้วหยุด */
    DMA_MODE_CIRCULAR = 1   /**< Circular mode - ถ่ายโอนวนซ้ำ (ring buffer) */
} DMA_Mode;

/**
 * @brief สถานะของ DMA channel
 */
typedef enum {
    DMA_STATUS_IDLE       = 0,  /**< ไม่ได้ใช้งาน */
    DMA_STATUS_BUSY       = 1,  /**< กำลังถ่ายโอนข้อมูล */
    DMA_STATUS_COMPLETE   = 2,  /**< ถ่ายโอนเสร็จสมบูรณ์ */
    DMA_STATUS_ERROR      = 3   /**< เกิด error */
} DMA_Status;

/**
 * @brief Peripheral ที่รองรับ DMA
 */
typedef enum {
    DMA_PERIPH_ADC1    = 0,  /**< ADC1 */
    DMA_PERIPH_USART1  = 1,  /**< USART1 */
    DMA_PERIPH_SPI1    = 2,  /**< SPI1 */
    DMA_PERIPH_I2C1    = 3,  /**< I2C1 */
    DMA_PERIPH_TIM1    = 4,  /**< Timer 1 */
    DMA_PERIPH_TIM2    = 5   /**< Timer 2 */
} DMA_Peripheral;

/* ========== Type Definitions ========== */

/**
 * @brief Callback function type สำหรับ Transfer Complete
 * @param channel DMA channel ที่เสร็จสิ้น
 */
typedef void (*DMA_TransferCompleteCallback)(DMA_Channel channel);

/**
 * @brief Callback function type สำหรับ Error
 * @param channel DMA channel ที่เกิด error
 */
typedef void (*DMA_ErrorCallback)(DMA_Channel channel);

/**
 * @brief Callback function type สำหรับ Half-Transfer (ใช้กับ ping-pong buffer)
 * @param channel DMA channel ที่ transfer มาครึ่งทาง
 */
typedef void (*DMA_HalfTransferCallback)(DMA_Channel channel);

/**
 * @brief DMA Configuration Structure
 */
typedef struct {
    DMA_Channel channel;           /**< DMA channel ที่ต้องการใช้ */
    DMA_Direction direction;       /**< ทิศทางการถ่ายโอน */
    DMA_Priority priority;         /**< ระดับ priority */
    DMA_DataSize data_size;        /**< ขนาดข้อมูล */
    DMA_Mode mode;                 /**< โหมดการทำงาน (Normal/Circular) */
    uint8_t mem_increment;         /**< เพิ่ม memory address อัตโนมัติ (0=ไม่เพิ่ม, 1=เพิ่ม) */
    uint8_t periph_increment;      /**< เพิ่ม peripheral address อัตโนมัติ (0=ไม่เพิ่ม, 1=เพิ่ม) */
    uint32_t periph_addr;          /**< Peripheral address */
    uint32_t mem_addr;             /**< Memory address */
    uint16_t buffer_size;          /**< จำนวนข้อมูลที่ต้องการถ่ายโอน */
} DMA_Config_t;

/* ========== Function Prototypes ========== */

/* ----- Basic DMA Functions ----- */

/**
 * @brief เริ่มต้นการใช้งาน DMA channel
 * @param config pointer ไปยัง DMA configuration structure
 * 
 * @note ฟังก์ชันนี้จะ:
 *       1. เปิด Clock สำหรับ DMA
 *       2. ตั้งค่า DMA channel ตาม configuration
 *       3. เปิดใช้งาน interrupts (ถ้ามี callback)
 * 
 * @example
 * DMA_Config_t config = {
 *     .channel = DMA_CH1,
 *     .direction = DMA_DIR_MEM_TO_MEM,
 *     .priority = DMA_PRIORITY_HIGH,
 *     .data_size = DMA_SIZE_BYTE,
 *     .mode = DMA_MODE_NORMAL,
 *     .mem_increment = 1,
 *     .periph_increment = 1,
 *     .periph_addr = (uint32_t)src,
 *     .mem_addr = (uint32_t)dst,
 *     .buffer_size = 100
 * };
 * DMA_SimpleInit(&config);
 */
void DMA_SimpleInit(DMA_Config_t* config);

/**
 * @brief เริ่มการถ่ายโอนข้อมูล
 * @param channel DMA channel ที่ต้องการเริ่ม
 * 
 * @example
 * DMA_Start(DMA_CH1);
 */
void DMA_Start(DMA_Channel channel);

/**
 * @brief หยุดการถ่ายโอนข้อมูล
 * @param channel DMA channel ที่ต้องการหยุด
 * 
 * @example
 * DMA_Stop(DMA_CH1);
 */
void DMA_Stop(DMA_Channel channel);

/**
 * @brief ตรวจสอบสถานะของ DMA channel
 * @param channel DMA channel ที่ต้องการตรวจสอบ
 * @return สถานะของ channel
 * 
 * @example
 * if (DMA_GetStatus(DMA_CH1) == DMA_STATUS_COMPLETE) {
 *     printf("Transfer complete!\n");
 * }
 */
DMA_Status DMA_GetStatus(DMA_Channel channel);

/**
 * @brief รอให้การถ่ายโอนเสร็จสิ้น (blocking)
 * @param channel DMA channel ที่ต้องการรอ
 * @param timeout_ms timeout ในหน่วย milliseconds (0 = รอไม่จำกัด)
 * @return 1 = เสร็จสิ้น, 0 = timeout
 * 
 * @example
 * DMA_Start(DMA_CH1);
 * if (DMA_WaitComplete(DMA_CH1, 1000)) {
 *     printf("Transfer done!\n");
 * }
 */
uint8_t DMA_WaitComplete(DMA_Channel channel, uint32_t timeout_ms);

/**
 * @brief ตั้งค่า callback function สำหรับ Transfer Complete
 * @param channel DMA channel
 * @param callback pointer ไปยัง callback function
 * 
 * @example
 * void on_complete(DMA_Channel ch) {
 *     printf("Channel %d complete!\n", ch);
 * }
 * 
 * DMA_SetTransferCompleteCallback(DMA_CH1, on_complete);
 */
void DMA_SetTransferCompleteCallback(DMA_Channel channel, DMA_TransferCompleteCallback callback);

/**
 * @brief ตั้งค่า callback function สำหรับ Error
 * @param channel DMA channel
 * @param callback pointer ไปยัง callback function
 * 
 * @example
 * void on_error(DMA_Channel ch) {
 *     printf("Channel %d error!\n", ch);
 * }
 * 
 * DMA_SetErrorCallback(DMA_CH1, on_error);
 */
void DMA_SetErrorCallback(DMA_Channel channel, DMA_ErrorCallback callback);

/**
 * @brief ตั้งค่า callback function สำหรับ Half-Transfer
 * @param channel DMA channel
 * @param callback pointer ไปยัง callback function
 * 
 * @note ใช้สำหรับ ping-pong buffer: เมื่อ transfer มาครึ่งทาง
 *       callback จะถูกเรียก ให้สลับไปอ่าน buffer ครึ่งแรก
 * 
 * @example
 * void on_half(DMA_Channel ch) {
 *     // buffer ครึ่งแรก (0..N/2) พร้อมอ่าน
 *     process_data(&buf[0], N/2);
 * }
 * 
 * DMA_SetHalfTransferCallback(DMA_CH1, on_half);
 */
void DMA_SetHalfTransferCallback(DMA_Channel channel, DMA_HalfTransferCallback callback);

/**
 * @brief รีเซ็ต DMA channel
 * @param channel DMA channel ที่ต้องการรีเซ็ต
 * 
 * @example
 * DMA_Reset(DMA_CH1);
 */
void DMA_Reset(DMA_Channel channel);

/**
 * @brief ตรวจสอบจำนวนข้อมูลที่เหลืออยู่
 * @param channel DMA channel
 * @return จำนวนข้อมูลที่ยังไม่ได้ถ่ายโอน
 * 
 * @example
 * uint16_t remaining = DMA_GetRemainingCount(DMA_CH1);
 * printf("Remaining: %d bytes\n", remaining);
 */
uint16_t DMA_GetRemainingCount(DMA_Channel channel);

/* ----- Memory Transfer Functions ----- */

/**
 * @brief Copy memory ด้วย DMA (blocking)
 * @param dst pointer ไปยัง destination
 * @param src pointer ไปยัง source
 * @param size จำนวน bytes ที่ต้องการ copy
 * 
 * @note ฟังก์ชันนี้จะรอจนกว่าการ copy จะเสร็จสิ้น
 * 
 * @example
 * uint8_t src[100], dst[100];
 * DMA_MemCopy(dst, src, 100);
 */
void DMA_MemCopy(void* dst, const void* src, uint16_t size);

/**
 * @brief Copy memory ด้วย DMA (non-blocking)
 * @param channel DMA channel ที่ต้องการใช้
 * @param dst pointer ไปยัง destination
 * @param src pointer ไปยัง source
 * @param size จำนวน bytes ที่ต้องการ copy
 * 
 * @note ฟังก์ชันนี้จะ return ทันที ใช้ DMA_GetStatus() เพื่อตรวจสอบสถานะ
 * 
 * @example
 * DMA_MemCopyAsync(DMA_CH1, dst, src, 100);
 * // ทำงานอื่นได้
 * while (DMA_GetStatus(DMA_CH1) != DMA_STATUS_COMPLETE);
 */
void DMA_MemCopyAsync(DMA_Channel channel, void* dst, const void* src, uint16_t size);

/**
 * @brief Set memory ด้วย DMA
 * @param dst pointer ไปยัง destination
 * @param value ค่าที่ต้องการ set
 * @param size จำนวน bytes
 * 
 * @example
 * uint8_t buffer[100];
 * DMA_MemSet(buffer, 0, 100);  // Clear buffer
 */
void DMA_MemSet(void* dst, uint8_t value, uint16_t size);

/* ----- ADC Integration Functions ----- */

/**
 * @brief เริ่มต้น DMA สำหรับ ADC continuous conversion
 * @param channel DMA channel ที่ต้องการใช้
 * @param buffer pointer ไปยัง buffer สำหรับเก็บค่า ADC
 * @param buffer_size จำนวนตัวอย่างที่ต้องการเก็บ
 * @param circular 1 = circular mode, 0 = normal mode
 * 
 * @note ต้องเรียก ADC_SimpleInit() ก่อนใช้ฟังก์ชันนี้
 * 
 * @example
 * uint16_t adc_buffer[100];
 * DMA_ADC_Init(DMA_CH1, adc_buffer, 100, 1);  // Circular mode
 * // เริ่ม ADC continuous conversion
 * ADC_Cmd(ADC1, ENABLE);
 * ADC_SoftwareStartConvCmd(ADC1, ENABLE);
 */
void DMA_ADC_Init(DMA_Channel channel, uint16_t* buffer, uint16_t buffer_size, uint8_t circular);

/**
 * @brief เริ่มต้น DMA สำหรับ ADC multi-channel
 * @param channel DMA channel ที่ต้องการใช้
 * @param buffer pointer ไปยัง buffer สำหรับเก็บค่า ADC
 * @param num_channels จำนวน ADC channels
 * @param samples_per_channel จำนวนตัวอย่างต่อ channel
 * 
 * @example
 * uint16_t adc_buffer[3 * 10];  // 3 channels, 10 samples each
 * DMA_ADC_InitMultiChannel(DMA_CH1, adc_buffer, 3, 10);
 */
void DMA_ADC_InitMultiChannel(DMA_Channel channel, uint16_t* buffer, uint8_t num_channels, uint16_t samples_per_channel);

/* ----- USART Integration Functions ----- */

/**
 * @brief เริ่มต้น DMA สำหรับ USART transmission
 * @param channel DMA channel ที่ต้องการใช้
 * @param buffer pointer ไปยัง TX buffer
 * @param buffer_size ขนาดของ buffer
 * 
 * @note ต้องเรียก USART_SimpleInit() ก่อนใช้ฟังก์ชันนี้
 * 
 * @example
 * uint8_t tx_buffer[256];
 * DMA_USART_InitTx(DMA_CH2, tx_buffer, 256);
 */
void DMA_USART_InitTx(DMA_Channel channel, uint8_t* buffer, uint16_t buffer_size);

/**
 * @brief เริ่มต้น DMA สำหรับ USART reception
 * @param channel DMA channel ที่ต้องการใช้
 * @param buffer pointer ไปยัง RX buffer
 * @param buffer_size ขนาดของ buffer
 * @param circular 1 = circular mode, 0 = normal mode
 * 
 * @note แนะนำให้ใช้ circular mode สำหรับ RX
 * 
 * @example
 * uint8_t rx_buffer[256];
 * DMA_USART_InitRx(DMA_CH3, rx_buffer, 256, 1);  // Circular mode
 */
void DMA_USART_InitRx(DMA_Channel channel, uint8_t* buffer, uint16_t buffer_size, uint8_t circular);

/**
 * @brief ส่งข้อมูลผ่าน USART ด้วย DMA
 * @param channel DMA channel ที่ใช้สำหรับ TX
 * @param data pointer ไปยังข้อมูลที่ต้องการส่ง
 * @param length จำนวน bytes
 * 
 * @example
 * char msg[] = "Hello World!";
 * DMA_USART_Transmit(DMA_CH2, (uint8_t*)msg, strlen(msg));
 */
void DMA_USART_Transmit(DMA_Channel channel, const uint8_t* data, uint16_t length);

/**
 * @brief ตรวจสอบจำนวนข้อมูลที่รับได้ (สำหรับ circular mode)
 * @param channel DMA channel ที่ใช้สำหรับ RX
 * @param buffer_size ขนาดของ RX buffer
 * @return จำนวน bytes ที่รับได้
 * 
 * @example
 * uint16_t received = DMA_USART_GetReceivedCount(DMA_CH3, 256);
 */
uint16_t DMA_USART_GetReceivedCount(DMA_Channel channel, uint16_t buffer_size);

/* ----- SPI Integration Functions ----- */

/**
 * @brief เริ่มต้น DMA สำหรับ SPI transmission
 * @param tx_channel DMA channel สำหรับ TX
 * @param rx_channel DMA channel สำหรับ RX
 * 
 * @note ต้องเรียก SPI_SimpleInit() ก่อนใช้ฟังก์ชันนี้
 * 
 * @example
 * DMA_SPI_Init(DMA_CH4, DMA_CH5);
 */
void DMA_SPI_Init(DMA_Channel tx_channel, DMA_Channel rx_channel);

/**
 * @brief ส่งและรับข้อมูลผ่าน SPI ด้วย DMA
 * @param tx_channel DMA channel สำหรับ TX
 * @param rx_channel DMA channel สำหรับ RX
 * @param tx_data pointer ไปยังข้อมูลที่ต้องการส่ง
 * @param rx_data pointer ไปยัง buffer สำหรับรับข้อมูล
 * @param length จำนวน bytes
 * 
 * @example
 * uint8_t tx[10], rx[10];
 * DMA_SPI_TransferBuffer(DMA_CH4, DMA_CH5, tx, rx, 10);
 */
void DMA_SPI_TransferBuffer(DMA_Channel tx_channel, DMA_Channel rx_channel, 
                           const uint8_t* tx_data, uint8_t* rx_data, uint16_t length);

/* ----- I2C Integration Functions ----- */

/**
 * @brief เริ่มต้น DMA สำหรับ I2C1 transmission
 * @param channel DMA channel (ต้องใช้ DMA_CH4 สำหรับ I2C1_TX)
 * 
 * @note ต้องเรียก I2C_SimpleInit() ก่อนใช้ฟังก์ชันนี้
 * @note CH32V003: I2C1_TX fixed-map ที่ DMA_CH4, I2C1_RX fixed-map ที่ DMA_CH5
 * 
 * @example
 * DMA_I2C_InitTx(DMA_CH4);
 */
void DMA_I2C_InitTx(DMA_Channel channel);

/**
 * @brief เริ่มต้น DMA สำหรับ I2C1 reception
 * @param channel DMA channel (ต้องใช้ DMA_CH5 สำหรับ I2C1_RX)
 * 
 * @note ต้องเรียก I2C_SimpleInit() ก่อนใช้ฟังก์ชันนี้
 * 
 * @example
 * DMA_I2C_InitRx(DMA_CH5);
 */
void DMA_I2C_InitRx(DMA_Channel channel);

/**
 * @brief ส่งและรับข้อมูลผ่าน I2C ด้วย DMA
 * @param tx_channel DMA channel สำหรับ TX
 * @param rx_channel DMA channel สำหรับ RX
 * @param tx_data pointer ไปยังข้อมูลที่ต้องการส่ง
 * @param rx_data pointer ไปยัง buffer สำหรับรับข้อมูล
 * @param length จำนวน bytes
 * 
 * @note I2C DMA ต้องใช้ร่วมกับ I2C_WriteReg/I2C_ReadReg สำหรับ
 *       ส่ง slave address และ register address ก่อน
 * 
 * @example
 * uint8_t tx_buf[32], rx_buf[32];
 * // I2C_WriteReg(I2C1_ADDR, REG_ADDR, data);  // ตั้ง register ก่อน
 * DMA_I2C_Transfer(DMA_CH4, DMA_CH5, tx_buf, rx_buf, 32);
 */
void DMA_I2C_Transfer(DMA_Channel tx_channel, DMA_Channel rx_channel,
                     const uint8_t* tx_data, uint8_t* rx_data, uint16_t length);

/* ----- TIM Integration Functions ----- */

/**
 * @brief เริ่มต้น DMA สำหรับ TIM capture (TIM → RAM)
 * @param channel DMA channel (TIM1_CH4→DMA_CH4, TIM1_CH3→DMA_CH6, TIM2_CH2→DMA_CH7)
 * @param TIMx pointer ไปยัง TIM_TypeDef (TIM1 หรือ TIM2)
 * @param buffer pointer ไปยัง buffer สำหรับเก็บค่าที่จับได้
 * @param length จำนวนค่าที่ต้องการจับ
 * @param ccx_addr address ของ TIM capture register (เช่น &TIM1->CH4CVR)
 * 
 * @note ต้องตั้งค่า TIM input capture ก่อนใช้ฟังก์ชันนี้
 * 
 * @example
 * // จับค่าความถี่สัญญาณจาก TIM1_CH4
 * uint16_t cap_buf[100];
 * DMA_TIM_InitCapture(DMA_CH4, TIM1, cap_buf, 100, (uint32_t)&TIM1->CH4CVR);
 * TIM_ICInit(TIM1, &ic_config);  // ตั้งค่า input capture
 * DMA_Start(DMA_CH4);
 */
void DMA_TIM_InitCapture(DMA_Channel channel, TIM_TypeDef* TIMx,
                        uint16_t* buffer, uint16_t length,
                        uint32_t ccx_addr);

/**
 * @brief เริ่มต้น DMA สำหรับอัปเดต PWM duty cycle (RAM → TIM CCR)
 * @param channel DMA channel (TIM1_CH4→DMA_CH4, TIM1_Update→DMA_CH5, ฯลฯ)
 * @param TIMx pointer ไปยัง TIM_TypeDef
 * @param ccr_addr address ของ TIM CCR register (เช่น &TIM1->CH1CVR)
 * @param waveform pointer ไปยัง lookup table ของ duty cycle
 * @param length จำนวนค่าใน lookup table
 * @param circular 1 = วนซ้ำ (สร้าง waveform ต่อเนื่อง), 0 = ครั้งเดียว
 * 
 * @note ใช้สร้าง waveform แบบ complex (sine, sawtooth, ฯลฯ)
 *       โดย DMA จะเขียนค่า duty cycle ใหม่ทุกครั้งที่ TIM อัปเดต
 * 
 * @example
 * // สร้าง sine wave 64 samples ที่ PWM output
 * uint16_t sine_table[64] = { ... };  // คำนวณล่วงหน้า
 * DMA_TIM_UpdatePWM(DMA_CH5, TIM1, (uint32_t)&TIM1->CH1CVR,
 *                   sine_table, 64, 1);
 * TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);
 * DMA_Start(DMA_CH5);
 */
void DMA_TIM_UpdatePWM(DMA_Channel channel, TIM_TypeDef* TIMx,
                      uint32_t ccr_addr, const uint16_t* waveform,
                      uint16_t length, uint8_t circular);

/**
 * @brief แปลง TIM channel number เป็น CCR register address
 * @param TIMx pointer ไปยัง TIM_TypeDef
 * @param channel TIM channel number (1-4)
 * @return address ของ TIMx->CHxCVR
 * 
 * @example
 * uint32_t ccr1_addr = DMA_TIM_GetCCRAddress(TIM1, 1);  // &TIM1->CH1CVR
 */
uint32_t DMA_TIM_GetCCRAddress(TIM_TypeDef* TIMx, uint8_t channel);

/* ----- USART One-Shot Wrapper ----- */

/**
 * @brief ส่งข้อมูลผ่าน USART DMA แบบ blocking (init + transmit + wait)
 * @param channel DMA channel สำหรับ TX (แนะนำ DMA_CH2 หรือ DMA_CH7)
 * @param data pointer ไปยังข้อมูลที่ต้องการส่ง
 * @param length จำนวน bytes
 * 
 * @note ฟังก์ชันนี้จะ init USART DMA TX, ส่งข้อมูล, และรอจนเสร็จ
 *       ครบจบในฟังก์ชันเดียว
 * 
 * @example
 * uint8_t msg[] = "Hello DMA!\\r\\n";
 * DMA_USART_Send(DMA_CH2, msg, sizeof(msg)-1);
 */
void DMA_USART_Send(DMA_Channel channel, const uint8_t* data, uint16_t length);

/* ----- Simplified analogRead DMA Functions ----- */

/**
 * @brief เริ่มต้น DMA สำหรับ analogRead แบบง่าย (single channel)
 * @param pin GPIO pin ที่ต้องการอ่าน ADC (ต้องเป็น ADC pin)
 * @param buffer pointer ไปยัง buffer สำหรับเก็บค่า ADC
 * @param buffer_size จำนวนตัวอย่างที่ต้องการเก็บ
 * @param continuous 1 = circular mode (อ่านต่อเนื่อง), 0 = normal mode (อ่านครั้งเดียว)
 * 
 * @note ฟังก์ชันนี้จะ:
 *       1. เปิด ADC channel ที่เกี่ยวข้อง
 *       2. ตั้งค่า DMA สำหรับ ADC
 *       3. เริ่ม ADC continuous conversion
 * 
 * @example
 * // อ่านค่า ADC จาก PD2 แบบต่อเนื่อง 100 ตัวอย่าง
 * uint16_t adc_buffer[100];
 * DMA_analogReadStart(PD2, adc_buffer, 100, 1);
 * 
 * // อ่านค่าล่าสุด
 * uint16_t latest = adc_buffer[99];
 */
void DMA_analogReadStart(uint8_t pin, uint16_t* buffer, uint16_t buffer_size, uint8_t continuous);

/**
 * @brief อ่านค่าเฉลี่ยจาก DMA buffer
 * @param buffer pointer ไปยัง ADC buffer
 * @param buffer_size ขนาดของ buffer
 * @return ค่าเฉลี่ย ADC
 * 
 * @example
 * uint16_t avg = DMA_analogReadAverage(adc_buffer, 100);
 * float voltage = ADC_ToVoltage(avg, 3.3);
 */
uint16_t DMA_analogReadAverage(uint16_t* buffer, uint16_t buffer_size);

/**
 * @brief หยุดการอ่าน ADC ด้วย DMA
 * 
 * @example
 * DMA_analogReadStop();
 */
void DMA_analogReadStop(void);

/**
 * @brief ตรวจสอบว่า DMA ADC กำลังทำงานหรือไม่
 * @return 1 = กำลังทำงาน, 0 = หยุดแล้ว
 * 
 * @example
 * if (DMA_analogReadBusy()) {
 *     printf("ADC DMA is running\n");
 * }
 */
uint8_t DMA_analogReadBusy(void);

/**
 * @brief เปลี่ยน DMA channel สำหรับ analogRead DMA
 * @param channel DMA channel ที่ต้องการใช้
 * 
 * @note ต้องเรียกก่อน DMA_analogReadStart()
 *       default คือ DMA_CH1
 * 
 * @example
 * DMA_SetAnalogReadChannel(DMA_CH2);
 * DMA_analogReadStart(PD2, buf, 100, 1);
 */
void DMA_SetAnalogReadChannel(DMA_Channel channel);

/* ----- Helper Functions ----- */

/**
 * @brief แปลง DMA_Channel เป็น DMA_Channel_TypeDef*
 * @param channel DMA channel enum
 * @return pointer ไปยัง DMA channel structure
 */
DMA_Channel_TypeDef* DMA_GetChannelBase(DMA_Channel channel);

/**
 * @brief แปลง DMA_Channel เป็น IRQn
 * @param channel DMA channel enum
 * @return IRQ number
 */
IRQn_Type DMA_GetChannelIRQn(DMA_Channel channel);

/**
 * @brief เปิดใช้งาน DMA interrupt
 * @param channel DMA channel
 * @param enable 1 = เปิด, 0 = ปิด
 */
void DMA_EnableInterrupt(DMA_Channel channel, uint8_t enable);

#ifdef __cplusplus
}
#endif

#endif  // __SIMPLE_DMA_H
