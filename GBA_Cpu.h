#pragma once

#include "GBA_Memory.h"
#include "opcodes.h"
#include <fmt/core.h>
#include <iostream>
#include "bit_utils.h"
#include <capstone/capstone.h>

//the following are UBUNTU/LINUX, and MacOS ONLY terminal color codes.
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */


class GBA_Cpu
{
public:
    enum class ExecutionMode { ARM, THUMB };

    GBA_Cpu(GBA_Memory& memory);
    ~GBA_Cpu();
    GBA_Cpu(const GBA_Cpu&) = delete;
    GBA_Cpu& operator=(const GBA_Cpu&) = delete;
    GBA_Cpu(GBA_Cpu&&) = delete;
    GBA_Cpu& operator=(GBA_Cpu&&) = delete;

    /**
     * @brief Flushes the execution pipeline.
     * 
     * Whenever a branch instruction occurs, the pipeline must be flushed.
     * Flushing the pipeline basically means skipping 2 instructions.
     * This will make PC point to 2 words after the executing instruction.
     * 
     */
    void flush_pipeline();
    
    /**
     * @brief Step the execution pipeline.
     * 
     * This will make PC point to the next instruction. And the already fetched opcodes
     * follow the following pipeline: word@PC -> fetching -> decoding -> executing.
     */
    void fetch_next();
    
    
    /**
     * @brief Cycles one instruction.
     * 
     * Execute the current instruction. This can potentially move the PC more than 1 word,
     * depending on the instruction type.
     * 
     * @return bool Returns false is the opcode wasn't processed.
     */
    bool cycle();
    

    
    void debug_save_registers();
    
    void debug_print_register_changes() const;
    
    struct CPSR_pack
    {
        uint8_t mode_bits;
        bool state_bit;
        bool FIQ_disable;
        bool IRQ_disable;
        bool sticky_overflow; // IDk what this is about
        bool overflow_flag;
        bool carry_flag;
        bool zero_flag;
        bool sign_flag;
        
        explicit CPSR_pack(uint32_t value);
        explicit operator uint32_t() const;
    };
    
    bool test_cond(uint8_t condition_bits) const;

    void add_break_point(uint32_t instruction_address);
    void find_command(const std::vector<std::string>& tokens) const;
    void dump_command(const std::vector<std::string>& tokens) const;
    void dissa_command(const std::vector<std::string>& tokens) const;
    void disst_command(const std::vector<std::string>& tokens) const;

    void set_mode(ExecutionMode new_mode);

private:
    bool cycle_arm();
    bool cycle_thumb();
public:
    uint32_t executing = 0x69696969;
    uint32_t decoding = 0x69696969;
    uint32_t fetching = 0x69696969;
    
    int instruction_size = 4;
    GBA_Memory& memory;
    uint32_t R[16] = { 0 };
    uint32_t CPSR = 0;
    uint32_t& PC = R[15];
    uint32_t& LR = R[14];
    uint32_t& SP = R[13];
    ExecutionMode mode = ExecutionMode::ARM;
    
    uint32_t R_bak[16];
    uint32_t CPSR_bak;
    std::vector<uint32_t> break_points;

    csh cs_arm;
    csh cs_tmb;
};
