/**
 * @example ex07_Battery_Data_Logger.c
 * @brief Battery-Powered Data Logger — ADC → Flash → Standby → Wake → Repeat
 *
 * @details
 * Flow:
 *   1. วัด VDD + อุณหภูมิ (ADC)
 *   2. บันทึกลง Flash (Flash_SaveConfig)
 *   3. เข้า Standby 60 วิ (PWR_Standby)
 *   4. ตื่นเพราะ AWU → system reset → main() ใหม่ → วนต่อ
 *
 * ใช้ร่วมกับ:
 *   - ADC_GetVDD() — วัดแรงดันแบตเตอรี่
 *   - Flash_SaveConfig/LoadConfig — เก็บบันทึกแบบ non-volatile
 *   - PWR_Standby() — deep sleep ประหยัดไฟสูงสุด (~5µA)
 *   - PWR_WasStandbyWakeup() — รู้ว่าตื่นจาก standby
 *
 * @note ต้องต่อ VDD เข้ากับแบตเตอรี่ (ไม่ใช่ USB) ถึงจะเห็นผล Standby
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["Flash_Init + ADC_Init"]
 *     C --> D{"Standby wakeup?"}
 *     D -->|"Yes"| E["Clear flag + print"]
 *     D -->|"No"| F["Cold start print"]
 *     E --> G["ADC_GetVDD()"]
 *     F --> G
 *     G --> H["ADC_Read() sensor"]
 *     H --> I["Flash_LoadConfig()"]
 *     I --> J["Update + Save log"]
 *     J --> K["Print sample info"]
 *     K --> L{"Battery < 20%?"}
 *     L -->|"Yes"| M["Print warning"]
 *     M --> N["PWR_Standby(60000)"]
 *     L -->|"No"| N
 *     N --> O["System reset on wake"]
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
/* CH32V003 has no hardware FPU � float/double use software emulation (~800 cycles) */
#include "SimpleHAL.h"

typedef struct {
    uint32_t sample_id;
    uint16_t vdd_mv;
    uint16_t last_adc;
    uint16_t crc;  // ต้องเป็น field สุดท้าย — Flash_LoadConfig/SaveConfig ใช้ CRC validation
} LogEntry;

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    Flash_Init();
    ADC_SimpleInit();

    // ตรวจสอบว่า wakeup มาจาก standby หรือไม่
    if (PWR_WasStandbyWakeup()) {
        PWR_ClearStandbyFlag();
        USART_Print("\r\n=== Woke from Standby! ===\r\n");
    } else {
        USART_Print("\r\n=== Battery Data Logger - Cold Start ===\r\n");
    }

    // 1. วัด VDD
    float vdd = ADC_GetVDD();
    uint16_t vdd_mv = (uint16_t)(vdd * 1000);
    USART_Print("VDD: ");
    USART_PrintNum(vdd_mv / 1000);
    USART_Print(".");
    USART_PrintNum(vdd_mv % 1000 / 10);
    USART_Print("V\r\n");

    // 2. วัด ADC (เซ็นเซอร์)
    uint16_t adc = ADC_Read(ADC_CH_PD2);

    // 3. โหลด log เก่าจาก Flash
    LogEntry log;
    if (!Flash_LoadConfig(&log, sizeof(log) - sizeof(log.crc))) {
        // ครั้งแรก — ตั้งค่าเริ่มต้น
        log.sample_id = 0;
        log.vdd_mv = 0;
        log.last_adc = 0;
    }

    // 4. อัปเดต log
    log.sample_id++;
    log.vdd_mv = vdd_mv;
    log.last_adc = adc;

    // 5. บันทึกลง Flash
    Flash_SaveConfig(&log, sizeof(log) - sizeof(log.crc));

    USART_Print("Sample #"); USART_PrintNum(log.sample_id);
    USART_Print(" ADC="); USART_PrintNum(adc);
    USART_Print(" — saved to Flash\r\n");

    // 6. เตือนถ้าแบตต่ำ
    float batt_pct = ADC_GetBatteryPercent(vdd, 3.0f, 4.2f);
    if (batt_pct < 20.0f) {
        USART_Print("WARNING: Low battery! (");
        USART_PrintNum((int32_t)batt_pct);
        USART_Print("%)\r\n");
    }

    // 7. เข้า Standby 60 วินาที
    USART_Print("Entering Standby for 60s...\r\n");
    USART_Flush();
    PWR_Standby(60000);

    // ไม่ถึงตรงนี้ — MCU reset หลัง standby
    while (1) {}
}
