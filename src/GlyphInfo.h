#pragma once
#include <stdint.h>

struct GlyphInfo
{
    uint16_t page = 0;
    int x = 0;
    int y = 0;

    int getWidth() const
    {
        return maxx - minx;
    }

    int getHeight() const
    {
        return maxy - miny;
    }

    bool isEmpty() const
    {
        return ((getWidth() == 0) && (getHeight() == 0));
    }

    bool isInvalid() const
    {
        return ((!isEmpty()) && ((getWidth() <= 0) || (getHeight() <= 0)));
    }

    int minx = 0;
    int maxx = 0;
    int miny = 0;
    int maxy = 0;
    int advance = 0;
};
