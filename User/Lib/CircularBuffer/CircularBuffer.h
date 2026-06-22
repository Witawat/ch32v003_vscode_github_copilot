/**
 * @file CircularBuffer.h
 * @brief Generic Circular Buffer (FIFO) Library สำหรับ CH32V003
 * @version 1.0
 * @date 2026-06-22
 *
 * @details
 * Library สำหรับจัดเก็บข้อมูลแบบ FIFO (First-In-First-Out) โดยใช้ circular buffer
 * เหมาะสำหรับใช้เป็น receive buffer ของ serial, SPI, I2C หรือเก็บข้อมูลชั่วคราวทั่วไป
 *
 * **คุณสมบัติ:**
 * - ขนาด buffer กำหนดเองได้ (user-provided buffer)
 * - Push/Pop รายการเดี่ยวและหลายรายการ
 * - Peek (ดูค่าโดยไม่ดึงออก)
 * - Thread-safe สำหรับ single-producer, single-consumer
 *
 * @example
 * #include "CircularBuffer.h"
 *
 * uint8_t storage[64];
 * CircularBuffer cb;
 *
 * CircularBuffer_Init(&cb, storage, sizeof(storage));
 * CircularBuffer_Push(&cb, 0x55);
 * CircularBuffer_Push(&cb, 0xAA);
 *
 * uint8_t data;
 * while (CircularBuffer_Pop(&cb, &data) == CIRCULAR_BUFFER_OK) {
 *     printf("0x%02X ", data);
 * }
 *
 * @note ไม่ใช้ทรัพยากรฮาร์ดแวร์ใดๆ ไม่ต้อง init อะไรก่อนใช้
 */

#ifndef __CIRCULAR_BUFFER_H
#define __CIRCULAR_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ========== Status ========== */

typedef enum {
    CIRCULAR_BUFFER_OK    = 0,
    CIRCULAR_BUFFER_FULL  = 1,
    CIRCULAR_BUFFER_EMPTY = 2,
    CIRCULAR_BUFFER_ERROR = 3
} CircularBuffer_Status;

/* ========== Instance ========== */

typedef struct {
    uint8_t*  buffer;
    uint16_t  head;
    uint16_t  tail;
    uint16_t  size;
    uint16_t  count;
} CircularBuffer;

/* ========== Function Prototypes ========== */

CircularBuffer_Status CircularBuffer_Init(CircularBuffer* cb, uint8_t* buf, uint16_t size);

CircularBuffer_Status CircularBuffer_Push(CircularBuffer* cb, uint8_t data);

CircularBuffer_Status CircularBuffer_Pop(CircularBuffer* cb, uint8_t* data);

CircularBuffer_Status CircularBuffer_Peek(CircularBuffer* cb, uint16_t index, uint8_t* data);

uint16_t CircularBuffer_Available(CircularBuffer* cb);

uint16_t CircularBuffer_Remaining(CircularBuffer* cb);

bool CircularBuffer_IsEmpty(CircularBuffer* cb);

bool CircularBuffer_IsFull(CircularBuffer* cb);

void CircularBuffer_Flush(CircularBuffer* cb);

CircularBuffer_Status CircularBuffer_PushMulti(CircularBuffer* cb, const uint8_t* data, uint16_t len);

uint16_t CircularBuffer_PopMulti(CircularBuffer* cb, uint8_t* data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __CIRCULAR_BUFFER_H */
