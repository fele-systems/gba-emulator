#pragma once

#include <cstdint>
#include <ostream>

class GBA_Cpu;

std::string disassemble_register_name(uint8_t register_number);

const char* disasemble_condition(uint8_t cond_bits);

std::string disassemble_LDR_immediate(uint32_t self);

std::string disassemble_B(const GBA_Cpu& cpu, uint32_t self);