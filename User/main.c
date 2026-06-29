/**
 * @file main.c
 * @author MAKER WITAWAT (https://www.makerwitawat.com)
 * @brief CH32V003 Application Entry Point
 * @version 0.1
 */

/* ============================================================
 *  ตั้งค่าที่นี่จุดเดียว:
 *    1 = เปิด printf  (Development / Debug)
 *    0 = ปิด printf   (Production / ประหยัด Flash)
 * ============================================================ */
#define ENABLE_PRINTF  0

/* ตั้งค่า PACKAGE ของ MCU — เลือกให้ตรงกับแพ็กเกจที่ใช้จริง */
// #define CH32V003_PACKAGE  PACKAGE_SOP8      // SOP-8 (J4M6, 6 pins)
// #define CH32V003_PACKAGE  PACKAGE_SOP16     // SOP-16 (A4M6, 14+ pins)
#define CH32V003_PACKAGE  PACKAGE_TSSOP20   // TSSOP-20 (F4P6, 18 pins) ← default
// #define CH32V003_PACKAGE  PACKAGE_QFN20     // QFN-20 (F4U6, 18 pins)

#include <main.h>
#include "debug.h"

/* Global typedef */

/* Global define */

/* Global Variable */

int main(void) {
    // 1. System init — ต้องเรียกเอง ไม่มี auto-init
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();   // ★ ต้องเป็นบรรทัดแรก — อัปเดต system clock
    Timer_Init();              // ★ ต้องเรียกเอง — เริ่มต้น SysTick สำหรับ Delay/Timer

#if ENABLE_PRINTF
    SDI_Printf_Enable();       // ★ ต้องเรียกก่อน printf() ครั้งแรก
#endif

    // 2. Debug print (เฉพาะเมื่อ ENABLE_PRINTF=1)
#if ENABLE_PRINTF
    Delay_Ms(100);
    printf("SystemClk:%lu\r\n", (unsigned long)SystemCoreClock);
    printf("ChipID:%08lx\r\n", (unsigned long)DBGMCU_GetCHIPID());
    printf("CH32V003 Ready! Package: TSSOP20\r\n");
#endif

    // 3. Application init — ตัวอย่าง: LED blink
    pinMode(PC0, PIN_MODE_OUTPUT);

    // 4. Main loop
    while (1) {
        digitalToggle(PC0);
        Delay_Ms(500);
    }
}
