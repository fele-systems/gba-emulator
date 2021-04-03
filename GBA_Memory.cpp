#include "GBA_Memory.h"

#include <cassert>
#include <sstream>
#include <fmt/core.h>

GBA_Memory::GBA_Memory()
    : memory_buffer(0x0FFFFFFF, 0u)
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
              memory_buffer.begin() + rom_base);
}

uint32_t GBA_Memory::read_word(uint32_t address) const
{
    return memory_buffer[size_t(address)]
        | (memory_buffer[size_t(address) + 1] << 8)
        | (memory_buffer[size_t(address) + 2] << 16)
        | (memory_buffer[size_t(address) + 3] << 24);
}

uint16_t GBA_Memory::read_halfword(uint32_t address) const
{
    return memory_buffer[size_t(address)]
        | (memory_buffer[size_t(address) + 1] << 8);
}

uint8_t GBA_Memory::read_byte(uint32_t address) const
{
    return memory_buffer[size_t(address)];
}

void GBA_Memory::write_word(uint32_t address, uint32_t word)
{
    memory_buffer[size_t(address)] = word & 0xFF;
    memory_buffer[size_t(address) + 1] = (word >> 8) & 0xFF;
    memory_buffer[size_t(address) + 2] = (word >> 16) & 0xFF;
    memory_buffer[size_t(address) + 3] = (word >> 24) & 0xFF;
}

std::string GBA_Memory::dump(uint32_t align, uint32_t begin, uint32_t end)
{
    auto line_start = begin - (begin % align);

    std::stringstream ss;

    while (line_start < end)
    {
        ss << fmt::format("[0x{:0>8x}] ", line_start);

        for (auto i = 0u; i < align; i++)
        {
            auto c = line_start + i;
            if (begin <= c && c < end)
            {
                ss << fmt::format("{:0<2x} ", read_byte(c));
            }
            else
            {
                ss << "?? ";
            }
        }

        line_start += align;
        ss << std::endl;
    }

    return ss.str();
}

uint32_t GBA_Memory::find_word(uint32_t value, uint32_t begin, uint32_t end) const
{
    if (begin >= end)
    {
        return end;
    }

    for (; begin != end; begin++)
    {
        if (read_word(begin) == value) break;
    }

    return begin;
}
