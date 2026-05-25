/**
 * ============================================================
 * ตัวอย่างที่ 3: ส่งข้อมูลผ่าน USART ด้วย DMA (DMA USART Transmit)
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
 *     LED ที่ PC0 แสดงว่า main loop ยังทำงานระหว่าง DMA ถ่ายโอนข้อมูล
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 * - "Hello from DMA!" ปรากฏบน Serial Monitor
 * - LED ที่ PC0 กระพริบขณะ DMA กำลังถ่ายโอนข้อมูล (non-blocking)
 * - main loop ทำงานต่อเนื่องไม่ต้องรอ DMA
 * ============================================================
 * คำเตือน (WARNINGS):
 * - DMA_USART_Send() เป็นแบบ blocking (รอจนเสร็จ)
 * - DMA_USART_Transmit() เป็นแบบ non-blocking
 * - ต้องเรียก USART_SimpleInit() ก่อนใช้ DMA_USART functions
 * - DMA_USART_InitTx() ต้องเรียกก่อน DMA_USART_Transmit() เสมอ
 * ============================================================
 */

#include <SimpleHAL.h>    // รวมไลบรารี SimpleHAL ทั้งหมด
#include <string.h>       // รวมไลบรารี string.h สำหรับ strlen

int main(void)            // ฟังก์ชันหลัก จุดเริ่มต้นโปรแกรม
{
    SystemCoreClockUpdate(); // อัปเดตค่าความถี่สัญญาณนาฬิกา (จำเป็นทุกครั้ง)

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT); // เริ่มต้น USART ที่ 115200 baud
    pinMode(PC0, PIN_MODE_OUTPUT); // ตั้งค่า PC0 เป็น output สำหรับ LED แสดงสถานะ

    uint8_t tx_buffer[64]; // Buffer สำหรับ DMA TX (ขนาดพอเหมาะ)
    DMA_USART_InitTx(DMA_CH2, tx_buffer, sizeof(tx_buffer)); // เริ่มต้น DMA USART TX บน DMA_CH2

    const char* message = "Hello from DMA!\r\n"; // ข้อความที่ต้องการส่งผ่าน DMA
    uint16_t msg_len = strlen(message); // คำนวณความยาวข้อความ

    DMA_USART_Transmit(DMA_CH2, (const uint8_t*)message, msg_len); // ส่งข้อความแบบ non-blocking

    uint32_t blink_count = 0; // ตัวนับรอบการกระพริบ LED

    while (DMA_GetStatus(DMA_CH2) != DMA_STATUS_COMPLETE) // รอจนกว่า DMA จะส่งเสร็จ
    {
        blink_count++;       // เพิ่มจำนวนรอบ
        digitalWrite(PC0, blink_count % 2); // กระพริบ LED ด้วยการหาร modulo 2
        Delay_Ms(50);        // หน่วง 50ms ให้เห็นการกระพริบชัดเจน
    }

    Delay_Ms(500);           // หน่วง 500ms ก่อนทดสอบแบบ blocking

    DMA_USART_Send(DMA_CH2, (const uint8_t*)"Blocking send done!\r\n", 22); // ส่งแบบ blocking รอจนเสร็จ

    USART_Print("DMA USART example complete\r\n"); // พิมพ์ข้อความสิ้นสุดด้วย USART ปกติ

    while (1)                // วนลูปอนันต์
    {
        digitalWrite(PC0, !digitalRead(PC0)); // สลับสถานะ LED (toggle)
        Delay_Ms(500);       // หน่วง 500ms ต่อรอบ
    }                        // สิ้นสุด while loop
}                            // สิ้นสุดฟังก์ชัน main
