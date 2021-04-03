#pragma once

#include <string_view>
#include <vector>
#include <array>
#include <string>
#include "GBA_Cpu.h"

enum class REPL_ArgumentType
{
    POINTER = 1,
    INTEGER = 1 << 1,
    RANGE = 1 << 2,
    OPTIONAL = 1 << 3
};

struct REPL_Argument
{
public:
    REPL_ArgumentType type;
    std::string name;
    std::string description;
public:
    static bool is_pointer(const std::string& argument);
    static bool is_integer(const std::string& argument);
    static bool is_range(const std::string& argument);
    
    static uint32_t get_integer(const std::string& argument);
    static uint32_t get_pointer(std::string argument);
    static std::pair<uint32_t, uint32_t> get_range(const std::string& argument);
};

typedef std::vector<std::string> REPL_Signature;
typedef void (GBA_Cpu::*REPL_Procedure)(const REPL_Signature&) const;

class REPL_Command
{
public:
    REPL_Command(std::string name, std::vector<REPL_Argument>&& expected_arguments, REPL_Procedure procedure);
public:
    std::string name;
    std::vector<REPL_Argument> expected_arguments;
    REPL_Procedure procedure;
};

class REPL
{
public:
    std::vector<std::string> split_tokens(const std::string& source) const;
    const REPL_Command& find_command(const REPL_Signature& signature) const;
public:
    const std::array<REPL_Command, 1> commands = {
        REPL_Command("find",
                    {
                        { REPL_ArgumentType::INTEGER, "value", "Value to be found" },
                        { REPL_ArgumentType::RANGE, "address", "Address range to be searched" }
                    },
                    &GBA_Cpu::find_command)
    };
};


