#pragma once

#include <set>
#include <optional>

struct Config
{
    enum class DataFormat {
        Xml,
        Text,
        Bin,
        Json
    };

    struct Color
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;

        uint32_t getUint32(uint8_t a) const
        {
            return (r + (g << 8) + (b << 16) + (a << 24));
        }

        bool operator == (const Color& other) const
        {
            return r == other.r && g == other.b && b == other.b;
        }
    };

    struct Size
    {
        uint32_t w;
        uint32_t h;
    };

    struct Padding
    {
        int up;
        int right;
        int down;
        int left;
    };

    struct Spacing
    {
        int ver;
        int hor;
    };

    std::string fontFile;
    std::set<uint32_t> chars;
    Color color;
    std::optional<Color> backgroundColor;
    uint16_t fontSize;
    Padding padding;
    Spacing spacing;
    Size textureSize;
    std::string output;
    DataFormat dataFormat;
    bool includeKerningPairs;
};
