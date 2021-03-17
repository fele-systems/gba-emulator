#include "GBA_Cpu.h"
#include "assembly.h"
#include <cassert>

GBA_Cpu::GBA_Cpu(GBA_Memory& memory)
    : memory(memory)
 {
    R[15] = 0x8000000; // ROM Start
    flush_pipeline();
}

void GBA_Cpu::flush_pipeline()
{
    // Prepare the pipeline...
    executing = memory.read_word(R[15]);
    R[15] += instruction_size;
    decoding = memory.read_word(R[15]);
    R[15] += instruction_size;
    fetching = memory.read_word(R[15]);
}

void GBA_Cpu::fetch_next()
{
    R[15] += 4;
    executing = decoding;
    decoding = fetching;
    fetching = memory.read_word(R[15]);
}   

bool GBA_Cpu::cycle()
{
    uint8_t* executing_bytes = reinterpret_cast<uint8_t*>(&executing);
    auto debug_info = fmt::format(" ; PC={:#x}, Opcode={:#x}, Bytes={:0>2x} {:0>2x} {:0>2x} {:0>2x}", R[15], executing,
                                  (int)executing_bytes[0],
                                  (int)executing_bytes[1],
                                  (int)executing_bytes[2],
                                  (int)executing_bytes[3]);
    auto handled = false;

    
    if (is_LDR_immediate(executing)) // bits[27-26]=01 | bits[20]=1
    {
        handled = execute_LDR_immediate(*this, executing);
    }
    else if (is_B(executing)) // bits[27-25]=101
    {
        handled = execute_B(*this, executing);
    }
    
        
    if (!handled) // Skip this switch if already handled
    switch ((executing >> 24) & 0b00001110) // Opcode mask
    {
    case 0b00000010: // ALU_imm
    {
        uint8_t condition = (executing >> 28);
        switch ((executing >> 21) & 0x0F)
        {
            case 0X05: // ADC
            {
                int destination_reg = (executing >> 12) & 0x0F;
                int operand_reg = (executing >> 16) & 0x0F;
                bool set_condition = (executing >> 20) & 1;
                uint8_t position = (executing >> 8) & 0x0F;
                int8_t _8bit_imm = executing & 0xFF;
                uint32_t second_operand = rotr32(_8bit_imm, position);

                fmt::print("ADC{}{} R{}, R{}, {}",
                            disasemble_condition(condition),
                            set_condition ? "S" : "",
                            destination_reg,
                            operand_reg,
                            second_operand);
                fetch_next();
                handled = true;
                break;
            }
            case 0x0D: // MOV
            {
                int destination_reg = (executing >> 12) & 0x0F;
                int operand_reg = (executing >> 16) & 0x0F;
                assert(operand_reg == 0b0000);
                bool set_condition = (executing >> 20) & 1;
                uint8_t position = (executing >> 8) & 0x0F;
                int8_t _8bit_imm = executing & 0xFF;
                uint32_t second_operand = rotr32(_8bit_imm, position);

                fmt::print("MOV{}{} R{}, #{}",
                            disasemble_condition(condition),
                            set_condition ? "S" : "",
                            destination_reg,
                            second_operand);
                fetch_next();
                handled = true;
                break;
            }
        }
        break;
    }
    case 0b00000000: // MSR Transfers
    /*
     * MSR are instructions used for moving values (bit[25]=1) or register values (bit[25]=0)
     * to the specified CPSR registers. Which flags will be written to is marked by bit[16..20], where:
     *      bit[16]=Control
     *      bit[17]=eXtension
     *      bit[18]=Status
     *      bit[19]=Flags
     */
    {
        uint8_t condition = (executing >> 28);
        if ( ((executing >> 23) & 0x03) == 0x02 ) // PSR Transfer
        {
            uint8_t psr = (executing >> 22) & 0x01;
            assert( ((executing >> 20) & 0x01) == 0 ); // Must be 0 for this. Otherwise TST, TEQ, CMP, CMN
            if ( ((executing >> 21) & 0x01) == 0) // MRS
            {
            }
            else // MSR
            {
                //  msr cpsr_fc, r0
                bool write_to_flags     = (executing >> 19) & 0x01;
                bool write_to_status    = (executing >> 18) & 0x01;
                bool write_to_extension = (executing >> 17) & 0x01;
                bool write_to_control   = (executing >> 16) & 0x01;
                assert( ((executing >> 12) & 0x0F) == 0x0F);
                
                assert( ((executing >> 4) & 0xFF) == 0x0); // Must be 0 for this. Otherwise BX
                int src_register = executing & 0x0F;
                using namespace std::string_literals;
                std::string disassembled = "MSR"s + disasemble_condition(condition) + " ";
                if (psr == 0) // CPSR
                {
                    disassembled += "CPSR_";
                    if (write_to_flags) disassembled += 'F';
                    if (write_to_status) disassembled += 'S';
                    if (write_to_extension) disassembled += 'E';
                    if (write_to_control) disassembled += 'C';
                    
                    disassembled += fmt::format(" R{}", src_register);
                    std::cout << disassembled;
                    fetch_next();
                    handled = true;
                    break;
                }
            }
        }
        break;
    }
    }
    if (!handled) 
        std::cout << "Unhandled opcode: " << debug_info << std::endl;
    else 
        std::cout << debug_info << std::endl;
    return handled;
}
