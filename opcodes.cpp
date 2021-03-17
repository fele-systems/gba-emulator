#include "opcodes.h"
#include "GBA_Cpu.h"
#include "assembly.h"
#include "bit_utils.h"
#include <iostream>

bool execute_LDR_immediate(GBA_Cpu& cpu, uint32_t self)
{
    // uint8_t condition = (self >> 28);
    bool _I = (self >> 25) & 1; // 1=Register 0=Imm
    //bool _P = (self >> 24) & 1; // 1=Pre-indexed 0=Post-indexed
    bool _U = (self >> 23) & 1; // 1=Add offset 0=Substract offset
    bool _B = (self >> 22) & 1; // 1=u8 0=i32
    //bool _W = (self >> 21) & 1; // P=1 1=Write back P=0 1=User mode access
            
    uint8_t _Rn = (self >> 16) & 0xF; // Base register
    uint8_t _Rd = (self >> 12) & 0xF; // Destination register
    int offset = self & 0xFFF;
            
    if (!_I) {
        std::cout << disassemble_LDR_immediate(self);
                
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
    std::cout << disassemble_B(cpu, self);
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

bool execute_ADD(GBA_Cpu& cpu, uint32_t self)
{
    return false;
}

