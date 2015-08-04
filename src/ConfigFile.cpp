#include "ConfigFile.h"
#include <set>
#include <string>

namespace fs = boost::filesystem;
using json = nlohmann::json;

ConfigFile::ConfigFile(const boost::filesystem::path &configFilePath)
{
    // https://github.com/nlohmann/json

    if (!fs::is_regular_file(configFilePath))
        throw std::runtime_error("config not found");

    std::ifstream ifs(configFilePath.generic_string(), std::ifstream::binary);
    if (!ifs)
        throw std::runtime_error("can't open config file");

    //TODO: move to https://github.com/open-source-parsers/jsoncpp (support comments).

    //TODO: output parse errors, check value's ranges.
    json j;
    j << ifs;
    config.fontFile = j["fontFile"].get<std::string>();
    config.textureWidth = jsonGetIntOptional(j, "textureWidth", 1).value_or(256);
    config.textureHeight = jsonGetIntOptional(j, "textureHeight", 1).value_or(256);
    config.fontSize = jsonGetIntOptional(j, "fontSize", 1).value_or(32);


    config.output = j["output"].is_null() ? configFilePath.stem().generic_string() : j["output"].get<std::string>();


    //TODO: Check if other type.
    config.dataFormat = j["dataFormat"].is_null() ? "xml" : j["dataFormat"].get<std::string>();
    std::transform(config.dataFormat.begin(), config.dataFormat.end(), config.dataFormat.begin(), ::tolower);

    if ((config.dataFormat != "xml") && (config.dataFormat != "txt"))
        throw std::runtime_error("unknown data file format");

    //TODO: Check for unknown keys in config.
    //TODO: Make all options optional.

    //TODO: Check if other type.
    config.includeKerningPairs = j["dataFormat"].is_null() ? false : j["includeKerningPairs"].get<bool>();

    // Add padding to glyph, affect metrics (w/h, xoffset, yoffset).
    config.paddingUp = jsonGetIntOptional(j, "paddingUp", 0).value_or(0);
    config.paddingRight = jsonGetIntOptional(j, "paddingRight", 0).value_or(0);
    config.paddingDown = jsonGetIntOptional(j, "paddingDown", 0).value_or(0);
    config.paddingLeft = jsonGetIntOptional(j, "paddingLeft", 0).value_or(0);
    // Add spaces on target texure, doesnt affect metrics.
    config.spacingVert = jsonGetIntOptional(j, "spacingVert", 0).value_or(0);
    config.spacingHoriz = jsonGetIntOptional(j, "spacingHoriz", 0).value_or(0);

    ///////////////////////////////////////



    ///////////////////////////////////////

    config.glyphColorRgb = jsonGetColorOptional(j, "color").value_or(Color(255, 255, 255));
    config.glyphBackgroundColorRgb = jsonGetColorOptional(j, "backgroundColor");

    ///////////////////////////////////////






    ///////////////////////////////////////

    json charsJson = j["chars"];
    if (charsJson.is_null())
        charsJson = {{32, 127}};

    if (!charsJson.is_array())
        throw std::runtime_error("config chars list must be an array");


    for (auto el: charsJson) {
        // Every element is number or array of two numbers.
        if (el.is_number_integer())
        {
            //TODO: check if exists.
            config.glyphCodes.insert(el.get<Uint16>());
        }
        else if (el.is_array())
        {
            //TODO: extended error report.
            if (el.size() != 2)
                throw std::runtime_error("invalid chars list");
            if (!el[0].is_number_integer())
                throw std::runtime_error("invalid chars list");
            if (!el[1].is_number_integer())
                throw std::runtime_error("invalid chars list");
            int min = el[0];
            int max = el[1];
            if ((min < 0) || (min > 65535))
                throw std::runtime_error("invalid chars list");
            if ((max < 0) || (max > 65535))
                throw std::runtime_error("invalid chars list");
            for (Uint16 i = static_cast<Uint16>(min); i <= max; ++i)
                //TODO: check if exists.
                config.glyphCodes.insert(i);
        }
        else
        {
            throw std::runtime_error("invalid chars list");
        }
    }

}

ConfigFile::Color ConfigFile::jsonGetColor( const json& j, const std::string& key )
{
    const json& k = j[key];
    if (k.is_null())
        throw std::runtime_error(key + " not found");
    if (!k.is_array())
        throw std::runtime_error("color must be an array");
    if (k.size() != 3)
        throw std::runtime_error("invalid color value");
    std::array<Uint8, 3> rgb;
    for (size_t i = 0; i < k.size(); ++i)
    {
        json c = k[i];
        if (!c.is_number_integer())
            throw std::runtime_error("invalid color value");
        int64_t ci = c;
        if ((ci < 0) || (ci > 255))
            throw std::runtime_error("invalid color value");
        rgb.at(i) = static_cast<Uint8>(ci);
    }

    return Color(rgb);
}

SDL2pp::Optional<ConfigFile::Color> ConfigFile::jsonGetColorOptional( const json& j, const std::string& key )
{
    if (j[key].is_null())
        return SDL2pp::NullOpt;
    return jsonGetColor(j, key);
}

int ConfigFile::jsonGetInt( const json& j, const std::string& key, int min, int max )
{
    const json& k = j[key];
    if (k.is_null())
        throw std::runtime_error(key + " not found");
    if (!k.is_number_integer())
        throw std::runtime_error(std::string("invalid ") + key + ", required integer number");
    int64_t result = k;
    if (result < min)
        throw std::runtime_error(std::string("invalid ") + key + " value, must be greater or equal " + std::to_string(min));
    if (result > max)
        throw std::runtime_error(std::string("invalid ") + key + " value, must be lower or equal " + std::to_string(max));
    return static_cast<int>(result);
}

SDL2pp::Optional<int> ConfigFile::jsonGetIntOptional( const json& j, const std::string& key, int min, int max )
{
    if (j[key].is_null())
        return SDL2pp::NullOpt;
    return jsonGetInt(j, key, min, max);
}

ConfigFile::Config ConfigFile::getConfig() const
{
    return config;
}

