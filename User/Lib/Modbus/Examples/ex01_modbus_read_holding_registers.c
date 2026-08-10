/**
 * @file ex01_modbus_read_holding_registers.c
 * @brief Modbus RTU Master — อ่าน Holding Registers (FC 0x03) ทุก 1 วินาที
 *        โหมด USART (ring buffer) — วิธีใช้พื้นฐานสุด
 *
 * วงจร:  PD5(TX) → Slave RX, PD6(RX) ← Slave TX, GND → GND
 *        (ตัวอย่างนี้สมมุติ slave address = 1)
 *
 * Build: ไม่รวมในโปรเจกต์หลัก (อยู่ในโฟลเดอร์ Examples ที่ exclude ไว้)
 *        คัดลอกไปเป็น main.c หรือเปิดเป็นโปรเจกต์แยก
 */

#define CH32V003_PACKAGE PACKAGE_TSSOP20
#include "SimpleHAL.h"
#include "Modbus.h"

Modbus mb;
uint16_t regs[10];

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();

    /* debug print ใช้ USART1 ตัวเดียวกันกับ Modbus (9600 baud) */
    if (MODBUS_Init(&mb, 1, MODBUS_TRANSPORT_USART, USART_PINS_DEFAULT) != MODBUS_OK) {
        USART_Println("MODBUS_Init FAILED!");
        while (1);
    }
    USART_Println("Modbus Master (USART mode) - Slave=1, read 10 regs, poll 1s");

    while (1) {
        Modbus_Status st = MODBUS_ReadHoldingRegisters(&mb, 0x0000, 10, regs);

        if (st == MODBUS_OK) {
            for (uint8_t i = 0; i < 10; i++) {
                USART_Print("reg[");
                USART_PrintNum(i);
                USART_Print("]=");
                USART_PrintNum(regs[i]);
                USART_Print("  ");
            }
            USART_Println("");
        } else {
            USART_Print("Error: ");
            USART_Print(MODBUS_StatusStr(st));
            if (st == MODBUS_ERROR_EXCEPT) {
                USART_Print(" ex=0x");
                USART_PrintHex(MODBUS_GetLastException(&mb), 1);
            }
            USART_Println("");
        }

        Delay_Ms(1000);
    }
}
