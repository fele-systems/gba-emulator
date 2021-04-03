#include "repl.h"
#include "repl.h"
#include "repl.h"
#include <algorithm>
#include <stdexcept>
#include <cctype>
#include <charconv>
#include "string_utils.h"
#include <fmt/core.h>

REPL_Command::REPL_Command(std::string name, std::vector<REPL_Argument>&& expected_arguments, REPL_Procedure procedure)
    :
    name(name),
    expected_arguments(std::move(expected_arguments)),
    procedure(procedure)
{
}

std::vector<std::string> REPL::split_tokens(const std::string& source) const
{
    size_t i = source.find(' ');
    size_t j = 0;
    std::vector<std::string> tokens;
    
    while (i != std::string::npos)
    {
        tokens.emplace_back(source.substr(j, i - j));
        j = i + 1;
        i = source.find(' ', j);
    }
    
    tokens.emplace_back(source.substr(j));
    return tokens;
}

const REPL_Command& REPL::find_command(const REPL_Signature& signature) const
{
    auto command = std::find_if(commands.begin(), commands.end(), [&](auto& command) { return command.name == signature[0]; });
    if (command == commands.end())
    {
        throw std::runtime_error{"Could not find requested command"};
    }
    
    if (!std::equal(command->expected_arguments.begin(), command->expected_arguments.end(),
                    signature.begin() + 1,
                     [&](auto& expected_argument, auto& signature_argument) -> bool
                     {
                         switch (expected_argument.type)
                         {
                            case REPL_ArgumentType::INTEGER:
                                return REPL_Argument::is_integer(signature_argument);
                            case REPL_ArgumentType::RANGE:
                                 return REPL_Argument::is_range(signature_argument);
                            case REPL_ArgumentType::POINTER:
                                 return REPL_Argument::is_pointer(signature_argument);
                            default:
                                return false;
                         }
                     }))
    {
        throw std::runtime_error{"Invalid signature"};
    }
    
    return *command;
}

bool REPL::running() const
{
    return !stop;
}

void REPL::process_command(GBA_Cpu& cpu)
{
    std::string source;
    std::getline(std::cin, source);

    if (source == "continue")
    {
        stop = true;
        return;
    }

    auto tokens = split_tokens(source);
    auto command = find_command(tokens);
    (cpu.*command.procedure)(tokens);
}


bool REPL_Argument::is_integer(const std::string& argument)
{
    if (argument.find("0x") == 0)
    {
        auto count = std::count_if(argument.begin() + 2, argument.end(), [](unsigned char c){ return std::isxdigit(c); });
       
        return static_cast<size_t>(count) == argument.size() - 2;
    }
    else
    {
        return static_cast<size_t>(std::count_if(argument.begin(), argument.end(), [](unsigned char c){ return std::isdigit(c); })) == argument.size();
    }
}

bool REPL_Argument::is_pointer(const std::string& argument)
{
    return argument.size() > 3
        && argument[0] == '['
        && argument.find(':') == std::string::npos
        && argument[argument.size() - 1] == ']';
}

bool REPL_Argument::is_range(const std::string& argument)
{
    return argument.size() > 3
        && argument[0] == '['
        && argument.find(':') != std::string::npos
        && argument[argument.size() - 1] == ']';
}

uint32_t REPL_Argument::get_integer(const std::string& argument)
{
    uint32_t value;
    std::from_chars_result result;
    const auto end = argument.c_str() + argument.size();
    
    if (argument.find("0x") == 0)
    {
        result = std::from_chars(argument.c_str() + 2, end, value, 16);
    }
    else
    {
        result = std::from_chars(argument.c_str(), end, value);
    }
    
    if (result.ptr == end)
    {
        return value;
    }
    else
    {
        throw std::runtime_error{"Could not parse value"};
    }
}

uint32_t REPL_Argument::get_pointer(std::string argument)
{
    argument = argument.substr(1, argument.size() - 2); // Remove brackets
    lr_trim<' '>(argument);
    
    uint32_t value;
    std::from_chars_result result;
    const auto end = argument.c_str() + argument.size();
    
    if (argument.find("0x") == 0)
    {
        result = std::from_chars(argument.c_str() + 2, end, value, 16);
    }
    else
    {
        result = std::from_chars(argument.c_str(), end, value);
    }
    
    if (result.ptr == end)
    {
        return value;
    }
    else
    {
        throw std::runtime_error{"Could not parse value"};
    }
}

std::pair<uint32_t, uint32_t> REPL_Argument::get_range(const std::string& argument)
{
    auto start = argument.c_str() + 1;
    auto end = argument.c_str() + argument.size() - 1;
    std::pair<uint32_t, uint32_t> value;
    std::from_chars_result result;
    
    if (argument.find("0x") == 1)
    {
        result = std::from_chars(start + 2, end, value.first, 16);
    }   
    else
    {
        result = std::from_chars(start, end, value.first);
    }
    
    if (result.ptr == end || *result.ptr != ':')
    {
        throw std::runtime_error{"Expected a colon"};
    }
    
    if (std::string_view{ result.ptr + 1 }.find("0x") == 0)
    {
        result = std::from_chars(result.ptr + 3, end, value.second, 16);
    }
    else
    {
        result = std::from_chars(result.ptr + 1, end, value.second);
    }
    
    if (result.ptr == end)
    {
        return value;
    }
    else
    {
        throw std::runtime_error{fmt::format("Could not parse value {}. Remove this character: {}({})",
                                             argument,
                                             result.ptr[0],
                                             std::distance(argument.c_str(), result.ptr))};
    }
}
