/**
 * @file Modbus_transport.h
 * @brief Transport layer ของ Modbus RTU — ภายใน (ไม่ใช่ public API)
 *
 * รองรับ 2 โหมด:
 * - MODBUS_TRANSPORT_USART — SimpleUSART ring buffer (RXNE interrupt)
 * - MODBUS_TRANSPORT_DMA   — DMA RX circular + USART IDLE interrupt + DMA TX
 */

#ifndef __MODBUS_TRANSPORT_H
#define __MODBUS_TRANSPORT_H

#include "Modbus.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief เริ่มต้น transport (USART init + โหมดเฉพาะ)
 * @param mb instance
 */
void MBT_Init(Modbus* mb);

/**
 * @brief ล้างข้อมูล RX ค้างเก่า (เรียกก่อนส่งคำขอทุกครั้ง)
 * @param mb instance
 */
void MBT_FlushRx(Modbus* mb);

/**
 * @brief ส่ง bytes ผ่าน USART (blocking)
 * @param mb   instance
 * @param data ข้อมูล
 * @param len  ความยาว
 */
void MBT_SendBytes(Modbus* mb, const uint8_t* data, uint16_t len);

/**
 * @brief อ่าน 1 byte จาก frame ที่กำลังรับ (blocking พร้อม timeout)
 * @param mb   instance
 * @param byte [out] byte ที่อ่านได้
 * @return MODBUS_OK หรือ MODBUS_ERROR_TIMEOUT
 */
Modbus_Status MBT_ReadByte(Modbus* mb, uint8_t* byte);

#ifdef __cplusplus
}
#endif

#endif /* __MODBUS_TRANSPORT_H */
