#include "ProgramOptions.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/locale.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

Config ProgramOptions::parseCommandLine(int argc, const char* const argv[])
{
    Config config;

    std::string chars;
    fs::path charsFile;
    std::string color;
    std::string backgroundColor;
    std::string dataFormat;

    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message" )
        ("font-file,F", po::value<fs::path>(&config.fontFile)->required(), "path to ttf file, required")
        ("chars", po::value<std::string>(&chars)->default_value("32-127"), "required characters, for example: 32-64,92,120-126\ndefault value is 32-127")
        ("chars-file", po::value<fs::path>(&charsFile), "optional path to UTF-8 text file with additional required characters")
        ("color", po::value<std::string>(&color)->default_value("255,255,255"), "foreground RGB color, for example: 32,255,255, default value is 255,255,255")
        ("background-color", po::value<std::string>(&backgroundColor), "background color RGB color, for example: 0,0,128, transparent, if not exists")
        ("font-size,S", po::value<uint16_t>(&config.fontSize)->default_value(32), "font size, default value is 32")
        ("padding-up", po::value<int>(&config.padding.up)->default_value(0), "padding up, default valie is 0")
        ("padding-right", po::value<int>(&config.padding.right)->default_value(0), "padding right, default valie is 0")
        ("padding-down", po::value<int>(&config.padding.down)->default_value(0), "padding down, default valie is 0")
        ("padding-left", po::value<int>(&config.padding.left)->default_value(0), "padding left, default valie is 0")
        ("spacing-vert", po::value<int>(&config.spacing.ver)->default_value(0), "spacing vert, default valie is 0")
        ("spacing-horiz", po::value<int>(&config.spacing.hor)->default_value(0), "spacing horiz, default valie is 0")
        ("texture-width", po::value<uint32_t>(&config.textureSize.w)->default_value(256), "texture width, default valie is 256")
        ("texture-height", po::value<uint32_t>(&config.textureSize.h)->default_value(256), "texture height, default valie is 256")
        ("output,O", po::value<std::string>(&config.output)->required(), "output files name without extension, required")
        ("data-format", po::value<std::string>(&dataFormat)->default_value("txt"), "output data file format, \"xml\" or \"txt\", default \"xml\"")
        ("include-kerning-pairs", "include kerning pairs to output file");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        throw std::exception();
    }

    po::notify(vm);

    config.chars = parseCharsString(chars);
    if (!charsFile.empty())
    {
        auto c = getCharsFromFile(charsFile);
        config.chars.insert(c.begin(), c.end());
    }

    config.color = parseColor(color);
    if (backgroundColor.empty())
        config.backgroundColor = boost::none;
    else
        config.backgroundColor = parseColor(backgroundColor);

    boost::algorithm::to_lower(dataFormat);
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

    config.includeKerningPairs = (vm.count("include-kerning-pairs") > 0);

    //TODO: check values range

    return config;
}

std::set<uint32_t> ProgramOptions::parseCharsString(std::string str) const
{
    // remove whitespace characters
    str.erase(std::remove_if(str.begin(), str.end(), std::bind( std::isspace<char>, std::placeholders::_1, std::locale::classic() )), str.end());

    const boost::regex e("^\\d{1,5}(-\\d{1,5})?(,\\d{1,5}(-\\d{1,5})?)*$");
    if (!boost::regex_match(str, e))
        throw std::logic_error("invalid chars value");

    std::vector<std::string> ranges;
    boost::split(ranges, str, boost::is_any_of(","));

    std::vector<std::pair<uint32_t, uint32_t>> charList;
    for (auto range: ranges)
    {
        std::vector<std::string> minMaxStr;
        boost::split(minMaxStr, range, boost::is_any_of("-"));
        if (minMaxStr.size() == 1)
            minMaxStr.push_back(minMaxStr[0]);

        try
        {
            charList.emplace_back(boost::lexical_cast<uint16_t>(minMaxStr[0]),
                                boost::lexical_cast<uint16_t>(minMaxStr[1]));
        }
        catch(boost::bad_lexical_cast &)
        {
            throw std::logic_error("incorrect chars value (out of range)");
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

std::set<uint32_t> ProgramOptions::getCharsFromFile(const boost::filesystem::path& f) const
{
    if (!fs::is_regular_file(f))
        throw std::runtime_error("chars file not found");

    std::ifstream t(f.generic_string(), std::ifstream::binary);
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());

    std::wstring fileContent = boost::locale::conv::utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());

    std::set<uint32_t> result;
    for (wchar_t wc: fileContent)
        result.insert(static_cast<uint32_t>(wc));
    return result;
}

Config::Color ProgramOptions::parseColor(const std::string& str) const
{
    const boost::regex e("^\\s*\\d{1,3}\\s*,\\s*\\d{1,3}\\s*,\\s*\\d{1,3}\\s*$");
    if (!boost::regex_match(str, e))
        throw std::logic_error("invalid color");

    std::vector<std::string> rgbStr;
    boost::split(rgbStr, str, boost::is_any_of(","));

    auto colorToUint8 = [](const std::string& s)
    {
        int v = boost::lexical_cast<int>(s);
        if ((v < 0) || (v > 255))
            throw std::logic_error("invalid color");
        return static_cast<uint8_t>(v);
    };

    Config::Color color;
    color.r = colorToUint8(rgbStr[0]);
    color.g = colorToUint8(rgbStr[1]);
    color.b = colorToUint8(rgbStr[2]);

    return color;
}
