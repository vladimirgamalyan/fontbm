#include "ProgramOptions.h"
#include <iostream>
#include <fstream>
#include <functional>
#include <regex>
#include <sstream>
#include <string>
#include <charconv>
#include <iostream>
#include "HelpException.h"
#include "external/cxxopts.hpp"
#include "external/utf8cpp/utf8.h"
#include "utils/splitStrByDelim.h"

//TODO: warn about unknown options

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

        cxxopts::Options options("fontbm", "Command line bitmap font generator, compatible with bmfont");
        options.add_options()
            ("help", "produce help message")
            ("font-file", "path to ttf file, required", cxxopts::value<std::string>(config.fontFile))
            ("secondary-font-file", "path to ttf file, optional", cxxopts::value<std::string>(config.secondaryFontFile))
            (charsOptionName, "required characters, for example: 32-64,92,120-126\ndefault value is 32-126 if 'chars-file' option is not defined", cxxopts::value<std::string>(chars))
            (charsFileOptionName, "optional path to UTF-8 text file with required characters (will be combined with 'chars' option)", cxxopts::value<std::vector<std::string>>(charsFile))
            ("color", "foreground RGB color, for example: 32,255,255, default value is 255,255,255", cxxopts::value<std::string>(color)->default_value("255,255,255"))
            (backgroundColorOptionName, "background color RGB color, for example: 0,0,128, transparent by default", cxxopts::value<std::string>(backgroundColor))
            ("font-size", "font size, default value is 32", cxxopts::value<std::uint16_t>(config.fontSize)->default_value("32"))
            ("padding-up", "padding up, default value is 0", cxxopts::value<std::uint32_t>(config.padding.up)->default_value("0"))
            ("padding-right", "padding right, default value is 0", cxxopts::value<std::uint32_t>(config.padding.right)->default_value("0"))
            ("padding-down", "padding down, default value is 0", cxxopts::value<std::uint32_t>(config.padding.down)->default_value("0"))
            ("padding-left", "padding left, default value is 0", cxxopts::value<std::uint32_t>(config.padding.left)->default_value("0"))
            ("spacing-vert", "spacing vert, default value is 0", cxxopts::value<std::uint32_t>(config.spacing.ver)->default_value("0"))
            ("spacing-horiz", "spacing horiz, default value is 0", cxxopts::value<std::uint32_t>(config.spacing.hor)->default_value("0"))
            ("output", "output files name without extension, required", cxxopts::value<std::string>(config.output))
            ("data-format", R"(output data file format: "txt", "xml", "json", "bin", "cbor", default: "txt")", cxxopts::value<std::string>(dataFormat)->default_value("txt"))
            ("kerning-pairs", R"("generate kerning pairs: "disabled", "basic", "regular" (tuned by hinter), "extended" (bigger output size, but more precise), default: "disabled")", cxxopts::value<std::string>(kerningPairs)->default_value("disabled"))
            ("all-chars", "retrieve all characters from font", cxxopts::value<bool>(config.allChars))
            ("monochrome", "disable anti-aliasing", cxxopts::value<bool>(config.monochrome))
            ("light-hinting", "use a lighter hinting algorithm", cxxopts::value<bool>(config.lightHinting))
            ("no-hinting", "disable hinting completely", cxxopts::value<bool>(config.noHinting))
            ("tabular-numbers", "enables non-proportional numbers", cxxopts::value<bool>(config.tabularNumbers))
            ("slashed-zero", "enables slashed zero", cxxopts::value<bool>(config.slashedZero))
            ("extra-info", "write extra information to data file", cxxopts::value<bool>(config.extraInfo))
            (textureSizeListOptionName, "list of texture sizes (will be tried from left to right to fit glyphs)", cxxopts::value<std::string>(textureSizeList))
            ("texture-crop-width", "crop unused parts of output textures (width)", cxxopts::value<bool>(config.cropTexturesWidth))
            ("texture-crop-height", "crop unused parts of output textures (height)", cxxopts::value<bool>(config.cropTexturesHeight))
            ("align-horiz", "align glyph horizontal position", cxxopts::value<std::uint32_t>(config.alignment.hor))
            ("align-vert", "align glyph vertical position", cxxopts::value<std::uint32_t>(config.alignment.ver))
            ("verbose", "verbose output", cxxopts::value<bool>(config.verbose))
            ("max-texture-count", "maximum generated textures", cxxopts::value<std::uint32_t>(config.maxTextureCount))
            ("texture-name-suffix", R"(texture name suffix: "index_aligned", "index", "none", default: "index_aligned")", cxxopts::value<std::string>(textureNameSuffix)->default_value("index_aligned"))
            ;

        auto result = options.parse(argc, argv);

        if (result.count("help"))
        {
            std::cout << options.help() << std::endl;
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
        else if (dataFormat == "cbor")
            config.dataFormat = Config::DataFormat::Cbor;
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

        if (result.count(textureSizeListOptionName))
            config.textureSizeList = parseTextureSize(textureSizeList);
        else
            config.textureSizeList = {
                {32, 32},
                {64, 32},
                {64, 64},
                {128, 64},
                {128, 128},
                {256, 128},
                {256, 256},
                {512, 256},
                {512, 512},
                {1024, 512},
                {1024, 1024},
                {2048, 1024},
                {2048, 2048},
                {4096, 2048},
                {4096, 4096},
                {8192, 4096},
                {8192, 8192},
        };

        if (!config.alignment.hor)
            throw std::runtime_error("invalid --align-horiz");
        if (!config.alignment.ver)
            throw std::runtime_error("invalid --align-vert");

        return config;
    }
    catch (const cxxopts::OptionException& e)
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        throw std::exception();
    }
}

std::set<std::uint32_t> ProgramOptions::parseCharsString(std::string str)
{
    str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());

    if (str.empty())
        return std::set<std::uint32_t>();

    std::set<uint32_t> result;

    std::istringstream ss(str);
    std::string segment;
    while (std::getline(ss, segment, ',')) {
        size_t dash_pos = segment.find('-');
        
        auto parse_value = [](const std::string& s) {
            uint32_t val = 0;
            std::from_chars_result res;
            if (s.starts_with("0x") || s.starts_with("0X"))
                res = std::from_chars(s.data() + 2, s.data() + s.size(), val, 16);
            else
                res = std::from_chars(s.data(), s.data() + s.size(), val, 10);
            return val;
        };

        const auto maxUtf32 = 0x10FFFFul;
        if (dash_pos != std::string::npos) {
            uint32_t start = parse_value(segment.substr(0, dash_pos));
            uint32_t end = parse_value(segment.substr(dash_pos + 1));
            if (start > maxUtf32 || end > maxUtf32)
                throw std::out_of_range("invalid utf-32 value (out of range 0x000000..0x10ffff)");
            for (uint32_t i = start; i <= end; ++i) {
                result.insert(i);
            }
        } else {
            uint32_t v = parse_value(segment);
            if (v > maxUtf32)
                throw std::out_of_range("invalid utf-32 value (out of range 0x000000..0x10ffff)");
            result.insert(v);
        }
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
