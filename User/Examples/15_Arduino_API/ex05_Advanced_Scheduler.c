/**
 * ตัวอย่าง: Advanced Task Scheduler (Arduino-Style)
 *
 * แสดงการใช้งานร่วมกันของฟังก์ชัน Arduino API:
 * - millis()  สำหรับ task scheduling แบบ non-blocking
 * - yield()   สำหรับ cooperative multitasking (IWDG_Feed)
 * - _randomMax() / _randomRange() สำหรับสุ่มค่า runtime
 * - dtostrf() สำหรับแปลง float log
 * - USART_Println / USART_PrintFloat สำหรับ formatted output
 *
 * ผังวงจร:
 * - PC0 -> LED1 (task1 blink ทุก 1000ms)
 * - PC1 -> LED2 (task2 blink 200-500ms สุ่ม)
 * - PC2 -> LED3 (task3 blink ทุก 100ms)
 *
 * ผลลัพธ์:
 * - 3 tasks LED ทำงานพร้อมกันแบบ time-sliced โดยไม่ใช้ delay()
 * - แสดง log สถานะทาง USART ทุก 2 วินาที
 * - feed watchdog ผ่าน yield() ป้องกัน reset
 *
 * หมายเหตุ:
 * - IWDG ต้อง init ก่อนใช้ yield()
 * - ทุก task ใช้ millis() แทน delay() เพื่อไม่บล็อก task อื่น
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART + IWDG + pinMode x3"]
 *     C --> D["randomSeed()"]
 *     D --> E["while(1)"]
 *     E --> F{"now - t1 >= 1000?"}
 *     F -->|"Yes"| G["Toggle LED1"]
 *     G --> H{"now - t2 >= task2_int?"}
 *     F -->|"No"| H
 *     H -->|"Yes"| I["Toggle LED2"]
 *     I --> J{"now - t3 >= 100?"}
 *     H -->|"No"| J
 *     J -->|"Yes"| K["Toggle LED3"]
 *     K --> L{"now - t_log >= 2000?"}
 *     J -->|"No"| L
 *     L -->|"Yes"| M["Print status report"]
 *     M --> N["Update task2_interval"]
 *     N --> O["yield()"]
 *     L -->|"No"| O
 *     O --> E
 * ============================================================
 */
#define ENABLE_USART_PRINTLN  1
#define ENABLE_USART_PRINTFLOAT 1
#define CH32V003_PACKAGE  PACKAGE_TSSOP20
/* CH32V003 has no hardware FPU � float/double use software emulation (~800 cycles) */
#include <SimpleHAL.h>

#define LED1  PC0
#define LED2  PC1
#define LED3  PC2

#define TEMP_MIN  25.0f
#define TEMP_MAX  35.0f

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    IWDG_Init(IWDG_PRESCALER_256, 4095);

    pinMode(LED1, PIN_MODE_OUTPUT);
    pinMode(LED2, PIN_MODE_OUTPUT);
    pinMode(LED3, PIN_MODE_OUTPUT);

    randomSeed(Get_CurrentMs());

    uint32_t t_last_1 = 0, t_last_2 = 0, t_last_3 = 0;
    uint32_t t_last_log = 0;
    uint32_t loop_count = 0;
    uint32_t task1_count = 0, task2_count = 0, task3_count = 0;
    uint32_t task2_interval = 350;
    char log_buf[24];

    USART_Println("=== Advanced Task Scheduler ===");
    USART_Println("LED1=1s  LED2=350ms  LED3=100ms");

    while (1) {
        uint32_t now = millis();
        loop_count++;

        if (now - t_last_1 >= 1000) {
            t_last_1 = now;
            digitalToggle(LED1);
            task1_count++;
        }

        if (now - t_last_2 >= task2_interval) {
            t_last_2 = now;
            digitalToggle(LED2);
            task2_count++;
        }

        if (now - t_last_3 >= 100) {
            t_last_3 = now;
            digitalToggle(LED3);
            task3_count++;
        }

        if (now - t_last_log >= 2000) {
            t_last_log = now;

            USART_Println("====== Status Report ======");
            USART_Print("Loops: ");
            USART_PrintlnNum(loop_count);
            USART_Print("Task1: ");
            USART_PrintNum(task1_count);
            USART_Println(" toggles");
            USART_Print("Task2: ");
            USART_PrintNum(task2_count);
            USART_Println(" toggles");
            USART_Print("Task3: ");
            USART_PrintNum(task3_count);
            USART_Println(" toggles");

            float sim_temp = TEMP_MIN
                + (float)_randomMax(100) * (TEMP_MAX - TEMP_MIN) / 100.0f;
            dtostrf(sim_temp, 6, 2, log_buf);
            USART_Print("Sim temp: ");
            USART_Println(log_buf);

            task2_interval = _randomRange(200, 501);
            USART_Print("Task2 interval: ");
            USART_PrintNum(task2_interval);
            USART_Println(" ms");

            USART_Println("===========================");
            USART_Println("");
        }

        yield();
    }
}
