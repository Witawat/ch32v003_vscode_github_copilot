/**
 * ============================================================
 * PVD Monitor
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *  USART1: PA9 (TX), PA10 (RX) --- CP2102 --- PC
 * 
 *  แหล่งจ่ายไฟ VDD --- วัดด้วยมัลติมิเตอร์ (ใช้ Power Supply แบบปรับค่าได้)
 *  หรือใช้ Voltage Divider เพื่อทดสอบ PVD Threshold ที่ 3.3V
 * 
 *  คำแนะนำ: ใช้ Variable Power Supply ปรับแรงดันจาก 3.6V ลงไปจนถึง 3.0V
 *  เพื่อดูการทำงานของ PVD ที่ Threshold 3.3V
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   - "PVD set at 3.3V. Monitoring..."
 *   - เมื่อ VDD ต่ำกว่า 3.3V -> "LOW VOLTAGE DETECTED!"
 *   - เมื่อ VDD กลับมาสูงกว่า 3.3V -> "Voltage restored to normal."
 * ============================================================
 * คำเตือน (WARNINGS):
 *   1. PVD สามารถสร้าง Interrupt ไปยัง PVD_IRQHandler ได้
 *      แต่ในตัวอย่างนี้ใช้วิธี Polling โดยตรวจสอบสถานะด้วย PWR_GetPVDStatus()
 *   2. ต้องปรับแรงดันให้ต่ำกว่า VDD ปกติเพื่อทดสอบ ควรใช้ Power Supply แบบปรับค่าได้
 *   3. PVD Threshold มีความคลาดเคลื่อน ±5% ตาม datasheet
 * ============================================================
 */

#include <SimpleHAL.h>  // รวมไลบรารี SimpleHAL สำหรับการเข้าถึงฟังก์ชันของชิป CH32V003

int main(void)
{
    SystemCoreClockUpdate();

    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    Delay_Ms(100);

    PWR_EnablePVD(PWR_PVD_3V3);
    USART_Print("PVD set at 3.3V. Monitoring...\r\n");

    uint8_t lastState = 0;  // ตัวแปรเก็บสถานะ PVD ก่อนหน้าสำหรับตรวจจับการเปลี่ยนแปลง

    while (1)  // วนลูปตรวจสอบแรงดันไฟอย่างต่อเนื่อง
    {
        uint8_t pvdStatus = PWR_GetPVDStatus();  // อ่านค่า PVD ปัจจุบัน (1 = แรงดันต่ำ, 0 = ปกติ)

        if (pvdStatus && !lastState)  // ตรวจสอบว่าเพิ่งเกิดแรงดันต่ำ (เปลี่ยนจากปกติเป็นต่ำ)
        {
            USART_Print("LOW VOLTAGE DETECTED!\r\n");
        }
        else if (!pvdStatus && lastState)
        {
            USART_Print("Voltage restored to normal.\r\n");
        }

        lastState = pvdStatus;  // อัปเดตสถานะล่าสุดเพื่อใช้เปรียบเทียบในรอบถัดไป
        Delay_Ms(200);  // หน่วงเวลา 200 มิลลิวินาทีก่อนอ่านค่า PVD อีกครั้งเพื่อลดภาระ CPU
    }
}
