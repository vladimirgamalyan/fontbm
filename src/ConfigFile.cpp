#include "ConfigFile.h"
#include <set>
#include <string>
#include <fstream>
#include "JsonCommentsInputFilter.h"

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


    //TODO: output parse errors, check value's ranges.

    boost::iostreams::filtering_istream strippingIfs;
    strippingIfs.push(JsonCommentsInputFilter());
    strippingIfs.push(ifs);

    json j;
    j << strippingIfs;

    config.fontFile = get<std::string>(j, "fontFile");
    config.textureWidth = get<int>(j, "textureWidth", 256);  //TODO: Check range.
    config.textureHeight = get<int>(j, "textureHeight", 256); //TODO: Check range.
    config.fontSize = get<int>(j, "fontSize", 32); //TODO: Check range.
    config.output = get<std::string>(j, "output", configFilePath.stem().generic_string());

    config.dataFormat = get<std::string>(j, "dataFormat", std::string("xml"));
    std::transform(config.dataFormat.begin(), config.dataFormat.end(), config.dataFormat.begin(), ::tolower);

    if ((config.dataFormat != "xml") && (config.dataFormat != "txt"))
        throw std::runtime_error("unknown data file format");


    config.includeKerningPairs = get<bool>(j, "includeKerningPairs", false);

    // Add padding to glyph, affect metrics (w/h, xoffset, yoffset).
    //TODO: Check range (padding and spacing).
    config.paddingUp = get<int>(j, "paddingUp", 0);
    config.paddingRight = get<int>(j, "paddingRight", 0);
    config.paddingDown = get<int>(j, "paddingDown", 0);
    config.paddingLeft = get<int>(j, "paddingLeft", 0);

    // Add spaces on target texture, doesn't affect metrics.
    config.spacingVert = get<int>(j, "spacingVert", 0);
    config.spacingHoriz = get<int>(j, "spacingHoriz", 0);


    config.glyphColorRgb = getColor(j, "color").value_or(Color(255, 255, 255));
    config.glyphBackgroundColorRgb = getColor(j, "backgroundColor");

    ///////////////////////////////////////

    json charsJson;
    if (j.find("chars") == j.end())
        charsJson = {{32, 127}};
    else
    {
        charsJson = j["chars"];
        j.erase("chars");
    }


    //TODO: Make message in same form as in get() method
    if ((!charsJson.is_array()) && (!charsJson.is_string()))
        throw std::runtime_error("config char list must be an array or string");


    if (charsJson.is_string())
    {
        std::string charsStr = charsJson.get<std::string>();
        for (const char& c: charsStr)
            config.glyphCodes.insert(static_cast<Uint16>(c));
    }

    if (charsJson.is_array())
    {
        for (auto el: charsJson)
        {
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

    if (j.size())
    {
        for (json::iterator it = j.begin(); it != j.end(); ++it)
        {
            std::cout << "unknown option: " << it.key() << std::endl;
        }

        throw std::runtime_error("one or more unknown options found");
    }
}

SDL2pp::Optional<ConfigFile::Color> ConfigFile::getColor( json& j, const std::string& key ) const
{
    if (j.find(key) == j.end())
        return SDL2pp::NullOpt;

    const json& k = j[key];
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

    j.erase(key);

    return Color(rgb);
}

ConfigFile::Config ConfigFile::getConfig() const
{
    return config;
}

template<class T>
T ConfigFile::get(nlohmann::json &j, const std::string &key, const SDL2pp::Optional<T>& defaultValue) const
{
    if (j.find(key) == j.end())
    {
        if (defaultValue)
            return *defaultValue;
        else
            throw std::runtime_error(std::string("required option \"") + key + "\" is not found");
    }

    try
    {
        T result = j[key].get<T>();
        j.erase(key);
        return result;
    }
    catch(std::domain_error& e)
    {
        // e.what() == type must be object, but is ...
        throw std::runtime_error(std::string("invalid \"") + key + "\" option, " + e.what());
    }
}
