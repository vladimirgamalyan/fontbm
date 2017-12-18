#include "ProgramOptions.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <codecvt>
#include <functional>
#include <regex>
#include "HelpException.h"
#include "cxxopts.hpp"
#include "splitStrByDelim.h"

Config helpers::parseCommandLine(int argc, char* argv[])
{
    try
    {
        Config config;
        std::string chars;
        std::string charsFile;
        std::string color;
        std::string backgroundColor;
        const std::string backgroundColorOptionName = "background-color";
        std::string dataFormat;

        cxxopts::Options options("fontbm", "Command line bitmap font generator, compatible with bmfont");
        options.add_options()
                ("help", "produce help message")
                ("font-file", "path to ttf file, required", cxxopts::value<std::string>(config.fontFile))
                ("chars", "required characters, for example: 32-64,92,120-126\ndefault value is 32-127 if chars-file not defined", cxxopts::value<std::string>(chars))
                ("chars-file", "optional path to UTF-8 text file with required characters (will be combined with chars)", cxxopts::value<std::string>(charsFile))
                ("color", "foreground RGB color, for example: 32,255,255, default value is 255,255,255", cxxopts::value<std::string>(color)->default_value("255,255,255"))
                (backgroundColorOptionName, "background color RGB color, for example: 0,0,128, transparent by default", cxxopts::value<std::string>(backgroundColor))
                ("font-size", "font size, default value is 32", cxxopts::value<uint16_t>(config.fontSize)->default_value("32"))
                ("padding-up", "padding up, default valie is 0", cxxopts::value<int>(config.padding.up)->default_value("0"))
                ("padding-right", "padding right, default valie is 0", cxxopts::value<int>(config.padding.right)->default_value("0"))
                ("padding-down", "padding down, default valie is 0", cxxopts::value<int>(config.padding.down)->default_value("0"))
                ("padding-left", "padding left, default valie is 0", cxxopts::value<int>(config.padding.left)->default_value("0"))
                ("spacing-vert", "spacing vert, default valie is 0", cxxopts::value<int>(config.spacing.ver)->default_value("0"))
                ("spacing-horiz", "spacing horiz, default valie is 0", cxxopts::value<int>(config.spacing.hor)->default_value("0"))
                ("texture-width", "texture width, default valie is 256", cxxopts::value<uint32_t>(config.textureSize.w)->default_value("256"))
                ("texture-height", "texture height, default valie is 256", cxxopts::value<uint32_t>(config.textureSize.h)->default_value("256"))
                ("output", "output files name without extension, required", cxxopts::value<std::string>(config.output))
                ("data-format", "output data file format, \"xml\" or \"txt\", default \"xml\"", cxxopts::value<std::string>(dataFormat)->default_value("txt"))
                ("include-kerning-pairs", "include kerning pairs to output file", cxxopts::value<bool>(config.includeKerningPairs));

        auto result = options.parse(argc, argv);

        if (result.count("help"))
        {
            std::cout << options.help({"", "Group"}) << std::endl;
            throw HelpException();
        }

        if (!result.count("font-file"))
            throw std::runtime_error("--font-file options required");
        if (!result.count("output"))
            throw std::runtime_error("--output options required");

        if (chars.empty() && charsFile.empty())
            chars = "32-127";
        config.chars = parseCharsString(chars);
        if (!charsFile.empty())
        {
            auto c = getCharsFromFile(charsFile);
            config.chars.insert(c.begin(), c.end());
        }

        config.color = parseColor(color);
        config.backgroundTransparent = (result.count(backgroundColorOptionName) == 0);
        if (!config.backgroundTransparent)
            config.backgroundColor = parseColor(backgroundColor);

        std::transform(dataFormat.begin(), dataFormat.end(), dataFormat.begin(), ::tolower);

        if (dataFormat == "txt")
            config.dataFormat = Config::DataFormat::Text;
        else if (dataFormat == "xml")
            config.dataFormat = Config::DataFormat::Xml;
        else if (dataFormat == "bin")
            config.dataFormat = Config::DataFormat::Bin;
        else if (dataFormat == "json")
            config.dataFormat = Config::DataFormat::Json;
        else
            throw std::runtime_error("invalid data format");

        //TODO: check values range
        return config;
    }
    catch (const cxxopts::OptionException& e)
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        throw std::exception();
    }
}

std::set<uint32_t> helpers::parseCharsString(std::string str)
{
    // remove whitespace characters
    str.erase(std::remove_if(str.begin(), str.end(), std::bind( std::isspace<char>, std::placeholders::_1, std::locale::classic() )), str.end());

    if (str.empty())
        return std::set<uint32_t>();

    const std::regex re("^\\d{1,5}(-\\d{1,5})?(,\\d{1,5}(-\\d{1,5})?)*$");
    if (!std::regex_match(str, re))
        throw std::logic_error("invalid chars value");

    std::vector<std::string> ranges = splitStrByDelim(str, ',');

    std::vector<std::pair<uint32_t, uint32_t>> charList;
    for (auto range: ranges)
    {
        std::vector<std::string> minMaxStr = splitStrByDelim(range, '-');
        if (minMaxStr.size() == 1)
            minMaxStr.push_back(minMaxStr[0]);

        try
        {
            unsigned long v0 = std::stoul(minMaxStr[0]);
            unsigned long v1 = std::stoul(minMaxStr[1]);
            charList.emplace_back(static_cast<uint32_t>(v0), static_cast<uint32_t>(v1));
        }
        catch (std::out_of_range &)
        {
            throw std::logic_error("incorrect character value (out of range)");
        }
    }

    std::set<uint32_t> result;
    for (auto range: charList)
    {
        //TODO: check too big result
        for (uint32_t v = range.first; v < range.second; ++v)
            result.insert(v);
        result.insert(range.second);
    }

    return result;
}

std::set<uint32_t> helpers::getCharsFromFile(const std::string& f)
{
    std::ifstream fs(f, std::ifstream::binary);
    if (!fs)
        throw std::runtime_error("can`t open characters file");
    std::string str((std::istreambuf_iterator<char>(fs)),
                    std::istreambuf_iterator<char>());

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;
    std::u32string utf32str = cvt.from_bytes(str);

    std::set<uint32_t> result;
    for (auto c: utf32str)
        result.insert(static_cast<uint32_t>(c));
    return result;
}

Config::Color helpers::parseColor(const std::string& str)
{
    const std::regex e("^\\s*\\d{1,3}\\s*,\\s*\\d{1,3}\\s*,\\s*\\d{1,3}\\s*$");
    if (!std::regex_match(str, e))
        throw std::logic_error("invalid color");

    std::vector<std::string> rgbStr = splitStrByDelim(str, ',');

    auto colorToUint8 = [](const std::string& s)
    {
        unsigned long v = std::stoul(s);
        if (v > 255)
            throw std::logic_error("invalid color");
        return static_cast<uint8_t>(v);
    };

    Config::Color color;
    color.r = colorToUint8(rgbStr[0]);
    color.g = colorToUint8(rgbStr[1]);
    color.b = colorToUint8(rgbStr[2]);

    return color;
}
