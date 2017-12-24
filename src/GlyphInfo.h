#pragma once
#include <cstdint>

struct GlyphInfo
{
    uint32_t page = 0;

    // position on texture
    int x = 0;
    int y = 0;

    // size on texture
    uint32_t width = 0;
    uint32_t height = 0;

    // shift before render
    int xOffset = 0;
    int yOffset = 0;

    int xAdvance = 0;

    bool isEmpty() const
    {
        return (width == 0) || (height == 0);
    }
};
