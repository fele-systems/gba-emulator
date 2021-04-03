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

bool execute_B_thumb_1(GBA_Cpu& cpu, uint16_t self);

inline bool is_B_thumb_1(uint16_t self)
{
    return (self & 0xF000) == 0xD000;
}

bool execute_B_thumb_2(GBA_Cpu& cpu, uint16_t self);

inline bool is_B_thumb_2(uint16_t self)
{
    return (self & 0xF100) == 0xE000;
}

/**
 * @brief Execute MOVS with unshifted Immediate.
 * 
 * Syntax: MOVS Rd, #8imm8
 * 
 * Moves a 8bit immediate to register Rd.
 * 
 * Flags: NZ
 * 
 * Encoding
 * [0, 7] 8 bit immediate
 * [8, 10] Register number (R0..R7)
 * [11, 15] Must be 0b00100 for this instruction
 * 
 * @param cpu p_cpu: The cpu who's executing this instruction.
 * @param self p_self: The opcode to be executed.
 * @return bool Whether the opcode was handled.
 */
bool execute_MOVS_thumb_1(GBA_Cpu& cpu, uint16_t self);

inline bool is_MOVS_thumb_1(uint16_t self)
{
    return (self & 0xF800) == 0x2000;
}

/**
 * @brief Executes MOVS between low registers.
 * 
 * Syntax: MOVS Rd, Rs
 * 
 * Flags: NZ CV=0
 * 
 * Encoding
 * [0, 2] Destination register (R0..R7)
 * [3, 5] Source register (R0..R7)
 * [6, 9] Must be cleared for this instruction
 * [10,15] Must be 0b000111 for this instruction (opcode id)
 * 
 * @remark This is a pseudo instruction equivalent to ADDS Rd, Rs, #0x00
 * 
 * @param cpu p_cpu: The cpu who's executing this instruction.
 * @param self p_self: The opcode to be executed.
 * @return bool Whether the opcode was handled.
 */
bool execute_MOVS_thumb_2(GBA_Cpu& cpu, uint16_t self);

inline bool is_MOVS_thumb_2(uint16_t self)
{
    return (self & 0xF100) == 0xE000;
}

/**
 * @brief Executes MOVS between high registers.
 * 
 * Syntax: MOVS Rd, Rs
 * 
 * Flags: none
 * 
 * Encoding
 * [0, 2] Lower bits of Rd
 * [3, 5] Lower bits of Rs
 * [6] Higher bits of Rs
 * [7] Higher bits of Rd
 * [8, 15] 0b01000110
 * 
 * @remark Both higher bits must be set. Its only allowed in other instructions within the
 * same category.
 * 
 * @param cpu p_cpu: The cpu who's executing this instruction.
 * @param self p_self: The opcode to be executed.
 * @return bool Whether the opcode was handled.
 */
bool execute_MOVS_thumb_3(GBA_Cpu& cpu, uint16_t self);

inline bool is_MOVS_thumb_3(uint16_t self)
{
    return (self & 0xF100) == 0xE000;
}

/**
 * @brief Executes a long branch with link.
 *
 * Syntax: 
 */
bool execute_BL_thumb(GBA_Cpu& cpu, uint16_t self);