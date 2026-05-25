/**
 * ============================================================
 * ex03_String_And_Struct.c
 * โปรแกรมเขียน/อ่านสตริงและโครงสร้างข้อมูลใน Flash
 * (Write/Read string and data structure in Flash)
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
 *   "--- Flash String and Struct ---"
 *   "Writing string..."
 *   "String read: Hello World"
 *   "Writing struct..."
 *   "Struct read: id=12345, value=6789"
 *   "--- Done ---"
 * ============================================================
 * คำเตือน (WARNINGS):
 *   ⚠ ความยาวสตริงสูงสุด = FLASH_MAX_STRING_LENGTH (60 ตัวอักษร รวม null)
 *     (Max string length = FLASH_MAX_STRING_LENGTH (60 chars including null))
 * ============================================================
 */

#include <SimpleHAL.h>                      // รวมไลบรารี SimpleHAL (Include SimpleHAL library)

// --------------------------------------------------------------------------
// โครงสร้างข้อมูลสำหรับทดสอบ (Test data structure)
// --------------------------------------------------------------------------

typedef struct {
    uint32_t id;                             // รหัสประจำตัว (ID number)
    uint32_t value;                          // ค่าที่ต้องการเก็บ (Value to store)
} __attribute__((packed)) SensorData_t;      // packed ป้องกัน padding (packed to prevent padding)

// --------------------------------------------------------------------------
// ฟังก์ชันหลัก (Main function)
// --------------------------------------------------------------------------

int main(void)
{
    SystemCoreClockUpdate();                 // ต้องมาก่อนบรรทัดแรกเสมอ (Must be the very first line)

    // ตัวแปรสตริงและโครงสร้าง (String and struct variables)
    char        strWrite[] = "Hello World";  // สตริงต้นทางสำหรับเขียน (Source string to write)
    char        strRead[FLASH_MAX_STRING_LENGTH] = {0};  // บัฟเฟอร์ปลายทางสำหรับอ่าน (Destination buffer for reading)

    SensorData_t dataWrite;                  // โครงสร้างต้นทางสำหรับเขียน (Source struct to write)
    SensorData_t dataRead;                   // โครงสร้างปลายทางสำหรับอ่าน (Destination struct for reading)

    int32_t     result;                      // ตัวแปรรับค่าผลลัพธ์ (Variable for result value)

    // ที่อยู่ใน Flash (Flash address)
    #define FLASH_ADDR_STRING  0x0800FC00    // ที่อยู่สำหรับเก็บสตริง (Address to store string)
    #define FLASH_ADDR_STRUCT  0x0800FC40    // ที่อยู่สำหรับเก็บโครงสร้าง (Address to store struct)

    // ---- ส่วนเริ่มต้น (Initialization) ----
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);                      // เริ่มต้น USART 115200 baud (Initialize USART)
    USART_Print("--- Flash String and Struct ---\r\n");  // แสดงหัวข้อ (Display title)

    Flash_Init();                            // เริ่มต้นโมดูล Flash (Initialize Flash module)

    // ---- ขั้นตอนที่ 1: ลบหน้าที่จะใช้ (Step 1: Erase pages to use) ----
    USART_Print("Erasing pages...\r\n");    // แจ้งว่ากำลังลบ (Notify erasing)
    Flash_ErasePage(255);                    // ลบหน้า 255 เพื่อเตรียมพื้นที่ (Erase page 255 to prepare space)

    // ---- ขั้นตอนที่ 2: เขียนสตริง (Step 2: Write string) ----
    USART_Print("Writing string...\r\n");   // แจ้งว่าเริ่มเขียนสตริง (Notify writing string)
    result = Flash_WriteString(FLASH_ADDR_STRING, strWrite);  // เขียนสตริงลง Flash (Write string to Flash)
    if (result != 0) {                       // ตรวจสอบว่าสำเร็จหรือไม่ (Check if succeeded)
        USART_Print("ERROR: WriteString failed (code "); USART_PrintNum((int32_t)result); USART_Print(")\r\n");
        while (1);                           // หยุด (Halt)
    }

    // ---- ขั้นตอนที่ 3: อ่านสตริง (Step 3: Read string) ----
    result = Flash_ReadString(FLASH_ADDR_STRING, strRead, FLASH_MAX_STRING_LENGTH);  // อ่านสตริงจาก Flash (Read string from Flash)
    if (result != 0) {                       // ตรวจสอบว่าสำเร็จหรือไม่ (Check if succeeded)
        USART_Print("ERROR: ReadString failed (code "); USART_PrintNum((int32_t)result); USART_Print(")\r\n");
        while (1);                           // หยุด (Halt)
    }
    USART_Print("String read: "); USART_Print(strRead); USART_Print("\r\n");

    // ---- ขั้นตอนที่ 4: เตรียมและเขียนโครงสร้าง (Step 4: Prepare and write struct) ----
    dataWrite.id    = 12345;                 // กำหนดค่า id (Set id value)
    dataWrite.value = 6789;                  // กำหนดค่า value (Set value field)

    USART_Print("Writing struct...\r\n");   // แจ้งว่าเริ่มเขียนโครงสร้าง (Notify writing struct)
    result = Flash_WriteStruct(FLASH_ADDR_STRUCT, &dataWrite, sizeof(SensorData_t));  // เขียนโครงสร้างลง Flash (Write struct to Flash)
    if (result != 0) {                       // ตรวจสอบว่าสำเร็จหรือไม่ (Check if succeeded)
        USART_Print("ERROR: WriteStruct failed (code "); USART_PrintNum((int32_t)result); USART_Print(")\r\n");
        while (1);                           // หยุด (Halt)
    }

    // ---- ขั้นตอนที่ 5: อ่านโครงสร้าง (Step 5: Read struct) ----
    result = Flash_ReadStruct(FLASH_ADDR_STRUCT, &dataRead, sizeof(SensorData_t));  // อ่านโครงสร้างจาก Flash (Read struct from Flash)
    if (result != 0) {                       // ตรวจสอบว่าสำเร็จหรือไม่ (Check if succeeded)
        USART_Print("ERROR: ReadStruct failed (code "); USART_PrintNum((int32_t)result); USART_Print(")\r\n");
        while (1);                           // หยุด (Halt)
    }

    // ---- ขั้นตอนที่ 6: แสดงผล (Step 6: Display results) ----
    USART_Print("Struct read: id="); USART_PrintNum((int32_t)dataRead.id); USART_Print(", value="); USART_PrintNum((int32_t)dataRead.value); USART_Print("\r\n");

    USART_Print("--- Done ---\r\n");        // แจ้งสิ้นสุด (Notify end)

    while (1);                               // วังวนไม่รู้จบ (Infinite loop)
}
