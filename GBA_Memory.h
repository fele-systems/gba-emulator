#pragma once

#include <cstdint>
#include <fstream>
#include <vector>

struct GBA_CartridgeHeader
{
    uint32_t entry_point;
    uint8_t nintendo_logo[156];
    char game_title[12] = { 0 };
    char game_code[4] = { 0 };
    char maker_code[2] = { 0 };
    uint8_t fixed_value;
    uint8_t main_unit_code;
    uint8_t device_type;
    uint8_t reserved_area_0[7];
    uint8_t software_version;
    uint8_t complement_check;
    uint16_t reserved_area_1;
};

class GBA_Memory
{
public:
    GBA_Memory();
    // Loads stream content (ROM) into 0x800_0000 position
    /**
     * @brief Loads stream content (ROM) into memory at 0x0800_0000.
     * 
     * @param gba_file Stream with ROM contents.
     * @param header_ptr (Optional) Pointer to a struct to store the cartridge header.
     */
    void load_rom(std::ifstream& gba_file, GBA_CartridgeHeader* header_ptr);
    
    /**
     * @brief Reads a word (32 bit value) from memory.
     * 
     * This includes the bytes at address to address + 3, so the next word would be at address + 4.
     * 
     * @param address Address to read.
     * @return uint32_t Word at address.
     */
    uint32_t read_word(uint32_t address) const;
    
    uint16_t read_halfword(uint32_t address) const;

    uint8_t read_byte(uint32_t address) const;

    void write_word(uint32_t address, uint32_t word);

    std::string dump(uint32_t align, uint32_t begin, uint32_t end);

    uint32_t find_word(uint32_t value, uint32_t begin, uint32_t end) const;
public:
    static constexpr uint32_t rom_base = 0x08000000;
    static constexpr uint32_t word_size = 4;
private:
    std::vector<uint8_t> memory_buffer;
};
