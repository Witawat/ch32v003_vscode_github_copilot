/**
 * @file SimpleArduino.c
 * @brief Arduino-compatible API implementations สำหรับ CH32V003
 * @version 1.0
 * @date 2026-06-16
 *
 * @details
 * Implementation ของฟังก์ชัน Arduino-style utility
 * - digitalPinToInterrupt: แปลง pin → EXTI line
 * - random/randomSeed: LCG PRNG
 * - yield: cooperative multitasking placeholder
 * - dtostrf: lightweight float-to-string
 * - USART Println/PrintFloat (optional compile-time)
 *
 * @author CH32V003 Library Team
 */
#include "SimpleArduino.h"
#include "SimpleDelay.h"
#include "SimpleGPIO.h"
#include "SimpleUSART.h"
#include "SimpleIWDG.h"

/* ========== digitalPinToInterrupt ========== */

/**
 * @brief แปลง GPIO pin number เป็น EXTI interrupt line
 *
 * CH32V003 มี EXTI 8 lines (0-7) แชร์กับทุก pins
 * ใช้ GPIO_EXTILineConfig() จับคู่ port+pin กับ EXTI line
 * pin_source ใน pin_map table (SimpleGPIO.c) คือ EXTI line
 */
uint8_t digitalPinToInterrupt(uint8_t pin) {
    switch (pin) {
        case PA1: return 1;
        case PA2: return 2;
        case PC0: return 0;
        case PC1: return 1;
        case PC2: return 2;
        case PC3: return 3;
        case PC4: return 4;
        case PC5: return 5;
        case PC6: return 6;
        case PC7: return 7;
        case PD2: return 2;
        case PD3: return 3;
        case PD4: return 4;
        case PD5: return 5;
        case PD6: return 6;
        case PD7: return 7;
        default: return 0xFF;
    }
}

/* ========== Random Number Generator (LCG) ========== */

/** @brief PRNG state */
static uint32_t _random_seed = 1;

/**
 * Glibc-style LCG: X_{n+1} = (1103515245 * X_n + 12345) mod 2^31
 * ให้ผลลัพธ์ 31-bit (0 ถึง 0x7FFFFFFF)
 */
static uint32_t _random_next(void) {
    _random_seed = (_random_seed * 1103515245UL + 12345UL) & 0x7FFFFFFFUL;
    return _random_seed;
}

void randomSeed(uint32_t seed) {
    _random_seed = seed ? seed : 1;
}

long _randomMax(long max) {
    if (max <= 0) return 0;
    return (long)(_random_next() % (uint32_t)max);
}

long _randomRange(long min, long max) {
    if (min >= max) return min;
    return min + (long)(_random_next() % (uint32_t)(max - min));
}

/* ========== yield() ========== */

static uint8_t _iwdg_active = 0;  // set by SimpleIWDG when IWDG is enabled

/**
 * @brief ให้ CPU ทำงานพื้นหลัง (cooperative multitasking)
 *        ถ้า IWDG ถูกเปิดใช้งาน → feed watchdog
 *        ปลอดภัยที่จะเรียกแม้ไม่ได้เปิด IWDG
 */
void yield(void) {
    if (_iwdg_active) {
        IWDG_Feed();
    }
}

/**
 * @brief แจ้ง SimpleArduino ว่า IWDG ถูกเปิดใช้งานแล้ว
 *        เรียกจาก SimpleIWDG.c ตอน IWDG_Init/IWDG_SimpleInit
 */
void arduino_SetIWDGActive(void) {
    _iwdg_active = 1;
}

/* ========== dtostrf() — Lightweight Float-to-String ========== */

/**
 * แปลง double → string โดยใช้ integer arithmetic ล้วน
 * ไม่ต้องใช้ FPU หรือ libc sprintf (ซึ่งกิน flash ~2KB)
 *
 * อัลกอริทึม:
 * 1. ปัดเศษตาม precision
 * 2. แยก integer part และ fractional part
 * 3. สร้าง string ใน temp buffer (กลับด้าน)
 * 4. reverse + pad space ตาม width
 */
char* dtostrf(double val, int width, unsigned int precision, char* buf) {
    if (buf == NULL) return NULL;

    int is_neg = 0;
    if (val < 0.0) {
        is_neg = 1;
        val = -val;
    }

    // คำนวณค่า rounding และ divisor
    unsigned long div = 1;
    double rounder = 0.5;
    for (unsigned int i = 0; i < precision; i++) {
        rounder /= 10.0;
        div *= 10UL;
    }
    if (precision == 0) {
        rounder = 0.5;
        div = 1;
    }

    // ปัดเศษและแยกส่วน
    val += rounder;
    unsigned long int_part = (unsigned long)val;
    unsigned long frac_part = (unsigned long)((val - (double)int_part) * (double)div + 0.5);

    // จัดการ carry จาก fractional → integer
    if (frac_part >= div) {
        frac_part = 0;
        int_part++;
    }

    // สร้าง string ใน temp buffer (จากขวาไปซ้าย)
    char temp[32];
    int idx = 0;

    // fractional part
    if (precision > 0) {
        unsigned long fp = frac_part;
        for (unsigned int i = 0; i < precision; i++) {
            temp[idx++] = '0' + (char)(fp % 10);
            fp /= 10;
        }
        temp[idx++] = '.';
    }

    // integer part
    if (int_part == 0) {
        temp[idx++] = '0';
    } else {
        unsigned long ip = int_part;
        while (ip > 0) {
            temp[idx++] = '0' + (char)(ip % 10);
            ip /= 10;
        }
    }

    // เครื่องหมายลบ
    if (is_neg) {
        temp[idx++] = '-';
    }

    // reverse temp → buf พร้อม padding
    int len = idx;
    int pad = (width > len) ? (width - len) : 0;

    int out = 0;
    for (int i = 0; i < pad; i++) {
        buf[out++] = ' ';
    }
    for (int i = len - 1; i >= 0; i--) {
        buf[out++] = temp[i];
    }
    buf[out] = '\0';

    return buf;
}

/* ========== USART Print Extensions (Optional) ========== */

#if ENABLE_USART_PRINTLN

void USART_Println(const char* str) {
    USART_Print(str);
    USART_Print("\r\n");
}

void USART_PrintlnNum(int32_t num) {
    USART_PrintNum(num);
    USART_Print("\r\n");
}

void USART_PrintlnHex(uint32_t num, uint8_t uppercase) {
    USART_PrintHex(num, uppercase);
    USART_Print("\r\n");
}

#endif /* ENABLE_USART_PRINTLN */

#if ENABLE_USART_PRINTFLOAT

void USART_PrintFloat(float val, uint8_t decimal_places) {
    if (decimal_places > 20) decimal_places = 20;
    char buf[64];
    dtostrf((double)val, 0, decimal_places, buf);
    USART_Print(buf);
}

#if ENABLE_USART_PRINTLN
void USART_PrintlnFloat(float val, uint8_t decimal_places) {
    USART_PrintFloat(val, decimal_places);
    USART_Print("\r\n");
}
#endif

#endif /* ENABLE_USART_PRINTFLOAT */
