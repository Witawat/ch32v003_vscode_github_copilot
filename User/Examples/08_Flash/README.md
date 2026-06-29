# 08_Flash — ตัวอย่าง Flash Storage

## ไฟล์ในโฟลเดอร์

| ไฟล์ | คำอธิบาย |
|------|----------|
| `ex01_Read_Write_Basics.c` | Flash Read/Write/Erase (Byte/HalfWord/Word) |
| `ex02_Config_With_CRC.c` | Config + CRC (Flash_SaveConfig, Flash_LoadConfig) |
| `ex03_String_And_Struct.c` | จัดเก็บ String และ Struct |
| `ex04_AutoErase_Operations.c` | Auto-Erase (Flash_WriteByteWithErase) |
| `ex05_Factory_Reset.c` | Factory Reset (Flash_EraseAll, กดปุ่ม 3 วิ) |

## Flash Storage Area

- ใช้ 2 หน้าสุดท้ายของ Flash (หน้า 254-255)
- Address: `0x0803F80` - `0x0803FFF` (128 bytes)
- ตํองแน่ใจว่าโปรแกรมไม่ใช้พื้นที่นี้

## Quick Start

```c
Flash_Init();

// Simple read/write
Flash_ErasePage(FLASH_CONFIG_PAGE);
Flash_WriteHalfWord(FLASH_CONFIG_ADDR, 0x1234);
uint16_t value = Flash_ReadHalfWord(FLASH_CONFIG_ADDR);
```
