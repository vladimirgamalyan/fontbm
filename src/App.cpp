#include "App.h"
#include "ProgramOptions.h"
#include <SDL2/SDL.h>
#include <string>
#include <fstream>
#include <array>
#include <limits>
#include <algorithm>
#include <iomanip>
#include "FontInfo.h"
#include "extractFileName.h"
#include "lodepng/lodepng.h"

std::vector<rbp::RectSize> App::getSrcRects(const Glyphs &glyphs, int additionalWidth, int additionalHeight)
{
    std::vector<rbp::RectSize> result;
    for (auto& kv : glyphs)
    {
        const GlyphInfo& glyphInfo = kv.second;
        if (!glyphInfo.isEmpty())
        {
            rbp::RectSize rs;
            rs.width = glyphInfo.getWidth() + additionalHeight;
            rs.height = glyphInfo.getHeight() + additionalWidth;
            rs.tag = kv.first;
            result.push_back(rs);
        }
    }
    return result;
}

App::Glyphs App::collectGlyphInfo(const ft::Face& face,
                                  const std::set<uint32_t> &codes,
                                  uint32_t maxTextureSizeX,
                                  uint32_t maxTextureSizeY)
{
    int fontAscent = face.ascent;

    Glyphs glyphs;

    for (auto& id : codes)
    {
        if ((id > 0xFFFF) || (!face.isGlyphProvided(id)))
        {
            std::cout << "warning: glyph " << id << " not found " << std::endl;
            continue;
        }

        GlyphInfo glyphInfo;
        face.GetGlyphMetrics(static_cast<Uint16>(id),
                             glyphInfo.minx,
                             glyphInfo.maxx,
                             glyphInfo.miny,
                             glyphInfo.maxy,
                             glyphInfo.advance);

        if (fontAscent < glyphInfo.maxy)
            throw std::runtime_error("invalid glyph (maxy > ascent)");

        if ((glyphInfo.getWidth() > static_cast<int>(maxTextureSizeX)) || (glyphInfo.getHeight() > static_cast<int>(maxTextureSizeY)))
            throw std::runtime_error("no room for glyph");

        //TODO: add more checks for glyph.

        if (glyphInfo.isInvalid())
            throw std::runtime_error("invalid glyph (zero or negative width or height)");

        //TODO: emplace.
        glyphs[id] = glyphInfo;
    }

    return glyphs;
}

SDL_Color App::makeSdlColor(Config::Color c, uint8_t a)
{
    return SDL_Color{c.r, c.g, c.b, a};
}

uint16_t App::arrangeGlyphs(Glyphs& glyphs, const Config& config)
{
    std::vector< rbp::RectSize > srcRects = getSrcRects(glyphs, config.spacing.hor + config.padding.left + config.padding.right,
                                                        config.spacing.ver + config.padding.up + config.padding.down);

    rbp::MaxRectsBinPack mrbp;
    uint16_t pageCount = 0;
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

    return pageCount;
}

int App::getDigitCount(uint16_t x)
{
    return (x < 10 ? 1 :
            (x < 100 ? 2 :
             (x < 1000 ? 3 :
              (x < 10000 ? 4 :
               5))));
}

void App::execute(int argc, char* argv[])
{
    const Config config = helpers::parseCommandLine(argc, argv);

    ft::Library library;
    ft::Face face(library, config.fontFile, config.fontSize);
    face.debugInfo();

    SDL2pp::SDLTTF ttf;
    SDL2pp::Font font(config.fontFile, config.fontSize);

    Glyphs glyphs = collectGlyphInfo(face, config.chars, config.textureSize.w, config.textureSize.h);

    const uint16_t pageCount = arrangeGlyphs(glyphs, config);

    const int fontAscent = face.ascent;

    /////////////////////////////////////////////////////////////

    std::vector<std::string> pageNames;

    //TODO: should we decrement pageCount before calculate?
    int pageNameDigits = getDigitCount(pageCount);

    // Render every page
    for (size_t page = 0; page < pageCount; ++page)
    {
        std::vector<uint32_t> surface(config.textureSize.w * config.textureSize.h);
        std::cout << "surface size: " << surface.size() << std::endl;

        //SDL2pp::Surface outputSurface(0, config.textureSize.w, config.textureSize.h, 32,
        //                              0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

        // If the color value contains an alpha component then the destination is simply
        // filled with that alpha information, no blending takes place.
        //if (config.backgroundTransparent)
        //    outputSurface.FillRect(SDL2pp::NullOpt, config.color.getUint32(0));
        //else
        //    outputSurface.FillRect(SDL2pp::NullOpt, config.backgroundColor.getUint32(255));

        // Render every glyph
        //TODO: do not repeat same glyphs (with same index)
        for (auto kv: glyphs)
        {
            const GlyphInfo& glyph = kv.second;
            if (glyph.page != static_cast<int>(page))
                continue;

            // Get glyph images
            //SDL2pp::Surface glyphSurface = font.RenderGlyph_Blended(kv.first, makeSdlColor(config.color));

            int x = glyph.x - glyph.minx;
            int y = glyph.y - (fontAscent - glyph.maxy);

            if (!glyph.isEmpty())
            {
                x += config.padding.left;
                y += config.padding.up;

                face.renderGlyph(&surface[0], config.textureSize.w, config.textureSize.h, x, y, kv.first);
                //SDL2pp::Rect dstRect(x, y, glyph.getWidth(), glyph.getHeight());
                //glyphSurface.Blit(SDL2pp::NullOpt, outputSurface, dstRect);
            }
        }

        std::stringstream ss;
        ss << config.output << "_" << std::setfill ('0') << std::setw(pageNameDigits) << page << ".png";
        std::string pageName = ss.str();

        pageNames.push_back(extractFileName(pageName));

        //if (!config.backgroundTransparent)
        //    outputSurface = outputSurface.Convert(SDL_PIXELFORMAT_RGB24);

        //unsigned error = lodepng::encode(pageName, reinterpret_cast<unsigned char*>(&surface[0]), config.textureSize.w, config.textureSize.h, LCT_RGBA, 8);
        //if (error)
        //    throw std::runtime_error("encoder error " + std::to_string(error) + ": " + lodepng_error_text(error));
    }

    /////////////////////////////////////////////////////////////

    FontInfo f;

    f.info.face = face.getFamilyName("unknown");
    f.info.size = config.fontSize;
    f.info.unicode = true;
    f.info.aa = 1;
    f.info.padding.up = static_cast<uint8_t>(config.padding.up);
    f.info.padding.right = static_cast<uint8_t>(config.padding.right);
    f.info.padding.down = static_cast<uint8_t>(config.padding.down);
    f.info.padding.left = static_cast<uint8_t>(config.padding.left);
    f.info.spacing.horizontal = static_cast<uint8_t>(config.spacing.hor);
    f.info.spacing.vertical = static_cast<uint8_t>(config.spacing.ver);

    f.common.lineHeight = static_cast<uint16_t>(face.lineskip);
    f.common.base = static_cast<uint16_t>(face.ascent);
    f.common.scaleW = static_cast<uint16_t>(config.textureSize.w);
    f.common.scaleH = static_cast<uint16_t>(config.textureSize.h);
    f.common.pages = pageCount;

    for (size_t i = 0; i < pageCount; ++i )
        f.pages.push_back(pageNames.at(i));

    for (auto kv: glyphs)
    {
        //TODO: page = 0 for empty flyphs.
        const GlyphInfo &glyph = kv.second;
        FontInfo::Char c;
        c.id = kv.first;
        if (!glyph.isEmpty())
        {
            c.x = static_cast<uint16_t>(glyph.x);
            c.y = static_cast<uint16_t>(glyph.y);
            c.width = static_cast<uint16_t>(glyph.getWidth() + config.padding.left + config.padding.right);
            c.height = static_cast<uint16_t>(glyph.getHeight() + config.padding.up + config.padding.down);
            c.page = static_cast<uint8_t>(glyph.page);
            c.xoffset = static_cast<int16_t>(glyph.minx - config.padding.left);
            c.yoffset = static_cast<int16_t>(fontAscent - glyph.maxy - config.padding.up);
        }
        c.xadvance = static_cast<int16_t>(glyph.advance);
        c.chnl = 15;

        f.chars.push_back(c);
    }

    if (config.includeKerningPairs)
    {
        //TODO: test if getKerning(font, ch0, ch1) != getKerning(font, ch1, ch0) (and change logic if so)
        std::set<uint32_t> glyphCodes2(config.chars);
        //TODO: remove bruteforcing
        for (auto& ch0 : config.chars)
        {
            for (auto& ch1 : glyphCodes2)
            {
                int16_t k = static_cast<int16_t>(face.getKerning(ch0, ch1));
                //int16_t k = static_cast<int16_t>(getKerning(face, font, ch0, ch1));
                if (k)
                {
                    FontInfo::Kerning kerning;
                    kerning.first = ch0;
                    kerning.second = ch1;
                    kerning.amount = k;
                    f.kernings.push_back(kerning);
                }
            }
            glyphCodes2.erase(ch0);
        }
    }

    const std::string dataFileName = config.output + ".fnt";
    switch (config.dataFormat) {
        case Config::DataFormat::Xml:
            f.writeToXmlFile(dataFileName);
            break;
        case Config::DataFormat::Text:
            f.writeToTextFile(dataFileName);
            break;
        case Config::DataFormat::Bin:
            f.writeToBinFile(dataFileName);
            break;
        case Config::DataFormat::Json:
            f.writeToJsonFile(dataFileName);
            break;
    }
}
