#include <stdexcept>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "FontInfo.h"
#include "external/tinyxml2/tinyxml2.h"
#include "external/json.hpp"
#include "external/cbor/cbor_encoder_ostream.h"

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
    infoElement->SetAttribute("bold", info.bold ? 1 : 0);
    infoElement->SetAttribute("italic", info.italic ? 1 : 0);
    infoElement->SetAttribute("charset", info.unicode ? "" : getCharSetName(info.charset).c_str());
    infoElement->SetAttribute("unicode", info.unicode ? 1 : 0);
    infoElement->SetAttribute("stretchH", info.stretchH);
    infoElement->SetAttribute("smooth", info.smooth ? 1 : 0);
    infoElement->SetAttribute("aa", info.aa);
    infoElement->SetAttribute("padding", padding.str().c_str());
    infoElement->SetAttribute("spacing", spacing.str().c_str());
    infoElement->SetAttribute("outline", info.outline);
    if (extraInfo) {
        infoElement->SetAttribute("style", info.style.c_str());
    }
    root->InsertEndChild(infoElement);

    tinyxml2::XMLElement* commonElement = doc.NewElement("common");
    commonElement->SetAttribute("lineHeight", common.lineHeight);
    commonElement->SetAttribute("base", common.base);
    commonElement->SetAttribute("scaleW", common.scaleW);
    commonElement->SetAttribute("scaleH", common.scaleH);
    commonElement->SetAttribute("pages", static_cast<int>(pages.size()));
    commonElement->SetAttribute("packed", static_cast<int>(common.packed));
    commonElement->SetAttribute("alphaChnl", common.alphaChnl);
    commonElement->SetAttribute("redChnl", common.redChnl);
    commonElement->SetAttribute("greenChnl", common.greenChnl);
    commonElement->SetAttribute("blueChnl", common.blueChnl);
    if (extraInfo) 
    {
        commonElement->SetAttribute("base", common.descent);
        commonElement->SetAttribute("totalHeight", common.totalHeight);
    }
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
        << " outline=" << static_cast<int>(info.outline);
    if (extraInfo) {
        f << " style=\"" << info.style << "\"";
    }
    f << std::endl;

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
        << " blueChnl=" << static_cast<int>(common.blueChnl);
    if (extraInfo) 
    {
        f << " totalHeight=" << common.totalHeight;
        f << " descent=" << common.descent;
    }
    f << std::endl;

    for (size_t i = 0; i < pages.size(); ++i)
        f << "page id=" << i << " file=\"" << pages[i] << "\"" << std::endl;

    f << "chars count=" << chars.size() << std::endl;
    f << std::left;
    for(auto c: chars)
    {
        f << "char"
            << " id=" <<  std::setw(4) << c.id
            << " x=" << std::setw(5) << c.x
            << " y=" << std::setw(5) << c.y
            << " width=" << std::setw(5) << c.width
            << " height=" << std::setw(5) << c.height
            << " xoffset=" << std::setw(5) << c.xoffset
            << " yoffset=" << std::setw(5) << c.yoffset
            << " xadvance=" << std::setw(5) << c.xadvance
            << " page=" << std::setw(2) << static_cast<int>(c.page)
            << " chnl=" << std::setw(2) << static_cast<int>(c.chnl)
            << std::endl;
    }
    f << std::right;
    if (!kernings.empty())
    {
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
}

void FontInfo::writeToBinFile(const std::string &fileName) const
{
    if (extraInfo)
        throw std::runtime_error("--extra-info flag is not compatible with binary format");

    for (size_t i = 1; i < pages.size(); ++i)
        if (pages[0].length() != pages[i].length())
            throw std::runtime_error("texture names have different length (incompatible with bin format)");

    std::ofstream f(fileName, std::ios::binary);

#pragma pack(push, 1)
    struct InfoBlock
    {
        std::int32_t blockSize;
        std::int16_t fontSize;
        std::int8_t smooth:1;
        std::int8_t unicode:1;
        std::int8_t italic:1;
        std::int8_t bold:1;
        std::int8_t reserved:4;
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
        std::uint8_t reserved:7;
        std::uint8_t packed:1;
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
    infoBlock.smooth = info.smooth;
    infoBlock.unicode = info.unicode;
    infoBlock.italic = info.italic;
    infoBlock.bold = info.bold;
    infoBlock.reserved = 0;
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
    commonBlock.reserved = 0;
    commonBlock.alphaChnl = common.alphaChnl;
    commonBlock.redChnl = common.redChnl;
    commonBlock.greenChnl = common.greenChnl;
    commonBlock.blueChnl = common.blueChnl;

    f << '\2';
    f.write((const char*)&commonBlock, sizeof(commonBlock));

    f << '\3';
    std::int32_t pageBlockSize = pages.empty() ? 1 : (pages[0].length() + 1) * pages.size();
    f.write((const char*)&pageBlockSize, sizeof(pageBlockSize));
    if (pages.empty())
    {
        //TODO: check if we need this byte when there are no pages
        f << '\0';
    }
    else
    {
        for (const auto& s: pages)
        {
            f << s;
            f << '\0';
        }
    }

    f << '\4';
    std::int32_t charsBlockSize = chars.size() * sizeof(CharBlock);
    f.write((const char*)&charsBlockSize, sizeof(charsBlockSize));
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
        f.write((const char*)&kerningPairsBlockSize, sizeof(kerningPairsBlockSize));

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

    nlohmann::json infoNode;
    infoNode["size"] = info.size;
    infoNode["smooth"] = info.smooth ? 1 : 0;
    infoNode["unicode"] = info.unicode ? 1 : 0;
    infoNode["italic"] = info.italic ? 1 : 0;
    infoNode["bold"] = info.bold ? 1 : 0;
    infoNode["charset"] = info.unicode ? "" : getCharSetName(info.charset);
    infoNode["stretchH"] = info.stretchH;
    infoNode["aa"] = info.aa;
    infoNode["padding"] = {info.padding.up, info.padding.right, info.padding.down, info.padding.left};
    infoNode["spacing"] = {info.spacing.horizontal, info.spacing.vertical};
    infoNode["outline"] = info.outline;
    infoNode["face"] = info.face;
    if (extraInfo) {
        infoNode["style"] = info.style;
    }

    nlohmann::json commonNode;
    commonNode["lineHeight"] = common.lineHeight;
    commonNode["base"] = common.base;
    commonNode["scaleW"] = common.scaleW;
    commonNode["scaleH"] = common.scaleH;
    commonNode["pages"] = pages.size();
    commonNode["packed"] = static_cast<int>(common.packed);
    commonNode["alphaChnl"] = common.alphaChnl;
    commonNode["redChnl"] = common.redChnl;
    commonNode["greenChnl"] = common.greenChnl;
    commonNode["blueChnl"] = common.blueChnl;
    if (extraInfo)
    {
        commonNode["totalHeight"] = common.totalHeight;
        commonNode["descent"] = common.descent;
    }

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

void FontInfo::writeToCborFile(const std::string &fileName) const
{
    if (extraInfo)
        throw std::runtime_error("--extra-info flag is not compatible with cbor format");

    std::ofstream f(fileName, std::fstream::binary);
    f.exceptions(std::fstream::failbit | std::fstream::badbit);
    cbor_encoder_ostream encoder(f);

    encoder.write_indefinite_array();
    encoder.write_string("BMF");
    encoder.write_uint(3);
    encoder.write_uint(1);

    // info
    encoder.write_int(info.size);
    encoder.write_bool(info.smooth);
    encoder.write_bool(info.unicode);
    encoder.write_bool(info.italic);
    encoder.write_bool(info.bold);
    encoder.write_uint(info.unicode ? 0 : info.charset);
    encoder.write_uint(info.stretchH);
    encoder.write_int(info.aa);
    encoder.write_uint(info.padding.up);
    encoder.write_uint(info.padding.right);
    encoder.write_uint(info.padding.down);
    encoder.write_uint(info.padding.left);
    encoder.write_uint(info.spacing.horizontal);
    encoder.write_uint(info.spacing.vertical);
    encoder.write_uint(info.outline);

    // common
    encoder.write_uint(common.lineHeight);
    encoder.write_uint(common.base);
    encoder.write_uint(common.scaleW);
    encoder.write_uint(common.scaleH);
    encoder.write_uint(static_cast<std::uint16_t>(pages.size()));
    encoder.write_bool(common.packed);
    encoder.write_uint(common.alphaChnl);
    encoder.write_uint(common.redChnl);
    encoder.write_uint(common.greenChnl);
    encoder.write_uint(common.blueChnl);

    // pages
    encoder.write_array(pages.size());
    for (const auto& s: pages)
        encoder.write_string(s);

    // characters
    encoder.write_array(chars.size() * 10u);
    for (auto c: chars)
    {
        encoder.write_uint(c.id);
        encoder.write_uint(c.x);
        encoder.write_uint(c.y);
        encoder.write_uint(c.width);
        encoder.write_uint(c.height);
        encoder.write_int(c.xoffset);
        encoder.write_int(c.yoffset);
        encoder.write_int(c.xadvance);
        encoder.write_int(c.page);
        encoder.write_int(c.chnl);
    }

    // kernings
    encoder.write_array(kernings.size() * 3u);
    for (auto k: kernings)
    {
        encoder.write_uint(k.first);
        encoder.write_uint(k.second);
        encoder.write_int(k.amount);
    }

    encoder.write_break();
}
