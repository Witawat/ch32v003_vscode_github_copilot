/**
 * ============================================================
 * ตัวอย่างที่ 6: ส่งข้อมูลผ่าน I2C ด้วย DMA (DMA I2C Transfer)
 * ============================================================
 *
 * แผนผังวงจร (Circuit Diagram):
 *
 *     CH32V003              BH1750 (Light Sensor)
 *     --------              ------
 *     PC1 (SDA) --/\/\/\---+ SDA
 *                 4.7kΩ    |
 *     PC2 (SCL) --/\/\/\---+ SCL
 *                 4.7kΩ    |
 *     VCC (3.3V) ---------- VCC
 *     GND    --------------- GND
 *     ADDR pin ของ BH1750:
 *     - ลอย = address 0x23
 *     - GND = address 0x23 (เหมือนกัน)
 *     - VCC = address 0x5C
 * 
 *     หมายเหตุ: BH1750 เป็นเซ็นเซอร์วัดความสว่าง (Ambient Light Sensor)
 *     Address 7-bit: 0x23 (ADDR pin ลอยหรือต่อ GND)
 *
 * ============================================================
 * ผลลัพธ์ที่คาดหวัง (Expected Results):
 * - "DMA I2C: 450 lux" (ค่าความสว่างปัจจุบัน หน่วย lux)
 * - ถ้าใช้ BH1750 จริง ค่าจะเปลี่ยนตามแสงแวดล้อม
 * - ถ้าไม่มี BH1750 จะขึ้น timeout หรือ error
 * ============================================================
 * คำเตือน (WARNINGS):
 * - I2C DMA fixed mapping: DMA_CH4 = I2C1_TX, DMA_CH5 = I2C1_RX
 * - ต้องมี pull-up resistor 4.7kΩ ทั้ง SDA และ SCL
 * - ต้องเรียก I2C_SimpleInit() ก่อนใช้ DMA_I2C functions
 * - BH1750 ต้องใช้คำสั่ง 0x10 (Continuous High Resolution Mode) เพื่อเริ่มวัด
 * - ค่า lux = (H_byte << 8 | L_byte) / 1.2
 * ============================================================
 * ผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["USART_SimpleInit()"]
 *     C --> D["I2C_SimpleInit(100kHz)"]
 *     D --> E["DMA_I2C_InitTx(DMA_CH4)"]
 *     E --> F["DMA_I2C_InitRx(DMA_CH5)"]
 *     F --> G["I2C_WriteReg(BH1750, PWR_ON)"]
 *     G --> H["I2C_WriteReg(BH1750, HRES)"]
 *     H --> I["Delay_Ms(180)"]
 *     I --> J["I2C_ReadRegMulti(BH1750, lux_data, 2)"]
 *     J --> K["Calculate lux = raw / 1.2"]
 *     K --> L["USART_Print(DMA I2C: X lux)"]
 *     L --> M["while(1)"]
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
/* CH32V003 has no hardware FPU � float/double use software emulation (~800 cycles) */
#include <SimpleHAL.h>

#define BH1750_ADDR    0x23
#define BH1750_CMD_PWR_ON 0x01
#define BH1750_CMD_HRES   0x10

int main(void)
{
    SystemCoreClockUpdate();

    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);

    DMA_I2C_InitTx(DMA_CH4);
    DMA_I2C_InitRx(DMA_CH5);

    I2C_WriteReg(BH1750_ADDR, 0x00, BH1750_CMD_PWR_ON);
    I2C_WriteReg(BH1750_ADDR, 0x00, BH1750_CMD_HRES);

    Delay_Ms(180);

    uint8_t lux_data[2];

    I2C_ReadRegMulti(BH1750_ADDR, 0x00, lux_data, 2);

    uint16_t lux_raw = ((uint16_t)lux_data[0] << 8) | lux_data[1];
    uint16_t lux = (uint16_t)((float)lux_raw / 1.2f);

    USART_Print("DMA I2C: ");
    USART_PrintNum((int32_t)lux);
    USART_Print(" lux\r\n");

    while (1) { }
}
