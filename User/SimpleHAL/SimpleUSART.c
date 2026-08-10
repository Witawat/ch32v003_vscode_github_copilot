/**
 * @file SimpleUSART.c
 * @brief Simple USART Library Implementation
 * @version 1.0
 * @date 2025-12-12
 */

#include "SimpleUSART.h"
#include <string.h>

#warning "USART_PINS_FULL_REMAP: Pin mapping needs CH32V003 datasheet verification."

/* ========== RX Ring Buffer ========== */

// Hardware RXNE buffer is only 1 byte — without this, a byte arriving while
// the app is busy is silently overwritten by the next one. Filled by
// USART1_IRQHandler(), drained by USART_Read()/USART_Available().
static volatile uint8_t s_rx_buffer[USART_RX_BUFFER_SIZE];
static volatile uint16_t s_rx_head = 0;
static volatile uint16_t s_rx_tail = 0;

/**
 * @brief Weak default — override in your own code to tap into RX bytes
 *        without needing your own USART1_IRQHandler (see SimpleUSART.h)
 */
__attribute__((weak)) void USART_RxByteHook(uint8_t data) {
    (void)data;
}

/**
 * @brief Weak default — override to detect end-of-frame via USART IDLE line
 *        (ใช้กับ DMA RX circular buffer — เรียกจาก ISR เมื่อพบ IDLE line)
 */
__attribute__((weak)) void USART_IdleHook(void) {
}

/* ========== Private Helper Functions ========== */

/**
 * @brief ส่ง 1 character ผ่าน USART (internal)
 */
static void USART_SendChar(char ch) {
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, (uint8_t)ch);
}

/**
 * @brief แปลงตัวเลขเป็น string (internal)
 */
static void Int32ToString(int32_t num, char* str) {
    int i = 0;
    int is_negative = 0;
    
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
    
    if (num < 0) {
        is_negative = 1;
        uint32_t unum = (uint32_t)(-(num + 1)) + 1;
        while (unum != 0) {
            str[i++] = (unum % 10) + '0';
            unum = unum / 10;
        }
    } else {
        uint32_t unum = (uint32_t)num;
        while (unum != 0) {
            str[i++] = (unum % 10) + '0';
            unum = unum / 10;
        }
    }
    
    if (is_negative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    
    // Reverse string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

/* ========== Public Functions ========== */

/**
 * @brief เริ่มต้นการใช้งาน USART
 */
void USART_SimpleInit(USART_BaudRate baud, USART_PinConfig pin_config) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    
    // 1. เปิด Clock สำหรับ USART1 และ GPIOD
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);
    
    // 2. ตั้งค่า Pin Remapping และ GPIO
    switch(pin_config) {
        case USART_PINS_DEFAULT:
            // Default: TX=PD5, RX=PD6 (ไม่ต้อง remap)
            
            // TX - Alternate Function Push-Pull
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            
            // RX - Input Floating
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            break;
            
        case USART_PINS_REMAP1:
            // Remap1: TX=PD0, RX=PD1
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
            GPIO_PinRemapConfig(GPIO_PartialRemap1_USART1, ENABLE);
            
            // TX
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            
            // RX
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            break;
            
        case USART_PINS_REMAP2:
            // Remap2: TX=PD6, RX=PD5
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
            GPIO_PinRemapConfig(GPIO_PartialRemap2_USART1, ENABLE);
            
            // TX
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            
            // RX
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            break;
            
        case USART_PINS_FULL_REMAP:
            // Full Remap: TX=PD6, RX=PD5 (รวมบิต REMAP1+REMAP2)
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
            // @note pin mapping อาจแตกต่าง — ตรวจสอบ CH32V003 datasheet
            GPIO_PinRemapConfig(GPIO_FullRemap_USART1, ENABLE);
            
            // TX
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            
            // RX
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            break;
    }
    
    // 3. ตั้งค่า USART
    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    
    USART_Init(USART1, &USART_InitStructure);

    // 4. เปิดใช้งาน USART
    USART_Cmd(USART1, ENABLE);

    // 5. เปิด RX interrupt เพื่อเติม ring buffer (กัน byte หายเมื่ออ่านไม่ทัน)
    s_rx_head = 0;
    s_rx_tail = 0;
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief ส่งข้อความแบบ string
 */
void USART_Print(const char* str) {
    while(*str) {
        USART_SendChar(*str++);
    }
}

/**
 * @brief ส่งตัวเลขแบบ decimal
 */
void USART_PrintNum(int32_t num) {
    char buffer[12];  // เพียงพอสำหรับ int32_t
    Int32ToString(num, buffer);
    USART_Print(buffer);
}

/**
 * @brief ส่งตัวเลขแบบ hexadecimal
 */
void USART_PrintHex(uint32_t num, uint8_t uppercase) {
    char buffer[11];  // "0x" + 8 hex digits + null
    const char* hex_chars = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    
    buffer[0] = '0';
    buffer[1] = 'x';
    
    // แปลงเป็น hex (8 digits)
    for(int i = 9; i >= 2; i--) {
        buffer[i] = hex_chars[num & 0x0F];
        num >>= 4;
    }
    
    buffer[10] = '\0';
    USART_Print(buffer);
}

/**
 * @brief ส่ง 1 byte
 */
void USART_WriteByte(uint8_t data) {
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, data);
}

/**
 * @brief ตรวจสอบว่ามีข้อมูลรอรับหรือไม่ (เช็ค ring buffer ไม่ใช่ hardware flag ตรงๆ)
 */
uint8_t USART_Available(void) {
    return (s_rx_head != s_rx_tail) ? 1 : 0;
}

/**
 * @brief อ่านข้อมูล 1 byte จาก ring buffer (blocking จนกว่า ISR จะเติมข้อมูล)
 */
uint8_t USART_Read(void) {
    while (!USART_Available());

    __disable_irq();
    uint8_t data = s_rx_buffer[s_rx_tail];
    s_rx_tail = (uint16_t)((s_rx_tail + 1) % USART_RX_BUFFER_SIZE);
    __enable_irq();

    return data;
}

/**
 * @brief อ่านข้อมูลหลาย bytes
 */
uint16_t USART_ReadBytes(uint8_t* buffer, uint16_t length) {
    uint16_t count = 0;
    
    while(count < length && USART_Available()) {
        buffer[count++] = USART_Read();
    }
    
    return count;
}

/**
 * @brief ล้างข้อมูลใน receive buffer (ring buffer)
 */
void USART_Flush(void) {
    __disable_irq();
    s_rx_head = 0;
    s_rx_tail = 0;
    __enable_irq();
}

/* ========== Interrupt Handler ========== */

/**
 * @brief USART1 RX interrupt handler — เติม ring buffer ทุกครั้งที่มี byte เข้ามา
 * @warning มี ISR ได้แค่ตัวเดียวต่อ vector — ห้ามใช้ library อื่นที่ต้องการ
 *          USART1_IRQHandler ของตัวเอง (เช่น TJC) พร้อมกับ SimpleUSART
 */
void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        // อ่านข้อมูล (การอ่านจะเคลียร์ RXNE flag ไปในตัว)
        uint8_t data = (uint8_t)USART_ReceiveData(USART1);

        uint16_t next_head = (uint16_t)((s_rx_head + 1) % USART_RX_BUFFER_SIZE);
        if (next_head != s_rx_tail) {
            s_rx_buffer[s_rx_head] = data;
            s_rx_head = next_head;
        }
        // buffer เต็ม — ทิ้ง byte นี้ (ดีกว่าเขียนทับข้อมูลเก่าที่ยังไม่ได้อ่าน)

        // ให้โมดูลอื่น (เช่น TJC) แอบดู byte นี้ได้โดยไม่ต้องแย่งชิง ISR vector
        USART_RxByteHook(data);
    }

    // IDLE line — จุดจบเฟรมของโปรโตคอลแบบ frame-based (เช่น Modbus RTU)
    // เฉพาะ library ที่ enable USART_IT_IDLE เองเท่านั้นที่จะถูกเรียก (เช่น
    // โหมด DMA ของ Modbus) — โหมดปกติไม่มีใคร enable → ไม่มีผลใดๆ
    if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET) {
        USART_ClearITPendingBit(USART1, USART_IT_IDLE);
        USART_IdleHook();
    }
}
