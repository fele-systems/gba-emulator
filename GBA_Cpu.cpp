#include "GBA_Cpu.h"
#include "assembly.h"
#include <cassert>
#include <bitset>
#include "repl.h"

GBA_Cpu::GBA_Cpu(GBA_Memory& memory)
    : memory(memory)
 {
    R[15] = 0x8000000; // ROM Start
    flush_pipeline();
    if (cs_open(CS_ARCH_ARM, CS_MODE_ARM, &cs_arm) != CS_ERR_OK)
        throw std::runtime_error{ "Failed to instanciate Capstone ARM engine." };
    if (cs_open(CS_ARCH_ARM, CS_MODE_THUMB, &cs_tmb) != CS_ERR_OK)
    {
        cs_close(&cs_arm);
        throw std::runtime_error{ "Failed to instanciate Capstone ARM engine." };
    }
}

GBA_Cpu::~GBA_Cpu()
{
    cs_close(&cs_arm);
    cs_close(&cs_tmb);
}

void GBA_Cpu::flush_pipeline()
{
    if (mode == ExecutionMode::ARM)
    {
        executing = memory.read_word(R[15]);
        R[15] += instruction_size;
        decoding = memory.read_word(R[15]);
        R[15] += instruction_size;
        fetching = memory.read_word(R[15]);
    }
    else
    {
        executing = memory.read_halfword(R[15]);
        R[15] += instruction_size;
        decoding = memory.read_halfword(R[15]);
        R[15] += instruction_size;
        fetching = memory.read_halfword(R[15]);
    }
}

void GBA_Cpu::fetch_next()
{
    R[15] += instruction_size;
    executing = decoding;
    decoding = fetching;
    if (mode == ExecutionMode::ARM)
    {
        fetching = memory.read_word(R[15]);
    }
    else
    {
        fetching = memory.read_halfword(R[15]);
    }
}   

GBA_Cpu::CPSR_pack::CPSR_pack(uint32_t value)
{
    std::bitset<32> set = value;
    sign_flag = set[31];
    zero_flag = set[30];
    carry_flag = set[29];
    overflow_flag = set[28];
    sticky_overflow = set[27];
    IRQ_disable = set[7];
    FIQ_disable = set[6];
    state_bit = set[5];
    mode_bits = (value & 0x1F);
}

GBA_Cpu::CPSR_pack::operator uint32_t() const
{
    std::bitset<32> set{ 0 };
    set[31] = sign_flag;
    set[30] = zero_flag;
    set[29] = carry_flag;
    set[28] = overflow_flag;
    set[27] = sticky_overflow;
    set[7] = IRQ_disable;
    set[6] = FIQ_disable;
    set[5] = state_bit;
    
    return (static_cast<uint32_t>(set.to_ulong()) | (mode_bits & 0x1F));
}

bool GBA_Cpu::cycle_arm()
{
    debug_save_registers();
    uint8_t* executing_bytes = reinterpret_cast<uint8_t*>(&executing);
    auto ins_add = PC - instruction_size * 2;
    auto debug_info = fmt::format(" ; PC={:#x}, Ins.Addr={:#x}, Opcode={:#x}, Bytes={:0>2x} {:0>2x} {:0>2x} {:0>2x}", PC, ins_add, executing,
                                  (int)executing_bytes[0],
                                  (int)executing_bytes[1],
                                  (int)executing_bytes[2],
                                  (int)executing_bytes[3]);
    cs_insn* insn;
    auto count = cs_disasm(cs_arm, executing_bytes, 4, ins_add, 0, &insn);
    if (count != 1)
    {
        return false;
    }

    fmt::print("{}\t{}", insn[0].mnemonic, insn[0].op_str);
    cs_free(insn, count);
    auto handled = false;

    
    if (is_LDR_immediate(executing)) // bits[27-26]=01 | bits[20]=1
    {
        handled = execute_LDR_immediate(*this, executing);
    }
    else if (is_STR_immediate(executing))
    {
        handled = execute_STR_immediate(*this, executing);
    }
    else if (is_B(executing)) // bits[27-25]=101
    {
        handled = execute_B(*this, executing);
    }
    else if (is_ADD(executing))
    {
        handled = execute_ADD(*this, executing);
    }
    else if (is_BX(executing))
    {
        handled = execute_BX(*this, executing);
    }
    else if (is_MOV(executing))
    {
        handled = execute_MOV(*this, executing);
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
                uint32_t second_operand = rotr32_shiftsq(_8bit_imm, position);

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
                uint32_t second_operand = rotr32_shiftsq(_8bit_imm, position);

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
            if (((executing >> 23) & 0x03) == 0x02) // PSR Transfer
            {
                uint8_t psr = (executing >> 22) & 0x01;
                assert(((executing >> 20) & 0x01) == 0); // Must be 0 for this. Otherwise TST, TEQ, CMP, CMN
                if (((executing >> 21) & 0x01) == 0) // MRS
                {
                }
                else // MSR
                {
                    //  msr cpsr_fc, r0
                    bool write_to_flags = (executing >> 19) & 0x01;
                    bool write_to_status = (executing >> 18) & 0x01;
                    bool write_to_extension = (executing >> 17) & 0x01;
                    bool write_to_control = (executing >> 16) & 0x01;
                    assert(((executing >> 12) & 0x0F) == 0x0F);

                    if (!(((executing >> 4) & 0xFF) == 0x0))
                        break;

                    assert(((executing >> 4) & 0xFF) == 0x0); // Must be 0 for this. Otherwise BX
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
    
    debug_print_register_changes();
    
    return handled;
}

bool GBA_Cpu::cycle_thumb()
{
    debug_save_registers();
    uint8_t* executing_bytes = reinterpret_cast<uint8_t*>(&executing);
    auto ins_add = PC - instruction_size * 2;
    auto debug_info = fmt::format(" ; PC={:#x}, Ins.Addr={:#x}, Opcode={:#x}, Bytes={:0>2x} {:0>2x}", PC, ins_add, static_cast<uint16_t>(executing),
        (int)executing_bytes[0],
        (int)executing_bytes[1]);
    cs_insn* insn;
    auto count = cs_disasm(cs_tmb, executing_bytes, 2, ins_add, 0, &insn);
    if (count != 1)
    {
        return false;
    }

    fmt::print("{}\t{}", insn[0].mnemonic, insn[0].op_str);
    cs_free(insn, count);

    auto handled = false;

    auto opcode = static_cast<uint16_t>(executing);
    if (is_LDR_thumb_1(opcode))
    {
        handled = execute_LDR_thumb_1(*this, opcode);
    }
    else if (is_LDR_thumb_3(opcode))
    {
        handled = execute_LDR_thumb_3(*this, opcode);
    }
    else if (is_LSLS_thumb_1(opcode))
    {
        handled = execute_LSLS_thumb_1(*this, opcode);
    }
    else if (is_B_thumb_1(opcode))
    {
        handled = execute_B_thumb_1(*this, opcode);
    }
    else if (is_B_thumb_2(opcode))
    {
        handled = execute_B_thumb_2(*this, opcode);
    }
    else if (is_MOVS_thumb_1(opcode))
    {
        handled = execute_MOVS_thumb_1(*this, opcode);
    }
    else if (is_MOVS_thumb_2(opcode))
    {
        handled = execute_MOVS_thumb_2(*this, opcode);
    }
    else if (is_MOVS_thumb_3(opcode))
    {
        handled = execute_MOVS_thumb_3(*this, opcode);
    }

    

    if (!handled)
        std::cout << "Unhandled opcode: " << debug_info << std::endl;
    else
        std::cout << debug_info << std::endl;
    
    debug_print_register_changes();
    
    return handled;
}

bool GBA_Cpu::cycle()
{
    auto instr_addr = PC - instruction_size * 2;
    if (std::find(break_points.begin(), break_points.end(), instr_addr) != break_points.end())
    {
        std::cout << "Breakpoint! @" << std::hex << instr_addr << std::endl;

        REPL repl;

        while (repl.running()) {
            repl.process_command(*this);
        }

        for (int i = 0; i < 16; i++)
        {
            std::cout << BLUE << fmt::format("r{} = {:#x}", i, R[i]) << RESET << std::endl;
        }
    }

    if (mode == ExecutionMode::ARM)
    {
        return cycle_arm();
    }
    else
    {
        return cycle_thumb();
    }
}

void GBA_Cpu::set_mode(ExecutionMode new_mode)
{
    bool has_changed = mode != new_mode;
    if (new_mode == ExecutionMode::ARM)
    {
        mode = ExecutionMode::ARM;
        instruction_size = 4;
        if (has_changed)
            std::cout << ".ARM";
    }
    else
    {
        mode = ExecutionMode::THUMB;
        instruction_size = 2;
        if (has_changed)
            std::cout << ".THUMB";
    }
}

void GBA_Cpu::debug_save_registers()
{
    std::copy_n(R, 15, R_bak);
    CPSR_bak = CPSR;
}
        
    
void GBA_Cpu::debug_print_register_changes() const
{
    for (int i = 0; i < 15; i++)
    {
        if (R[i] != R_bak[i])
        {
            std::cout << RED << fmt::format("r{} -> {:#x}", i, R[i]) << RESET << std::endl;
        }
    }
    
    CPSR_pack cpsr { CPSR };
    CPSR_pack cpsr_bak { CPSR_bak };
    
    std::cout << RED;
    if (cpsr.sign_flag != cpsr_bak.sign_flag)
        std::cout << "N=" << cpsr.sign_flag << ' ';
    if (cpsr.zero_flag != cpsr_bak.zero_flag)
        std::cout << "Z=" << cpsr.zero_flag << ' ';
    if (cpsr.carry_flag != cpsr_bak.carry_flag)
        std::cout << "C=" << cpsr.carry_flag << ' ';
    if (cpsr.overflow_flag != cpsr_bak.overflow_flag)
        std::cout << "V=" << cpsr.overflow_flag << ' ';
    if (cpsr.sticky_overflow != cpsr_bak.sticky_overflow)
        std::cout << "Q=" << cpsr.sticky_overflow << ' ';
    if (cpsr.IRQ_disable != cpsr_bak.IRQ_disable)
        std::cout << "I=" << cpsr.IRQ_disable << ' ';
    if (cpsr.FIQ_disable != cpsr_bak.FIQ_disable)
        std::cout << "F=" << cpsr.FIQ_disable << ' ';
    if (cpsr.state_bit != cpsr_bak.state_bit)
        std::cout << "T=" << cpsr.state_bit << ' ';
    if (cpsr.mode_bits != cpsr_bak.mode_bits)
        std::cout << "M=" << fmt::format("{:#x}", cpsr.mode_bits) << ' ';
    
    std::cout << RESET << std::endl;
}

bool GBA_Cpu::test_cond(uint8_t condition_bits) const
{
    CPSR_pack cpsr { CPSR };
    assert(condition_bits <= 0xF);
    switch (condition_bits)
    {
        case 0x0:   return cpsr.zero_flag;  // EQ
        case 0x1:   return !cpsr.zero_flag; // NE
        case 0x2:   return cpsr.carry_flag; // CS
        case 0x3:   return !cpsr.carry_flag;// CC
        case 0x4:   return cpsr.sign_flag;  // MI
        case 0x5:   return !cpsr.sign_flag; // PL
        case 0x6:   return cpsr.overflow_flag;  // VS
        case 0x7:   return !cpsr.overflow_flag; // VC
        case 0x8:   return cpsr.carry_flag && !cpsr.zero_flag; // HI
        case 0x9:   return !cpsr.carry_flag || cpsr.zero_flag; // LS
        case 0xA:   return cpsr.sign_flag == cpsr.overflow_flag; // GE
        case 0xB:   return cpsr.sign_flag != cpsr.overflow_flag; // LT
        case 0xC:   return !cpsr.zero_flag && cpsr.sign_flag == cpsr.overflow_flag; // GT
        case 0xD:   return cpsr.zero_flag || cpsr.sign_flag == cpsr.overflow_flag; // LE
        case 0xE:   return true;
        case 0xF:   return false;
        default:    return true;
    }
}

void GBA_Cpu::add_break_point(uint32_t instruction_address)
{
    break_points.push_back(instruction_address);
}

void GBA_Cpu::find_command(const std::vector<std::string>& tokens) const
{
    auto value = REPL_Argument::get_integer(tokens[1]);
    auto range = REPL_Argument::get_range(tokens[2]);
    
    auto address = memory.find_word(value, range.first, range.second);

    std::cout << std::hex << address << std::endl;
    
}

void GBA_Cpu::dump_command(const REPL_Signature& tokens) const
{
    auto range = REPL_Argument::get_range(tokens[1]);

    std::cout << memory.dump(4, range.first, range.second);
}
