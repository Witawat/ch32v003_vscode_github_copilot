/**
 * @file WS2812Matrix.h
 * @brief WS2812 8x8 LED Matrix Library สำหรับ CH32V003 / CH32V006
 * @version 1.0
 * @date 2026-05-03
 *
 * @details
 * Library สำหรับควบคุม WS2812 RGB LEDs ที่ต่อเป็น Matrix 8x8
 * ใช้ NeoPixel เป็น low-level driver + Instance struct pattern
 * รองรับ 2 wiring patterns: Zigzag (ซิกแซก) และ Snake (งูเลื้อย)
 *
 * **คุณสมบัติ:**
 * - ควบคุม Matrix 8x8 (ขยายได้ถึง 32x32)
 * - รูปแบบการต่อสาย: Zigzag และ Snake
 * - ระบบพิกัด (x, y) ที่ใช้งานง่าย
 * - ฟังก์ชันวาดรูปพื้นฐาน (เส้น, สี่เหลี่ยม, วงกลม)
 * - ใช้ SimpleGPIO pins (PC4, PD2, ...)
 * - Instance struct pattern (แบบ P10/PIR/HCSR04)
 * - Brightness control (0-255)
 *
 * **WS2812 Protocol ใช้ร่วมกับ:**
 * - WS2812B, WS2813, WS2815, SK6812 — timing protocol เดียวกัน
 *
 * **Hardware Connection:**
 * ```
 *   WS2812 8x8 Matrix       CH32V003
 *   DIN  (Data In)  ------> GPIO Pin (PC4)
 *   VCC              ------> 5V
 *   GND              ------> GND
 *
 *   หมายเหตุ: บาง matrix มี DOUT (Data Out) — ใช้ cascade ต่อพ่วงแผงถัดไป
 *   ต้องใช้ Level Shifter ถ้า MCU 3.3V (CH32V003) และ Matrix 5V
 * ```
 *
 * **Wiring Patterns:**
 *
 * Zigzag (ซิกแซกเริ่มจากซ้าย):
 * ```
 * Row 0:  0→ 1→ 2→ 3→ 4→ 5→ 6→ 7
 * Row 1: 15←14←13←12←11←10← 9← 8
 * Row 2: 16→17→18→19→20→21→22→23
 * Row 3: 31←30←29←28←27←26←25←24
 * ```
 *
 * Snake (งูเลื้อยต่อเนื่อง):
 * ```
 * Row 0:  0→ 1→ 2→ 3→ 4→ 5→ 6→ 7
 * Row 1:  8→ 9→10→11→12→13→14→15
 * Row 2: 16→17→18→19→20→21→22→23
 * Row 3: 24→25→26→27→28→29→30→31
 * ```
 *
 * @example
 * #include "WS2812Matrix.h"
 *
 * int main(void) {
 *     SystemCoreClockUpdate();
 *     Timer_Init();
 *
 *     WS2812M_Instance matrix;
 *     WS2812M_Init(&matrix, PC4, 8, 8, WIRING_ZIGZAG);
 *
 *     WS2812M_Clear(&matrix);
 *     WS2812M_SetPixel(&matrix, 3, 3, 255, 0, 0);  // จุดกลางสีแดง
 *     WS2812M_Show(&matrix);
 *
 *     while(1);
 * }
 *
 * @note ต้องเรียก SystemCoreClockUpdate() และ Timer_Init() ก่อน
 * @note NeoPixel ใช้ static buffer → รองรับ 1 instance เท่านั้น
 *
 * @author CH32V003 Library Team
 * @copyright MIT License
 */

#ifndef __WS2812MATRIX_H
#define __WS2812MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../SimpleHAL/SimpleHAL.h"
#include "../NeoPixel/NeoPixel.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/* ========== Configuration (overrideable) ========== */

#ifndef WS2812M_MAX_WIDTH
#define WS2812M_MAX_WIDTH   32  /**< ความกว้างสูงสุด (px) */
#endif

#ifndef WS2812M_MAX_HEIGHT
#define WS2812M_MAX_HEIGHT  32  /**< ความสูงสูงสุด (px) */
#endif

#ifndef WS2812M_MAX_INSTANCES
#define WS2812M_MAX_INSTANCES  1  /**< จำนวน instances สูงสุด (NeoPixel รองรับ 1) */
#endif

/* ========== Wiring Pattern Enum ========== */

/**
 * @brief รูปแบบการต่อสาย LED Matrix
 *
 * **WIRING_ZIGZAG**: ซิกแซกเริ่มจากซ้าย (ใช้บ่อยที่สุด)
 * - แถวคู่: วิ่งซ้าย→ขวา
 * - แถวคี่: วิ่งขวา→ซ้าย
 *
 * **WIRING_SNAKE**: งูเลื้อยต่อเนื่อง
 * - ทุกแถววิ่งซ้าย→ขวา
 */
typedef enum {
    WIRING_ZIGZAG = 0,  /**< ซิกแซกเริ่มจากซ้าย (default) */
    WIRING_SNAKE  = 1   /**< งูเลื้อยแบบต่อเนื่อง */
} WS2812M_Wiring;

/* ========== Instance Structure ========== */

/**
 * @brief WS2812 Matrix Instance
 */
typedef struct {
    /* ----- Configuration ----- */
    uint8_t          data_pin;     /**< SimpleGPIO data pin (PC4, PD2, ...) */
    uint8_t          width;        /**< ความกว้าง (px) */
    uint8_t          height;       /**< ความสูง (px) */
    uint16_t         num_pixels;   /**< จำนวน LEDs ทั้งหมด (width × height) */
    WS2812M_Wiring   wiring;       /**< รูปแบบการต่อสาย */

    /* ----- Flags ----- */
    uint8_t initialized;           /**< Initialized flag */
} WS2812M_Instance;

/* ========== Function Prototypes ========== */

/**
 * @brief เริ่มต้น WS2812 LED Matrix
 * @param inst ตัวชี้ไปยัง WS2812M_Instance
 * @param data_pin SimpleGPIO data pin (PC4, PD2, ...)
 * @param width ความกว้างของ matrix (px)
 * @param height ความสูงของ matrix (px)
 * @param wiring รูปแบบการต่อสาย (WIRING_ZIGZAG หรือ WIRING_SNAKE)
 * @return 1 = สำเร็จ, 0 = ล้มเหลว
 *
 * @example
 * WS2812M_Instance matrix;
 * WS2812M_Init(&matrix, PC4, 8, 8, WIRING_ZIGZAG);
 */
uint8_t WS2812M_Init(WS2812M_Instance* inst, uint8_t data_pin,
                     uint8_t width, uint8_t height, WS2812M_Wiring wiring);

/**
 * @brief ตั้งค่าสีของพิกเซลที่ตำแหน่ง (x, y)
 * @param inst ตัวชี้ไปยัง WS2812M_Instance
 * @param x พิกัด X (0 = ซ้ายสุด)
 * @param y พิกัด Y (0 = บนสุด)
 * @param r ค่าสีแดง (0-255)
 * @param g ค่าสีเขียว (0-255)
 * @param b ค่าสีน้ำเงิน (0-255)
 *
 * @note ต้องเรียก WS2812M_Show() เพื่ออัพเดทการแสดงผล
 *
 * @example
 * WS2812M_SetPixel(&matrix, 3, 3, 255, 0, 0);
 * WS2812M_Show(&matrix);
 */
void WS2812M_SetPixel(WS2812M_Instance* inst, uint8_t x, uint8_t y,
                      uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief ตั้งค่าสีของพิกเซลด้วย 32-bit color
 * @param inst ตัวชี้ไปยัง WS2812M_Instance
 * @param x พิกัด X
 * @param y พิกัด Y
 * @param color สี 32-bit (0xRRGGBB)
 *
 * @example
 * WS2812M_SetPixelColor(&matrix, 3, 3, 0xFF0000);  // สีแดง
 */
void WS2812M_SetPixelColor(WS2812M_Instance* inst, uint8_t x, uint8_t y,
                           uint32_t color);

/**
 * @brief อ่านค่าสีของพิกเซล
 * @param inst ตัวชี้ไปยัง WS2812M_Instance
 * @param x พิกัด X
 * @param y พิกัด Y
 * @return สี 32-bit (0xRRGGBB) หรือ 0 ถ้า error
 *
 * @example
 * uint32_t color = WS2812M_GetPixel(&matrix, 3, 3);
 */
uint32_t WS2812M_GetPixel(WS2812M_Instance* inst, uint8_t x, uint8_t y);

/**
 * @brief ลบ Matrix ทั้งหมด (ดับทุกดวง)
 * @param inst ตัวชี้ไปยัง WS2812M_Instance
 */
void WS2812M_Clear(WS2812M_Instance* inst);

/**
 * @brief เติมสีทั้ง Matrix
 * @param inst ตัวชี้ไปยัง WS2812M_Instance
 * @param r Red (0-255)
 * @param g Green (0-255)
 * @param b Blue (0-255)
 *
 * @example
 * WS2812M_Fill(&matrix, 0, 255, 0);  // เติมสีเขียว
 */
void WS2812M_Fill(WS2812M_Instance* inst, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief อัพเดทการแสดงผล (ส่งข้อมูลไปยัง LEDs)
 * @param inst ตัวชี้ไปยัง WS2812M_Instance
 *
 * @note ต้องเรียกหลังจาก SetPixel / Clear / Fill
 *
 * @example
 * WS2812M_SetPixel(&matrix, 0, 0, 255, 0, 0);
 * WS2812M_Show(&matrix);
 */
void WS2812M_Show(WS2812M_Instance* inst);

/**
 * @brief ตั้งค่าความสว่าง (0-255)
 * @param inst ตัวชี้ไปยัง WS2812M_Instance
 * @param brightness ความสว่าง (0=ดับ, 255=สว่างสุด)
 *
 * @example
 * WS2812M_SetBrightness(&matrix, 50);  // ลดความสว่าง
 */
void WS2812M_SetBrightness(WS2812M_Instance* inst, uint8_t brightness);

/**
 * @brief หยุดการทำงานของ Matrix
 * @param inst ตัวชี้ไปยัง WS2812M_Instance
 */
void WS2812M_Deinit(WS2812M_Instance* inst);

/* ========== Drawing Primitives ========== */

/**
 * @brief วาดเส้นตรง
 * @param inst ตัวชี้ไปยัง WS2812M_Instance
 * @param x0 พิกัด X จุดเริ่มต้น
 * @param y0 พิกัด Y จุดเริ่มต้น
 * @param x1 พิกัด X จุดสิ้นสุด
 * @param y1 พิกัด Y จุดสิ้นสุด
 * @param r Red
 * @param g Green
 * @param b Blue
 *
 * ใช้ Bresenham's line algorithm
 */
void WS2812M_DrawLine(WS2812M_Instance* inst, int16_t x0, int16_t y0,
                      int16_t x1, int16_t y1, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief วาดเส้นตรงด้วย 32-bit color
 */
void WS2812M_DrawLineColor(WS2812M_Instance* inst, int16_t x0, int16_t y0,
                           int16_t x1, int16_t y1, uint32_t color);

/**
 * @brief วาดสี่เหลี่ยม (เฉพาะขอบ)
 * @param inst ตัวชี้ไปยัง WS2812M_Instance
 * @param x พิกัด X มุมซ้ายบน
 * @param y พิกัด Y มุมซ้ายบน
 * @param w ความกว้าง
 * @param h ความสูง
 */
void WS2812M_DrawRect(WS2812M_Instance* inst, int16_t x, int16_t y,
                      uint8_t w, uint8_t h, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief วาดสี่เหลี่ยมแบบทึบ
 */
void WS2812M_FillRect(WS2812M_Instance* inst, int16_t x, int16_t y,
                      uint8_t w, uint8_t h, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief วาดวงกลม (เฉพาะขอบ)
 * @param inst ตัวชี้ไปยัง WS2812M_Instance
 * @param x0 พิกัด X ศูนย์กลาง
 * @param y0 พิกัด Y ศูนย์กลาง
 * @param radius รัศมี
 */
void WS2812M_DrawCircle(WS2812M_Instance* inst, int16_t x0, int16_t y0,
                        uint8_t radius, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief วาดวงกลมแบบทึบ
 */
void WS2812M_FillCircle(WS2812M_Instance* inst, int16_t x0, int16_t y0,
                        uint8_t radius, uint8_t r, uint8_t g, uint8_t b);

/* ========== Utility ========== */

/**
 * @brief แปลงพิกัด (x, y) เป็น LED index ตาม wiring pattern
 * @param x พิกัด X
 * @param y พิกัด Y
 * @param width ความกว้างของ matrix
 * @param wiring รูปแบบการต่อสาย
 * @return LED index (0-based)
 *
 * @note ฟังก์ชัน internal แต่เปิดให้เรียกใช้เพื่อ debug
 *
 * @example
 * uint16_t idx = WS2812M_XYtoIndex(3, 3, 8, WIRING_ZIGZAG);
 */
uint16_t WS2812M_XYtoIndex(uint8_t x, uint8_t y, uint8_t width,
                           WS2812M_Wiring wiring);

#ifdef __cplusplus
}
#endif

#endif /* __WS2812MATRIX_H */
