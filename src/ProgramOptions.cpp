#include "ProgramOptions.h"
#include <iostream>
#include <fstream>
#include <functional>
#include <regex>
#include "HelpException.h"
#include "external/cxxopts.hpp"
#include "external/utf8cpp/utf8.h"
#include "utils/splitStrByDelim.h"

//TODO: warn about unknown options

namespace {

const char* const defaultTextureSizeList =
    "32x32,64x32,64x64,128x64,128x128,256x128,256x256,512x256,512x512,"
    "1024x512,1024x1024,2048x1024,2048x2048";

std::string makeHelpEpilog()
{
    return std::string(R"(
 Produced files:
      <output>.fnt              font metrics, always with the .fnt extension,
                                whatever --data-format is used
      <output>_0.png            one or more texture pages, see
                                --texture-name-suffix

 Default --texture-size value:
      )") + defaultTextureSizeList + R"(

 Examples:
      fontbm --font-file FreeSans.ttf --output myfont
      fontbm --font-file FreeSans.ttf --output myfont --font-size 24 --data-format json
      fontbm --font-file FreeSans.ttf --output myfont --chars 32-126,1040-1103 --kerning-pairs regular
      fontbm --font-file FreeSans.ttf --output myfont --color 255,200,0 --background-color 0,0,0

 Exit code is 0 on success, 1 on error.
 The .fnt format is described at https://www.angelcode.com/products/bmfont/doc/file_format.html)";
}

}

Config ProgramOptions::parseCommandLine(int argc, char* argv[])
{
    try
    {
        Config config;
        std::string chars;
        std::vector<std::string> charsFile;
        std::string color;
        std::string textureSizeList;
        std::string backgroundColor;
        const std::string backgroundColorOptionName = "background-color";
        const std::string charsFileOptionName = "chars-file";
        const std::string charsOptionName = "chars";
        const std::string textureSizeListOptionName = "texture-size";
        std::string dataFormat;
        std::string kerningPairs;
        std::string textureNameSuffix;

        cxxopts::Options options("fontbm",
            "Renders a TrueType/OpenType font into a bitmap font (texture pages plus\n"
            "metrics), compatible with AngelCode BMFont.");
        options.custom_help("--font-file <path> --output <name> [OPTION...]");

        options.add_options()
            ("help", "print this help and exit")
            ("verbose", "print the FreeType version being used", cxxopts::value<bool>(config.verbose))
            ;

        options.add_options("Font")
            ("font-file", "font to render, .ttf or .otf (required)", cxxopts::value<std::string>(config.fontFile), "<path>")
            ("font-size", "character height in pixels (same meaning as the BMFont size value with \"Match char height\" ticked)", cxxopts::value<std::uint16_t>(config.fontSize)->default_value("32"), "<px>")
            (charsOptionName, "characters to render, as decimal Unicode code points: single values and/or first-last ranges, comma separated, spaces ignored, for example: 32-64,92,120-126 (default: 32-126, unless --chars-file is given)", cxxopts::value<std::string>(chars), "<ranges>")
            (charsFileOptionName, "UTF-8 text file, every character occurring in it is added to --chars; may be given several times", cxxopts::value<std::vector<std::string>>(charsFile), "<path>")
            ;

        options.add_options("Output")
            ("output", "output name without extension (required)", cxxopts::value<std::string>(config.output), "<name>")
            ("data-format", "format of the .fnt file: txt, xml, json, bin", cxxopts::value<std::string>(dataFormat)->default_value("txt"), "<format>")
            ("texture-name-suffix", "how the page number is added to the .png name: index_aligned (myfont_00.png, zero padded to the highest page number), index (myfont_0.png), none (myfont.png, single page only)", cxxopts::value<std::string>(textureNameSuffix)->default_value("index_aligned"), "<mode>")
            ;

        options.add_options("Appearance")
            ("color", "glyph color, decimal R,G,B in range 0-255, for example: 32,255,255", cxxopts::value<std::string>(color)->default_value("255,255,255"), "<r,g,b>")
            (backgroundColorOptionName, "background color, decimal R,G,B in range 0-255, for example: 0,0,128; the background is transparent if this option is omitted", cxxopts::value<std::string>(backgroundColor), "<r,g,b>")
            ("monochrome", "disable anti-aliasing", cxxopts::value<bool>(config.monochrome))
            ("kerning-pairs", "kerning pairs to write into the .fnt file: disabled, basic, regular (tuned by hinter), extended (more precise, bigger output)", cxxopts::value<std::string>(kerningPairs)->default_value("disabled"), "<mode>")
            ;

        options.add_options("Texture")
            (textureSizeListOptionName, "allowed page sizes, <width>x<height> comma separated; tried from left to right, the first one all glyphs fit into is used; the default list is printed below", cxxopts::value<std::string>(textureSizeList), "<sizes>")
            ("texture-crop-width", "shrink every page to the rightmost used pixel", cxxopts::value<bool>(config.cropTexturesWidth))
            ("texture-crop-height", "shrink every page to the lowest used pixel", cxxopts::value<bool>(config.cropTexturesHeight))
            ("max-texture-count", "fail if more pages than this are needed (unlimited if omitted)", cxxopts::value<std::uint32_t>(config.maxTextureCount), "<n>")
            ;

        options.add_options("Glyph layout")
            ("padding-up", "pixels added above each glyph, inside its rectangle", cxxopts::value<std::uint32_t>(config.padding.up)->default_value("0"), "<px>")
            ("padding-right", "pixels added right of each glyph, inside its rectangle", cxxopts::value<std::uint32_t>(config.padding.right)->default_value("0"), "<px>")
            ("padding-down", "pixels added below each glyph, inside its rectangle", cxxopts::value<std::uint32_t>(config.padding.down)->default_value("0"), "<px>")
            ("padding-left", "pixels added left of each glyph, inside its rectangle", cxxopts::value<std::uint32_t>(config.padding.left)->default_value("0"), "<px>")
            ("spacing-vert", "pixels left between glyph rectangles, vertically", cxxopts::value<std::uint32_t>(config.spacing.ver)->default_value("0"), "<px>")
            ("spacing-horiz", "pixels left between glyph rectangles, horizontally", cxxopts::value<std::uint32_t>(config.spacing.hor)->default_value("0"), "<px>")
            ("align-horiz", "round glyph rectangle width up to a multiple of this, must be greater than 0", cxxopts::value<std::uint32_t>(config.alignment.hor)->default_value("1"), "<px>")
            ("align-vert", "round glyph rectangle height up to a multiple of this, must be greater than 0", cxxopts::value<std::uint32_t>(config.alignment.ver)->default_value("1"), "<px>")
            ;

        auto result = options.parse(argc, argv);

        if (result.count("help"))
        {
            std::cout << options.help({"", "Font", "Output", "Appearance", "Texture", "Glyph layout"})
                      << makeHelpEpilog() << std::endl;
            throw HelpException();
        }

        if (!result.count("font-file"))
            throw std::runtime_error("--font-file required");
        if (!result.count("output"))
            throw std::runtime_error("--output required");

        config.useMaxTextureCount = result.count("max-texture-count");

        if (!result.count(charsOptionName) && !result.count(charsFileOptionName))
            chars = "32-126";
        config.chars = parseCharsString(chars);
        if (result.count(charsFileOptionName))
            for (const auto& f : charsFile)
                getCharsFromFile(f, config.chars);

        config.color = parseColor(color);
        config.backgroundTransparent = result.count(backgroundColorOptionName) == 0;
        if (!config.backgroundTransparent)
            config.backgroundColor = parseColor(backgroundColor);

        std::transform(dataFormat.begin(), dataFormat.end(), dataFormat.begin(), tolower);

        if (dataFormat == "txt")
            config.dataFormat = Config::DataFormat::Text;
        else if (dataFormat == "xml")
            config.dataFormat = Config::DataFormat::Xml;
        else if (dataFormat == "bin")
            config.dataFormat = Config::DataFormat::Bin;
        else if (dataFormat == "json")
            config.dataFormat = Config::DataFormat::Json;
        else
            throw std::runtime_error("unknown --data-format value");

        std::transform(kerningPairs.begin(), kerningPairs.end(), kerningPairs.begin(), tolower);
        if (kerningPairs == "disabled")
            config.kerningPairs = Config::KerningPairs::Disabled;
        else if (kerningPairs == "basic")
            config.kerningPairs = Config::KerningPairs::Basic;
        else if (kerningPairs == "regular")
            config.kerningPairs = Config::KerningPairs::Regular;
        else if (kerningPairs == "extended")
            config.kerningPairs = Config::KerningPairs::Extended;
        else
            throw std::runtime_error("unknown --kerning-pairs value");

        if (textureNameSuffix == "index_aligned")
            config.textureNameSuffix = Config::TextureNameSuffix::IndexAligned;
        else if (textureNameSuffix == "index")
            config.textureNameSuffix = Config::TextureNameSuffix::Index;
        else if (textureNameSuffix == "none")
            config.textureNameSuffix = Config::TextureNameSuffix::None;
        else
            throw std::runtime_error("unknown --texture-name-suffix value");

        config.textureSizeList = parseTextureSize(
                result.count(textureSizeListOptionName) ? textureSizeList : defaultTextureSizeList);

        if (!config.alignment.hor)
            throw std::runtime_error("invalid --align-horiz");
        if (!config.alignment.ver)
            throw std::runtime_error("invalid --align-vert");

        return config;
    }
    catch (const cxxopts::OptionException& e)
    {
        throw std::runtime_error(std::string("error parsing options: ") + e.what());
    }
}

std::set<std::uint32_t> ProgramOptions::parseCharsString(std::string str)
{
    str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());

    if (str.empty())
        return std::set<std::uint32_t>();

    const std::regex re(R"(^\d{1,7}(-\d{1,7})?(,\d{1,7}(-\d{1,7})?)*$)");
    if (!std::regex_match(str, re))
        throw std::logic_error("invalid chars value");

    const auto ranges = splitStrByDelim(str, ',');

    std::vector<std::pair<std::uint32_t, std::uint32_t>> charList;
    for (const auto& range : ranges)
    {
        auto minMaxStr = splitStrByDelim(range, '-');
        if (minMaxStr.size() == 1)
            minMaxStr.push_back(minMaxStr[0]);

        const auto v0 = std::stoul(minMaxStr[0]);
        const auto v1 = std::stoul(minMaxStr[1]);

        const auto maxUtf32 = 0x10FFFFul;
        if (v0 > maxUtf32 || v1 > maxUtf32)
            throw std::out_of_range("invalid utf-32 value (out of range 0x000000..0x10ffff)");

        charList.emplace_back(static_cast<std::uint32_t>(v0), static_cast<std::uint32_t>(v1));
    }

    std::set<std::uint32_t> result;
    for (const auto& range : charList)
    {
        for (auto v = range.first; v < range.second; ++v)
            result.insert(v);
        result.insert(range.second);
    }

    return result;
}

void ProgramOptions::getCharsFromFile(const std::string& fileName, std::set<std::uint32_t>& result)
{
    std::ifstream fs(fileName, std::ifstream::binary);
    if (!fs)
        throw std::runtime_error("can't open characters file");
    std::string str((std::istreambuf_iterator<char>(fs)),
                    std::istreambuf_iterator<char>());

    for (auto it = str.begin(); it != str.end();)
        result.insert(utf8::next(it, str.end()));
}

Config::Color ProgramOptions::parseColor(const std::string& str)
{
    static const std::regex e(R"(^\s*\d{1,3}\s*,\s*\d{1,3}\s*,\s*\d{1,3}\s*$)");
    if (!std::regex_match(str, e))
        throw std::logic_error("invalid color");

    const auto rgbStr = splitStrByDelim(str, ',');

    const auto colorToUint8 = [](const std::string& s)
    {
        const auto v = std::stoul(s);
        if (v > 255)
            throw std::logic_error("invalid color");
        return static_cast<std::uint8_t>(v);
    };

    return Config::Color{colorToUint8(rgbStr[0]), colorToUint8(rgbStr[1]), colorToUint8(rgbStr[2])};
}

std::vector<Config::Size> ProgramOptions::parseTextureSize(const std::string& s)
{
    std::vector<Config::Size> result;

    try
    {
        for (const auto& p: string_split(s, ",", false))
        {
            auto ss = string_split(p, "x", false);
            if (ss.size() != 2)
                throw std::exception();

            static const std::regex e(R"(^[1-9]\d{0,9}$)");     // only integers > 0, no more than 10 digits (enough for 4294967295 - max uint32_t)
            if (!std::regex_match(ss[0], e))
                throw std::exception();
            if (!std::regex_match(ss[1], e))
                throw std::exception();

            Config::Size size;
            size.w = std::stoul(ss[0]);
            size.h = std::stoul(ss[1]);
            if (size.w  > 65536 || size.h > 65536)
                throw std::exception();
            result.emplace_back(size);
        }

        if (result.empty())
            throw std::exception();
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("invalid texture size argument");
    }

    return result;
}
