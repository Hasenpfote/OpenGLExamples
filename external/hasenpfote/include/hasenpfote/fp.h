/*!
* @file fp.h
* @brief Floating point number.
* @author Hasenpfote
* @date 2016/04/27
*/
#pragma once
#include <cstdint>

namespace hasenpfote{

union FP16
{
    std::uint16_t u;
    struct
    {
        std::uint16_t mantissa : 10;
        std::uint16_t exponent : 5;
        std::uint16_t sign : 1;
    };
};

union FP32
{
    std::uint32_t u;
    float f;
    struct
    {
        std::uint32_t mantissa : 23;
        std::uint32_t exponent : 8;
        std::uint32_t sign : 1;
    };
};

std::ostream& operator << (std::ostream& os, const FP16& fp);
std::ostream& operator << (std::ostream& os, const FP32& fp);

std::uint16_t float_to_half(float f);
float half_to_float(std::uint16_t h);

}