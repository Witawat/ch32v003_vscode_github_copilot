/**
 * ============================================================
 * ex02_IWDG_Advanced.c
 * โปรแกรมสาธิต IWDG แบบกำหนดค่าเอง พร้อมตรวจสอบสาเหตุรีเซ็ต
 * (Advanced IWDG with custom config and reset cause detection)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *   CH32V003
 *   ------
 *   PC0 (OUT) ----[220?]----+---- LED ---- GND
 *                            |
 *   PD5 (TX)  ----> USB-UART (RX)
 *   PD6 (RX)  <---- USB-UART (TX)
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 *   บูตครั้งแรก: "Clean boot"  feed IWDG 5 วินาที  หยุด feed  รีเซ็ต
 *   บูตครั้งที่สอง: "Watchdog reset! Last timeout: 2000ms"
 *   (First boot: "Clean boot"  feed 5s  stop  reset)
 *   (Second boot: "Watchdog reset! Last timeout: 2000ms")
 * ============================================================
 * คำเตือน (WARNINGS):
 *   IWDG_WasResetCause() ตรวจสอบ RCC flag — ต้องลบหลังอ่าน
 *     เพื่อป้องกันการตรวจจับซ้ำในบูตถัดไป
 *     (Clear flag after reading to avoid false detection on next boot)
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>                      // รวมไลบรารี SimpleHAL (Include SimpleHAL library)

// --------------------------------------------------------------------------
// ฟังก์ชันหลัก (Main function)
// --------------------------------------------------------------------------

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    // ตัวแปรต่าง ๆ (Variables)
    uint8_t  resetCause = 0;                 // สาเหตุการรีเซ็ต (Reset cause flag)
    uint32_t timeoutMs   = 0;                // ค่า timeout ที่คำนวณได้ (Calculated timeout in ms)
    uint8_t  feedCount   = 0;                // ตัวนับรอบการ feed (Feed counter)
    uint8_t  isBusy      = 0;                // สถานะ IWDG กำลังทำงาน (IWDG busy flag)

    // ---- ส่วนเริ่มต้น (Initialization) ----
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);                      // เริ่มต้นพอร์ตอนุกรม (Initialize USART)
    pinMode(PC0, PIN_MODE_OUTPUT);          // กำหนด PC0 เป็นเอาต์พุต (Set PC0 as PIN_MODE_OUTPUT)
    digitalWrite(PC0, LOW);                 // เริ่มต้น LED ดับ (Initialize LED off)

    USART_Print("--- IWDG Advanced ---\r\n");  // แสดงหัวข้อ (Display title)

    // ---- ตรวจสอบสาเหตุการรีเซ็ต (Check reset cause) ----
    resetCause = IWDG_WasResetCause();       // ตรวจสอบว่ารีเซ็ตจาก IWDG หรือไม่ (Check if IWDG caused reset)
    if (resetCause == 1) {                   // ถ้ารีเซ็ตจาก IWDG (If reset from IWDG)
        USART_Print("Watchdog reset! Last timeout: "); USART_PrintNum((int32_t)IWDG_GetTimeout(IWDG_PRESCALER_64, 1249)); USART_Print("ms\r\n");  // แจ้งว่ารีเซ็ตจาก IWDG พร้อมค่า timeout (Notify IWDG reset with timeout)
        IWDG_ClearResetFlag();               // ลบ flag เพื่อไม่ให้ตรวจจับซ้ำในบูตหน้า (Clear flag to avoid false detection)
    } else {                                 // ถ้าเป็นการบูตปกติ (If normal boot)
        USART_Print("Clean boot\r\n");      // แจ้งว่าบูตปกติ (Notify clean boot)
    }

    // ---- เริ่ม IWDG ด้วยค่าที่กำหนดเอง (Initialize IWDG with custom config) ----
    // prescaler=32, reload=1249  timeout ? (32*1249)/40000 = 0.9992s ? 1000ms
    // แต่ใช้ 2000ms (prescaler=64, reload=1249)
    IWDG_Init(64, 1249);                     // เริ่ม IWDG: prescaler=64, reload=1249  ~2000ms (Init IWDG)
    USART_Print("IWDG initialized: prescaler=64, reload=1249\r\n");  // แจ้งค่าเริ่มต้น (Notify init params)

    timeoutMs = IWDG_GetTimeout(IWDG_PRESCALER_64, 1249);           // ดึงค่า timeout ที่คำนวณได้ (Get calculated timeout)
    USART_Print("Calculated timeout: "); USART_PrintNum((int32_t)timeoutMs); USART_Print("ms\r\n");  // แสดงค่า timeout (Display timeout)

    // ---- ตรวจสอบสถานะ IWDG (Check IWDG status) ----
    isBusy = IWDG_IsBusy();                  // ตรวจสอบว่า IWDG กำลังยุ่งหรือไม่ (Check if IWDG is busy)
    USART_Print("IWDG busy status: "); USART_PrintNum(isBusy); USART_Print("\r\n");  // แสดงสถานะ busy (Display busy status)

    // ---- Feed IWDG 5 วินาที (Feed IWDG for 5 seconds) ----
    USART_Print("Feeding watchdog for 5s...\r\n");  // แจ้งว่าเริ่ม feed 5 วินาที (Notify feeding for 5s)
    for (feedCount = 0; feedCount < 25; feedCount++) {  // 25 รอบ ? 200ms = 5 วินาที (25 cycles ? 200ms = 5s)
        digitalWrite(PC0, !digitalRead(PC0));  // กลับสถานะ LED (Toggle LED)
        IWDG_Feed();                         // รีเซ็ตตัวนับ IWDG (Reset IWDG counter)
        Delay_Ms(200);                       // หน่วง 200ms (Delay 200ms)
    }

    // ---- หยุด feed — รอ IWDG รีเซ็ต (Stop feeding — wait for IWDG reset) ----
    USART_Print("Stop feeding! IWDG will reset in ~"); USART_PrintNum((int32_t)timeoutMs); USART_Print("ms...\r\n");  // แจ้งว่าหยุด feed จะรีเซ็ต (Notify stop feeding, reset pending)
    digitalWrite(PC0, LOW);                 // ดับ LED (Turn LED off)

    // IWDG จะนับถอยหลังและรีเซ็ต MCU (IWDG counts down and resets MCU)
    while (1) {                              // วังวนรอรีเซ็ต (Wait for reset)
        // ไม่มีการ feed อีก — IWDG จะรีเซ็ต MCU (No more feeding — IWDG will reset MCU)
    }
}
