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
        Json,
        Cbor
    };

    enum class KerningPairs {
        Disabled,
        Basic,
        Regular,
        Extended
    };

    enum class TextureNameSuffix {
        IndexAligned,
        Index,
        None
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
        Size() = default;
        Size(std::uint32_t w, std::uint32_t h) : w(w), h(h) {}
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

    struct Alignment
    {
        std::uint32_t ver = 1;
        std::uint32_t hor = 1;
    };

    std::string fontFile;
    std::string secondaryFontFile;
    std::set<std::uint32_t> chars; // utf32
    Color color;
    Color backgroundColor;
    bool backgroundTransparent = true;
    std::uint16_t fontSize = 16;
    Padding padding;
    Spacing spacing;
    Alignment alignment;
    std::vector<Size> textureSizeList;
    std::string output;
    DataFormat dataFormat = DataFormat::Text;
    KerningPairs kerningPairs = KerningPairs::Disabled;
    std::uint32_t maxTextureCount = 0;
    bool useMaxTextureCount = false;
    bool monochrome = false;
    bool lightHinting = false;
    bool noHinting = false;
    bool allChars = false;
    bool extraInfo = false;
    bool cropTexturesWidth = false;
    bool cropTexturesHeight = false;
    bool verbose = false;
    bool slashedZero = false;
    bool tabularNumbers = false;
    TextureNameSuffix textureNameSuffix = TextureNameSuffix::IndexAligned;
};
