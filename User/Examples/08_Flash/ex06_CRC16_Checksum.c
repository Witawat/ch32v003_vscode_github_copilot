/**
 * @example ex06_CRC16_Checksum.c
 * @brief Flash_CalculateCRC16 — คำนวณ CRC16-CCITT checksum
 *
 * ใช้ CRC16 ตรวจสอบความถูกต้องของข้อมูล ก่อน/หลังบันทึกลง Flash
 *
 * ============================================================
 * คำเตือน (WARNINGS):
 *   ⚠ CRC16-CCITT ใช้ polynomial 0x1021
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit()"]
 *     C --> D["Flash_Init()"]
 *     D --> E["Flash_CalculateCRC16(data)"]
 *     E --> F["USART_Print(CRC16)"]
 *     F --> G["Flash_ErasePage(DATA_PAGE)"]
 *     G --> H["Flash_WriteStruct(data)"]
 *     H --> I["Flash_ReadStruct(readback)"]
 *     I --> J["Flash_CalculateCRC16(readback)"]
 *     J --> K{"CRC match?"}
 *     K -->|"Yes"| L["USART_Print(CRC OK)"]
 *     K -->|"No"| M["USART_Print(CRC FAIL)"]
 *     L --> N["Flash_IsAddressValid()"]
 *     M --> N
 *     N --> O["while(1)"]
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    Flash_Init();

    // ข้อมูลทดสอบ
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};

    // Flash_CalculateCRC16 — CRC16-CCITT (poly 0x1021)
    uint16_t crc = Flash_CalculateCRC16(data, sizeof(data));
    USART_Print("Data: 01 02 03 04 05\r\n");
    USART_Print("CRC16: 0x");
    USART_PrintHex(crc, 1);
    USART_Print("\r\n");

    // ตัวอย่าง: บันทึกข้อมูล + CRC ไว้ตรวจสอบภายหลัง
    Flash_ErasePage(FLASH_DATA_PAGE);
    Flash_WriteStruct(FLASH_DATA_ADDR, data, sizeof(data));

    // อ่านกลับมาตรวจสอบ
    uint8_t readback[sizeof(data)];
    Flash_ReadStruct(FLASH_DATA_ADDR, readback, sizeof(readback));
    uint16_t readback_crc = Flash_CalculateCRC16(readback, sizeof(readback));

    if (readback_crc == crc) {
        USART_Print("CRC verification: OK — data intact\r\n");
    } else {
        USART_Print("CRC verification: FAIL — data corrupted!\r\n");
    }

    // ตรวจสอบ address ก่อนเขียน (Flash_IsAddressValid)
    if (!Flash_IsAddressValid(0x08000000)) {
        USART_Print("0x08000000 is outside storage area — protected\r\n");
    }

    while (1) { Delay_Ms(1000); }
}
