#pragma once

#include <cstdint>
#include <cstddef>

inline std::size_t getNumberLen(std::uint32_t x)
{
    std::size_t digits = 0;
    do
    {
        x /= 10;
        ++digits;
    }
    while (x);

    return digits;
}
