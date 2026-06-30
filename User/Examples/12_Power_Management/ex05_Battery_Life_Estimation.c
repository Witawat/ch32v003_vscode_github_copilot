/**
 * ============================================================
 * Battery Life Estimation — คำนวณจริงด้วย PWR_CalculateBatteryLife()
 * ============================================================
 *
 * Circuit: USART: TX=PD5, RX=PD6
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit()"]
 *     C --> D["Set battery_mAh=1000, active_pct=1, active_mA=20, standby_uA=5"]
 *     D --> E["PWR_CalculateBatteryLife()"]
 *     E --> F["hours / 24 = days"]
 *     F --> G["USART_Print(battery parameters)"]
 *     G --> H["PWR_EstimateStandbyCurrent(PVD=off, AWU=on)"]
 *     H --> I["USART_Print(standby current)"]
 *     I --> J["while(1) Delay_Ms(1000)"]
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    Delay_Ms(100);

    // Parameters: battery_mAh, active_time_percent, active_current_mA, standby_current_uA
    uint16_t battery_mAh = 1000;
    uint8_t  active_pct  = 1;   // 1% active
    uint16_t active_mA   = 20;  // 20mA active
    uint16_t standby_uA  = 5;   // 5uA standby

    uint32_t hours = PWR_CalculateBatteryLife(battery_mAh, active_pct, active_mA, standby_uA);
    uint32_t days = hours / 24;

    USART_Print("=== Battery Life Estimation ===\r\n");
    USART_Print("Battery: "); USART_PrintNum(battery_mAh); USART_Print(" mAh\r\n");
    USART_Print("Active: "); USART_PrintNum(active_pct); USART_Print("% @ ");
    USART_PrintNum(active_mA); USART_Print(" mA\r\n");
    USART_Print("Standby: "); USART_PrintNum(100-active_pct); USART_Print("% @ ");
    USART_PrintNum(standby_uA); USART_Print(" uA\r\n");
    USART_Print("Estimated battery life: ");
    USART_PrintNum((int32_t)hours); USART_Print(" hours (");
    USART_PrintNum((int32_t)days); USART_Print(" days)\r\n");

    // Standby current estimation
    uint32_t standby_cur = PWR_EstimateStandbyCurrent(0, 1);  // PVD off, AWU on
    USART_Print("Estimated standby current: ");
    USART_PrintNum((int32_t)standby_cur); USART_Print(" uA\r\n");

    while (1)
    {
        Delay_Ms(1000);
    }
}
