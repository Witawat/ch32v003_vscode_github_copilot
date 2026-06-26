/**
 * ============================================================
 * ตัวอย่างที่ 3: Data Logger to Flash
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *   Potentiometer -> center pin -> PD2 (ADC input)
 *   Potentiometer -> left pin -> 3.3V
 *   Potentiometer -> right pin -> GND
 *   USB-Serial: TX=PD5, RX=PD6
 *   Button -> PC1 -> 10kΩ pull-up -> 3.3V (กดเพื่ออ่าน log)
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   ทุก 5 วินาที: อ่าน ADC -> บันทึก Flash -> "Logged #1: ADC=512, V=1.65V @0s"
 *   กด Button: "=== Data Log ===" -> dump ทุก entry
 * ============================================================
 * คำเตือน (WARNINGS):
 *   - Flash มีอายุการเขียนจำกัด (~10,000 cycles)
 *   - ควรเพิ่ม wear leveling สำหรับ production
 *   - Flash struct ต้องมี CRC เพื่อตรวจสอบความถูกต้อง
 * ============================================================
 */

#include <SimpleHAL.h>

#define ADC_PIN  PD2
#define BTN_PIN  PC1
#define LOG_MS   5000
#define MAX_LOG  128

typedef struct {
    uint32_t crc;
    uint32_t timestamp;
    uint16_t adcValue;
    uint16_t reserved;
} LogEntry;

LogEntry logBuffer[MAX_LOG];
uint16_t logCount = 0;
uint32_t flashAddr = 0x08006000;

uint32_t logStart = 0;
uint32_t seconds = 0;

uint32_t CalcCRC32(uint8_t *data, uint16_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFF;
}

uint8_t SaveLogToFlash(LogEntry *entry) {
    entry->crc = CalcCRC32((uint8_t*)entry + 4, sizeof(LogEntry) - 4);
    uint32_t addr = flashAddr + (logCount * sizeof(LogEntry));

    if (logCount >= MAX_LOG) return 0;

    Flash_WriteStruct(addr, (uint8_t*)entry, sizeof(LogEntry));
    return 1;
}

uint8_t LoadLogFromFlash(uint16_t index, LogEntry *entry) {
    if (index >= MAX_LOG) return 0;

    uint32_t addr = flashAddr + (index * sizeof(LogEntry));
    Flash_ReadStruct(addr, (uint8_t*)entry, sizeof(LogEntry));

    uint32_t calcCRC = CalcCRC32((uint8_t*)entry + 4, sizeof(LogEntry) - 4);
    return (calcCRC == entry->crc) ? 1 : 0;
}

void DumpAllLogs(void) {
    USART_Print("\r\n=== Data Log ===\r\n");

    for (uint16_t i = 0; i < logCount; i++) {
        LogEntry entry;
        if (LoadLogFromFlash(i, &entry)) {
            uint16_t secs = (uint16_t)(entry.timestamp / 1000);
            uint32_t mv = (uint32_t)(entry.adcValue * 3300 / 4095);

            USART_Print("Log #");
            USART_PrintNum((int32_t)(i + 1));
            USART_Print(": ADC=");
            USART_PrintNum((int32_t)entry.adcValue);
            USART_Print(", V=");
            USART_PrintNum((int32_t)(mv / 1000));
            USART_Print(".");
            uint16_t frac = (mv % 1000) / 10;
            if (frac < 10) USART_Print("0");
            USART_PrintNum((int32_t)frac);
            USART_Print("V @ ");
            USART_PrintNum((int32_t)secs);
            USART_Print("s\r\n");
        }
    }
    USART_Print("=== End ===\r\n");
}

int main(void) {
    SystemCoreClockUpdate();

    Timer_Init();
    ADC_SimpleInit();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    pinMode(BTN_PIN, PIN_MODE_INPUT_PULLUP);

    USART_Print("Data Logger Ready\r\n");

    logStart = Stopwatch_GetTotalSeconds() * 1000;

    while (1) {
        uint32_t now = Stopwatch_GetTotalSeconds() * 1000;
        if ((now - logStart) >= LOG_MS) {
            uint16_t adcVal = ADC_Read(ADC_CH_PD2);
            uint32_t mv = (uint32_t)(adcVal * 3300 / 4095);

            LogEntry entry;
            entry.timestamp = seconds * 1000;
            entry.adcValue = adcVal;
            entry.crc = 0;
            entry.reserved = 0;

            if (SaveLogToFlash(&entry)) {
                logCount++;
                USART_Print("Logged #");
                USART_PrintNum((int32_t)logCount);
                USART_Print(": ADC=");
                USART_PrintNum((int32_t)adcVal);
                USART_Print(", V=");
                USART_PrintNum((int32_t)(mv / 1000));
                USART_Print(".");
                uint16_t frac = (mv % 1000) / 10;
                if (frac < 10) USART_Print("0");
                USART_PrintNum((int32_t)frac);
                USART_Print("V @ ");
                USART_PrintNum((int32_t)seconds);
                USART_Print("s\r\n");
            }

            seconds += 5;
            logStart = now;
        }

        if (digitalRead(BTN_PIN) == LOW) {
            Delay_Ms(50);
            while (digitalRead(BTN_PIN) == LOW);
            DumpAllLogs();
        }
    }
}
