#include "bit_utils.h"

uint32_t rotr32_shiftsq(uint8_t value, uint8_t shift)
{
    if (shift == 0 || shift >= 0x10)
        return value;

    return (value >> shift * 2) | (value << (32 - shift * 2));
}

uint32_t rotr32(uint8_t value, uint8_t shift)
{
    if (shift == 0 || shift >= 32)
        return value;

    return (value >> shift) | (value << (32 - shift));
}

uint32_t rotl32(uint8_t value, uint8_t shift)
{
    if (shift == 0 || shift >= 32)
        return value;

    return (value << shift) | (value >> (32 - shift));
}

uint32_t sign_extend_24_32(uint32_t x)
{
    const int bits = 24;
    uint32_t m = 1u << (bits - 1);
    return (x ^ m) - m;
}
