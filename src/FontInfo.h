#pragma once
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
            uint8_t up = 0;
            uint8_t right = 0;
            uint8_t down = 0;
            uint8_t left = 0;
        };

        struct Spacing
        {
            uint8_t horizontal = 0;
            uint8_t vertical = 0;
        };

        uint16_t size = 0;
        bool smooth = false;
        bool unicode = false;
        bool italic = false;
        bool bold = false;
        uint8_t charset = 0;
        uint16_t stretchH = 0;
        uint8_t aa = 0;
        Padding padding;
        Spacing spacing;
        uint8_t outline = 0;
        std::string face;
    };

    struct Common
    {
        uint16_t lineHeight = 0;
        uint16_t base = 0;
        uint16_t scaleW = 0;
        uint16_t scaleH = 0;
        bool packed = false;
        uint8_t alphaChnl = 0;
        uint8_t redChnl = 0;
        uint8_t greenChnl = 0;
        uint8_t blueChnl = 0;
    };

    struct Char
    {
        uint32_t id = 0;
        uint16_t x = 0;
        uint16_t y = 0;
        uint16_t width = 0;
        uint16_t height = 0;
        int16_t xoffset = 0;
        int16_t yoffset = 0;
        int16_t xadvance = 0;
        int8_t page = 0;
        int8_t chnl = 0;
    };

    struct Kerning
    {
        uint32_t first = 0;
        uint32_t second = 0;
        int16_t amount = 0;
    };

    Info info;
    Common common;
    std::vector<std::string> pages;
    std::vector<Char> chars;
    std::vector<Kerning> kernings;

    void writeToXmlFile(const std::string &fileName) const;
    void writeToTextFile(const std::string &fileName) const;
    void writeToBinFile(const std::string &fileName) const;
    void writeToJsonFile(const std::string &fileName) const;

private:
    void testPages() const;
    static std::string getCharSetName(uint8_t charSet);
};
