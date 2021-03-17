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

bool execute_ADD(GBA_Cpu& cpu, uint32_t self);

inline bool is_ADD(uint32_t self)
{
    return (self & 0xFE00000) == 0x02800000;
}