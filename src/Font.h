#pragma once
#include <string>
#include <vector>
#include <sstream>

// http://www.angelcode.com/products/bmfont/doc/file_format.html

struct Font
{
    struct Info
    {
        struct Padding
        {
            std::string toString() const
            {
                std::stringstream ss;
                ss << up << "," << right << "," << down << "," << left;
                return ss.str();
            }
            int up;
            int right;
            int down;
            int left;
        };

        struct Spacing
        {
            std::string toString() const
            {
                std::stringstream ss;
                ss << horizontal << "," << vertical;
                return ss.str();
            }
            int horizontal;
            int vertical;
        };

        std::string toString() const
        {
            std::stringstream ss;
            ss << "face=\"" << face << "\""
                << " size=" << size
                << " bold=" << bold
                << " italic=" << italic
                << " charset=\"" << charset << "\""
                << " unicode=" << unicode
                << " stretchH=" << stretchH
                << " smooth=" << smooth
                << " aa=" << aa
                << " padding=" << padding.toString()
                << " spacing=" << spacing.toString()
                << " outline=" << outline;
            return ss.str();
        }

        std::string face;
        int size;
        int bold;
        int italic;
        std::string charset;
        int unicode;
        int stretchH;
        int smooth;
        int aa;
        Padding padding;
        Spacing spacing;
        int outline;
    };

    struct Common
    {
        std::string toString() const
        {
            std::stringstream ss;
            ss << "lineHeight=" << lineHeight
                << " base=" << base
                << " scaleW=" << scaleW
                << " scaleH=" << scaleH
                << " pages=" << pages
                << " packed=" << packed
                << " alphaChnl=" << alphaChnl
                << " redChnl=" << redChnl
                << " greenChnl=" << greenChnl
                << " blueChnl=" << blueChnl;
            return ss.str();
        }

        int lineHeight;
        int base;
        int scaleW;
        int scaleH;
        int pages;
        int packed;
        int alphaChnl;
        int redChnl;
        int greenChnl;
        int blueChnl;
    };

    struct Page
    {
        std::string toString() const
        {
            std::stringstream ss;
            ss << "id=" << id << " file=\"" << file << "\"";
            return ss.str();
        }

        int id;
        std::string file;
    };

    struct Char
    {
        std::string toString() const
        {
            std::stringstream ss;
            ss << "id=" << id
                << " x=" << x
                << " y=" << y
                << " width=" << width
                << " height=" << height
                << " xoffset=" << xoffset
                << " yoffset=" << yoffset
                << " xadvance=" << xadvance
                << " page=" << page
                << " chnl=" << chnl;
            return ss.str();
        }

        int id;
        int x;
        int y;
        int width;
        int height;
        int xoffset;
        int yoffset;
        int xadvance;
        int page;
        int chnl;
    };

    struct Kerning
    {
        std::string toString() const
        {
            std::stringstream ss;
            ss << "first=" << first << " second=" << second << " amount=" << amount;
            return ss.str();
        }

        int first;
        int second;
        int amount;
    };

    Info info;
    Common common;
    std::vector<Page> pages;
    std::vector<Char> chars;
    std::vector<Kerning> kernings;

    void debugFillValues();
    void writeToXmlFile(const std::string &fileName) const;
    void writeToTextFile(const std::string &fileName) const;
    void writeToBinFile(const std::string &fileName) const;
};
