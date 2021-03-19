#pragma once

#include <cstdint>

class GBA_Cpu;

/**
 * @brief LDR Immediate offset/index
 * 
 * https://heyrick.eu/aw/index.php?title=LDR
 * 
 * @param cpu The cpu who's executing this instruction.
 * @param self The opcode to be executed.
 * @return True if the opcode was handled
 */
bool execute_LDR_immediate(GBA_Cpu& cpu, uint32_t self);


inline bool is_LDR_immediate(uint32_t self)
{
    return (self & 0xc100000) == 0x4100000;
}

/**
 * @brief B / BL : Branch / Branch with Link
 * 
 * https://heyrick.eu/armwiki/B
 * 
 * @param cpu The cpu who's executing this instruction.
 * @param self The opcode to be executed.
 * @return bool if the opcode was handled
 */
bool execute_B(GBA_Cpu& cpu, uint32_t self);

inline bool is_B(uint32_t self)
{
    return (self & 0x0E000000) == 0x0A000000;
}

bool execute_BX(GBA_Cpu& cpu, uint32_t self);

inline bool is_BX(uint32_t self)
{
    return ((self >> 8) & 0x0FFFFF) == 0b00010010111111111111;
}

bool execute_ADD(GBA_Cpu& cpu, uint32_t self);

inline bool is_ADD(uint32_t self)
{
    return (self & 0xFE00000) == 0x02800000;
}

bool execute_STR_immediate(GBA_Cpu& cpu, uint32_t self);

inline bool is_STR_immediate(uint32_t self)
{
    return (self & 0xc100000) == 0x4000000;
}

bool execute_MOV(GBA_Cpu& cpu, uint32_t self);

inline bool is_MOV(uint32_t self)
{
    return (self & 0xDEF0000) == 0x1A00000;
}

bool execute_LDR_thumb_1(GBA_Cpu& cpu, uint16_t self);

inline bool is_LDR_thumb_1(uint16_t self)
{
    return (self & 0xF800) == 0x6800;
}

bool execute_LDR_thumb_3(GBA_Cpu& cpu, uint16_t self);

inline bool is_LDR_thumb_3(uint16_t self)
{
    return (self & 0xF800) == 0x4800;
}

bool execute_LSLS_thumb_1(GBA_Cpu& cpu, uint16_t self);

inline bool is_LSLS_thumb_1(uint16_t self)
{
    return (self & 0xF800) == 0x0000;
}