#include "assembly.h"
#include "GBA_Cpu.h"
#include <fmt/core.h>

std::string disassemble_register_name(uint8_t register_number)
{
    if (register_number == 15)
    {
        return "PC";
    }
    else if (register_number == 14)
    {
        return "LR";
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
        case 0x2:   return "CS"; // /HS
        case 0x3:   return "CC"; // /LO
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


std::string disassemble_LDR_STR_immediate(uint32_t self)
{
    uint8_t condition = (self >> 28);
    bool _I = (self >> 25) & 1; // 1=Register 0=Imm
    bool _P = (self >> 24) & 1; // 1=Pre-indexed 0=Post-indexed
    bool _U = (self >> 23) & 1; // 1=Add offset 0=Substract offset
    bool _B = (self >> 22) & 1; // 1=u8 0=i32
    bool _W = (self >> 21) & 1; // P=1 1=Write back P=0 1=User mode access
    bool _L = (self >> 20) & 1; // 1=Load 0=Store
    uint8_t _Rn = (self >> 16) & 0xF; // Base register
    uint8_t _Rd = (self >> 12) & 0xF; // Destination register
    int offset = self & 0xFFF;
            
    if (!_I) // Register
    {
        if (_P)
        {
            return fmt::format("{}{}{} {}, [{} {}#{:#x}]{}",
                _L ? "LDR" : "STR",
                disasemble_condition(condition),
                _B ? "B" : "",
                disassemble_register_name(_Rd),
                disassemble_register_name(_Rn),
                _U ? "" : "-",
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

std::string disassemble_ADD(uint32_t self)
{
    uint8_t condition = (self >> 28);
    
    bool _I = (self >> 25) & 1; // 2nd operand is 1=Immediate 0=Register
    bool _S = (self >> 20) & 1; // 0=ADD 1=ADDS
    uint8_t dest = (self >> 12) & 0x0F;
    uint8_t op_1 = (self >> 16) & 0x0F;
    
    if (!_S && _I) {
        uint8_t shift = (self >> 8) & 0x0F;
        uint8_t immediate = self & 0xFF;
        
        auto op_2 = rotr32_shiftsq(immediate, shift);
        return fmt::format("ADD{}{} {}, {}, #{:#x}",
            _S ? "S" : "",
            disasemble_condition(condition),
            disassemble_register_name(dest),
            disassemble_register_name(op_1),
            op_2);
    }
    return "opcode?";
}

std::string disassemble_BX(const GBA_Cpu& cpu, uint32_t self)
{
    uint8_t condition = (self >> 28);
    uint8_t _B = (self >> 4) & 0x0F;
    uint8_t _Rn = self & 0x0F;
    
    if (_B == 0b0001)
    {
        return fmt::format("BX{} {} // ${:#x}",
                           disasemble_condition(condition), 
                           disassemble_register_name(_Rn), 
                           cpu.R[_Rn]);
    }
    else if (_B == 0b0010)
    {
        return "BXJ?";
    }
    else if (_B == 0b0011)
    {
        return fmt::format("BLX{} {} // ${:#x}",
                           disasemble_condition(condition), 
                           disassemble_register_name(_Rn), 
                           cpu.R[_Rn]);
    }
    
    return "BX?";
}

std::string disassemble_MOV(uint32_t self)
{
    uint8_t condition = self >> 28;
    bool _I = (self >> 25) & 1;
    bool _S = (self >> 20) & 1;
    uint8_t _Rd = (self >> 12) & 0x0F;
    uint8_t shift = (self >> 8) & 0x0F;
    uint8_t immediate = self & 0xFF;
        
    if (_I)
    {
        uint32_t op_2 = rotr32_shiftsq(immediate, shift);
        return fmt::format("MOV{}{} {} #{:#x}",
                           _S ? "S" : "",
                           disasemble_condition(condition),
                           disassemble_register_name(_Rd),
                           op_2);
    }
    
    return "MOV?";
}

std::string disassemble_LDR_thumb_3(uint16_t self)
{
    uint8_t _V = self & 0xFF;
    uint8_t _Rd = (self >> 8) & 0x07;

    return fmt::format("LDR {}, [PC, #{:#x}]",
                       disassemble_register_name(_Rd),
                       _V * 4);
   
}

std::string disassemble_LSLS_thumb_1(uint16_t self)
{
    uint8_t _V = (self >> 6) & 0x1F;
    uint8_t _Rn = (self >> 3) & 0x07;
    uint8_t _Rd = self & 0x07;

    return fmt::format("LSLS {}, {}, #{:#x}",
                       disassemble_register_name(_Rd),
                       disassemble_register_name(_Rn),
                       _V);
}

std::string disassemble_LDR_thumb_1(uint16_t self)
{
    uint8_t _V = (self >> 6) & 0x1F;
    uint8_t _Rs = (self >> 3) & 0x07;
    uint8_t _Rd = self & 0x07;

    return fmt::format("LDR {}, [{}, #{:#x}]",
                       disassemble_register_name(_Rd),
                       disassemble_register_name(_Rs),
                       _V * 4);
}

std::string disassemble_B_thumb_1(const GBA_Cpu& cpu, uint16_t self)
{
    uint8_t condition = (self >> 8) & 0x0F;
    uint8_t target = self & 0xFF;
    
    return fmt::format("B{} #{:#x} // ${:#x}",
                       disasemble_condition(condition),
                       target,
                       2 * (cpu.PC + target) + cpu.instruction_size * 2);
}

std::string disassemble_B_thumb_2(const GBA_Cpu& cpu, uint16_t self)
{
    uint16_t target = self & 0x7FF;
    
    return fmt::format("B #{:#x} // ${:#x}",
                       target,
                       2 * (cpu.PC + target) + cpu.instruction_size * 2);
}

std::string disassemble_MOVS_thumb_1(uint16_t self)
{
    uint8_t Rd = (self >> 10) & 0x07;
    uint8_t value = (self & 0xFF);

    return fmt::format("MOVS {}, #{:#x}",
                       disassemble_register_name(Rd),
                       value);
}

std::string disassemble_MOVS_thumb_2(uint16_t self)
{
    uint8_t Rs = (self >> 3) & 0x07;
    uint8_t Rd = self & 0x07;

    return fmt::format("MOVS {}, {}",
                       disassemble_register_name(Rd),
                       disassemble_register_name(Rs));
}

std::string disassemble_MOVS_thumb_3(uint16_t self)
{
    uint8_t Rs = (self & 0x80) | ((self >> 3) & 0x07);
    uint8_t Rd = (self & 0x40) | (self & 0x07);

    return fmt::format("MOVS {}, {}",
                       disassemble_register_name(Rd),
                       disassemble_register_name(Rs));
}
