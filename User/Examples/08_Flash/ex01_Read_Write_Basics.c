/**
 * ============================================================
 * ex01_Read_Write_Basics.c
 * โปรแกรมสาธิตการอ่าน/เขียน Flash พื้นฐาน (Demonstrates basic Flash read/write)
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
 *   ไม่ต้องใช้อุปกรณ์ภายนอก (No external components required)
 *   พอร์ตอนุกรม: 115200 8N1
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   หลังจากรันโปรแกรม จะเห็นข้อความทาง USART:
 *   "--- Flash Read/Write Basics ---"
 *   "Byte: 0x55, HalfWord: 0x1234, Word: 0x12345678"
 *   "--- Done ---"
 * ============================================================
 * คำเตือน (WARNINGS):
 *   ⚠ ต้องลบ (Erase) หน้าก่อนเขียนทุกครั้ง!
 *     Flash สามารถเปลี่ยนบิตจาก 1→0 ได้เท่านั้น
 *     การเปลี่ยน 0→1 ต้องลบทั้งหน้าก่อน
 *   ⚠ Flash มีอายุการเขียนประมาณ 10,000 รอบต่อหน้า
 * ============================================================
 */

#include <SimpleHAL.h>                      // รวมไลบรารี SimpleHAL (Include SimpleHAL library)

// --------------------------------------------------------------------------
// ส่วนหัวของฟังก์ชันหลัก (Main function)
// --------------------------------------------------------------------------

int main(void)
{
    SystemCoreClockUpdate();                 // ต้องมาก่อนบรรทัดแรกเสมอ (Must be the very first line)

    // ตัวแปรสำหรับเก็บค่าที่อ่านจาก Flash (Variables to store values read from Flash)
    uint8_t  bRead  = 0;                     // ตัวแปรไว้เก็บข้อมูล 1 ไบต์ที่อ่านได้ (Variable for 1-byte read data)
    uint16_t hwRead = 0;                     // ตัวแปรไว้เก็บข้อมูลครึ่งคำ (16 บิต) (Variable for half-word, 16-bit read data)
    uint32_t wRead  = 0;                     // ตัวแปรไว้เก็บข้อมูลเต็มคำ (32 บิต) (Variable for word, 32-bit read data)

    // ที่อยู่ Flash ที่จะใช้ (Flash addresses to use) — ใช้หน้าสุดท้าย (Page 255)
    // หน้า 255 เริ่มต้นที่ 0x0800FC00 (ขนาด 64 ไบต์ต่อหน้า)
    #define FLASH_ADDR_BYTE      0x0800FC00  // ที่อยู่สำหรับทดสอบเขียนแบบไบต์ (Address for byte write test)
    #define FLASH_ADDR_HALFWORD  0x0800FC10  // ที่อยู่สำหรับทดสอบเขียนแบบครึ่งคำ (Address for half-word write test)
    #define FLASH_ADDR_WORD      0x0800FC20  // ที่อยู่สำหรับทดสอบเขียนแบบเต็มคำ (Address for word write test)

    // ---- ส่วนเริ่มต้นระบบ (System Initialization) ----

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);                      // เริ่มต้นพอร์ตอนุกรมที่ 115200 baud (Initialize USART at 115200 baud)
    USART_Print("--- Flash Read/Write Basics ---\r\n");  // แสดงข้อความเริ่มต้น (Print start message)

    // ---- ขั้นตอนที่ 1: ลบหน้าที่ยังไม่ได้ใช้ (Step 1: Erase unused page) ----
    // ต้องลบหน้าก่อนเขียน เพราะ Flash เขียน 1→0 ได้อย่างเดียว (Must erase before write — Flash can only write 1→0)
    Flash_Init();                            // เริ่มต้นโมดูล Flash (Initialize Flash module)
    USART_Print("Erasing page 255...\r\n"); // แจ้งว่ากำลังลบหน้า 255 (Notify erasing page 255)
    Flash_ErasePage(255);                    // ลบข้อมูลหน้า Flash ที่ 255 (Erase Flash page 255)

    // ---- ขั้นตอนที่ 2: ทดสอบเขียนและอ่านแบบ Byte (Step 2: Byte write/read test) ----
    USART_Print("Writing byte 0x55 to 0x"); USART_PrintHex(FLASH_ADDR_BYTE, 1); USART_Print("...\r\n");
    Flash_WriteByte(FLASH_ADDR_BYTE, 0x55);  // เขียนค่า 0x55 (85) ลงที่อยู่ Flash (Write value 0x55 to Flash address)
    bRead = Flash_ReadByte(FLASH_ADDR_BYTE); // อ่านค่าหนึ่งไบต์จากที่อยู่เดิม (Read one byte from the same address)
    USART_Print("Byte read: 0x"); USART_PrintHex(bRead, 1); USART_Print("\r\n");

    // ---- ขั้นตอนที่ 3: ทดสอบเขียนและอ่านแบบ HalfWord (16-bit) (Step 3: HalfWord write/read test) ----
    USART_Print("Writing half-word 0x1234 to 0x"); USART_PrintHex(FLASH_ADDR_HALFWORD, 1); USART_Print("...\r\n");
    Flash_WriteHalfWord(FLASH_ADDR_HALFWORD, 0x1234);  // เขียนค่า 0x1234 (4660) (Write value 0x1234)
    hwRead = Flash_ReadHalfWord(FLASH_ADDR_HALFWORD);  // อ่านค่าครึ่งคำ (16 บิต) (Read half-word, 16-bit value)
    USART_Print("HalfWord read: 0x"); USART_PrintHex(hwRead, 1); USART_Print("\r\n");

    // ---- ขั้นตอนที่ 4: ทดสอบเขียนและอ่านแบบ Word (32-bit) (Step 4: Word write/read test) ----
    USART_Print("Writing word 0x12345678 to 0x"); USART_PrintHex(FLASH_ADDR_WORD, 1); USART_Print("...\r\n");
    Flash_WriteWord(FLASH_ADDR_WORD, 0x12345678);  // เขียนค่า 0x12345678 (305419896) (Write value 0x12345678)
    wRead = Flash_ReadWord(FLASH_ADDR_WORD);  // อ่านค่าเต็มคำ (32 บิต) (Read word, 32-bit value)
    USART_Print("Word read: 0x"); USART_PrintHex(wRead, 1); USART_Print("\r\n");

    // ---- ขั้นตอนที่ 5: แสดงผลลัพธ์รวม (Step 5: Display combined results) ----
    USART_Print("\r\n");
    USART_Print("Byte: 0x"); USART_PrintHex(bRead, 1); USART_Print(", HalfWord: 0x"); USART_PrintHex(hwRead, 1); USART_Print(", Word: 0x"); USART_PrintHex(wRead, 1); USART_Print("\r\n");

    USART_Print("--- Done ---\r\n");        // แจ้งสิ้นสุดโปรแกรม (Notify program end)

    // ---- วังวนไม่รู้จบ (Infinite loop) ----
    while (1);                               // หยุดรอ (Wait forever)
}
