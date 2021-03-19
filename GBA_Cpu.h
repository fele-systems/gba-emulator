#pragma once

#include "GBA_Memory.h"
#include "opcodes.h"
#include <fmt/core.h>
#include <iostream>
#include "bit_utils.h"

class GBA_Cpu
{
public:
    GBA_Cpu(GBA_Memory& memory);
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
    uint32_t& SP = R[13];
};
