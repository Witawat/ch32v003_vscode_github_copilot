/**
 * ตัวอย่าง: Float Printing + Arduino Println Extensions
 *
 * แสดงการใช้งาน:
 * - dtostrf()  — แปลง float/double เป็น string (lightweight)
 * - USART_Println() — พิมพ์ string + newline
 * - USART_PrintlnNum() — พิมพ์ตัวเลข + newline
 * - USART_PrintlnHex() — พิมพ์เลขฐาน 16 + newline
 * - USART_PrintFloat() — พิมพ์ float
 * - USART_PrintlnFloat() — พิมพ์ float + newline
 *
 * ผังวงจร:
 * - ไม่ต้องใช้อุปกรณ์เพิ่ม (ใช้ USART TX/RX ปกติ)
 *
 * ผลลัพธ์:
 * - แสดง formatted float, integer, hex ทาง USART
 *
 * หมายเหตุ:
 * - ต้อง define ENABLE_USART_PRINTLN=1 และ ENABLE_USART_PRINTFLOAT=1
 *   ก่อน include SimpleHAL.h
 * - dtostrf() ใช้ integer arithmetic ล้วน ไม่ต้องใช้ FPU
 *   ประหยัด flash ~2KB เมื่อเทียบกับ sprintf
 */
#define ENABLE_USART_PRINTLN  1
#define ENABLE_USART_PRINTFLOAT 1
#define CH32V003_PACKAGE  PACKAGE_TSSOP20
/* CH32V003 has no hardware FPU � float/double use software emulation (~800 cycles) */
#include <SimpleHAL.h>

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    USART_Println("=== Float Print Demo ===");

    char buf[24];

    float pi = 3.14159265f;
    dtostrf(pi, 8, 4, buf);
    USART_Print("dtostrf(pi, 8, 4) = ");
    USART_Println(buf);

    dtostrf(-2.71828, 8, 3, buf);
    USART_Print("dtostrf(-e, 8, 3) = ");
    USART_Println(buf);

    dtostrf(123.456789, 10, 5, buf);
    USART_Print("dtostrf(123.456789, 10, 5) = ");
    USART_Println(buf);

    dtostrf(0.001, 8, 3, buf);
    USART_Print("dtostrf(0.001, 8, 3) = ");
    USART_Println(buf);

    USART_Print("USART_PrintFloat(pi, 2) = ");
    USART_PrintFloat(pi, 2);
    USART_Print("\r\n");

    USART_Print("USART_PrintlnFloat(-e, 3) = ");
    USART_PrintlnFloat(-2.71828f, 3);

    USART_Println("");
    USART_Println("=== Println Extensions ===");
    USART_Println("String with automatic newline");
    USART_PrintlnNum(-12345);
    USART_PrintlnHex(0xDEADBEEF, 1);
    USART_PrintlnHex(0xdeadbeef, 0);

    while (1) {
        Delay_Ms(5000);
        USART_Println("Still alive... (every 5s)");
    }
}
