/**
 * @example ex07_DMA_Advanced.c
 * @brief DMA_SimpleInit + DMA_Stop + DMA_Reset + ErrorCallback + HalfTransferCallback
 *
 * สาธิต: Memory-to-Memory DMA พร้อม custom config, callbacks, และ lifecycle control
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include "SimpleHAL.h"

static volatile uint8_t transfer_done = 0;
static volatile uint8_t half_done = 0;

static void on_complete(DMA_Channel ch) { (void)ch; transfer_done = 1; }
static void on_half(DMA_Channel ch)     { (void)ch; half_done = 1; }

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    uint8_t src[64], dst[64];
    for (int i = 0; i < 64; i++) src[i] = i;

    // 1. DMA_SimpleInit — ตั้งค่า channel แบบ custom config
    DMA_Config_t cfg = {
        .channel = DMA_CH1, .direction = DMA_DIR_MEM_TO_MEM,
        .data_size = DMA_SIZE_BYTE, .mode = DMA_MODE_NORMAL,
        .mem_increment = 1, .periph_increment = 1,
        .periph_addr = (uint32_t)src, .mem_addr = (uint32_t)dst,
        .buffer_size = 64
    };
    DMA_SimpleInit(&cfg);

    // 2. DMA_SetTransferCompleteCallback + DMA_SetHalfTransferCallback
    DMA_SetTransferCompleteCallback(DMA_CH1, on_complete);
    DMA_SetHalfTransferCallback(DMA_CH1, on_half);
    DMA_EnableInterrupt(DMA_CH1, 1);

    // 3. DMA_Start
    DMA_Start(DMA_CH1);
    USART_Print("DMA started...\r\n");

    // รอ transfer เสร็จ
    while (!transfer_done) { /* non-blocking in real app */ Delay_Ms(1); }
    USART_Print(half_done ? "Half-transfer triggered\r\n" : "Half-transfer NOT triggered (buffer may be too small)\r\n");
    USART_Print("Transfer complete! ");

    // ตรวจสอบผลลัพธ์
    uint8_t ok = 1;
    for (int i = 0; i < 64; i++) { if (dst[i] != i) ok = 0; }
    USART_Print(ok ? "Data OK\r\n" : "Data MISMATCH!\r\n");

    // 4. DMA_Stop — หยุด channel (กรณีต้องการยกเลิกก่อนจบ)
    DMA_Stop(DMA_CH1);

    // 5. DMA_Reset — รีเซ็ต channel ให้กลับสภาพเริ่มต้น
    DMA_Reset(DMA_CH1);
    USART_Print("DMA channel reset complete\r\n");

    while (1) { Delay_Ms(1000); }
}
