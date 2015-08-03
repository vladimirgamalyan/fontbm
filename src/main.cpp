#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <array>
#include <limits>
#include <algorithm>
#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "sdlSavePng/savepng.h"
#include "Font.h"
#include "maxRectsBinPack/MaxRectsBinPack.h"
#include "json.hpp"


namespace po = boost::program_options;
namespace fs = boost::filesystem;
using json = nlohmann::json;


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

int getKerning(const SDL2pp::Font& font, Uint16 ch0, Uint16 ch1) {
    Uint16 text[ 3 ] = {ch0, ch1, 0};
    return font.GetSizeUNICODE(text).x - ( font.GetGlyphAdvance(ch0) + font.GetGlyphAdvance(ch1) );
}

void printGlyphData(const SDL2pp::Font& font, Uint16 ch) {
    int minx, maxx, miny, maxy, advance;
    font.GetGlyphMetrics(ch, minx, maxx, miny, maxy, advance);
    std::cout << "minx=" << minx
        << ", maxx=" << maxx
        << ", miny=" << miny
        << ", maxy=" << maxy
        << ", advance: " << advance
        << std::endl;
}

struct GlyphInfo
{
    GlyphInfo() {}
    GlyphInfo(Uint16 code) : code(code) {}

    Uint16 code;

    int page;

    int x;
    int y;
    int w;
    int h;

    int minx;
    int maxx;
    int miny;
    int maxy;
    int advance;
};

void getSrcRects(const std::map<Uint16, GlyphInfo> &glyphs, int additionalWidth, int additionalHeight, std::vector<rbp::RectSize> &srcRects)
{
    srcRects.clear();
    for (auto& kv : glyphs)
    {
        const GlyphInfo& glyphInfo = kv.second;
        bool empty = (glyphInfo.w == 0) && (glyphInfo.h == 0);
        if (!empty)
        {
            rbp::RectSize rs;
            rs.width = glyphInfo.w + additionalHeight;
            rs.height = glyphInfo.h + additionalWidth;
            rs.tag = glyphInfo.code;
            srcRects.push_back(rs);
        }
    }
}

void checkGlyphSize(const std::map<Uint16, GlyphInfo>& glyphs, int maxTextureSizeX, int maxTextureSizeY)
{
    for (auto& kv : glyphs)
    {
        const GlyphInfo &glyphInfo = kv.second;
        if ( (glyphInfo.w > maxTextureSizeX) || (glyphInfo.h > maxTextureSizeY))
            throw std::runtime_error("no room for glyph");
    }
}

void collectGlyphInfo(const SDL2pp::Font& font, const std::set<Uint16>& codes, std::map<Uint16, GlyphInfo>& glyphs )
{
    int fontAscent = font.GetAscent();

    glyphs.clear();

    for (auto& id : codes)
    {
        if ( !font.IsGlyphProvided(id) )
            continue;

        GlyphInfo glyphInfo(id);
        font.GetGlyphMetrics(id, glyphInfo.minx, glyphInfo.maxx, glyphInfo.miny, glyphInfo.maxy, glyphInfo.advance);

        //glyphInfo.id = id;
        glyphInfo.x = 0;
        glyphInfo.y = 0;
        glyphInfo.w = glyphInfo.maxx - glyphInfo.minx;
        glyphInfo.h = glyphInfo.maxy - glyphInfo.miny;

        if (fontAscent < glyphInfo.maxy)
            throw std::runtime_error("invalid glyph (maxy > ascent)");

        //TODO: add more cheks for glyph.



        bool empty = (glyphInfo.w == 0) && (glyphInfo.h == 0);
        if (!empty)
            if ((glyphInfo.w <= 0) || (glyphInfo.h <= 0))
                throw std::runtime_error("invalid glyph (zero or negative width or height)");

        //TODO: emplace.
        glyphs[id] = glyphInfo;
    }
}

Color jsonGetColor( const json& j, const std::string& key )
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

SDL2pp::Optional<Color> jsonGetColorOptional( const json& j, const std::string& key )
{
    if (j[key].is_null())
        return SDL2pp::NullOpt;
    return jsonGetColor(j, key);
}

int jsonGetInt( const json& j, const std::string& key, int min = std::numeric_limits<int>::min(), int max = std::numeric_limits<int>::max() )
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

SDL2pp::Optional<int> jsonGetIntOptional( const json& j, const std::string& key, int min = std::numeric_limits<int>::min(), int max = std::numeric_limits<int>::max() )
{
    if (j[key].is_null())
        return SDL2pp::NullOpt;
    return jsonGetInt(j, key, min, max);
}

int main(int argc, char** argv) try {

    po::options_description desc( "Allowed options" );
    fs::path configPath;
    desc.add_options()
            ( "help", "produce help message" )
            ( "config", po::value< fs::path >( &configPath)->required(), "config file" );
    po::variables_map vm;
    po::store( po::parse_command_line( argc, argv, desc ), vm );

    if ( vm.count( "help" ) )
    {
        std::cout << desc << std::endl;
        return 1;
    }

    po::notify( vm );

    ///////////////////////////////////////

    // https://github.com/nlohmann/json

    if (!fs::is_regular_file(configPath))
        throw std::runtime_error("config not found");

    std::ifstream ifs(configPath.generic_string(), std::ifstream::binary);
    if (!ifs)
        throw std::runtime_error("can't open config file");

    //TODO: move to https://github.com/open-source-parsers/jsoncpp (support comments).

    //TODO: output parse errors, check value's ranges.
    json j;
    j << ifs;
    const std::string fontFile = j["fontFile"];
    const int textureWidth = jsonGetIntOptional(j, "textureWidth", 1).value_or(256);
    const int textureHeight = jsonGetIntOptional(j, "textureHeight", 1).value_or(256);
    const int fontSize = jsonGetIntOptional(j, "fontSize", 1).value_or(32);
    //const std::string textureFile = j["textureFile"];
    //const std::string dataFile = j["dataFile"];

    const std::string output = j["output"];
    std::string textureFile = output + ".png";
    std::string dataFile = output + ".fnt";

    //TODO: Check if other type.
    std::string dataFormat = j["dataFormat"].is_null() ? "xml" : j["dataFormat"];
    std::transform(dataFormat.begin(), dataFormat.end(), dataFormat.begin(), ::tolower);

    if ((dataFormat != "xml") && (dataFormat != "txt"))
        throw std::runtime_error("unknown data file format");

    //TODO: Check for unknown keys in config.
    //TODO: Make all options optional.

//    std::cout << "fontFile: " << fontFile << std::endl;
//    std::cout << "textureWidth: " << textureWidth << std::endl;
//    std::cout << "textureHeight: " << textureHeight << std::endl;
//    std::cout << "fontSize: " << fontSize << std::endl;
//    std::cout << "textureFile: " << textureFile << std::endl;
//    std::cout << "dataFile: " << dataFile << std::endl;
//    std::cout << std::endl;

    //TODO: Check if other type.
    bool includeKerningPairs = j["dataFormat"].is_null() ? false : j["includeKerningPairs"].get<bool>();

    // Add padding to glyph, affect metrics (w/h, xoffset, yoffset).
    int paddingUp = jsonGetIntOptional(j, "paddingUp", 0).value_or(0);
    int paddingRight = jsonGetIntOptional(j, "paddingRight", 0).value_or(0);
    int paddingDown = jsonGetIntOptional(j, "paddingDown", 0).value_or(0);
    int paddingLeft = jsonGetIntOptional(j, "paddingLeft", 0).value_or(0);
    // Add spaces on target texure, doesnt affect metrics.
    int spacingVert = jsonGetIntOptional(j, "spacingVert", 0).value_or(0);
    int spacingHoriz = jsonGetIntOptional(j, "spacingHoriz", 0).value_or(0);

//    std::cout << "paddingUp: " << paddingUp << std::endl;
//    std::cout << "paddingRight: " << paddingRight << std::endl;
//    std::cout << "paddingDown: " << paddingDown << std::endl;
//    std::cout << "paddingLeft: " << paddingLeft << std::endl;
//    std::cout << "spacingVert: " << spacingVert << std::endl;
//    std::cout << "spacingHoriz: " << spacingHoriz << std::endl;

    ///////////////////////////////////////



    ///////////////////////////////////////

    Color glyphColorRgb = jsonGetColorOptional(j, "color").value_or(Color(255, 255, 255));
    SDL2pp::Optional<Color> glyphBackgroundColorRgb = jsonGetColorOptional(j, "backgroundColor");

    ///////////////////////////////////////

    //TODO: create directory only if there is no problem (exceptions), good place is before write outputs.
    fs::path fontFilePath(fontFile);
    if (!fontFilePath.is_absolute())
        fontFilePath = configPath.parent_path() / fontFilePath;
    if (!fs::is_regular_file(fontFilePath))
        throw std::runtime_error("font file not found");
    //std::cout << fontFilePath << std::endl;

    fs::path textureFilePath(textureFile);
    if (!textureFilePath.is_absolute())
        textureFilePath = configPath.parent_path() / textureFilePath;
    fs::create_directory(textureFilePath.parent_path());
    //std::cout << textureFilePath << std::endl;

    fs::path fntFilePath(dataFile);
    if (!fntFilePath.is_absolute())
        fntFilePath = configPath.parent_path() / fntFilePath;
    fs::create_directory(fntFilePath.parent_path());
    //std::cout << fntFilePath << std::endl;

    ///////////////////////////////////////

    json charsJson = j["chars"];
    if (charsJson.is_null())
        charsJson = {{32, 127}};

    if (!charsJson.is_array())
        throw std::runtime_error("config chars list must be an array");
    std::set<Uint16> glyphCodes;

    for (auto el: charsJson) {
        // Every element is number or array of two numbers.
        if (el.is_number_integer())
        {
            //TODO: check if exists.
            glyphCodes.insert(el.get<Uint16>());
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
                glyphCodes.insert(i);
        }
        else
        {
            throw std::runtime_error("invalid chars list");
        }
    }


    ///////////////////////////////////////

    SDL2pp::SDLTTF ttf;
    SDL2pp::Font font(fontFilePath.generic_string(), fontSize);


    int fontAscent = font.GetAscent();

    std::map<Uint16, GlyphInfo> glyphs;
    collectGlyphInfo(font, glyphCodes, glyphs);
    checkGlyphSize(glyphs, textureWidth, textureHeight);

    std::vector< rbp::RectSize > srcRects;
    getSrcRects(glyphs, spacingHoriz + paddingLeft + paddingRight,
                spacingVert + paddingUp + paddingDown, srcRects);

    rbp::MaxRectsBinPack mrbp;
    int pageCount = 0;
    for (;;)
    {
        //TODO: check if negative dimension.
        mrbp.Init(textureWidth - spacingHoriz, textureHeight - spacingVert);

        std::vector<rbp::Rect> readyRects;
        mrbp.Insert( srcRects, readyRects, rbp::MaxRectsBinPack::RectBestAreaFit );
        if ( readyRects.empty() )
        {
            if ( !srcRects.empty() )
                throw std::runtime_error("can not fit glyphs to texture");
            break;
        }

        for ( auto r: readyRects )
        {
            glyphs[r.tag].x = r.x + spacingHoriz;
            glyphs[r.tag].y = r.y + spacingVert;
            glyphs[r.tag].page = pageCount;
        }

        pageCount++;
    }

    /////////////////////////////////////////////////////////////

    std::vector<std::string> pageNames;

    for (int page = 0; page < pageCount; ++page)
    {
        //TODO: use real texture size instead max.
        SDL2pp::Surface outputSurface(0, textureWidth, textureHeight, 32,
                                      0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        //std::cout << "outputSurface blend mode " << outputSurface.GetBlendMode() << std::endl;
        // SDL_BLENDMODE_BLEND = 1 (alpha).

        // If the color value contains an alpha component then the destination is simply
        // filled with that alpha information, no blending takes place.
        if (glyphBackgroundColorRgb)
            outputSurface.FillRect(SDL2pp::NullOpt, glyphBackgroundColorRgb->getUint32(255));
        else
            outputSurface.FillRect(SDL2pp::NullOpt, glyphColorRgb.getUint32(0));

        for ( auto glyphIterator = glyphs.begin(); glyphIterator != glyphs.end(); ++glyphIterator )
        {
            const GlyphInfo& glyph = glyphIterator->second;
            if (glyph.page != page)
                continue;

            SDL2pp::Surface glyphSurface = font.RenderGlyph_Blended(glyph.code, glyphColorRgb.getSdlColor() );
            //std::cout << "blend mode " << glyphSurface.GetBlendMode() << std::endl;
            // SDL_BLENDMODE_BLEND = 1 (alpha):
            //      dstRGB = (srcRGB * srcA) + (dstRGB * (1-srcA))
            //      dstA = srcA + (dstA * (1-srcA))

            //boost::filesystem::path glyphFilePath = textureFilePath.parent_path() / "glyphs" / ( std::to_string(glyph.code) + ".png" );
            //SDL_SavePNG(glyphSurface.Get(), glyphFilePath.generic_string().c_str());

            int x = glyph.x - glyph.minx;
            if (glyph.minx < 0)
                x = glyph.x;
            int y = glyph.y + glyph.maxy - fontAscent;
            bool empty = (glyph.w == 0) && (glyph.h == 0);
            if (!empty)
            {
                x += paddingLeft;
                y += paddingUp;
                SDL2pp::Rect dstRect(x, y, glyph.w, glyph.h);
                // Blit with alpha blending.
                glyphSurface.Blit(SDL2pp::NullOpt, outputSurface, dstRect);
            }
        }

        std::string pageName = textureFilePath.stem().string() + "_" + std::to_string(page) + textureFilePath.extension().string();
        pageNames.push_back(pageName);

        if (glyphBackgroundColorRgb)
            outputSurface = outputSurface.Convert(SDL_PIXELFORMAT_RGB24);

        boost::filesystem::path newPath = textureFilePath.parent_path() / boost::filesystem::path(pageName);
        SDL_SavePNG(outputSurface.Get(), newPath.generic_string().c_str());
    }

    /////////////////////////////////////////////////////////////


    //TODO: add space character.

    Font f;
    f.debugFillValues();
    f.chars.clear();
    f.kernings.clear();
    f.pages.clear();

    if (includeKerningPairs)
    {
        std::set<Uint16> glyphCodes2(glyphCodes);
        for (auto& ch0 : glyphCodes)
        {
            for (auto& ch1 : glyphCodes2)
            {
                int k = getKerning(font, ch0, ch1);
                if (k)
                    f.kernings.emplace_back(Font::Kerning{ch0, ch1, k});
            }
            glyphCodes2.erase(ch0);
        }
    }



    for (int i = 0; i < pageCount; ++i )
        f.pages.emplace_back(Font::Page{i, pageNames.at(i)});

    for ( auto glyphIterator = glyphs.begin(); glyphIterator != glyphs.end(); ++glyphIterator )
    {
        const GlyphInfo &glyph = glyphIterator->second;
        //TODO: page = 0 for empty flyphs.
        f.chars.emplace_back(Font::Char{glyph.code,
                                        glyph.x,
                                        glyph.y,
                                        glyph.w + paddingLeft + paddingRight,
                                        glyph.h + paddingUp + paddingDown,
                                        glyph.minx - paddingLeft,
                                        fontAscent - glyph.maxy - paddingUp,
                                        glyph.advance,
                                        glyph.page,
                                        15});
    }

    //f.info.size = 48;
    f.info.face = font.GetFamilyName().value_or("unknown");

    f.common.lineHeight = font.GetLineSkip();
    f.common.base = font.GetAscent();
    f.common.scaleW = textureWidth;
    f.common.scaleH = textureHeight;

    if (dataFormat == "xml")
        f.writeToXmlFile(fntFilePath.generic_string());
    if (dataFormat == "txt")
        f.writeToTextFile(fntFilePath.generic_string());

	return 0;

} catch (std::exception& e) {
	std::cerr << "Error: " << e.what() << std::endl;
	return 1;
} catch (...) {
	std::cerr << "Unknown error" << std::endl;
	return 1;
}

