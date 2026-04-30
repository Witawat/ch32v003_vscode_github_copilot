/**
 * @file Buzzer.h
 * @brief Buzzer Control Library สำหรับ CH32V003
 * @version 1.0.0
 * @date 2025-12-22
 * 
 * @details
 * Library สำหรับควบคุม Buzzer แบบง่าย รองรับทั้ง Active High และ Active Low
 * สามารถควบคุมโทนเสียง เล่นทำนอง และสร้างจังหวะได้
 * 
 * **คุณสมบัติ:**
 * - รองรับ Active High และ Active Low buzzer
 * - ควบคุมความถี่ 20Hz - 20kHz
 * - เล่นทำนอง (melody) พร้อม musical notes
 * - สร้างจังหวะ (rhythm patterns)
 * - โหมด blocking และ non-blocking
 * - ปรับระดับเสียง (volume control)
 * - ใช้ PWM สำหรับความแม่นยำสูง
 * 
 * **Hardware Support:**
 * - รองรับ PWM pins: PA1, PC0, PC3, PC4, PD2, PD3, PD4, PD7
 * - ต่อผ่าน transistor/MOSFET สำหรับกระแสสูง
 * 
 * @example
 * // Basic beep
 * Buzzer_Init(PC0, BUZZER_ACTIVE_HIGH);
 * Buzzer_Beep(200);  // Beep 200ms
 * 
 * // Play tone
 * Buzzer_Tone(1000, 500);  // 1kHz for 500ms
 * 
 * // Play melody
 * Note melody[] = {
 *     {NOTE_C4, 250}, {NOTE_E4, 250}, {NOTE_G4, 500}
 * };
 * Buzzer_PlayMelody(melody, 3);
 * 
 * @author CH32V003 Library Team
 * @copyright MIT License
 */

#ifndef __BUZZER_H
#define __BUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>
#include "../../SimpleHAL/SimpleHAL.h"
#include <stdint.h>

/* ========== Configuration ========== */

/**
 * @brief Default buzzer frequency (Hz)
 */
#ifndef BUZZER_DEFAULT_FREQ
#define BUZZER_DEFAULT_FREQ  2000
#endif

/**
 * @brief Default beep duration (ms)
 */
#ifndef BUZZER_DEFAULT_DURATION
#define BUZZER_DEFAULT_DURATION  100
#endif

/**
 * @brief Default volume (0-100%)
 */
#ifndef BUZZER_DEFAULT_VOLUME
#define BUZZER_DEFAULT_VOLUME  50
#endif

/* ========== Type Definitions ========== */

/**
 * @brief Buzzer type (Active High or Active Low)
 */
typedef enum {
    BUZZER_ACTIVE_HIGH = 0,  /**< Active High - เปิดด้วย HIGH */
    BUZZER_ACTIVE_LOW = 1    /**< Active Low - เปิดด้วย LOW */
} BuzzerType;

/**
 * @brief Musical note structure
 */
typedef struct {
    uint16_t frequency;  /**< ความถี่ในหน่วย Hz (0 = rest/silence) */
    uint16_t duration;   /**< ระยะเวลาในหน่วย ms */
} Note;

/**
 * @brief Melody pattern structure
 */
typedef struct {
    const Note* notes;   /**< Array ของโน้ต */
    uint8_t length;      /**< จำนวนโน้ต */
    uint8_t repeat;      /**< จำนวนครั้งที่เล่นซ้ำ (0 = infinite) */
} Melody;

/**
 * @brief Buzzer state
 */
typedef enum {
    BUZZER_IDLE = 0,     /**< ไม่ทำงาน */
    BUZZER_PLAYING,      /**< กำลังเล่นเสียง */
    BUZZER_PAUSED        /**< หยุดชั่วคราว */
} BuzzerState;

/* ========== Musical Notes Definitions ========== */

// Octave 3
#define NOTE_C3   131
#define NOTE_CS3  139
#define NOTE_D3   147
#define NOTE_DS3  156
#define NOTE_E3   165
#define NOTE_F3   175
#define NOTE_FS3  185
#define NOTE_G3   196
#define NOTE_GS3  208
#define NOTE_A3   220
#define NOTE_AS3  233
#define NOTE_B3   247

// Octave 4 (Middle C)
#define NOTE_C4   262
#define NOTE_CS4  277
#define NOTE_D4   294
#define NOTE_DS4  311
#define NOTE_E4   330
#define NOTE_F4   349
#define NOTE_FS4  370
#define NOTE_G4   392
#define NOTE_GS4  415
#define NOTE_A4   440  // A4 = 440Hz (standard tuning)
#define NOTE_AS4  466
#define NOTE_B4   494

// Octave 5
#define NOTE_C5   523
#define NOTE_CS5  554
#define NOTE_D5   587
#define NOTE_DS5  622
#define NOTE_E5   659
#define NOTE_F5   698
#define NOTE_FS5  740
#define NOTE_G5   784
#define NOTE_GS5  831
#define NOTE_A5   880
#define NOTE_AS5  932
#define NOTE_B5   988

// Octave 6
#define NOTE_C6   1047
#define NOTE_CS6  1109
#define NOTE_D6   1175
#define NOTE_DS6  1245
#define NOTE_E6   1319
#define NOTE_F6   1397
#define NOTE_FS6  1480
#define NOTE_G6   1568
#define NOTE_GS6  1661
#define NOTE_A6   1760
#define NOTE_AS6  1865
#define NOTE_B6   1976

// Special
#define NOTE_REST 0    /**< Rest (silence) */

/* ========== Note Duration Definitions ========== */

/**
 * @brief Standard note durations (at 120 BPM)
 */
#define DURATION_WHOLE      2000  /**< Whole note */
#define DURATION_HALF       1000  /**< Half note */
#define DURATION_QUARTER    500   /**< Quarter note */
#define DURATION_EIGHTH     250   /**< Eighth note */
#define DURATION_SIXTEENTH  125   /**< Sixteenth note */

/* ========== Function Prototypes ========== */

/* ----- Initialization ----- */

/**
 * @brief เริ่มต้นการใช้งาน Buzzer
 * @param pin หมายเลข pin (ต้องเป็น PWM pin: PA1, PC0, PC3-4, PD2-4, PD7)
 * @param type ชนิดของ buzzer (BUZZER_ACTIVE_HIGH หรือ BUZZER_ACTIVE_LOW)
 * 
 * @note ฟังก์ชันนี้จะ init PWM อัตโนมัติ
 * @note ต้องเรียกฟังก์ชันนี้ก่อนใช้งาน buzzer
 * 
 * @example
 * Buzzer_Init(PC0, BUZZER_ACTIVE_HIGH);
 */
void Buzzer_Init(uint8_t pin, BuzzerType type);

/**
 * @brief ตั้งค่าระดับเสียง (volume)
 * @param volume ระดับเสียง 0-100 (%)
 * 
 * @note ควบคุมผ่าน PWM duty cycle
 * @note 0 = ปิดเสียง, 100 = เสียงดังสุด
 * 
 * @example
 * Buzzer_SetVolume(75);  // 75% volume
 */
void Buzzer_SetVolume(uint8_t volume);

/* ----- Basic Control ----- */

/**
 * @brief เปิด buzzer (continuous tone)
 * 
 * @note ใช้ความถี่ที่ตั้งไว้ล่าสุด หรือ default frequency
 * @note ต้องเรียก Buzzer_Off() เพื่อปิด
 * 
 * @example
 * Buzzer_On();
 * Delay_Ms(1000);
 * Buzzer_Off();
 */
void Buzzer_On(void);

/**
 * @brief ปิด buzzer
 * 
 * @example
 * Buzzer_Off();
 */
void Buzzer_Off(void);

/**
 * @brief สลับสถานะ buzzer (toggle)
 * 
 * @example
 * while(1) {
 *     Buzzer_Toggle();
 *     Delay_Ms(500);
 * }
 */
void Buzzer_Toggle(void);

/**
 * @brief Beep ครั้งเดียว (blocking)
 * @param duration_ms ระยะเวลา beep (milliseconds)
 * 
 * @note ฟังก์ชันนี้จะบล็อกการทำงานจนกว่า beep จะเสร็จ
 * @note ใช้ความถี่ที่ตั้งไว้ล่าสุด หรือ default frequency
 * 
 * @example
 * Buzzer_Beep(200);  // Beep 200ms
 */
void Buzzer_Beep(uint16_t duration_ms);

/**
 * @brief Beep หลายครั้ง (blocking)
 * @param count จำนวนครั้ง
 * @param on_time ระยะเวลาเปิด (ms)
 * @param off_time ระยะเวลาปิด (ms)
 * 
 * @example
 * Buzzer_BeepMultiple(3, 100, 100);  // Beep 3 ครั้ง
 */
void Buzzer_BeepMultiple(uint8_t count, uint16_t on_time, uint16_t off_time);

/* ----- Tone Control ----- */

/**
 * @brief เล่นโทนเสียงที่ความถี่กำหนด (blocking)
 * @param frequency ความถี่ในหน่วย Hz (20-20000)
 * @param duration_ms ระยะเวลา (milliseconds), 0 = continuous
 * 
 * @note ฟังก์ชันนี้จะบล็อกการทำงานจนกว่าเสียงจะเสร็จ
 * @note ถ้า duration = 0 จะเล่นต่อเนื่องจนกว่าจะเรียก Buzzer_NoTone()
 * 
 * @example
 * Buzzer_Tone(1000, 500);  // 1kHz for 500ms
 * Buzzer_Tone(440, 0);     // A4 continuous
 */
void Buzzer_Tone(uint16_t frequency, uint16_t duration_ms);

/**
 * @brief เล่นโทนเสียงแบบ non-blocking
 * @param frequency ความถี่ในหน่วย Hz (20-20000)
 * @param duration_ms ระยะเวลา (milliseconds), 0 = continuous
 * 
 * @note ฟังก์ชันนี้จะ return ทันที ไม่บล็อกการทำงาน
 * @note ใช้ Buzzer_IsBusy() เพื่อตรวจสอบสถานะ
 * 
 * @example
 * Buzzer_ToneAsync(1000, 500);
 * while(Buzzer_IsBusy()) {
 *     // Do other work
 * }
 */
void Buzzer_ToneAsync(uint16_t frequency, uint16_t duration_ms);

/**
 * @brief หยุดเล่นโทนเสียง
 * 
 * @note ใช้กับ Buzzer_Tone() ที่ duration = 0
 * 
 * @example
 * Buzzer_Tone(440, 0);  // Start continuous tone
 * Delay_Ms(1000);
 * Buzzer_NoTone();      // Stop
 */
void Buzzer_NoTone(void);

/* ----- Melody & Pattern ----- */

/**
 * @brief เล่นทำนอง (blocking)
 * @param melody array ของ Note structures
 * @param length จำนวนโน้ต
 * 
 * @note ฟังก์ชันนี้จะบล็อกการทำงานจนกว่าทำนองจะเล่นเสร็จ
 * 
 * @example
 * Note melody[] = {
 *     {NOTE_C4, 250}, {NOTE_E4, 250}, {NOTE_G4, 500}
 * };
 * Buzzer_PlayMelody(melody, 3);
 */
void Buzzer_PlayMelody(const Note* melody, uint8_t length);

/**
 * @brief เล่นทำนองแบบ non-blocking
 * @param melody array ของ Note structures
 * @param length จำนวนโน้ต
 * @param repeat จำนวนครั้งที่เล่นซ้ำ (0 = infinite)
 * 
 * @note ฟังก์ชันนี้จะ return ทันที
 * @note ต้องเรียก Buzzer_Update() ใน main loop
 * 
 * @example
 * Note melody[] = {{NOTE_C4, 250}, {NOTE_E4, 250}};
 * Buzzer_PlayMelodyAsync(melody, 2, 1);  // เล่น 1 ครั้ง
 * 
 * while(1) {
 *     Buzzer_Update();  // ต้องเรียกใน loop
 *     // Do other work
 * }
 */
void Buzzer_PlayMelodyAsync(const Note* melody, uint8_t length, uint8_t repeat);

/**
 * @brief Update melody playback (สำหรับ non-blocking mode)
 * 
 * @note ต้องเรียกฟังก์ชันนี้ใน main loop เมื่อใช้ async functions
 * 
 * @example
 * while(1) {
 *     Buzzer_Update();
 *     // Other tasks
 * }
 */
void Buzzer_Update(void);

/* ----- Advanced Functions ----- */

/**
 * @brief หยุดการเล่นทั้งหมด
 * 
 * @note หยุดทั้ง blocking และ non-blocking operations
 * 
 * @example
 * Buzzer_Stop();
 */
void Buzzer_Stop(void);

/**
 * @brief พักการเล่น (pause)
 * 
 * @note ใช้กับ non-blocking mode
 * @note เรียก Buzzer_Resume() เพื่อเล่นต่อ
 */
void Buzzer_Pause(void);

/**
 * @brief เล่นต่อจากที่พัก (resume)
 * 
 * @note ใช้กับ non-blocking mode
 */
void Buzzer_Resume(void);

/**
 * @brief ตรวจสอบว่า buzzer กำลังทำงานอยู่หรือไม่
 * @return 1 = กำลังทำงาน, 0 = ว่าง
 * 
 * @example
 * if(!Buzzer_IsBusy()) {
 *     Buzzer_Beep(100);
 * }
 */
uint8_t Buzzer_IsBusy(void);

/**
 * @brief อ่านสถานะปัจจุบันของ buzzer
 * @return BuzzerState (BUZZER_IDLE, BUZZER_PLAYING, BUZZER_PAUSED)
 * 
 * @example
 * if(Buzzer_GetState() == BUZZER_PLAYING) {
 *     // Buzzer is playing
 * }
 */
BuzzerState Buzzer_GetState(void);

/**
 * @brief Sweep ความถี่ (frequency sweep)
 * @param start_freq ความถี่เริ่มต้น (Hz)
 * @param end_freq ความถี่สุดท้าย (Hz)
 * @param duration_ms ระยะเวลาทั้งหมด (ms)
 * @param step_ms ระยะเวลาแต่ละ step (ms)
 * 
 * @note ฟังก์ชันนี้จะบล็อกการทำงาน
 * @note ใช้สำหรับสร้างเสียงพิเศษ เช่น siren
 * 
 * @example
 * // Siren effect
 * Buzzer_FrequencySweep(500, 2000, 1000, 10);
 */
void Buzzer_FrequencySweep(uint16_t start_freq, uint16_t end_freq, 
                           uint16_t duration_ms, uint16_t step_ms);

/* ========== Pre-defined Patterns ========== */

/**
 * @brief เล่น SOS pattern (... --- ...)
 * 
 * @example
 * Buzzer_PlaySOS();
 */
void Buzzer_PlaySOS(void);

/**
 * @brief เล่นเสียงเตือนภัย (alarm)
 * @param duration_ms ระยะเวลาทั้งหมด (ms), 0 = continuous
 * 
 * @example
 * Buzzer_PlayAlarm(5000);  // 5 seconds
 */
void Buzzer_PlayAlarm(uint16_t duration_ms);

/**
 * @brief เล่นเสียงสำเร็จ (success sound)
 * 
 * @example
 * Buzzer_PlaySuccess();
 */
void Buzzer_PlaySuccess(void);

/**
 * @brief เล่นเสียงผิดพลาด (error sound)
 * 
 * @example
 * Buzzer_PlayError();
 */
void Buzzer_PlayError(void);

/**
 * @brief เล่นเสียงเริ่มต้น (startup sound)
 * 
 * @example
 * Buzzer_PlayStartup();
 */
void Buzzer_PlayStartup(void);

#ifdef __cplusplus
}
#endif

#endif  // __BUZZER_H
