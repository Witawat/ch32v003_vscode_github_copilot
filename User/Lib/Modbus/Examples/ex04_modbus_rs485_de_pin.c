/**
 * @file ex04_modbus_rs485_de_pin.c
 * @brief Modbus RTU Master บน RS-485 (MAX485) — ควบคุม DE/RE pin ด้วยมือ
 *        อ่าน Holding Registers ผ่าน bus 2 สาย (A/B)
 *
 * วงจร:
 *   CH32V003      MAX485        Bus
 *   PD5 (TX)  →   DI            —
 *   PD6 (RX)  ←   RO            —
 *   PC0       →   DE (ต่อกับ RE) —
 *   —             A ──────────── A (485+)
 *   —             B ──────────── B (485-)
 *   GND       →   GND
 *
 *   หมายเหตุ: DE=1 ส่ง, DE=0 รับ — ไลบรารีไม่ได้จัดการให้อัตโนมัติ
 *             (ตัวอย่างนี้ตั้ง DE=1 ก่อนทุกคำขอ ใช้ pinMode+digitalWrite)
 *             ต้องมีตัวต้านทาน 120Ω จบสายที่ปลายทั้งสองฝั่ง
 *
 * Build: ไม่รวมในโปรเจกต์หลัก — คัดลอกไปเป็น main.c หรือโปรเจกต์แยก
 */

#define CH32V003_PACKAGE PACKAGE_TSSOP20
#include "SimpleHAL.h"
#include "Modbus.h"

#define DE_PIN  PC0

Modbus mb;
uint16_t regs[10];

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();

    pinMode(DE_PIN, PIN_MODE_OUTPUT);
    digitalWrite(DE_PIN, LOW);   /* เริ่มต้นเป็นรับ (RX) */

    if (MODBUS_Init(&mb, 1, MODBUS_TRANSPORT_USART, USART_PINS_DEFAULT) != MODBUS_OK) {
        while (1);
    }
    USART_Println("Modbus Master over RS-485");

    while (1) {
        /* เข้าสู่โหมดส่งก่อนออกคำขอ แล้วกลับเป็นรับหลังส่งเสร็จ
         * (ทำแบบง่าย: ตั้ง DE=1 รอสั้นๆ → ส่ง → DE=0 — ตัวอย่างแบบ blocking) */
        digitalWrite(DE_PIN, HIGH);
        Delay_Ms(1);

        Modbus_Status st = MODBUS_ReadHoldingRegisters(&mb, 0x0000, 10, regs);

        digitalWrite(DE_PIN, LOW);

        if (st == MODBUS_OK) {
            USART_Print("reg0=");
            USART_PrintNum(regs[0]);
            USART_Println("");
        } else {
            USART_Println(MODBUS_StatusStr(st));
        }

        Delay_Ms(1000);
    }
}
