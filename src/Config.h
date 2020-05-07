#pragma once

#include <set>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <vector>

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
        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;

        std::uint32_t getBGR() const
        {
            return r + (g << 8u) + (b << 16u);
        }

        bool operator == (const Color& other) const
        {
            return r == other.r && g == other.b && b == other.b;
        }
    };

    struct Size
    {
        std::uint32_t w = 0;
        std::uint32_t h = 0;
    };

    struct Padding
    {
        std::uint32_t up = 0;
        std::uint32_t right = 0;
        std::uint32_t down = 0;
        std::uint32_t left = 0;
    };

    struct Spacing
    {
        std::uint32_t ver = 0;
        std::uint32_t hor = 0;
    };

    std::vector<std::string> fontFile;
    std::set<std::uint32_t> chars;
    Color color;
    Color backgroundColor;
    bool backgroundTransparent = true;
    std::uint16_t fontSize = 16;
    Padding padding;
    Spacing spacing;
    Size textureSize;
    std::string output;
    DataFormat dataFormat = DataFormat::Text;
    bool includeKerningPairs = false;
    std::uint32_t maxTextureCount = 0;

    void validate() const
    {
        if (textureSize.w > 65536 || textureSize.h > 65536)
            throw std::runtime_error("too big texture size (width or height greater than 65536)");
    }
};
