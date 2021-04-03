#include "opcodes.h"
#include "GBA_Cpu.h"
#include "assembly.h"
#include "bit_utils.h"
#include <iostream>
#include <cassert>

bool execute_LDR_immediate(GBA_Cpu& cpu, uint32_t self)
{
    assert(is_LDR_immediate(self));
    uint8_t condition = (self >> 28);
    if (!cpu.test_cond(condition))
        return true;
    bool _I = (self >> 25) & 1; // 1=Register 0=Imm
    //bool _P = (self >> 24) & 1; // 1=Pre-indexed 0=Post-indexed
    bool _U = (self >> 23) & 1; // 1=Add offset 0=Substract offset
    bool _B = (self >> 22) & 1; // 1=u8 0=i32
    //bool _W = (self >> 21) & 1; // P=1 1=Write back P=0 1=User mode access
            
    uint8_t _Rn = (self >> 16) & 0xF; // Base register
    uint8_t _Rd = (self >> 12) & 0xF; // Destination register
    int offset = self & 0xFFF;
            
    if (!_I) {
        //std::cout << disassemble_LDR_STR_immediate(self);
                
        if (!_U)
            offset = -offset;
        
        if (!_B) {
            cpu.R[_Rd] = cpu.memory.read_word(cpu.R[_Rn] + offset);
            cpu.fetch_next();
            return true;
        }

    }
    
    return false;
}


bool execute_B(GBA_Cpu& cpu, uint32_t self)
{
    uint8_t condition = (self >> 28);
    if (!cpu.test_cond(condition))
        return true;
    // std::cout << disassemble_B(cpu, self);
    if (condition == 0x0F) // BLX
    {
        // uint32_t _25bit_offset = executing & 0x01FFFFFF;
        cpu.R[14] = cpu.R[15] + cpu.instruction_size; // Save the address
        // Change to thumb mode i guess
        return false;
    }
    else
    {
        bool _L = (self >> 24) & 1; // 1=BL 0=B
        
        uint32_t _24bit_offset = self & 0x00FFFFFF;
        if (_L) // BL
        {
            cpu.R[14] = cpu.R[15] + cpu.instruction_size; // Save the address
        }

        // The actual jump
        cpu.R[15] += sign_extend_24_32(_24bit_offset) * 4;
        cpu.flush_pipeline();
        
    }
    return true;
}

bool execute_BX(GBA_Cpu& cpu, uint32_t self)
{
    assert(is_BX(self));
    //std::cout << disassemble_BX(cpu, self);
    uint8_t condition = (self >> 28);
    if (!cpu.test_cond(condition))
        return true;
    uint8_t _B = (self >> 4) & 0x0F;
    uint8_t _Rn = self & 0x0F;
    bool _T = cpu.R[_Rn] & 1; // 1=Thumb 0=Arm
    
    if (_B == 0b0001)
    {
        if (_T)
        {
            cpu.PC = cpu.R[_Rn] - 1;
            cpu.set_mode(GBA_Cpu::ExecutionMode::THUMB);
            cpu.flush_pipeline();
            return true;
        }
        else
        {
            cpu.PC = cpu.R[_Rn];
            cpu.flush_pipeline();
            return true;
        }
        
    }
    else if (_B == 0b0010)
    {
        return false;
    }
    else if (_B == 0b0011)
    {
        if (_T)
        {
            cpu.SP = cpu.PC + cpu.instruction_size;
            cpu.PC = cpu.R[_Rn] - 1;
            cpu.set_mode(GBA_Cpu::ExecutionMode::THUMB);
            cpu.flush_pipeline();
            return true;
        }
        else
        {
            cpu.SP = cpu.PC + cpu.instruction_size;
            cpu.PC = cpu.R[_Rn];
            cpu.flush_pipeline();
            return true;
        }
        
    }
    
    return false;
}

bool execute_ADD(GBA_Cpu& cpu, uint32_t self)
{
    assert(is_ADD(self));
    uint8_t condition = (self >> 28);
    if (!cpu.test_cond(condition))
        return true;
    //std::cout << disassemble_ADD(self);
    bool _I = (self >> 25) & 1; // 2nd operand is 1=Immediate 0=Register
    bool _S = (self >> 20) & 1; // 0=ADD 1=ADDS
    uint8_t dest = (self >> 12) & 0x0F;
    uint8_t op_1 = (self >> 16) & 0x0F;
    
    if (!_S && _I) {
        uint8_t shift = (self >> 8) & 0x0F;
        uint8_t immediate = self & 0xFF;
        
        auto op_2 = rotr32_shiftsq(immediate, shift);
        cpu.R[dest] = cpu.R[op_1] + op_2;
        cpu.fetch_next();
        return true;
    }
    return false;
}

bool execute_STR_immediate(GBA_Cpu& cpu, uint32_t self)
{
    assert(is_STR_immediate(self));
    uint8_t condition = (self >> 28);
    if (!cpu.test_cond(condition))
        return true;
    bool _I = (self >> 25) & 1; // 1=Register 0=Imm
    //bool _P = (self >> 24) & 1; // 1=Pre-indexed 0=Post-indexed
    bool _U = (self >> 23) & 1; // 1=Add offset 0=Substract offset
    bool _B = (self >> 22) & 1; // 1=u8 0=i32
    //bool _W = (self >> 21) & 1; // P=1 1=Write back P=0 1=User mode access
            
    uint8_t _Rn = (self >> 16) & 0xF; // Base register
    uint8_t _Rd = (self >> 12) & 0xF; // Destination register
    int offset = self & 0xFFF;
            
    if (!_I) {
        //std::cout << disassemble_LDR_STR_immediate(self);
                
        if (!_U)
            offset = -offset;
        
        if (!_B) {
            cpu.memory.write_word(cpu.R[_Rn] + offset, cpu.R[_Rd]);
            cpu.fetch_next();
            return true;
        }

    }
    
    return false;
}

bool execute_MOV(GBA_Cpu& cpu, uint32_t self)
{
    assert(is_MOV(self));
    uint8_t condition = self >> 28;
    if (!cpu.test_cond(condition))
        return true;
    //std::cout << disassemble_MOV(self);
    bool _I = (self >> 25) & 1;
    bool _S = (self >> 20) & 1;
    uint8_t _Rd = (self >> 12) & 0x0F;
    uint8_t shift = (self >> 8) & 0x0F;
    uint8_t immediate = self & 0xFF;
        
    if (_I && !_S)
    {
        uint32_t op_2 = rotr32_shiftsq(immediate, shift);
        cpu.R[_Rd] = op_2;
        cpu.fetch_next();
        return true;
    }
    
    return false;
}

bool execute_LDR_thumb_1(GBA_Cpu& cpu, uint16_t self)
{
    assert(is_LDR_thumb_1(self));
    //std::cout << disassemble_LDR_thumb_1(self);
    uint8_t _V = (self >> 6) & 0x1F;
    uint8_t _Rs = (self >> 3) & 0x07;
    uint8_t _Rd = self & 0x07;

    cpu.R[_Rd] = cpu.R[_Rs] + (_V * 4);
    cpu.fetch_next();
    return true;
}

bool execute_LDR_thumb_3(GBA_Cpu& cpu, uint16_t self)
{
    assert(is_LDR_thumb_3(self));
    //std::cout << disassemble_LDR_thumb_3(self);
    uint8_t _V = self & 0xFF;
    uint8_t _Rd = (self >> 8) & 0x07;

    cpu.R[_Rd] = cpu.PC + (_V * 4);
    cpu.fetch_next();
    return true;
}

bool execute_LSLS_thumb_1(GBA_Cpu& cpu, uint16_t self)
{
    assert(is_LSLS_thumb_1(self));
    //std::cout << disassemble_LSLS_thumb_1(self);
    uint8_t _V = (self >> 6) & 0x1F;
    uint8_t _Rn = (self >> 3) & 0x07;
    uint8_t _Rd = self & 0x07;

    uint32_t value = cpu.R[_Rn] << _V;
    
    GBA_Cpu::CPSR_pack cpsr { cpu.CPSR };
    
    cpsr.zero_flag = value == 0;
    cpsr.sign_flag = (value >> 31) & 1;
    cpsr.carry_flag = value < cpu.R[_Rn];
    
    cpu.R[_Rd] = value;
    cpu.CPSR = static_cast<uint32_t>(cpsr);
    cpu.fetch_next();
    return true;
}

bool execute_B_thumb_1(GBA_Cpu& cpu, uint16_t self)
{
    assert(is_B_thumb_1(self));
    uint8_t condition = (self >> 8) & 0x0F;
    if (!cpu.test_cond(condition))
        return true;
    //std::cout << disassemble_B_thumb_1(cpu, self);
    uint8_t target = self & 0xFF;
    
    cpu.PC += target * 2;
    cpu.flush_pipeline();
    return true;
}

bool execute_B_thumb_2(GBA_Cpu& cpu, uint16_t self)
{
    assert(is_B_thumb_2(self));
    //std::cout << disassemble_B_thumb_2(cpu, self);
    uint16_t target = self & 0x7FF;
    
    cpu.PC += target * 2;
    cpu.flush_pipeline();
    return true;
}

bool execute_MOVS_thumb_1(GBA_Cpu& cpu, uint16_t self)
{
    assert(is_MOVS_thumb_1(self));
    //std::cout << disassemble_MOVS_thumb_1(self);
    
    uint8_t Rd = (self >> 10) & 0x07;
    uint8_t value = (self & 0xFF);
    
    GBA_Cpu::CPSR_pack cpsr{ cpu.CPSR };

    cpsr.sign_flag = static_cast<int8_t>(value) < 0;
    cpsr.zero_flag = value == 0;

    cpu.CPSR = static_cast<uint32_t>(cpsr);

    cpu.R[Rd] = value;
    cpu.fetch_next();
    return true;
}

bool execute_MOVS_thumb_2(GBA_Cpu& cpu, uint16_t self)
{
    assert(is_MOVS_thumb_2(self));
    //std::cout << disassemble_MOVS_thumb_2(self);

    uint8_t Rs = (self >> 3) & 0x07;
    uint8_t Rd = self & 0x07;

    cpu.R[Rd] = cpu.R[Rs];

    GBA_Cpu::CPSR_pack cpsr{ cpu.CPSR };

    cpsr.sign_flag = static_cast<int32_t>(cpu.R[Rd]) < 0;
    cpsr.zero_flag = cpu.R[Rd] == 0;
    cpsr.carry_flag = false;
    cpsr.overflow_flag = false;

    cpu.CPSR = static_cast<uint32_t>(cpsr);
    cpu.fetch_next();
    return true;
}

bool execute_MOVS_thumb_3(GBA_Cpu& cpu, uint16_t self)
{
    assert(is_MOVS_thumb_3(self));
    //std::cout << disassemble_MOVS_thumb_3(self);

    uint8_t Rs = (self & 0x80) | ((self >> 3) & 0x07);
    uint8_t Rd = (self & 0x40) | (self & 0x07);

    cpu.R[Rd] = cpu.R[Rs];

    return true;
}
