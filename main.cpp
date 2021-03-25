#include <iostream>
#include <fstream>
#include "GBA_Memory.h"
#include "GBA_Cpu.h"

namespace tests
{
    void test_mov()
    {
        std::ifstream gba_file { "test_mov.gba", std::ios::binary };
        if (!gba_file.is_open())
        {
            std::cerr << "TEST_MOV: Could not open rom file" << std::endl;
            return;
        }
        
        GBA_Memory mem;
        mem.load_rom(gba_file, nullptr);
        GBA_Cpu cpu { mem };
        cpu.add_break_point(0x800012a);
        while (cpu.cycle());
        
        
    }
}

int main()
{    
    tests::test_mov();
    
    
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
