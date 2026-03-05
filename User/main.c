/**
 * @file main.c
 * @author MAKER WITAWAT (https://www.makerwitawat.com)
 * @brief
 * @version 0.1
 *
 *
 *
 */

#include <main.h>
#include "debug.h"

/* Global typedef */

/* Global define */

/* Global Variable */

int main (void) {
    NVIC_PriorityGroupConfig (NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();

#if (SDI_PRINT == SDI_PR_OPEN && DISABLE_PRINTF == PRINTF_ON)
    SDI_Printf_Enable();
#endif

    Delay_Ms (1000);

    printf ("SystemClk:%d\r\n", SystemCoreClock);
    printf ("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf ("CH32V003 MAIN CODE..\r\n");

    while (1) {
      
    }
}
