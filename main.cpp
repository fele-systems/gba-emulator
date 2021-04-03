#include <iostream>
#include <fstream>
#include "GBA_Memory.h"
#include "GBA_Cpu.h"
#include "repl.h"

namespace tests
{
    void test_mov()
    {
        std::ifstream gba_file{ "test_mov.gba", std::ios::binary };
        if (!gba_file.is_open())
        {
            std::cerr << "TEST_MOV: Could not open rom file" << std::endl;
            return;
        }

        GBA_Memory mem;
        mem.load_rom(gba_file, nullptr);

        auto pointer = mem.find_word(0xe3a0001f, GBA_Memory::rom_base, GBA_Memory::rom_base + 0xFFFF);

        if (pointer != GBA_Memory::rom_base + 0xFFFF)
        {
            std::cout << mem.dump(4, pointer - 10, pointer + 10) << std::endl;
        }
        
        GBA_Cpu cpu { mem };
        cpu.add_break_point(0x800012a);
        while (cpu.cycle());
        
        
    }
}

int main()
{    
    std::string source = "find 255 0xFF [0x08000000:0x0800FFFF] [0x0800FFFF]";
    
    REPL repl;
    
    auto tokens = repl.split_tokens(source);
    auto command = repl.find_command(tokens);
     
    try
    {
        for (size_t i = 1; i < tokens.size(); i++)
        {
            if (REPL_Argument::is_integer(tokens[i]))
            {
                auto v = REPL_Argument::get_integer(tokens[i]);
                std::cout << "Integer: " << v << '\n';
            }
            else if (REPL_Argument::is_pointer(tokens[i]))
            {
                auto v = REPL_Argument::get_pointer(tokens[i]);
                std::cout << "Pointer: [" << v << "]\n";
            }
            else if (REPL_Argument::is_range(tokens[i]))
            {
                auto v = REPL_Argument::get_range(tokens[i]);
                std::cout << "Range: [" << v.first << ", " << v.second << ")\n";
            }
        }
    } catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    // tests::test_mov();
    
    
//     std::ifstream gba_file { "../fzero.gba", std::ios::binary };
//     if (!gba_file.is_open())
//     {
//         std::cerr << "Could not open rom file" << std::endl;
//         exit(1);
//     }
//     
//     GBA_CartridgeHeader cartridge_header;
//     GBA_Memory mem;
//     mem.load_rom(gba_file, &cartridge_header);
//     std::cout << std::string{ std::begin(cartridge_header.game_title), std::end(cartridge_header.game_title) } << std::endl;
// 
//     GBA_Cpu cpu { mem };
// 
//     while (cpu.cycle());

    std::cout << std::endl;

    return 0;
}
