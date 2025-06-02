#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>

// http://www.angelcode.com/products/bmfont/doc/file_format.html

struct FontInfo
{
    struct Info
    {
        struct Padding
        {
            std::uint8_t up = 0;
            std::uint8_t right = 0;
            std::uint8_t down = 0;
            std::uint8_t left = 0;
        };

        struct Spacing
        {
            std::uint8_t horizontal = 0;
            std::uint8_t vertical = 0;
        };

        std::int16_t size = 0;
        bool smooth = false;
        bool unicode = false;
        bool italic = false;
        bool bold = false;
        std::uint8_t charset = 0;
        std::uint16_t stretchH = 0;
        std::uint8_t aa = 0;
        Padding padding;
        Spacing spacing;
        std::uint8_t outline = 0;
        std::string face;
        std::string style;
    };

    struct Common
    {
        std::uint16_t lineHeight = 0;
        std::uint16_t base = 0;
        std::int16_t descent = 0;
        std::uint16_t scaleW = 0;
        std::uint16_t scaleH = 0;
        bool packed = false;
        std::uint8_t alphaChnl = 0;
        std::uint8_t redChnl = 0;
        std::uint8_t greenChnl = 0;
        std::uint8_t blueChnl = 0;
        std::uint16_t totalHeight = 0;  // non bmfont
    };

    struct Char
    {
        std::uint32_t id = 0;
        std::uint16_t x = 0;
        std::uint16_t y = 0;
        std::uint16_t width = 0;
        std::uint16_t height = 0;
        std::int16_t xoffset = 0;
        std::int16_t yoffset = 0;
        std::int16_t xadvance = 0;
        std::int8_t page = 0;
        std::int8_t chnl = 0;
    };

    struct Kerning
    {
        std::uint32_t first = 0;
        std::uint32_t second = 0;
        std::int16_t amount = 0;
    };

    Info info;
    Common common;
    std::vector<std::string> pages;
    std::vector<Char> chars;
    std::vector<Kerning> kernings;

    bool extraInfo = false;

    void writeToXmlFile(const std::string &fileName) const;
    void writeToTextFile(const std::string &fileName) const;
    void writeToBinFile(const std::string &fileName) const;
    void writeToJsonFile(const std::string &fileName) const;
    void writeToCborFile(const std::string &fileName) const;

private:
    static std::string getCharSetName(std::uint8_t charSet);
};
