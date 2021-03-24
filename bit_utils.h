#pragma once

#include <cstdint>
#include <cstddef>

/**
 * @brief Rotate a 8-bit vale inside a 32-bit bondary to right.
 * 
 * @param value p_value: Value to be rotated.
 * @param shift p_shift: Half of shift amount.
 * @return uint32_t 32-bit word.
 */
uint32_t rotr32_shiftsq(uint8_t value, uint8_t shift);


uint32_t rotr32(uint8_t value, uint8_t shift);

uint32_t rotl32(uint8_t value, uint8_t shift);

/**
 * @brief Extends a signed int to the desired size.
 * 
 * @param T int type.
 * @param x value
 * @param bits current size.
 * @return T value extended to T size.
 */
template<class T>
T sign_extend(T x, const int bits) {
    T m = 1;
    m <<= bits - 1;
    return (x ^ m) - m;
}

uint32_t sign_extend_24_32(uint32_t x);
