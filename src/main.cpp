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
    Uint16 id;
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

    ///////////////////////////////////////

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

    SDL2pp::SDLTTF ttf;
    SDL2pp::Font font(fontFilePath.generic_string(), fontSize);


    int fontAscent = font.GetAscent();

    std::map<int, GlyphInfo> glyphs;
    std::vector< rbp::RectSize > srcRects;
    for ( Uint16 id = 32; id < 128; ++id )
    {
        if ( !font.IsGlyphProvided(id) )
            continue;

        GlyphInfo glyphInfo;
        font.GetGlyphMetrics(id, glyphInfo.minx, glyphInfo.maxx, glyphInfo.miny, glyphInfo.maxy, glyphInfo.advance);

        glyphInfo.id = id;
        glyphInfo.x = 0;
        glyphInfo.y = 0;
        glyphInfo.w = glyphInfo.maxx - glyphInfo.minx;
        glyphInfo.h = glyphInfo.maxy - glyphInfo.miny;

        if (fontAscent < glyphInfo.maxy)
            throw std::runtime_error("invalid glyph");

        bool empty = (glyphInfo.w == 0) && (glyphInfo.h == 0);
        if (!empty)
            if ((glyphInfo.w == 0) || (glyphInfo.h == 0))
                throw std::runtime_error("invalid glyph");

        glyphs[id] = glyphInfo;

        if (!empty)
        {
            rbp::RectSize rs;
            rs.width = glyphInfo.w;
            rs.height = glyphInfo.h;
            rs.tag = glyphInfo.id;
            srcRects.push_back(rs);
        }
    }

    int textureWidth = maxTextureSizeX;
    int textureHeight = maxTextureSizeY;
    rbp::MaxRectsBinPack mrbp;
    mrbp.Init(textureWidth, textureHeight);
    std::vector<rbp::Rect> readyRects;
    mrbp.Insert( srcRects, readyRects, rbp::MaxRectsBinPack::RectBestAreaFit );
    for ( auto r: readyRects )
    {
        glyphs[r.tag].x = r.x;
        glyphs[r.tag].y = r.y;
    }



    SDL2pp::Surface outputSurface(0, textureWidth, textureHeight, 32,
                                  0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    outputSurface.FillRect(SDL2pp::NullOpt, 0xFFFFFF);
    for ( auto glyphIterator = glyphs.begin(); glyphIterator != glyphs.end(); ++glyphIterator )
    {
        const GlyphInfo& glyph = glyphIterator->second;
        SDL2pp::Surface glyphSurface = font.RenderGlyph_Blended(glyph.id, SDL_Color {255, 255, 255, 255} );
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

    SDL_SavePNG(outputSurface.Get(), textureFilePath.generic_string().c_str());

    Font f;
    f.debugFillValues();
    f.chars.clear();
    f.kernings.clear();
    f.pages.clear();

    f.pages.emplace_back(Font::Page{0, "output.png"});

    for ( auto glyphIterator = glyphs.begin(); glyphIterator != glyphs.end(); ++glyphIterator )
    {
        const GlyphInfo &glyph = glyphIterator->second;
        f.chars.emplace_back(Font::Char{glyph.id, glyph.x, glyph.y,
                                        glyph.w, glyph.h,
                                        glyph.minx, fontAscent - glyph.maxy,
                                        glyph.advance, 0, 15});
    }

    //f.info.size = 48;
    f.info.face = font.GetFamilyName().value_or("unknown");

    f.common.lineHeight = font.GetLineSkip();
    f.common.base = font.GetAscent();
    f.common.scaleW = textureWidth;
    f.common.scaleH = textureHeight;

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
