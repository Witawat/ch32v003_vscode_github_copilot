/**
 * ============================================================
 * TJC Example 3: Touch Coordinate
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *   TJC HMI Display (UART):
 *     TX  -> PD6 (MCU RX)
 *     RX  -> PD5 (MCU TX)
 *     VCC -> 5V หรือ 3.3V
 *     GND -> GND
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - ส่งค่าพิกัด X,Y ที่สัมผัสทาง Serial
 *   - แสดงค่าบนจอ TJC ใน component t_coord
 * ============================================================
 * TJC Editor Setup:
 *   - ส่งคำสั่ง sendxy=1 เพื่อเปิดการส่งพิกัด
 *   - สร้าง text component t_coord สำหรับแสดงผล
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>
#include <stdio.h>
#include "TJC.h"

void OnTouchCoord(TJC_TouchCoord_t *coord) {
    USART_Print("Touch XY: (");
    USART_PrintNum(coord->x);
    USART_Print(", ");
    USART_PrintNum(coord->y);
    USART_Print(") ");
    USART_Print(coord->event_type == 0x01 ? "Press" : "Release");
    USART_Print("\r\n");

    char buf[32];
    snprintf(buf, sizeof(buf), "t_coord.txt=\"(%d,%d)\"", coord->x, coord->y);
    TJC_SendCommand(buf);
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    TJC_Init(BAUD_115200, USART_PINS_DEFAULT);
    TJC_RegisterTouchCoordCallback(OnTouchCoord);

    Delay_Ms(100);
    USART_Print("=== TJC Touch Coordinate ===\r\n");

    TJC_SendCommand("sendxy=1");
    TJC_SendCommand("page 0");

    while (1) {
        TJC_ProcessResponse();
    }
}
