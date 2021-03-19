#include "GBA_Memory.h"

#include <cassert>

GBA_Memory::GBA_Memory()
    : memory_buffer(0xFFFFFFFF, 0u)
{
}

void GBA_Memory::load_rom(std::ifstream& gba_file, GBA_CartridgeHeader* header_ptr)
{
    if (header_ptr != nullptr)
    {
        gba_file.seekg(0);
        gba_file.read(reinterpret_cast<char*>(header_ptr), sizeof(GBA_CartridgeHeader));

        assert(header_ptr->fixed_value == 0x96);
    }
    gba_file.seekg(0);    
    std::copy(std::istreambuf_iterator<char>(gba_file),
              std::istreambuf_iterator<char>(),
              memory_buffer.begin() + 0x8000000);
}

uint32_t GBA_Memory::read_word(uint32_t address) const
{
    return memory_buffer[address]
        | (memory_buffer[address + 1] << 8)
        | (memory_buffer[address + 2] << 16)
        | (memory_buffer[address + 3] << 24);
}

void GBA_Memory::write_word(uint32_t address, uint32_t word)
{
    memory_buffer[address] = word & 0xFF;
    memory_buffer[address + 1] = (word >> 8) & 0xFF;
    memory_buffer[address + 2] = (word >> 16) & 0xFF;
    memory_buffer[address + 3] = (word >> 24) & 0xFF;
}
