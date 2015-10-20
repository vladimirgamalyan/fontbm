#include <stdexcept>
#include <fstream>
#include "FontInfo.h"
#include "tinyxml2/tinyxml2.h"

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
    infoElement->SetAttribute("bold", info.bold);
    infoElement->SetAttribute("italic", info.italic);
    infoElement->SetAttribute("charset", info.charset);
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
    commonElement->SetAttribute("pages", common.pages);
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
    std::ofstream f(fileName);

    f << "info"
        << " face=\"" << info.face << "\""
        << " size=" << info.size
        << " bold=" << info.bold
        << " italic=" << info.italic
        << " charset=\"" << static_cast<int>(info.charset) << "\""
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
        << " pages=" << common.pages
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
            << " page=" << c.page
            << " chnl=" << c.chnl
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
    if (!pages.empty())
    {
        size_t l = pages[0].length();
        if (!l)
            throw std::runtime_error("page name is empty");
        for (size_t i = 1; i < pages.size(); ++i)
            if (l != pages[i].length())
                throw std::runtime_error("page names have different length");
    }

    if (common.pages != pages.size())
        throw std::runtime_error("common.pages != pages.size()");

    std::fstream f(fileName, std::ios::binary);

#pragma pack(push)
#pragma pack(1)
    struct InfoBlock
    {
        int32_t blockSize;
        uint16_t fontSize;
        int8_t reserved:4;
        int8_t bold:1;
        int8_t italic:1;
        int8_t unicode:1;
        int8_t smooth:1;
        uint8_t charSet;
        uint16_t stretchH;
        int8_t aa;
        uint8_t paddingUp;
        uint8_t paddingRight;
        uint8_t paddingDown;
        uint8_t paddingLeft;
        uint8_t spacingHoriz;
        uint8_t spacingVert;
        uint8_t outline;
        int8_t fontName[1];
    };

    struct CommonBlock
    {
        int32_t blockSize;
        uint16_t lineHeight;
        uint16_t base;
        uint16_t scaleW;
        uint16_t scaleH;
        uint16_t pages;
        uint8_t packed:1;
        uint8_t reserved:7;
        uint8_t alphaChnl;
        uint8_t redChnl;
        uint8_t greenChnl;
        uint8_t blueChnl;
    };

    struct CharBlock
    {
        uint32_t id;
        uint16_t x;
        uint16_t y;
        uint16_t width;
        uint16_t height;
        int16_t xoffset;
        int16_t yoffset;
        int16_t xadvance;
        int8_t page;
        int8_t channel;
    };

    struct KerningPairsBlock
    {
        uint32_t first;
        uint32_t second;
        int16_t amount;
    };
#pragma pack(pop)

    f << "BMF";
    f << '\3';

    InfoBlock infoBlock;
    infoBlock.blockSize = sizeof(InfoBlock) - sizeof(InfoBlock::blockSize) + info.face.length();
    infoBlock.fontSize = info.size;
    infoBlock.bold = info.bold;
    infoBlock.italic = info.italic;
    infoBlock.unicode = info.unicode;
    infoBlock.smooth = info.smooth;
    infoBlock.charSet = info.charset;
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
    commonBlock.pages = common.pages;
    commonBlock.packed = common.packed;
    commonBlock.alphaChnl = common.alphaChnl;
    commonBlock.redChnl = common.redChnl;
    commonBlock.greenChnl = common.greenChnl;
    commonBlock.blueChnl = common.blueChnl;

    f << '\2';
    f.write((const char*)&commonBlock, sizeof(commonBlock));

    f << '\3';
    int32_t pageBlockSize = pages.empty() ? 0 : pages[0].length() * pages.size();
    f << pageBlockSize;
    for (auto s: pages)
        f << s;

    f << '\4';
    int32_t charsBlockSize = chars.size() * sizeof(CharBlock);
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
        int32_t kerningPairsBlockSize = kernings.size() * sizeof(KerningPairsBlock);
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
