#include "ProgramOptions.h"
#include <iostream>
#include <fstream>
#include <functional>
#include <regex>
#include "HelpException.h"
#include "external/cxxopts.hpp"
#include "external/utf8cpp/utf8.h"
#include "utils/splitStrByDelim.h"

Config ProgramOptions::parseCommandLine(int argc, char* argv[])
{
    try
    {
        Config config;
        std::string chars;
        std::string charsFile;
        std::string color;
        std::string backgroundColor;
        const std::string backgroundColorOptionName = "background-color";
        const std::string charsFileOptionName = "chars-file";
        const std::string charsOptionName = "chars";
        std::string dataFormat;

        cxxopts::Options options("fontbm", "Command line bitmap font generator, compatible with bmfont");
        //TODO: unit test to test that default values are set to config
        options.add_options()
                ("help", "produce help message")
                ("font-file", "path to ttf file, required", cxxopts::value<std::string>(config.fontFile))
                (charsOptionName, "required characters, for example: 32-64,92,120-126\ndefault value is 32-127 if 'chars-file' option is not defined", cxxopts::value<std::string>(chars))
                (charsFileOptionName, "optional path to UTF-8 text file with required characters (will be combined with 'chars' option)", cxxopts::value<std::string>(charsFile))
                ("color", "foreground RGB color, for example: 32,255,255, default value is 255,255,255", cxxopts::value<std::string>(color)->default_value("255,255,255"))
                (backgroundColorOptionName, "background color RGB color, for example: 0,0,128, transparent by default", cxxopts::value<std::string>(backgroundColor))
                ("font-size", "font size, default value is 32", cxxopts::value<uint16_t>(config.fontSize)->default_value("32"))
                ("padding-up", "padding up, default value is 0", cxxopts::value<int>(config.padding.up)->default_value("0"))
                ("padding-right", "padding right, default value is 0", cxxopts::value<int>(config.padding.right)->default_value("0"))
                ("padding-down", "padding down, default value is 0", cxxopts::value<int>(config.padding.down)->default_value("0"))
                ("padding-left", "padding left, default value is 0", cxxopts::value<int>(config.padding.left)->default_value("0"))
                ("spacing-vert", "spacing vert, default value is 0", cxxopts::value<int>(config.spacing.ver)->default_value("0"))
                ("spacing-horiz", "spacing horiz, default value is 0", cxxopts::value<int>(config.spacing.hor)->default_value("0"))
                ("texture-width", "texture width, default value is 256", cxxopts::value<uint32_t>(config.textureSize.w)->default_value("256"))
                ("texture-height", "texture height, default value is 256", cxxopts::value<uint32_t>(config.textureSize.h)->default_value("256"))
                ("output", "output files name without extension, required", cxxopts::value<std::string>(config.output))
                ("data-format", "output data file format, \"xml\" or \"txt\", default \"xml\"", cxxopts::value<std::string>(dataFormat)->default_value("txt"))
                ("include-kerning-pairs", "include kerning pairs to output file", cxxopts::value<bool>(config.includeKerningPairs));

        auto result = options.parse(argc, argv);

        if (result.count("help"))
        {
            std::cout << options.help() << std::endl;
            throw HelpException();
        }

        if (!result.count("font-file"))
            throw std::runtime_error("--font-file options required");
        if (!result.count("output"))
            throw std::runtime_error("--output options required");

        if ((config.padding.up < 0) || (config.padding.right < 0) || (config.padding.down < 0) || (config.padding.left < 0))
            throw std::runtime_error("invalid padding value");

        if ((config.spacing.ver < 0) || (config.spacing.hor < 0))
            throw std::runtime_error("invalid spacing value");

        if ((!result.count(charsOptionName)) && (!result.count(charsFileOptionName)))
            chars = "32-127";
        config.chars = parseCharsString(chars);
        if (result.count(charsFileOptionName))
            getCharsFromFile(charsFile, config.chars);

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

std::set<uint32_t> ProgramOptions::parseCharsString(std::string str)
{
    // remove whitespace characters
    str.erase(std::remove_if(str.begin(), str.end(), std::bind( std::isspace<char>, std::placeholders::_1, std::locale::classic() )), str.end());

    if (str.empty())
        return std::set<uint32_t>();

    const std::regex re("^\\d{1,7}(-\\d{1,7})?(,\\d{1,7}(-\\d{1,7})?)*$");
    if (!std::regex_match(str, re))
        throw std::logic_error("invalid chars value");

    std::vector<std::string> ranges = splitStrByDelim(str, ',');

    std::vector<std::pair<uint32_t, uint32_t>> charList;
    for (const auto& range: ranges)
    {
        std::vector<std::string> minMaxStr = splitStrByDelim(range, '-');
        if (minMaxStr.size() == 1)
            minMaxStr.push_back(minMaxStr[0]);

        unsigned long v0 = std::stoul(minMaxStr[0]);
        unsigned long v1 = std::stoul(minMaxStr[1]);

        const unsigned long maxUtf32 = 0x10FFFF;
        if ((v0 > maxUtf32) || (v1 > maxUtf32))
            throw std::out_of_range("invalid utf-32 value (out of range 0x000000..0x10ffff)");

        charList.emplace_back(static_cast<uint32_t>(v0), static_cast<uint32_t>(v1));
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

void ProgramOptions::getCharsFromFile(const std::string& fileName, std::set<uint32_t>& result)
{
    std::ifstream fs(fileName, std::ifstream::binary);
    if (!fs)
        throw std::runtime_error("can`t open characters file");
    std::string str((std::istreambuf_iterator<char>(fs)),
                    std::istreambuf_iterator<char>());

    for (std::string::iterator it = str.begin(); it != str.end(); )
        result.insert(utf8::next(it, str.end()));
}

Config::Color ProgramOptions::parseColor(const std::string& str)
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

    return Config::Color{colorToUint8(rgbStr[0]), colorToUint8(rgbStr[1]), colorToUint8(rgbStr[2])};
}
