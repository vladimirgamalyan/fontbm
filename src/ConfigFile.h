#pragma once
#include <set>
#include <boost/filesystem.hpp>
#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include "json.hpp"

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

    Config config;
};
