/**
 * ============================================================
 * ex02_Config_With_CRC.c
 * โปรแกรมบันทึก/โหลดคอนฟิกพร้อม CRC ตรวจสอบความถูกต้อง
 * (Config save/load with CRC integrity check)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *   CH32V003                    USB-UART
 *   ------                      --------
 *   PD5 (TX)  ----> รับข้อมูล (RX)
 *   PD6 (RX)  <---- ส่งข้อมูล (TX)
 *   GND       ----  GND
 *
 *   ไม่ต้องใช้อุปกรณ์ภายนอก
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   "--- Flash Config with CRC ---"
 *   "Saving config..."
 *   "Loading config..."
 *   "Config OK: brightness=50, version=1"
 *   "--- Done ---"
 * ============================================================
 * คำเตือน (WARNINGS):
 *   ⚠ สมาชิก CRC ต้องเป็น uint16_t และต้องอยู่ฟิลด์สุดท้ายของ struct
 *     (CRC field MUST be uint16_t and the LAST field)
 *   ⚠ ใช้ FLASH_CONFIG_SIZE_NO_CRC หรือ FLASH_SAVE_CONFIG macro
 *     เพื่อให้สะดวกและป้องกันข้อผิดพลาด
 * ============================================================
 */

#include <SimpleHAL.h>                      // รวมไลบรารี SimpleHAL (Include SimpleHAL library)

// --------------------------------------------------------------------------
// โครงสร้างข้อมูลคอนฟิก (Configuration data structure)
// --------------------------------------------------------------------------

// หมายเหตุ: CRC ต้องเป็นฟิลด์สุดท้ายเสมอ (Note: CRC must always be the last field)
typedef struct {
    uint32_t magic;                          // ค่า magic number ใช้ตรวจสอบว่ามีข้อมูลอยู่ (Magic number to verify data presence)
    uint8_t  version;                        // เวอร์ชันของคอนฟิก (Configuration version)
    uint16_t brightness;                     // ค่าความสว่าง 0-255 (Brightness value 0-255)
    uint16_t crc;                            // ค่า CRC สำหรับตรวจสอบความถูกต้อง — ต้องอยู่ฟิลด์สุดท้าย (CRC checksum — MUST be last field)
} __attribute__((packed)) Config_t;          // packed ป้องกันการเพิ่ม padding (packed to prevent padding)

// --------------------------------------------------------------------------
// ฟังก์ชันหลัก (Main function)
// --------------------------------------------------------------------------

int main(void)
{
    SystemCoreClockUpdate();                 // ต้องมาก่อนบรรทัดแรกเสมอ (Must be the very first line)

    // ตัวแปรคอนฟิกสำหรับบันทึกและโหลด (Config variables for save and load)
    Config_t cfgSave;                        // คอนฟิกต้นทางสำหรับบันทึก (Source config to save)
    Config_t cfgLoad;                        // คอนฟิกปลายทางสำหรับโหลด (Destination config after load)
    int32_t  result;                         // ตัวแปรรับค่าผลลัพธ์ (Variable for result value)

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);                      // เริ่มต้นพอร์ตอนุกรม 115200 baud (Initialize USART)

    USART_Print("--- Flash Config with CRC ---\r\n");  // แสดงหัวข้อ (Display title)

    // ---- ขั้นตอนที่ 1: เตรียมข้อมูลคอนฟิก (Step 1: Prepare config data) ----
    cfgSave.magic      = 0xA5A5A5A5;         // กำหนด magic number เพื่อบ่งชี้ว่ามีข้อมูล (Set magic number to indicate valid data)
    cfgSave.version    = 1;                  // เวอร์ชัน 1 (Version 1)
    cfgSave.brightness = 50;                 // ค่าความสว่าง 50% (Brightness 50%)
    // cfgSave.crc จะถูกคำนวณโดย Flash_SaveConfig อัตโนมัติ (crc auto-calculated by Flash_SaveConfig)

    // ---- ขั้นตอนที่ 2: บันทึกคอนฟิกลง Flash (Step 2: Save config to Flash) ----
    USART_Print("Saving config...\r\n");    // แจ้งว่ากำลังบันทึก (Notify saving)
    result = Flash_SaveConfig(&cfgSave, sizeof(Config_t));  // บันทึกคอนฟิกผ่านทาง Flash_SaveConfig (Save config via Flash_SaveConfig)
    if (result != 0) {                       // ตรวจสอบว่าบันทึกสำเร็จหรือไม่ (Check if save succeeded)
        USART_Print("ERROR: Save failed (code "); USART_PrintNum((int32_t)result); USART_Print(")\r\n");
        while (1);                           // หยุดโปรแกรมหากบันทึกไม่สำเร็จ (Halt if save failed)
    }

    // ---- ขั้นตอนที่ 3: โหลดคอนฟิกกลับมา (Step 3: Load config back) ----
    USART_Print("Loading config...\r\n");   // แจ้งว่ากำลังโหลด (Notify loading)
    if (!Flash_LoadConfig(&cfgLoad, sizeof(Config_t))) {  // โหลดคอนฟิกจาก Flash (Load config from Flash)
        USART_Print("ERROR: Load failed!\r\n");
        while (1);                           // หยุดโปรแกรมหากโหลดไม่สำเร็จ (Halt if load failed)
    }

    // ---- ขั้นตอนที่ 4: ตรวจสอบ CRC (Step 4: Verify CRC) ----
    if (!Flash_IsConfigValid()) {            // ตรวจสอบ CRC ของข้อมูลที่โหลดมา (Validate CRC of loaded data)
        USART_Print("ERROR: CRC mismatch!\r\n");  // แจ้งว่า CRC ไม่ถูกต้อง (Report CRC mismatch)
        while (1);                           // หยุดโปรแกรม (Halt)
    }

    // ---- ขั้นตอนที่ 5: แสดงผลลัพธ์ (Step 5: Display results) ----
    USART_Print("Config OK: brightness="); USART_PrintNum((int32_t)cfgLoad.brightness); USART_Print(", version="); USART_PrintNum((int32_t)cfgLoad.version); USART_Print("\r\n");

    // ---- สาธิตการใช้ MACRO (Macro usage demonstration) ----
    // FLASH_SAVE_CONFIG((uint8_t*)&cfgSave, sizeof(Config_t));
    // FLASH_LOAD_CONFIG((uint8_t*)&cfgLoad, sizeof(Config_t));
    // MACRO เหล่านี้ทำงานเหมือน Flash_SaveConfig / Flash_LoadConfig
    // แต่รวม Flash_Init() และ Flash_ErasePage() ไว้ภายใน (These macros internally call init & erase)

    USART_Print("--- Done ---\r\n");        // แจ้งสิ้นสุด (Notify end)

    while (1);                               // วังวนไม่รู้จบ (Infinite loop)
}
