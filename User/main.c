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

#include <main.h>
#include "debug.h"

/* Global typedef */

/* Global define */

/* Global Variable */

int main(void) {
    // 1. System init (Timer_Init ถูกเรียกอัตโนมัติจาก SimpleDelay constructor)
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();

#if ENABLE_PRINTF
    SDI_Printf_Enable();
#endif

    // 3. Debug print
    Delay_Ms(100);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf("CH32V003 MAIN CODE..\r\n");

    // 4. Application init

    // 5. Main loop
    while (1) {

    }
}
