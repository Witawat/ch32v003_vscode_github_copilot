/**
 * ============================================================
 * ex04_AutoErase_Operations.c
 * โปรแกรมสาธิตการเขียนแบบ Auto-Erase (ลบอัตโนมัติก่อนเขียน)
 * (Demonstrates auto-erase write operations)
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
 *   "--- Flash Auto-Erase Operations ---"
 *   "Auto-write #1 OK"
 *   "Auto-write #2 OK"
 *   "Auto-write #3 OK"
 *   "Read values: Byte=0x77 HalfWord=0xABCD Word=0xDEADBEEF"
 *   "--- Done ---"
 * ============================================================
 * คำเตือน (WARNINGS):
 *   ⚠ Auto-erase ช้า เพราะต้องลบทั้งหน้าก่อนเขียนแค่ไบต์เดียว
 *     (Auto-erase is SLOW — erases whole page even for 1 byte)
 *   ⚠ ไม่ควรใช้ Auto-erase หากต้องเขียนบ่อย ๆ เพราะเปลืองรอบการลบ/เขียน
 *     (Do NOT use for frequent writes — wastes erase/write cycles)
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>                      // รวมไลบรารี SimpleHAL (Include SimpleHAL library)

// --------------------------------------------------------------------------
// ฟังก์ชันหลัก (Main function)
// --------------------------------------------------------------------------

int main(void)
{
    SystemCoreClockUpdate();                 // ต้องมาก่อนบรรทัดแรกเสมอ (Must be the very first line)

    // ตัวแปรรับค่าที่อ่านได้ (Variables to store read values)
    uint8_t  bRead  = 0;                     // ค่าไบต์ที่อ่านได้ (Read byte value)
    uint16_t hwRead = 0;                     // ค่าครึ่งคำที่อ่านได้ (Read half-word value)
    uint32_t wRead  = 0;                     // ค่าเต็มคำที่อ่านได้ (Read word value)

    // ที่อยู่ใน Flash (Flash addresses)
    #define FL_ADDR_BYTE      0x0800FC00     // ที่อยู่ทดสอบ Auto-Erase ระดับไบต์ (Auto-erase byte address)
    #define FL_ADDR_HALFWORD  0x0800FC10     // ที่อยู่ทดสอบ Auto-Erase ระดับครึ่งคำ (Auto-erase half-word address)
    #define FL_ADDR_WORD      0x0800FC20     // ที่อยู่ทดสอบ Auto-Erase ระดับเต็มคำ (Auto-erase word address)

    // ---- ส่วนเริ่มต้น (Initialization) ----
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);                      // เริ่มต้นพอร์ตอนุกรม (Initialize USART)
    USART_Print("--- Flash Auto-Erase Operations ---\r\n");  // แสดงหัวข้อ (Display title)

    Flash_Init();                            // เริ่มต้นโมดูล Flash (Initialize Flash module)

    // ---- ขั้นตอนที่ 1: Auto-erase ครั้งที่ 1 (Step 1: Auto-erase #1) ----
    // เขียนด้วย Flash_WriteByteWithErase — ถ้ายังไม่เคยลบ จะลบอัตโนมัติ (Auto-erase if not yet erased)
    Flash_WriteByteWithErase(FL_ADDR_BYTE, 0x55);  // เขียน 0x55 ด้วย Auto-Erase (Write 0x55 with auto-erase)
    USART_Print("Auto-write #1 OK\r\n");    // แจ้งครั้งที่ 1 สำเร็จ (Notify #1 success)

    // ---- ขั้นตอนที่ 2: Auto-erase ครั้งที่ 2 (Step 2: Auto-erase #2) ----
    // เขียนทับค่าเดิม — Auto-Erase จะลบหน้าก่อนเขียนเอง (Overwrite — auto-erase will erase page first)
    // ใช้ Flash_WriteHalfWord ปกติ (หน้า�ให้ลบแล้วจาก WriteByteWithErase ด้านบน)
    Flash_WriteHalfWord(FL_ADDR_HALFWORD, 0xABCD);  // เขียน 0xABCD (Write 0xABCD)
    USART_Print("Auto-write #2 OK\r\n");    // แจ้งครั้งที่ 2 สำเร็จ (Notify #2 success)

    // ---- ขั้นตอนที่ 3: Auto-erase ครั้งที่ 3 (Step 3: Auto-erase #3) ----
    Flash_WriteWord(FL_ADDR_WORD, 0xDEADBEEF);  // เขียน 0xDEADBEEF (Write 0xDEADBEEF)
    USART_Print("Auto-write #3 OK\r\n");    // แจ้งครั้งที่ 3 สำเร็จ (Notify #3 success)

    // ---- ขั้นตอนที่ 4: อ่านค่ากลับมา (Step 4: Read back values) ----
    bRead  = Flash_ReadByte(FL_ADDR_BYTE);  // อ่านค่าไบต์ (Read byte)
    hwRead = Flash_ReadHalfWord(FL_ADDR_HALFWORD);  // อ่านค่าครึ่งคำ (Read half-word)
    wRead  = Flash_ReadWord(FL_ADDR_WORD);  // อ่านค่าเต็มคำ (Read word)

    // ---- ขั้นตอนที่ 5: แสดงผล (Step 5: Display results) ----
    USART_Print("Read values: Byte=0x"); USART_PrintHex(bRead, 1); USART_Print(" HalfWord=0x"); USART_PrintHex(hwRead, 1); USART_Print(" Word=0x"); USART_PrintHex(wRead, 1); USART_Print("\r\n");

    // ---- การใช้ MACRO (Macro usage) ----
    // FLASH_WRITE_AUTO(FL_ADDR_BYTE, 0x77, uint8_t);  // MACRO: Auto-erase + write ไบต์ (Auto-erase + write byte)
    // uint8_t val = FLASH_READ(uint8_t, FL_ADDR_BYTE); // MACRO: อ่านค่าไบต์ (Read byte via macro)

    USART_Print("--- Done ---\r\n");        // แจ้งสิ้นสุด (Notify end)

    while (1);                               // วังวนไม่รู้จบ (Infinite loop)
}
