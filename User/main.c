/**
 * @file main.c
 * @author MAKER WITAWAT (https://www.makerwitawat.com)
 * @brief
 * @version 0.1
 *
 *
 *
 */

/* ============================================================
 *  ตั้งค่าที่นี่จุดเดียว:
 *    1 = เปิด printf  (Development / Debug)
 *    0 = ปิด printf   (Production / ประหยัด Flash)
 * ============================================================ */
#define ENABLE_PRINTF  0

#include <main.h>
#include "SimpleHAL/SimpleHAL.h"
#include "debug.h"

/* Global typedef */

/* Global define */

/* Global Variable */

int main (void) {
    NVIC_PriorityGroupConfig (NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();    

#if ENABLE_PRINTF
    SDI_Printf_Enable();
#endif

    Delay_Ms (1000);

    printf ("SystemClk:%d\r\n", SystemCoreClock);
    printf ("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf ("CH32V003 MAIN CODE..\r\n");
    
    while (1) {
      
    }
}
