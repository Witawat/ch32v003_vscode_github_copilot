/**
 * @file CircularBuffer.c
 * @brief Circular Buffer Implementation
 */

#include "CircularBuffer.h"
#include <stddef.h>

CircularBuffer_Status CircularBuffer_Init(CircularBuffer* cb, uint8_t* buf, uint16_t size) {
    if (cb == NULL || buf == NULL || size == 0) return CIRCULAR_BUFFER_ERROR;

    cb->buffer = buf;
    cb->size   = size;
    cb->head   = 0;
    cb->tail   = 0;
    cb->count  = 0;
    return CIRCULAR_BUFFER_OK;
}

CircularBuffer_Status CircularBuffer_Push(CircularBuffer* cb, uint8_t data) {
    if (cb == NULL || cb->buffer == NULL) return CIRCULAR_BUFFER_ERROR;
    if (cb->count >= cb->size) return CIRCULAR_BUFFER_FULL;

    cb->buffer[cb->head] = data;
    cb->head = (cb->head + 1) % cb->size;
    cb->count++;
    return CIRCULAR_BUFFER_OK;
}

CircularBuffer_Status CircularBuffer_Pop(CircularBuffer* cb, uint8_t* data) {
    if (cb == NULL || cb->buffer == NULL || data == NULL) return CIRCULAR_BUFFER_ERROR;
    if (cb->count == 0) return CIRCULAR_BUFFER_EMPTY;

    *data = cb->buffer[cb->tail];
    cb->tail = (cb->tail + 1) % cb->size;
    cb->count--;
    return CIRCULAR_BUFFER_OK;
}

CircularBuffer_Status CircularBuffer_Peek(CircularBuffer* cb, uint16_t index, uint8_t* data) {
    if (cb == NULL || cb->buffer == NULL || data == NULL) return CIRCULAR_BUFFER_ERROR;
    if (index >= cb->count) return CIRCULAR_BUFFER_EMPTY;

    uint16_t pos = (cb->tail + index) % cb->size;
    *data = cb->buffer[pos];
    return CIRCULAR_BUFFER_OK;
}

uint16_t CircularBuffer_Available(CircularBuffer* cb) {
    if (cb == NULL) return 0;
    return cb->count;
}

uint16_t CircularBuffer_Remaining(CircularBuffer* cb) {
    if (cb == NULL) return 0;
    return cb->size - cb->count;
}

bool CircularBuffer_IsEmpty(CircularBuffer* cb) {
    return (cb == NULL) ? true : (cb->count == 0);
}

bool CircularBuffer_IsFull(CircularBuffer* cb) {
    return (cb == NULL) ? false : (cb->count >= cb->size);
}

void CircularBuffer_Flush(CircularBuffer* cb) {
    if (cb == NULL) return;
    cb->head  = 0;
    cb->tail  = 0;
    cb->count = 0;
}

CircularBuffer_Status CircularBuffer_PushMulti(CircularBuffer* cb, const uint8_t* data, uint16_t len) {
    if (cb == NULL || cb->buffer == NULL || data == NULL) return CIRCULAR_BUFFER_ERROR;
    if (CircularBuffer_Remaining(cb) < len) return CIRCULAR_BUFFER_FULL;

    for (uint16_t i = 0; i < len; i++) {
        CircularBuffer_Push(cb, data[i]);
    }
    return CIRCULAR_BUFFER_OK;
}

uint16_t CircularBuffer_PopMulti(CircularBuffer* cb, uint8_t* data, uint16_t len) {
    if (cb == NULL || cb->buffer == NULL || data == NULL) return 0;

    uint16_t available = cb->count;
    uint16_t to_read   = (len < available) ? len : available;

    for (uint16_t i = 0; i < to_read; i++) {
        CircularBuffer_Pop(cb, &data[i]);
    }
    return to_read;
}
