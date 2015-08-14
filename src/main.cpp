#include <string>
#include <iostream>
#include <fstream>
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
#include "ConfigFile.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

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



int main(int argc, char** argv) try {

    po::options_description desc( "Allowed options" );
    fs::path configFilePath;

    struct Args
    {
        std::string fontFile;
        std::string chars;
        std::string color;
        std::string backgroundColor;
        int fontSize;
        int paddingUp;
        int paddingRight;
        int paddingDown;
        int paddingLeft;
        int textureWidth;
        int textureHeight;
        std::string output;
        std::string dataFormat;
        bool includeKerningPairs;
    };
    Args args;

    desc.add_options()
            ( "help", "produce help message" )
            ( "config", po::value< fs::path >( &configFilePath)->required(), "config file" )
            ( "config", po::value< std::string >( &args.fontFile), "fontFile" )
            ( "config", po::value< std::string >( &args.chars), "chars" )
            ( "config", po::value< std::string >( &args.color), "color" )
            ( "config", po::value< std::string >( &args.backgroundColor), "backgroundColor" )
            ( "config", po::value< int >( &args.fontSize), "fontSize" )
            ( "config", po::value< int >( &args.paddingUp), "paddingUp" )
            ( "config", po::value< int >( &args.paddingRight), "paddingRight" )
            ( "config", po::value< int >( &args.paddingDown), "paddingDown" )
            ( "config", po::value< int >( &args.paddingLeft), "paddingLeft" )
            ( "config", po::value< int >( &args.textureWidth), "textureWidth" )
            ( "config", po::value< int >( &args.textureHeight), "textureHeight" )
            ( "config", po::value< std::string >( &args.output), "output" )
            ( "config", po::value< std::string >( &args.dataFormat), "dataFormat" )
            ( "config", po::value< bool >( &args.includeKerningPairs), "includeKerningPairs" );


    po::variables_map vm;
    po::store( po::parse_command_line( argc, argv, desc ), vm );

    if ( vm.count( "help" ) )
    {
        std::cout << desc << std::endl;
        return 1;
    }

    po::notify( vm );

    ///////////////////////////////////////

    ConfigFile configFile(configFilePath);
    ConfigFile::Config config = configFile.getConfig();

    if ( vm.count( "fontFile" ) )
        config.fontFile = args.fontFile;
//    config.chars = args.chars;
//    config.color = args.color;
//    config.backgroundColor = args.backgroundColor;
    if ( vm.count( "fontSize" ) )
        config.fontSize = args.fontSize;
    if ( vm.count( "paddingUp" ) )
        config.paddingUp = args.paddingUp;
    if ( vm.count( "paddingRight" ) )
        config.paddingRight = args.paddingRight;
    if ( vm.count( "paddingDown" ) )
        config.paddingDown = args.paddingDown;
    if ( vm.count( "paddingLeft" ) )
        config.paddingLeft = args.paddingLeft;
    if ( vm.count( "textureWidth" ) )
        config.textureWidth = args.textureWidth;
    if ( vm.count( "textureHeight" ) )
        config.textureHeight = args.textureHeight;
    if ( vm.count( "output" ) )
        config.output = args.output;
    if ( vm.count( "dataFormat" ) )
        config.dataFormat = args.dataFormat;
    if ( vm.count( "includeKerningPairs" ) )
        config.includeKerningPairs = args.includeKerningPairs;

    ///////////////////////////////////////

    fs::path outputPath(config.output);
    if (!outputPath.is_absolute())
        outputPath = fs::absolute(outputPath, configFilePath.parent_path());

    fs::path outputDirPath = outputPath.parent_path();

    //TODO: create directory only if there is no problem (exceptions), good place is before write outputs.
    fs::create_directory(outputDirPath);

    const std::string outputName = outputPath.stem().string();

    fs::path dataFilePath = outputDirPath / (outputName + ".fnt");

    fs::path fontFilePath(config.fontFile);
    if (!fontFilePath.is_absolute())
        fontFilePath = fs::absolute(fontFilePath, configFilePath.parent_path());
    if (!fs::is_regular_file(fontFilePath))
        throw std::runtime_error("font file not found");

    SDL2pp::SDLTTF ttf;
    SDL2pp::Font font(fontFilePath.generic_string(), config.fontSize);


    int fontAscent = font.GetAscent();

    std::map<Uint16, GlyphInfo> glyphs;
    collectGlyphInfo(font, config.glyphCodes, glyphs);
    checkGlyphSize(glyphs, config.textureWidth, config.textureHeight);

    std::vector< rbp::RectSize > srcRects;
    getSrcRects(glyphs, config.spacingHoriz + config.paddingLeft + config.paddingRight,
                config.spacingVert + config.paddingUp + config.paddingDown, srcRects);

    rbp::MaxRectsBinPack mrbp;
    int pageCount = 0;
    for (;;)
    {
        //TODO: check if negative dimension.
        mrbp.Init(config.textureWidth - config.spacingHoriz, config.textureHeight - config.spacingVert);

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
            glyphs[r.tag].x = r.x + config.spacingHoriz;
            glyphs[r.tag].y = r.y + config.spacingVert;
            glyphs[r.tag].page = pageCount;
        }

        pageCount++;
    }

    /////////////////////////////////////////////////////////////

    std::vector<std::string> pageNames;

    for (int page = 0; page < pageCount; ++page)
    {
        //TODO: use real texture size instead max.
        SDL2pp::Surface outputSurface(0, config.textureWidth, config.textureHeight, 32,
                                      0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        //std::cout << "outputSurface blend mode " << outputSurface.GetBlendMode() << std::endl;
        // SDL_BLENDMODE_BLEND = 1 (alpha).

        // If the color value contains an alpha component then the destination is simply
        // filled with that alpha information, no blending takes place.
        if (config.glyphBackgroundColorRgb)
            outputSurface.FillRect(SDL2pp::NullOpt, config.glyphBackgroundColorRgb->getUint32(255));
        else
            outputSurface.FillRect(SDL2pp::NullOpt, config.glyphColorRgb.getUint32(0));

        for ( auto glyphIterator = glyphs.begin(); glyphIterator != glyphs.end(); ++glyphIterator )
        {
            const GlyphInfo& glyph = glyphIterator->second;
            if (glyph.page != page)
                continue;

            SDL2pp::Surface glyphSurface = font.RenderGlyph_Blended(glyph.code, config.glyphColorRgb.getSdlColor() );
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
                x += config.paddingLeft;
                y += config.paddingUp;
                SDL2pp::Rect dstRect(x, y, glyph.w, glyph.h);
                // Blit with alpha blending.
                glyphSurface.Blit(SDL2pp::NullOpt, outputSurface, dstRect);
            }
        }

        std::string pageName = outputName + "_" + std::to_string(page) + ".png";
        pageNames.push_back(pageName);

        if (config.glyphBackgroundColorRgb)
            outputSurface = outputSurface.Convert(SDL_PIXELFORMAT_RGB24);

        boost::filesystem::path texturePath = outputDirPath / boost::filesystem::path(pageName);
        SDL_SavePNG(outputSurface.Get(), texturePath.generic_string().c_str());
    }

    /////////////////////////////////////////////////////////////

    Font f;
    f.debugFillValues();
    f.chars.clear();
    f.kernings.clear();
    f.pages.clear();

    if (config.includeKerningPairs)
    {
        std::set<Uint16> glyphCodes2(config.glyphCodes);
        for (auto& ch0 : config.glyphCodes)
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
                                        glyph.w + config.paddingLeft + config.paddingRight,
                                        glyph.h + config.paddingUp + config.paddingDown,
                                        glyph.minx - config.paddingLeft,
                                        fontAscent - glyph.maxy - config.paddingUp,
                                        glyph.advance,
                                        glyph.page,
                                        15});
    }

    //f.info.size = 48;
    f.info.face = font.GetFamilyName().value_or("unknown");

    f.common.lineHeight = font.GetLineSkip();
    f.common.base = font.GetAscent();
    f.common.scaleW = config.textureWidth;
    f.common.scaleH = config.textureHeight;

    if (config.dataFormat == "xml")
        f.writeToXmlFile(dataFilePath.generic_string());
    if (config.dataFormat == "txt")
        f.writeToTextFile(dataFilePath.generic_string());

	return 0;

} catch (std::exception& e) {
	std::cerr << "Error: " << e.what() << std::endl;
	return 1;
} catch (...) {
	std::cerr << "Unknown error" << std::endl;
	return 1;
}
