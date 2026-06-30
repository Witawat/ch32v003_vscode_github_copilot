/**
 * ============================================================
 * ตัวอย่างที่ 2: คัดลอกหน่วยความจำแบบอะซิงโครนัส (MemCopy Async)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003              USB-Serial          LED
 *     --------              ----------          ---
 *     PD5 (TX) ----------- RX
 *     PD6 (RX) ----------- TX
 *     GND    -------------- GND
 *     PC0 ----/\/\/\---->|---- GND
 *            220 Ohm
 * 
 *     LED ต่อที่ PC0 ผ่านตัวต้านทาน 220 Ohm แสดงสถานะการทำงาน
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 * - "Copy started..." (LED ติด)
 * - callback ถูกเรียกเมื่อคัดลอกเสร็จ
 * - "Copy complete!" (LED ดับ)
 * - DMA_WaitComplete พร้อม timeout เป็นอีกทางเลือก
 * ============================================================
 * คำเตือน (WARNINGS):
 * - Callback ทำงานใน ISR! ควรทำให้สั้นที่สุด
 * - ใช้ volatile สำหรับตัวแปรที่แชร์ระหว่าง main กับ ISR
 * - อย่าเรียก DMA_WaitComplete() ใน callback เพราะจะ deadlock
 * - DMA_CH1 ถูกใช้ในตัวอย่างนี้ สามารถเปลี่ยนเป็น channel อื่นได้
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["USART_SimpleInit()"]
 *     B --> C["pinMode(PC0, OUTPUT)"]
 *     C --> D["for(src[i] = i & 0xFF)"]
 *     D --> E["DMA_SetTransferCompleteCallback(DMA_CH1, callback)"]
 *     E --> F["digitalWrite(PC0, HIGH)"]
 *     F --> G["USART_Print(Copy started)"]
 *     G --> H["DMA_MemCopyAsync(DMA_CH1, dst, src, 256)"]
 *     H --> I["while(!transfer_done) __NOP()"]
 *     I --> J["Callback: digitalWrite(PC0, LOW)"]
 *     J --> K["Verify dst[i] == src[i]"]
 *     K --> L{"Match?"}
 *     L -->|"Yes"| M["USART_Print(Async MemCopy verified)"]
 *     L -->|"No"| N["while(1)"]
 *     M --> N
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>    // รวมไลบรารี SimpleHAL ทั้งหมด

#define BUFFER_SIZE 256   // กำหนดขนาด buffer 256 ไบต์

volatile uint8_t transfer_done = 0; // ตัวแปร volatile บอกว่า transfer เสร็จหรือยัง (ใช้ใน ISR ร่วมกับ main)

void on_transfer_complete(DMA_Channel channel) // Callback เมื่อ DMA ถ่ายโอนเสร็จ (ทำงานใน ISR)
{
    digitalWrite(PC0, LOW);     // ดับ LED ที่ PC0 เพื่อบอกว่าสิ้นสุดการทำงาน
    transfer_done = 1;          // ตั้งค่าตัวแปรสถานะว่า transfer เสร็จแล้ว (volatile)
    USART_Print("Copy complete!\r\n"); // พิมพ์ข้อความแจ้งเตือน (ระวัง: USART ใน ISR อาจช้า)
}

int main(void)            // ฟังก์ชันหลัก จุดเริ่มต้นโปรแกรม
{
    SystemCoreClockUpdate(); // อัปเดตค่าความถี่สัญญาณนาฬิกา (จำเป็นทุกครั้ง)

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT); // เริ่มต้น USART ที่ 115200 baud
    pinMode(PC0, PIN_MODE_OUTPUT); // ตั้งค่า PC0 เป็น output สำหรับควบคุม LED

    uint8_t src[BUFFER_SIZE]; // ต้นทาง: array ขนาด 256 ไบต์
    uint8_t dst[BUFFER_SIZE]; // ปลายทาง: array ขนาด 256 ไบต์

    for (uint16_t i = 0; i < BUFFER_SIZE; i++) // เตรียมข้อมูลต้นทาง
    {
        src[i] = (uint8_t)(i & 0xFF); // เก็บค่า 0-255 วนซ้ำ
    }

    DMA_SetTransferCompleteCallback(DMA_CH1, on_transfer_complete); // ลงทะเบียน callback สำหรับ DMA_CH1

    digitalWrite(PC0, HIGH);    // ติด LED ที่ PC0 เพื่อบอกว่ากำลังทำงาน
    USART_Print("Copy started...\r\n"); // พิมพ์ข้อความแจ้งเริ่มต้น

    DMA_MemCopyAsync(DMA_CH1, dst, src, BUFFER_SIZE); // เริ่มคัดลอกแบบไม่รอ (non-blocking) ใช้ DMA_CH1

    while (!transfer_done)      // รอ while loop จนกว่า callback จะตั้งค่า transfer_done = 1
    {
        __NOP();                // No operation: บอก CPU ให้หยุดรอ (ประหยัดพลังงาน)
    }

    uint8_t match = 1;          // ตัวแปรบอกว่าข้อมูลถูกต้อง
    for (uint16_t i = 0; i < BUFFER_SIZE; i++) // ตรวจสอบทีละไบต์
    {
        if (dst[i] != src[i])   // ถ้าค่าไม่ตรงกัน
        {
            match = 0;          // ตั้งค่าผิดพลาด
            break;              // ออกจาก loop
        }
    }

    if (match)                  // ถ้าข้อมูลครบถ้วนถูกต้อง
    {
        USART_Print("Async MemCopy verified!\r\n"); // พิมพ์ผลลัพธ์สำเร็จ
    }

    while (1) { }               // หยุดโปรแกรม ณ จุดนี้
}                               // สิ้นสุดฟังก์ชัน main
