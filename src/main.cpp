#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include <map>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "sdlSavePng/savepng.h"
#include "Font.h"
#include "maxRectsBinPack/MaxRectsBinPack.h"
#include "json.hpp"
#include <fstream>
#include <set>
#include <array>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
using json = nlohmann::json;

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

void getSrcRects(const std::map<Uint16, GlyphInfo> &glyphs, std::vector<rbp::RectSize> &srcRects)
{
    srcRects.clear();
    for (auto& kv : glyphs)
    {
        const GlyphInfo& glyphInfo = kv.second;
        bool empty = (glyphInfo.w == 0) && (glyphInfo.h == 0);
        if (!empty)
        {
            rbp::RectSize rs;
            rs.width = glyphInfo.w;
            rs.height = glyphInfo.h;
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
        if (empty)
            continue;

        if ((glyphInfo.w <= 0) || (glyphInfo.h <= 0))
            throw std::runtime_error("invalid glyph (zero or negative width or height)");

        //TODO: emplace.
        glyphs[id] = glyphInfo;
    }
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


    //TODO: output parse errors, check value's ranges.
    json j;
    j << ifs;
    const std::string fontFile = j["fontFile"];
    const int maxTextureSizeX = j["maxTextureSizeX"];
    const int maxTextureSizeY = j["maxTextureSizeY"];
    const int fontSize = j["fontSize"];
    const std::string textureFile = j["textureFile"];
    const std::string fntFile = j["fntFile"];


    std::cout << "fontFile: " << fontFile << std::endl;
    std::cout << "maxTextureSizeX: " << maxTextureSizeX << std::endl;
    std::cout << "maxTextureSizeY: " << maxTextureSizeY << std::endl;
    std::cout << "fontSize: " << fontSize << std::endl;
    std::cout << "textureFile: " << textureFile << std::endl;
    std::cout << "fntFile: " << fntFile << std::endl;
    std::cout << std::endl;

    bool includeKerningPairs = j["includeKerningPairs"];


    ///////////////////////////////////////

    json colorJson = j["color"];
    if (!colorJson.is_array())
        throw std::runtime_error("config color must be an array");
    if (colorJson.size() != 3)
        throw std::runtime_error("invalid color value");
    std::array<Uint8, 3> glyphColorRgb;
    for (size_t i = 0; i < colorJson.size(); ++i)
    {
        json c = colorJson[i];
        if (!c.is_number_integer())
            throw std::runtime_error("invalid color value");
        int ci = c;
        if ((ci < 0) || (ci > 255))
            throw std::runtime_error("invalid color value");
        glyphColorRgb[i] = static_cast<Uint8>(ci);
    }
    Uint32 glyphColor = glyphColorRgb[0] + (glyphColorRgb[1] << 8) + (glyphColorRgb[2] << 16);

    ///////////////////////////////////////

    //TODO: create directory only if there is no problem (exceptions), good place is before write outputs.
    fs::path fontFilePath(fontFile);
    if (!fontFilePath.is_absolute())
        fontFilePath = configPath.parent_path() / fontFilePath;
    if (!fs::is_regular_file(fontFilePath))
        throw std::runtime_error("font file not found");
    std::cout << fontFilePath << std::endl;

    fs::path textureFilePath(textureFile);
    if (!textureFilePath.is_absolute())
        textureFilePath = configPath.parent_path() / textureFilePath;
    fs::create_directory(textureFilePath.parent_path());
    std::cout << textureFilePath << std::endl;

    fs::path fntFilePath(fntFile);
    if (!fntFilePath.is_absolute())
        fntFilePath = configPath.parent_path() / fntFilePath;
    fs::create_directory(fntFilePath.parent_path());
    std::cout << fntFilePath << std::endl;

    ///////////////////////////////////////

    json charsJson = j["chars"];
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
    checkGlyphSize(glyphs, maxTextureSizeX, maxTextureSizeY);

    std::vector< rbp::RectSize > srcRects;
    getSrcRects(glyphs, srcRects);

    rbp::MaxRectsBinPack mrbp;
    int pageCount = 0;
    for (;;)
    {
        int textureWidth = maxTextureSizeX;
        int textureHeight = maxTextureSizeY;
        mrbp.Init(textureWidth, textureHeight);

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
            glyphs[r.tag].x = r.x;
            glyphs[r.tag].y = r.y;
            glyphs[r.tag].page = pageCount;
        }

        pageCount++;
    }

    /////////////////////////////////////////////////////////////

    std::vector<std::string> pageNames;

    for (int page = 0; page < pageCount; ++page)
    {
        //TODO: use real texture size instead max.
        SDL2pp::Surface outputSurface(0, maxTextureSizeX, maxTextureSizeY, 32,
                                      0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        outputSurface.FillRect(SDL2pp::NullOpt, glyphColor);
        for ( auto glyphIterator = glyphs.begin(); glyphIterator != glyphs.end(); ++glyphIterator )
        {
            const GlyphInfo& glyph = glyphIterator->second;
            if (glyph.page != page)
                continue;

            SDL2pp::Surface glyphSurface = font.RenderGlyph_Blended(glyph.code, SDL_Color {glyphColorRgb[0], glyphColorRgb[1], glyphColorRgb[2], 255} );
            int x = glyph.x - glyph.minx;
            if (glyph.minx < 0)
                x = glyph.x;
            int y = glyph.y + glyph.maxy - fontAscent;
            bool empty = (glyph.w == 0) && (glyph.h == 0);
            if (!empty)
            {
                SDL2pp::Rect dstRect(x, y, glyph.w, glyph.h);
                glyphSurface.Blit(SDL2pp::NullOpt, outputSurface, dstRect);
            }
        }

        std::string pageName = textureFilePath.stem().string() + "_" + std::to_string(page) + textureFilePath.extension().string();
        pageNames.push_back(pageName);

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
        f.chars.emplace_back(Font::Char{glyph.code, glyph.x, glyph.y,
                                        glyph.w, glyph.h,
                                        glyph.minx, fontAscent - glyph.maxy,
                                        glyph.advance,
                                        glyph.page,
                                        15});
    }

    //f.info.size = 48;
    f.info.face = font.GetFamilyName().value_or("unknown");

    f.common.lineHeight = font.GetLineSkip();
    f.common.base = font.GetAscent();
    //TODO: use real texture size instead max.
    f.common.scaleW = maxTextureSizeX;
    f.common.scaleH = maxTextureSizeY;

    f.writeToXmlFile(fntFilePath.generic_string());
    //f.writeToTextFile("output.txt");

	return 0;

} catch (std::exception& e) {
	std::cerr << "Error: " << e.what() << std::endl;
	return 1;
} catch (...) {
	std::cerr << "Unknown error" << std::endl;
	return 1;
}

