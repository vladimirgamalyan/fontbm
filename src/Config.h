#pragma once

#include <set>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

struct Config
{
    enum class DataFormat {
        Xml,
        Text
    };

    struct Color
    {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;

        uint32_t getUint32(uint8_t a) const
        {
            return (r + (g << 8) + (b << 16) + (a << 24));
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

    boost::filesystem::path fontFile;
    std::set<uint32_t> chars;
    Color color;
    boost::optional<Color> backgroundColor;
    uint32_t fontSize;
    Padding padding;
    Spacing spacing;
    Size textureSize;
    std::string output;
    DataFormat dataFormat;
    bool includeKerningPairs;
};
