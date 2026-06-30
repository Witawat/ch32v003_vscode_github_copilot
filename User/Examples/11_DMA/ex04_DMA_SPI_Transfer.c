/**
 * ============================================================
 * ตัวอย่างที่ 4: ส่งข้อมูลผ่าน SPI ด้วย DMA (DMA SPI Transfer)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     --- แบบที่ 1: ต่อกับ W25Qxx Flash ---
 *     CH32V003              W25Qxx
 *     --------              -------
 *     PC4 (CS) ----------- CS#
 *     PC5 (SCK) ---------- CLK
 *     PC6 (MOSI) --------- DI
 *     PC7 (MISO) --------- DO
 *     VCC (3.3V) --------- VCC
 *     GND    -------------- GND
 *
 *     --- แบบที่ 2: Loopback (ทดสอบโดยไม่ต้องมีอุปกรณ์) ---
 *     CH32V003
 *     --------
 *     PC6 (MOSI) ----+---- PC7 (MISO)
 *                    |
 *     PC4 (CS)  ---+ (ดึง Low ขณะ transfer)
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 * - กรณี W25Qxx: "JEDEC ID: EF 40 14"
 * - กรณี Loopback: "Loopback: AA BB CC" (ส่งอะไรรับค่านั้น)
 * ============================================================
 * คำเตือน (WARNINGS):
 * - DMA_SPI ต้องใช้ 2 channels (1 สำหรับ TX, 1 สำหรับ RX)
 * - ค่าเริ่มต้น: DMA_CH4 = TX, DMA_CH5 = RX
 * - ต้องเรียก SPI_SimpleInit() ก่อน DMA_SPI_Init()
 * - ถ้าใช้ loopback ต้องต่อ jumper ระหว่าง MOSI และ MISO
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["USART_SimpleInit()"]
 *     B --> C["SPI_SimpleInit(SPI_MODE0, 1MHz)"]
 *     C --> D["DMA_SPI_Init(DMA_CH4, DMA_CH5)"]
 *     D --> E["SPI_SetCS(0)"]
 *     E --> F["DMA_SPI_TransferBuffer(jedec_cmd, jedec_rx, 4)"]
 *     F --> G["SPI_SetCS(1)"]
 *     G --> H["USART_Print(JEDEC ID)"]
 *     H --> I["SPI_SetCS(0)"]
 *     I --> J["DMA_SPI_TransferBuffer(lb_tx, lb_rx, 4)"]
 *     J --> K["SPI_SetCS(1)"]
 *     K --> L["USART_Print(loopback data)"]
 *     L --> M["while(1)"]
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>    // รวมไลบรารี SimpleHAL ทั้งหมด
#include <stdio.h>        // รวมไลบรารี sprintf สำหรับจัดรูปแบบข้อความ

int main(void)            // ฟังก์ชันหลัก จุดเริ่มต้นโปรแกรม
{
    SystemCoreClockUpdate(); // อัปเดตค่าความถี่สัญญาณนาฬิกา (จำเป็นทุกครั้ง)

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT); // เริ่มต้น USART ที่ 115200 baud

    SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT); // เริ่มต้น SPI โหมด 0 ความเร็ว 1MHz

    DMA_SPI_Init(DMA_CH4, DMA_CH5); // เริ่มต้น DMA สำหรับ SPI: CH4=TX, CH5=RX

    // --- อ่าน JEDEC ID จาก W25Qxx ---
    uint8_t jedec_cmd[4] = {0x9F, 0x00, 0x00, 0x00}; // คำสั่ง JEDEC ID (9F) + dummy 3 bytes
    uint8_t jedec_rx[4] = {0}; // Buffer สำหรับรับ JEDEC ID 4 ไบต์

    SPI_SetCS(0);                // ตั้ง CS = LOW (เลือก W25Qxx)
    DMA_SPI_TransferBuffer(DMA_CH4, DMA_CH5, jedec_cmd, jedec_rx, 4); // ส่ง/รับ 4 ไบต์ด้วย DMA
    SPI_SetCS(1);                // ตั้ง CS = HIGH (ยกเลิกการเลือก)

    char buffer[32];             // Buffer สำหรับข้อความ
    sprintf(buffer, "JEDEC ID: %02X %02X %02X\r\n",
            jedec_rx[1], jedec_rx[2], jedec_rx[3]); // จัดรูปแบบ JEDEC ID
    USART_Print(buffer);         // พิมพ์ JEDEC ID ทาง USART

    // --- ทดสอบ Loopback (ต่อ jumper MOSI-MISO) ---
    uint8_t lb_tx[4] = {0xAA, 0xBB, 0xCC, 0xDD}; // ข้อมูลส่งแบบ loopback
    uint8_t lb_rx[4] = {0};     // Buffer สำหรับรับข้อมูล loopback

    SPI_SetCS(0);                // ตั้ง CS = LOW
    DMA_SPI_TransferBuffer(DMA_CH4, DMA_CH5, lb_tx, lb_rx, 4); // ส่ง/รับ loopback ด้วย DMA
    SPI_SetCS(1);                // ตั้ง CS = HIGH

    char msg[32];                // Buffer สำหรับข้อความ loopback
    sprintf(msg, "Loopback: %02X %02X %02X %02X\r\n",
            lb_rx[0], lb_rx[1], lb_rx[2], lb_rx[3]); // จัดรูปแบบข้อมูลที่รับได้
    USART_Print(msg);            // พิมพ์ข้อมูล loopback ทาง USART

    while (1) { }                // หยุดโปรแกรม ณ จุดนี้
}                                // สิ้นสุดฟังก์ชัน main
