#include "App.h"
#include "ProgramOptions.h"
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
            rs.width = glyphInfo.width + additionalHeight;
            rs.height = glyphInfo.height + additionalWidth;
            rs.tag = kv.first;
            result.push_back(rs);
        }
    }
    return result;
}

App::Glyphs App::collectGlyphInfo(ft::Face& face,
                                  const std::set<uint32_t> &codes,
                                  uint32_t maxTextureSizeX,
                                  uint32_t maxTextureSizeY)
{
    Glyphs glyphs;

    for (auto& id : codes)
    {
        if (!face.isGlyphProvided(id))
        {
            std::cout << "warning: glyph " << id << " not found " << std::endl;
            continue;
        }

        ft::Face::GlyphMetrics glyphMetrics = face.renderGlyph(nullptr, 0, 0, 0, 0, id, 0);
        if ((glyphMetrics.width > maxTextureSizeX) || (glyphMetrics.height > maxTextureSizeY))
            throw std::runtime_error("no room for glyph");

        GlyphInfo glyphInfo;
        glyphInfo.width = glyphMetrics.width;
        glyphInfo.height = glyphMetrics.height;
        glyphInfo.xAdvance = glyphMetrics.horiAdvance;
        glyphInfo.xOffset = glyphMetrics.horiBearingX;
        glyphInfo.yOffset = face.ascent - glyphMetrics.horiBearingY;

        //TODO: add more checks for glyph.

        glyphs[id] = glyphInfo;
    }

    return glyphs;
}

uint32_t App::arrangeGlyphs(Glyphs& glyphs, const Config& config)
{
    std::vector< rbp::RectSize > srcRects = getSrcRects(glyphs, config.spacing.hor + config.padding.left + config.padding.right,
                                                        config.spacing.ver + config.padding.up + config.padding.down);

    rbp::MaxRectsBinPack mrbp;
    uint32_t pageCount = 0;
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

void App::savePng(const std::string& fileName, const uint32_t* buffer, uint32_t w, uint32_t h, bool withAlpha)
{
    std::vector<unsigned char> png;
    lodepng::State state;

    state.encoder.add_id = 0; //Don't add LodePNG version chunk to save more bytes
    state.encoder.auto_convert = 0;
    state.info_png.color.colortype = withAlpha ? LCT_RGBA : LCT_RGB;


    ///state.encoder.text_compression = 1; //Not needed because we don't add text chunks, but this demonstrates another optimization setting
    //state.encoder.zlibsettings.nicematch = 258; //Set this to the max possible, otherwise it can hurt compression
    //state.encoder.zlibsettings.lazymatching = 1; //Definitely use lazy matching for better compression
    //state.encoder.zlibsettings.windowsize = 32768; //Use maximum possible window size for best compression

    unsigned error = lodepng::encode(png, reinterpret_cast<const unsigned char*>(buffer), w, h, state);
    if (error)
        throw std::runtime_error("PNG encoder error " + std::to_string(error) + ": " + lodepng_error_text(error));

    error = lodepng::save_file(png, fileName);
    if (error)
        throw std::runtime_error("PNG save to file error " + std::to_string(error) + ": " + lodepng_error_text(error));
}

void App::execute(int argc, char* argv[])
{
    const Config config = helpers::parseCommandLine(argc, argv);

    ft::Library library;
    ft::Face face(library, config.fontFile, config.fontSize);
    //face.debugInfo();

    Glyphs glyphs = collectGlyphInfo(face, config.chars, config.textureSize.w, config.textureSize.h);

    const uint32_t pageCount = arrangeGlyphs(glyphs, config);

    /////////////////////////////////////////////////////////////

    std::vector<std::string> pageNames;

    //TODO: should we decrement pageCount before calculate?
    int pageNameDigits = getDigitCount(pageCount);

    // Render every page
    for (uint32_t page = 0; page < pageCount; ++page)
    {
        std::vector<uint32_t> surface(config.textureSize.w * config.textureSize.h);
        memset(&surface[0], 0, surface.size() * sizeof(uint32_t));

        // Render every glyph
        //TODO: do not repeat same glyphs (with same index)
        for (auto kv: glyphs)
        {
            const GlyphInfo& glyph = kv.second;
            if (glyph.page != page)
                continue;

            if (!glyph.isEmpty())
            {
                int x = glyph.x + config.padding.left;
                int y = glyph.y + config.padding.up;

                assert(x >= 0);
                assert(y >= 0);

                face.renderGlyph(&surface[0], config.textureSize.w, config.textureSize.h, x, y, kv.first, config.color.getUint32(0));
            }
        }

        if (!config.backgroundTransparent)
        {
            uint32_t* cur = surface.data();
            uint32_t* end = &surface.back();

            uint32_t fgColor = config.color.getUint32(0);
            uint32_t bgColor = config.backgroundColor.getUint32(0);

            while (cur <= end)
            {
                uint32_t a0 = (*cur) >> 24;
                uint32_t a1 = 256 - a0;
                uint32_t rb1 = (a1 * (bgColor & 0xFF00FF)) >> 8;
                uint32_t rb2 = (a0 * (fgColor & 0xFF00FF)) >> 8;
                uint32_t g1  = (a1 * (bgColor & 0x00FF00)) >> 8;
                uint32_t g2  = (a0 * (fgColor & 0x00FF00)) >> 8;
                *cur =  ((rb1 | rb2) & 0xFF00FF) + ((g1 | g2) & 0x00FF00);
                ++cur;
            }
        }

        std::stringstream ss;
        ss << config.output << "_" << std::setfill ('0') << std::setw(pageNameDigits) << page << ".png";
        std::string pageName = ss.str();
        pageNames.push_back(extractFileName(pageName));

        savePng(pageName, &surface[0], config.textureSize.w, config.textureSize.h, config.backgroundTransparent);
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

    f.pages = pageNames;

    for (auto kv: glyphs)
    {
        //TODO: page = 0 for empty glyphs.
        const GlyphInfo &glyph = kv.second;
        FontInfo::Char c;
        c.id = kv.first;
        if (!glyph.isEmpty())
        {
            c.x = static_cast<uint16_t>(glyph.x);
            c.y = static_cast<uint16_t>(glyph.y);
            c.width = static_cast<uint16_t>(glyph.width + config.padding.left + config.padding.right);
            c.height = static_cast<uint16_t>(glyph.height + config.padding.up + config.padding.down);
            c.page = static_cast<uint8_t>(glyph.page);
            c.xoffset = static_cast<int16_t>(glyph.xOffset - config.padding.left);
            c.yoffset = static_cast<int16_t>(glyph.yOffset - config.padding.up);
        }
        c.xadvance = static_cast<int16_t>(glyph.xAdvance);
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
                auto k = static_cast<int16_t>(face.getKerning(ch0, ch1));
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
