/**
 * ตัวอย่าง: External Interrupt + yield()
 *
 * แสดงการใช้งาน:
 * - digitalPinToInterrupt(pin)  — แปลง pin number เป็น EXTI line
 * - attachInterrupt() — ตั้งค่า External Interrupt
 * - yield()  — cooperative multitasking (IWDG_Feed)
 *
 * ผังวงจร:
 * - PC1 -> ปุ่มกด (อีกขาต่อ GND, ใช้ PIN_MODE_INPUT_PULLUP)
 * - PC0 -> LED
 *
 * ผลลัพธ์:
 * - กดปุ่ม PC1 -> เปิด/ปิด LED PC0 (toggle ใน ISR)
 * - main loop เรียก yield() เพื่อ feed watchdog
 *
 * หมายเหตุ:
 * - digitalPinToInterrupt(PC1) = EXTI line 1
 * - CH32V003 มี EXTI 8 lines แชร์กับทุกพินใน line เดียวกัน
 * - IWDG ต้อง init ก่อนถึงจะใช้ yield() ได้
 */
#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>

static volatile uint8_t intr_flag = 0;

void btn_isr(void) {
    digitalToggle(PC0);
    intr_flag = 1;
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    IWDG_SimpleInit(1600);  // Arduino-style: ~1.6s timeout, auto-select best prescaler

    pinMode(PC0, PIN_MODE_OUTPUT);
    pinMode(PC1, PIN_MODE_INPUT_PULLUP);

    uint8_t exti_line = digitalPinToInterrupt(PC1);
    USART_Print("PC1 maps to EXTI line: ");
    USART_PrintNum(exti_line);
    USART_Print("\r\n");

    attachInterrupt(PC1, btn_isr, FALLING);

    USART_Print("Press button on PC1 to toggle LED\r\n");

    while (1) {
        if (intr_flag) {
            intr_flag = 0;
            USART_Print("Interrupt! LED toggled.\r\n");
        }

        yield();
    }
}
