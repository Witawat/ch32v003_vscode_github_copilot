/**
 * @file ex02_modbus_write_register.c
 * @brief Modbus RTU Master — เขียน Single Register (FC 0x06) และ Single Coil (FC 0x05)
 *        โหมด USART
 *
 * วงจร:  PD5(TX) → Slave RX, PD6(RX) ← Slave TX, GND → GND
 *
 * Build: ไม่รวมในโปรเจกต์หลัก — คัดลอกไปเป็น main.c หรือโปรเจกต์แยก
 */

#define CH32V003_PACKAGE PACKAGE_TSSOP20
#include "SimpleHAL.h"
#include "Modbus.h"

Modbus mb;

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();

    if (MODBUS_Init(&mb, 1, MODBUS_TRANSPORT_USART, USART_PINS_DEFAULT) != MODBUS_OK) {
        while (1);
    }
    USART_Println("Modbus Master - Write test");

    /* เขียนค่า 1000 ไป register 0x0000 */
    Modbus_Status st = MODBUS_WriteSingleRegister(&mb, 0x0000, 1000);
    USART_Println(st == MODBUS_OK ? "Write reg OK" : "Write reg FAIL");

    /* เปิด coil 3 */
    st = MODBUS_WriteSingleCoil(&mb, 3, 1);
    USART_Println(st == MODBUS_OK ? "Coil ON OK" : "Coil ON FAIL");

    /* เขียนหลาย register พร้อมกัน (FC 0x10) */
    uint16_t batch[4] = { 10, 20, 30, 40 };
    st = MODBUS_WriteMultipleRegisters(&mb, 0x0010, 4, batch);
    USART_Println(st == MODBUS_OK ? "Write multi OK" : "Write multi FAIL");

    while (1) {
        /* ปิด-เปิด coil 3 ทุก 2 วินาที (สลับกัน) */
        static uint8_t toggle = 0;
        toggle = !toggle;
        MODBUS_WriteSingleCoil(&mb, 3, toggle);
        Delay_Ms(2000);
    }
}
