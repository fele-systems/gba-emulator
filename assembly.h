#pragma once

#include <cstdint>
#include <ostream>

class GBA_Cpu;

std::string disassemble_register_name(uint8_t register_number);

const char* disasemble_condition(uint8_t cond_bits);

std::string disassemble_LDR_STR_immediate(uint32_t self);

std::string disassemble_ADD(uint32_t self);

std::string disassemble_B(const GBA_Cpu& cpu, uint32_t self);

std::string disassemble_BX(const GBA_Cpu& cpu, uint32_t self);

std::string disassemble_MOV(uint32_t self);

std::string disassemble_LDR_thumb_1(uint16_t self);

std::string disassemble_LDR_thumb_3(uint16_t self);

std::string disassemble_LSLS_thumb_1(uint16_t self);

std::string disassemble_B_thumb_1(const GBA_Cpu& cpu, uint16_t self);

std::string disassemble_B_thumb_2(const GBA_Cpu& cpu, uint16_t self);

std::string disassemble_MOVS_thumb_1(uint16_t self);

std::string disassemble_MOVS_thumb_2(uint16_t self);

std::string disassemble_MOVS_thumb_3(uint16_t self);