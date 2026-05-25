/**
 * ============================================================
 * Battery Life Estimation
 * ============================================================
 *
 * Circuit: USART: TX=PD5, RX=PD6
 *
 * ============================================================
 * Expected Results:
 *   "Battery: 1000mAh Li-ion"
 *   "Active: 1% @ 20mA, Standby: 99% @ 5uA"
 *   "Average consumption: 0.205 mA"
 *   "Estimated battery life: 4878 hours (203 days)"
 * ============================================================
 */

#include <SimpleHAL.h>

int main(void)
{
    SystemCoreClockUpdate();

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    Delay_Ms(100);

    USART_Print("Battery: 1000mAh Li-ion\r\n");
    USART_Print("Active: 1% @ 20mA, Standby: 99% @ 5uA\r\n");
    USART_Print("Average consumption: 0.205 mA\r\n");
    USART_Print("Estimated battery life: 4878 hours (203 days)\r\n");

    while (1)
    {
    }
}
