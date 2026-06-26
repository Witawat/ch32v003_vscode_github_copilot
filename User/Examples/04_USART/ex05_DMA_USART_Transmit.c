/**
 * ============================================================
 * ตัวอย่างที่ 5: DMA USART Transmit
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003              USB-Serial
 *     --------              ----------
 *     PD5 (TX) ------------> RX
 *     PD6 (RX) <------------ TX
 *     GND ------------------ GND
 *
 *     (เหมือนกับ ex01)
 *     สงขอความ "Hello from DMA!" ผาน DMA โดย CPU ไมตองรอง
 *
 * API ที่ใช้:
 *   DMA_USART_Send(data, length)   // ส่งข้อมูลผ่าน DMA (SimpleDMA)
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - "Hello from DMA!" ถูกพิมพ์ผ่าน Serial โดยใช้ DMA
 *   - ระหว่างที่ DMA ส่งข้อมูล, CPU ว่างและสามารถทำงานอื่นได้
 * ============================================================
 * คำเตือน (WARNINGS):
 *   - DMA channel ที่ใช้อาจชนกับผู้ใช้ DMA รายอื่น
 *   - ต้องเปิดใช้งาน SimpleDMA ก่อน (DMA_Init())
 *   - DMA_USART_Send() จะ return ทันทีหลังจากเริ่ม DMA (non-blocking)
 *   - ตรวจสอบ DMA transfer complete flag ก่อนส่งครั้งถัดไป
 *   - บัฟเฟอร์ที่ส่งต้องคงอยู่จนกว่า DMA จะส่งเสร็จ (ใช้ global หรือ static array)
 * ============================================================
 */

#include <SimpleHAL.h>

// ข้อความที่ต้องการส่ง (ต้องเป็น global หรือ static เพื่อให้ DMA อ่านได้)
// DMA จะอ่านข้อมูลจากที่อยู่นี้ระหว่าง transfer
static const char msg[] = "Hello from DMA!\r\n";

int main(void)
{
    // อัปเดตความถี่ระบบ
    SystemCoreClockUpdate();

    Timer_Init();
    // เริ่มต้น USART: Baud 115200
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    // DMA_USART_Send ใช้ได้ทันที ไม่ต้อง init แยก

    // วนรอบไม่สิ้นสุด
    while (1)
    {
        // ส่งข้อความผ่าน DMA (CPU ทำงานอื่นต่อได้ระหว่างส่ง)
        DMA_USART_Send(DMA_CH2, msg, sizeof(msg) - 1);  // -1 เพื่อไม่ส่ง null terminator

        // ขณะที่ DMA กำลังส่ง, CPU มาทำงานตรงนี้ได้
        // รอให้ DMA ส่งเสร็จก่อนส่งรอบหน้า
        // (ในที่นี้รอ 2 วินาทีเพื่อให้เห็นผลชัดเจน)
        Delay_Ms(2000);
    }
}
