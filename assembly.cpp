#include "assembly.h"
#include "GBA_Cpu.h"
#include <fmt/core.h>

std::string disassemble_register_name(uint8_t register_number)
{
    if (register_number == 15)
    {
        return "PC";
    }
    else if (register_number == 13)
    {
        return "SP";
    }
    else
    {
        return 'R' + std::to_string(register_number);
    }
}

const char* disasemble_condition(uint8_t cond_bits)
{
    switch (cond_bits)
    {
        case 0x0:   return "EQ";
        case 0x1:   return "NE";
        case 0x2:   return "CS/HS";
        case 0x3:   return "CC/LO";
        case 0x4:   return "MI";
        case 0x5:   return "PL";
        case 0x6:   return "VS";
        case 0x7:   return "VC";
        case 0x8:   return "HI";
        case 0x9:   return "LS";
        case 0xA:   return "GE";
        case 0xB:   return "LT";
        case 0xC:   return "GT";
        case 0xD:   return "LE";
        case 0xE:   return "";
        case 0xF:   return "NV";
        default:    return "condition?";
        }
}


std::string disassemble_LDR_immediate(uint32_t self)
{
    uint8_t condition = (self >> 28);
    bool _I = (self >> 25) & 1; // 1=Register 0=Imm
    bool _P = (self >> 24) & 1; // 1=Pre-indexed 0=Post-indexed
    bool _U = (self >> 23) & 1; // 1=Add offset 0=Substract offset
    bool _B = (self >> 22) & 1; // 1=u8 0=i32
    bool _W = (self >> 21) & 1; // P=1 1=Write back P=0 1=User mode access
            
    uint8_t _Rn = (self >> 16) & 0xF; // Base register
    uint8_t _Rd = (self >> 12) & 0xF; // Destination register
    int offset = self & 0xFFF;
            
    if (!_I) // Register
    {
        if (_P)
        {
            return fmt::format("LDR{}{} {}, [{} {}#{:#x}]{}",
                disasemble_condition(condition),
                _B ? "B" : "",
                disassemble_register_name(_Rd),
                disassemble_register_name(_Rn),
                _U ? "+" : "-",
                offset,
                _W ? "!" : "");
        }
    }
    
    return "opcode?";
}

std::string disassemble_B(const GBA_Cpu& cpu, uint32_t self)
{
    uint8_t condition = (self >> 28);
    if (condition == 0x0F) // BLX
    {
        // uint32_t _25bit_offset = executing & 0x01FFFFFF;
        // cpu.R[14] = cpu.R[15] + cpu.instruction_size; // Save the address
        // Change to thumb mode i guess
        return "BLX?";
    }
    else
    {
        bool _L = (self >> 24) & 1; // 1=BL 0=B

        int32_t _24bit_offset = self & 0x00FFFFFF;
        int32_t decoded_offset = _24bit_offset * 4 + 8;
        uint32_t target_address = cpu.PC + decoded_offset;
        if (_L) // BL
        {    
            return fmt::format("BL{} #{:#x} // ${:#x}", disasemble_condition(condition), decoded_offset, target_address);
        }
        else
        {
            return fmt::format("B{} #{:#x} // ${:#x}", disasemble_condition(condition), decoded_offset, target_address);
        }
    }
}
