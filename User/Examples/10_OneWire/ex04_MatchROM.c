/**
 * @example ex04_MatchROM.c
 * @brief OneWire_MatchROM — เลือก device เฉพาะบน multi-device bus
 *
 * ค้นหา devices ทั้งหมดด้วย OneWire_Search → เลือกทีละตัวด้วย MatchROM → อ่านข้อมูล
 * ใช้กับ DS18B20 หลายตัวบน bus เดียวกัน
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);

    OneWire_Bus* bus = OneWire_Init(PD2);
    if (!bus) { USART_Print("OneWire init failed\r\n"); while(1){} }

    // 1. ค้นหา devices ทั้งหมด
    uint8_t roms[4][8], count = 0;
    OneWire_ResetSearch(bus);
    while (OneWire_Search(bus) && count < 4) {
        for (int i = 0; i < 8; i++) roms[count][i] = bus->rom[i];
        count++;
    }

    USART_Print("Found "); USART_PrintNum(count); USART_Print(" devices:\r\n");
    for (int d = 0; d < count; d++) {
        USART_Print("  [");
        USART_PrintNum(d);
        USART_Print("] ");
        for (int i = 0; i < 8; i++) { USART_PrintHex(roms[d][i], 1); USART_Print(" "); }
        USART_Print("\r\n");
    }

    // 2. OneWire_MatchROM — เลือก device ทีละตัว
    for (int d = 0; d < count; d++) {
        // เลือก device ตัวที่ d
        OneWire_Select(bus, roms[d]);

        // อ่าน temperature (DS18B20)
        OneWire_WriteByte(bus, 0x44);  // Convert T
        Delay_Ms(800);                 // รอ conversion (12-bit = 750ms)
        OneWire_Reset(bus);
        OneWire_Select(bus, roms[d]);
        OneWire_WriteByte(bus, 0xBE);  // Read Scratchpad

        uint8_t temp_l = OneWire_ReadByte(bus);
        uint8_t temp_h = OneWire_ReadByte(bus);
        int16_t raw = ((int16_t)temp_h << 8) | temp_l;

        USART_Print("Device ");
        USART_PrintNum(d);
        USART_Print(": ");
        USART_PrintNum(raw / 16);
        USART_Print(".");
        USART_PrintNum((raw & 0x0F) * 100 / 16);
        USART_Print(" C\r\n");

        Delay_Ms(200);
    }

    while (1) { Delay_Ms(5000); }
}
