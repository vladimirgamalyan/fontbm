#pragma once
#include <set>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include "json.hpp"
#include "CharList.h"

class ConfigFile
{
public:

    struct Color
    {
        Color() : r(0), g(0), b(0) {};
        Color( Uint8 r, Uint8 g, Uint8 b ) : r(r), g(g), b(b) {}
        Color( const std::array<Uint8, 3>& rgb)
        {
            r = rgb[ 0 ];
            g = rgb[ 1 ];
            b = rgb[ 2 ];
        }

        Uint32 getUint32(Uint8 a = 255) const
        {
            return (r + (g << 8) + (b << 16) + (a << 24));
        }

        SDL_Color getSdlColor(Uint8 a = 255) const
        {
            return SDL_Color{r, g, b, a};
        }

        void fromString(const std::string& str)
        {
            const boost::regex e("^\\s*\\d{1,3}\\s*,\\s*\\d{1,3}\\s*,\\s*\\d{1,3}\\s*$");
            if (!boost::regex_match(str, e))
                throw std::logic_error("invalid color value");

            std::vector<std::string> rgbStr;
            boost::split(rgbStr, str, boost::is_any_of(","));
            try
            {
                r = boost::lexical_cast<Uint8>(rgbStr[0]);
                g = boost::lexical_cast<Uint8>(rgbStr[1]);
                b = boost::lexical_cast<Uint8>(rgbStr[2]);
            }
            catch (boost::bad_lexical_cast &)
            {
                throw std::logic_error("incorrect color value (out of range)");
            }
        }

        Uint8 r;
        Uint8 g;
        Uint8 b;
    };

    struct Config
    {
        std::string fontFile;
        int textureWidth;
        int textureHeight;
        int fontSize;
        bool includeKerningPairs;
        int paddingUp;
        int paddingRight;
        int paddingDown;
        int paddingLeft;
        int spacingVert;
        int spacingHoriz;
        Color glyphColorRgb;
        SDL2pp::Optional<Color> glyphBackgroundColorRgb;
        std::set<Uint16> glyphCodes;
        std::string output;
        std::string dataFormat;
    };

    ConfigFile(const boost::filesystem::path& configFilePath);
    Config getConfig() const;

private:
    SDL2pp::Optional<Color> getColor( nlohmann::json& j, const std::string& key ) const;
    template<class T> T get(nlohmann::json& j, const std::string& key, const SDL2pp::Optional<T>& defaultValue = SDL2pp::NullOpt) const;

    boost::filesystem::path parentPath;
    Config config;
};
