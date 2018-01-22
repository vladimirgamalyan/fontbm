#include <stdexcept>
#include <fstream>
#include <iostream>
#include "FontInfo.h"
#include "external/tinyxml2/tinyxml2.h"
#include "external/json.hpp"

std::string FontInfo::getCharSetName(std::uint8_t charSet)
{
    std::string str;

    switch(charSet)
    {
        case 0: // ANSI_CHARSET
            str = "ANSI";
            break;
        case 1: // DEFAULT_CHARSET
            str = "DEFAULT";
            break;
        case 2: // SYMBOL_CHARSET
            str = "SYMBOL";
            break;
        case 128: // SHIFTJIS_CHARSET
            str = "SHIFTJIS";
            break;
        case 129: // HANGUL_CHARSET
            str = "HANGUL";
            break;
        case 134: // GB2312_CHARSET
            str = "GB2312";
            break;
        case 136: // CHINESEBIG5_CHARSET
            str = "CHINESEBIG5";
            break;
        case 255: // OEM_CHARSET
            str = "OEM";
            break;
        case 130: // JOHAB_CHARSET
            str = "JOHAB";
            break;
        case 177: // HEBREW_CHARSET
            str = "HEBREW";
            break;
        case 178: // ARABIC_CHARSET
            str = "ARABIC";
            break;
        case 161: // GREEK_CHARSET
            str = "GREEK";
            break;
        case 162: // TURKISH_CHARSET
            str = "TURKISH";
            break;
        case 163: // VIETNAMESE_CHARSET
            str = "VIETNAMESE";
            break;
        case 222: // THAI_CHARSET
            str = "THAI";
            break;
        case 238: // EASTEUROPE_CHARSET
            str = "EASTEUROPE";
            break;
        case 204: // RUSSIAN_CHARSET
            str = "RUSSIAN";
            break;
        case 77:  // MAC_CHARSET
            str = "MAC";
            break;
        case 186: // BALTIC_CHARSET
            str = "BALTIC";
            break;

        default:
            str = std::to_string(charSet);
    }

    return str;
}

void FontInfo::writeToXmlFile(const std::string &fileName) const
{
    testPages();

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLDeclaration* declaration = doc.NewDeclaration("xml version=\"1.0\"");
    doc.InsertFirstChild(declaration);

    tinyxml2::XMLElement* root = doc.NewElement("font");
    doc.InsertEndChild(root);

    std::stringstream padding;
    padding << static_cast<int>(info.padding.up)
        << "," << static_cast<int>(info.padding.right)
        << "," << static_cast<int>(info.padding.down)
        << "," << static_cast<int>(info.padding.left);
    std::stringstream spacing;
    spacing << static_cast<int>(info.spacing.horizontal)
       << "," << static_cast<int>(info.spacing.vertical);

    tinyxml2::XMLElement* infoElement = doc.NewElement("info");
    infoElement->SetAttribute("face", info.face.c_str());
    infoElement->SetAttribute("size", info.size);
    infoElement->SetAttribute("bold", info.bold);
    infoElement->SetAttribute("italic", info.italic);
    infoElement->SetAttribute("charset", info.unicode ? "" : getCharSetName(info.charset).c_str());
    infoElement->SetAttribute("unicode", info.unicode);
    infoElement->SetAttribute("stretchH", info.stretchH);
    infoElement->SetAttribute("smooth", info.smooth);
    infoElement->SetAttribute("aa", info.aa);
    infoElement->SetAttribute("padding", padding.str().c_str());
    infoElement->SetAttribute("spacing", spacing.str().c_str());
    infoElement->SetAttribute("outline", info.outline);
    root->InsertEndChild(infoElement);

    tinyxml2::XMLElement* commonElement = doc.NewElement("common");
    commonElement->SetAttribute("lineHeight", common.lineHeight);
    commonElement->SetAttribute("base", common.base);
    commonElement->SetAttribute("scaleW", common.scaleW);
    commonElement->SetAttribute("scaleH", common.scaleH);
    commonElement->SetAttribute("pages", static_cast<int>(pages.size()));
    commonElement->SetAttribute("packed", common.packed);
    commonElement->SetAttribute("alphaChnl", common.alphaChnl);
    commonElement->SetAttribute("redChnl", common.redChnl);
    commonElement->SetAttribute("greenChnl", common.greenChnl);
    commonElement->SetAttribute("blueChnl", common.blueChnl);
    root->InsertEndChild(commonElement);

    tinyxml2::XMLElement* pagesElement = doc.NewElement("pages");
    root->InsertEndChild(pagesElement);

    tinyxml2::XMLElement* charsElement = doc.NewElement("chars");
    charsElement->SetAttribute("count", static_cast<int>(chars.size()));
    root->InsertEndChild(charsElement);

    tinyxml2::XMLElement* kerningsElement = doc.NewElement("kernings");
    kerningsElement->SetAttribute("count", static_cast<int>(kernings.size()));
    root->InsertEndChild(kerningsElement);

    for (size_t i = 0; i < pages.size(); ++i)
    {
        tinyxml2::XMLElement* pageElement = doc.NewElement("page");
        pageElement->SetAttribute("id", static_cast<unsigned int>(i));
        pageElement->SetAttribute("file", pages[i].c_str());
        pagesElement->InsertEndChild(pageElement);
    }

    for (auto c: chars )
    {
        tinyxml2::XMLElement* charElement = doc.NewElement("char");
        charElement->SetAttribute("id", c.id);
        charElement->SetAttribute("x", c.x);
        charElement->SetAttribute("y", c.y);
        charElement->SetAttribute("width", c.width);
        charElement->SetAttribute("height", c.height);
        charElement->SetAttribute("xoffset", c.xoffset);
        charElement->SetAttribute("yoffset", c.yoffset);
        charElement->SetAttribute("xadvance", c.xadvance);
        charElement->SetAttribute("page", c.page);
        charElement->SetAttribute("chnl", c.chnl);
        charsElement->InsertEndChild(charElement);
    }

    for (auto k: kernings)
    {
        tinyxml2::XMLElement* kerningElement = doc.NewElement("kerning");
        kerningElement->SetAttribute("first", k.first);
        kerningElement->SetAttribute("second", k.second);
        kerningElement->SetAttribute("amount", k.amount);
        kerningsElement->InsertEndChild(kerningElement);
    }

    tinyxml2::XMLError err = doc.SaveFile(fileName.c_str(), false);
    if (err)
        throw std::runtime_error("xml write to file error");
}

void FontInfo::writeToTextFile(const std::string &fileName) const
{
    testPages();

    std::ofstream f(fileName);

    f << "info"
        << " face=\"" << info.face << "\""
        << " size=" << info.size
        << " bold=" << info.bold
        << " italic=" << info.italic
        << " charset=\"" << (info.unicode ? "" : getCharSetName(info.charset)) << "\""
        << " unicode=" << info.unicode
        << " stretchH=" << info.stretchH
        << " smooth=" << info.smooth
        << " aa=" << static_cast<int>(info.aa)
        << " padding="
           << static_cast<int>(info.padding.up)
           << "," << static_cast<int>(info.padding.right)
           << "," << static_cast<int>(info.padding.down)
           << "," << static_cast<int>(info.padding.left)
        << " spacing="
            << static_cast<int>(info.spacing.horizontal)
            << "," << static_cast<int>(info.spacing.vertical)
        << " outline=" << static_cast<int>(info.outline)
        << std::endl;

    f << "common"
        << " lineHeight=" << common.lineHeight
        << " base=" << common.base
        << " scaleW=" << common.scaleW
        << " scaleH=" << common.scaleH
        << " pages=" << pages.size()
        << " packed=" << common.packed
        << " alphaChnl=" << static_cast<int>(common.alphaChnl)
        << " redChnl=" << static_cast<int>(common.redChnl)
        << " greenChnl=" << static_cast<int>(common.greenChnl)
        << " blueChnl=" << static_cast<int>(common.blueChnl)
        << std::endl;

    for (size_t i = 0; i < pages.size(); ++i)
        f << "page id=" << i << " file=\"" << pages[i] << "\"" << std::endl;

    f << "chars count=" << chars.size() << std::endl;
    for(auto c: chars)
    {
        f << "char"
            << " id=" << c.id
            << " x=" << c.x
            << " y=" << c.y
            << " width=" << c.width
            << " height=" << c.height
            << " xoffset=" << c.xoffset
            << " yoffset=" << c.yoffset
            << " xadvance=" << c.xadvance
            << " page=" << static_cast<int>(c.page)
            << " chnl=" << static_cast<int>(c.chnl)
            << std::endl;
    }

    f << "kernings count=" << kernings.size() << std::endl;
    for(auto k: kernings)
    {
        f << "kerning "
            << "first=" << k.first
            << " second=" << k.second
            << " amount=" << k.amount
            << std::endl;
    }
}

void FontInfo::writeToBinFile(const std::string &fileName) const
{
    testPages();

    std::ofstream f(fileName, std::ios::binary);

#pragma pack(push, 1)
    struct InfoBlock
    {
        std::int32_t blockSize;
        std::uint16_t fontSize;
        std::int8_t reserved:4;
        std::int8_t bold:1;
        std::int8_t italic:1;
        std::int8_t unicode:1;
        std::int8_t smooth:1;
        std::uint8_t charSet;
        std::uint16_t stretchH;
        std::int8_t aa;
        std::uint8_t paddingUp;
        std::uint8_t paddingRight;
        std::uint8_t paddingDown;
        std::uint8_t paddingLeft;
        std::uint8_t spacingHoriz;
        std::uint8_t spacingVert;
        std::uint8_t outline;
    };

    struct CommonBlock
    {
        std::int32_t blockSize;
        std::uint16_t lineHeight;
        std::uint16_t base;
        std::uint16_t scaleW;
        std::uint16_t scaleH;
        std::uint16_t pages;
        std::uint8_t packed:1;
        std::uint8_t reserved:7;
        std::uint8_t alphaChnl;
        std::uint8_t redChnl;
        std::uint8_t greenChnl;
        std::uint8_t blueChnl;
    };

    struct CharBlock
    {
        std::uint32_t id;
        std::uint16_t x;
        std::uint16_t y;
        std::uint16_t width;
        std::uint16_t height;
        std::int16_t xoffset;
        std::int16_t yoffset;
        std::int16_t xadvance;
        std::int8_t page;
        std::int8_t channel;
    };

    struct KerningPairsBlock
    {
        std::uint32_t first;
        std::uint32_t second;
        std::int16_t amount;
    };
#pragma pack(pop)

    f << "BMF";
    f << '\3';

    InfoBlock infoBlock;
    infoBlock.blockSize = sizeof(InfoBlock) - sizeof(InfoBlock::blockSize) + info.face.length() + 1;
    infoBlock.fontSize = info.size;
    infoBlock.bold = info.bold;
    infoBlock.italic = info.italic;
    infoBlock.unicode = info.unicode;
    infoBlock.smooth = info.smooth;
    infoBlock.charSet = info.unicode ? 0 : info.charset;
    infoBlock.stretchH = info.stretchH;
    infoBlock.aa = info.aa;
    infoBlock.paddingUp = info.padding.up;
    infoBlock.paddingRight = info.padding.right;
    infoBlock.paddingDown = info.padding.down;
    infoBlock.paddingLeft = info.padding.left;
    infoBlock.spacingHoriz = info.spacing.horizontal;
    infoBlock.spacingVert = info.spacing.vertical;
    infoBlock.outline = info.outline;

    f << '\1';
    f.write((const char*)&infoBlock, sizeof(infoBlock));
    f.write(info.face.c_str(), info.face.length() + 1);

    CommonBlock commonBlock;
    commonBlock.blockSize = sizeof(CommonBlock) - sizeof(CommonBlock::blockSize);
    commonBlock.lineHeight = common.lineHeight;
    commonBlock.base = common.base;
    commonBlock.scaleW = common.scaleW;
    commonBlock.scaleH = common.scaleH;
    commonBlock.pages = static_cast<std::uint16_t>(pages.size());
    commonBlock.packed = common.packed;
    commonBlock.alphaChnl = common.alphaChnl;
    commonBlock.redChnl = common.redChnl;
    commonBlock.greenChnl = common.greenChnl;
    commonBlock.blueChnl = common.blueChnl;

    f << '\2';
    f.write((const char*)&commonBlock, sizeof(commonBlock));

    f << '\3';
    std::int32_t pageBlockSize = pages.empty() ? 0 : pages[0].length() * pages.size();
    f << pageBlockSize;
    for (auto s: pages)
        f << s;

    f << '\4';
    std::int32_t charsBlockSize = chars.size() * sizeof(CharBlock);
    f << charsBlockSize;
    for (auto c: chars)
    {
        CharBlock charBlock;
        charBlock.id = c.id;
        charBlock.x = c.x;
        charBlock.y = c.y;
        charBlock.width = c.width;
        charBlock.height = c.height;
        charBlock.xoffset = c.xoffset;
        charBlock.yoffset = c.yoffset;
        charBlock.xadvance = c.xadvance;
        charBlock.page = c.page;
        charBlock.channel = c.chnl;

        f.write((const char*)&charBlock, sizeof(charBlock));
    }

    if (!kernings.empty())
    {
        f << '\5';
        std::int32_t kerningPairsBlockSize = kernings.size() * sizeof(KerningPairsBlock);
        f << kerningPairsBlockSize;

        for (auto k: kernings)
        {
            KerningPairsBlock kerningPairsBlock;
            kerningPairsBlock.first = k.first;
            kerningPairsBlock.second = k.second;
            kerningPairsBlock.amount = k.amount;

            f.write((const char*)&kerningPairsBlock, sizeof(kerningPairsBlock));
        }
    }
}

void FontInfo::writeToJsonFile(const std::string &fileName) const
{
    //TODO: test

    nlohmann::json j;

    nlohmann::json infoNodePadding;
    infoNodePadding["up"] = info.padding.up;
    infoNodePadding["right"] = info.padding.right;
    infoNodePadding["down"] = info.padding.down;
    infoNodePadding["left"] = info.padding.left;

    nlohmann::json infoNodeSpacing;
    infoNodeSpacing["horizontal"] = info.spacing.horizontal;
    infoNodeSpacing["vertical"] = info.spacing.vertical;

    nlohmann::json infoNode;
    infoNode["size"] = info.size;
    infoNode["smooth"] = info.smooth;
    infoNode["unicode"] = info.unicode;
    infoNode["italic"] = info.bold;
    infoNode["bold"] = info.bold;
    infoNode["charset"] = info.charset;
    infoNode["stretchH"] = info.stretchH;
    infoNode["aa"] = info.aa;
    infoNode["padding"] = infoNodePadding;
    infoNode["spacing"] = infoNodeSpacing;
    infoNode["outline"] = info.outline;
    infoNode["face"] = info.face;

    nlohmann::json commonNode;
    commonNode["lineHeight"] = common.lineHeight;
    commonNode["base"] = common.base;
    commonNode["scaleW"] = common.scaleW;
    commonNode["scaleH"] = common.scaleH;
    commonNode["pages"] = pages.size();
    commonNode["packed"] = common.packed;
    commonNode["alphaChnl"] = common.alphaChnl;
    commonNode["redChnl"] = common.redChnl;
    commonNode["greenChnl"] = common.greenChnl;
    commonNode["blueChnl"] = common.blueChnl;

    nlohmann::json charsNode = nlohmann::json::array();
    for(auto c: chars)
    {
        nlohmann::json charNode;
        charNode["id"] = c.id;
        charNode["x"] = c.x;
        charNode["y"] = c.y;
        charNode["width"] = c.width;
        charNode["height"] = c.height;
        charNode["xoffset"] = c.xoffset;
        charNode["yoffset"] = c.yoffset;
        charNode["xadvance"] = c.xadvance;
        charNode["page"] = c.page;
        charNode["chnl"] = c.chnl;
        charsNode.push_back(charNode);
    }

    nlohmann::json kerningsNode = nlohmann::json::array();
    for(auto k: kernings)
    {
        nlohmann::json kerningNode;
        kerningNode["first"] = k.first;
        kerningNode["second"] = k.second;
        kerningNode["amount"] = k.amount;
        kerningsNode.push_back(kerningNode);
    }

    j["info"] = infoNode;
    j["common"] = commonNode;
    j["pages"] = pages;
    j["chars"] = charsNode;
    j["kernings"] = kerningsNode;

    std::ofstream f(fileName);
    f << j.dump(4);
}

void FontInfo::testPages() const
{
    if (!pages.empty())
    {
        size_t l = pages[0].length();
        if (!l)
            throw std::runtime_error("page name is empty");
        for (size_t i = 1; i < pages.size(); ++i)
            if (l != pages[i].length())
                throw std::runtime_error("page names have different length");
    }
}
