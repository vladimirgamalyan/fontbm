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
#include "Config.h"
#include "ProgramOptions.h"


namespace fs = boost::filesystem;

int getKerning(const SDL2pp::Font& font, uint32_t char0, uint32_t char1)
{
    if ((char0 > 0xFFFF)|| (char1 > 0xFFFF))
        return 0;
    Uint16 ch0 = static_cast<Uint16>(char0);
    Uint16 ch1 = static_cast<Uint16>(char1);
    Uint16 text[ 3 ] = {ch0, ch1, 0};
    return font.GetSizeUNICODE(text).x - ( font.GetGlyphAdvance(ch0) + font.GetGlyphAdvance(ch1) );
}

void printGlyphData(const SDL2pp::Font& font, Uint16 ch)
{
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

void collectGlyphInfo(const SDL2pp::Font& font, const std::set<uint32_t>& codes, std::map<Uint16, GlyphInfo>& glyphs )
{
    int fontAscent = font.GetAscent();

    glyphs.clear();

    for (auto& id : codes)
    {
        if ( !font.IsGlyphProvided(id) )
        {
            std::cout << "warning: glyph " << id << " not found " << std::endl;
            continue;
        }

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

SDL_Color makeSdlColor(Config::Color c, uint8_t a = 255)
{
    return SDL_Color{c.r, c.g, c.b, a};
}

int main(int argc, char** argv)
{
    try {
        Config config = ProgramOptions::parseCommandLine(argc, argv);

        fs::path dataFilePath = fs::absolute(fs::path(config.output + ".fnt"));
        fs::path outputDirPath = dataFilePath.parent_path();
        const std::string outputName = dataFilePath.stem().string();

        //TODO: create directory only if there is no problem (exceptions), good place is right before write outputs.

        fs::create_directory(outputDirPath);

        if (!fs::is_regular_file(config.fontFile))
            throw std::runtime_error("font file not found");

        SDL2pp::SDLTTF ttf;
        SDL2pp::Font font(config.fontFile.generic_string(), config.fontSize);

        int fontAscent = font.GetAscent();

        std::map<Uint16, GlyphInfo> glyphs;
        collectGlyphInfo(font, config.chars, glyphs);
        checkGlyphSize(glyphs, config.textureSize.w, config.textureSize.h);

        std::vector< rbp::RectSize > srcRects;
        getSrcRects(glyphs, config.spacing.hor + config.padding.left + config.padding.right,
                    config.spacing.ver + config.padding.up + config.padding.down, srcRects);

        rbp::MaxRectsBinPack mrbp;
        int pageCount = 0;
        for (;;)
        {
            //TODO: check negative dimension.
            mrbp.Init(config.textureSize.w - config.spacing.hor, config.textureSize.h - config.spacing.ver);

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
                glyphs[r.tag].x = r.x + config.spacing.hor;
                glyphs[r.tag].y = r.y + config.spacing.ver;
                glyphs[r.tag].page = pageCount;
            }

            pageCount++;
        }

        /////////////////////////////////////////////////////////////

        std::vector<std::string> pageNames;

        for (int page = 0; page < pageCount; ++page)
        {
            //TODO: use real texture size instead max.
            SDL2pp::Surface outputSurface(0, config.textureSize.w, config.textureSize.h, 32,
                                          0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
            //std::cout << "outputSurface blend mode " << outputSurface.GetBlendMode() << std::endl;
            // SDL_BLENDMODE_BLEND = 1 (alpha).

            // If the color value contains an alpha component then the destination is simply
            // filled with that alpha information, no blending takes place.
            if (config.backgroundColor)
                outputSurface.FillRect(SDL2pp::NullOpt, config.backgroundColor->getUint32(255));
            else
                outputSurface.FillRect(SDL2pp::NullOpt, config.color.getUint32(0));

            for ( auto glyphIterator = glyphs.begin(); glyphIterator != glyphs.end(); ++glyphIterator )
            {
                const GlyphInfo& glyph = glyphIterator->second;
                if (glyph.page != page)
                    continue;

                SDL2pp::Surface glyphSurface = font.RenderGlyph_Blended(glyph.code, makeSdlColor(config.color));

                int x = glyph.x - glyph.minx;
                if (glyph.minx < 0)
                    x = glyph.x;
                int y = glyph.y + glyph.maxy - fontAscent;
                bool empty = (glyph.w == 0) && (glyph.h == 0);
                if (!empty)
                {
                    x += config.padding.left;
                    y += config.padding.up;
                    SDL2pp::Rect dstRect(x, y, glyph.w, glyph.h);
                    // Blit with alpha blending.
                    glyphSurface.Blit(SDL2pp::NullOpt, outputSurface, dstRect);
                }
            }

            std::string pageName = outputName + "_" + std::to_string(page) + ".png";
            pageNames.push_back(pageName);

            if (config.backgroundColor)
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
            std::set<uint32_t> glyphCodes2(config.chars);
            for (auto& ch0 : config.chars)
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
                                            glyph.w + config.padding.left + config.padding.right,
                                            glyph.h + config.padding.up + config.padding.down,
                                            glyph.minx - config.padding.left,
                                            fontAscent - glyph.maxy - config.padding.up,
                                            glyph.advance,
                                            glyph.page,
                                            15});
        }

        //f.info.size = 48;
        f.info.face = font.GetFamilyName().value_or("unknown");

        f.common.lineHeight = font.GetLineSkip();
        f.common.base = font.GetAscent();
        f.common.scaleW = config.textureSize.w;
        f.common.scaleH = config.textureSize.h;

        if (config.dataFormat == Config::DataFormat::Xml)
            f.writeToXmlFile(dataFilePath.generic_string());
        if (config.dataFormat == Config::DataFormat::Text)
            f.writeToTextFile(dataFilePath.generic_string());

        return 0;

    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error" << std::endl;
        return 1;
    }
}
