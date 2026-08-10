/**
 * @file ex03_modbus_dma_mode.c
 * @brief Modbus RTU Master — โหมด DMA (DMA RX circular + IDLE interrupt + DMA TX)
 *        อ่าน Input Registers (FC 0x04) ทุก 500ms
 *
 * จุดต่างจากโหมด USART: เปลี่ยนแค่ MODBUS_TRANSPORT_USART → MODBUS_TRANSPORT_DMA
 * - RX: DMA_CH3 รับทุก byte โดย CPU ไม่ถูก interrupt — จับจบเฟรมด้วย USART IDLE
 * - TX: DMA_CH2 ส่งคำขอ
 *
 * ⚠️ DMA_CH2 + DMA_CH3 ถูกยึดโดย Modbus — ห้ามใช้กับ DMA ตัวอื่น (เช่น ADC)
 * ⚠️ RAM: โหมดนี้ใช้เพิ่ม ~1.3KB (RAM ทั้งหมด 2KB)
 *
 * Build: ไม่รวมในโปรเจกต์หลัก — คัดลอกไปเป็น main.c หรือโปรเจกต์แยก
 */

#define CH32V003_PACKAGE PACKAGE_TSSOP20
#include "SimpleHAL.h"
#include "Modbus.h"

Modbus mb;
uint16_t regs[8];

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();

    if (MODBUS_Init(&mb, 1, MODBUS_TRANSPORT_DMA, USART_PINS_DEFAULT) != MODBUS_OK) {
        while (1);
    }
    USART_Println("Modbus Master (DMA mode) - read input regs, poll 500ms");

    while (1) {
        Modbus_Status st = MODBUS_ReadInputRegisters(&mb, 0x0000, 8, regs);

        if (st == MODBUS_OK) {
            USART_Print("reg0=");
            USART_PrintNum(regs[0]);
            USART_Print(" reg1=");
            USART_PrintNum(regs[1]);
            USART_Println("");
        } else {
            USART_Println(MODBUS_StatusStr(st));
        }

        Delay_Ms(500);
    }
}
