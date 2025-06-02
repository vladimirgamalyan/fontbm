#pragma once
#include <cstdint>

struct GlyphInfo
{
    std::uint32_t page = 0;

    // position on texture
    std::uint32_t x = 0;
    std::uint32_t y = 0;

    // size on texture
    std::uint32_t width = 0;
    std::uint32_t height = 0;

    std::uint32_t utf32 = 0;

    // shift before render
    int xOffset = 0;
    int yOffset = 0;

    int xAdvance = 0;

    bool secondaryFont = false;

    bool isEmpty() const
    {
        return (width == 0) || (height == 0);
    }
};
