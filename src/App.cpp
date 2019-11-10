#include "App.h"
#include "ProgramOptions.h"
#include <string>
#include <iomanip>
#include "FontInfo.h"
#include "utils/extractFileName.h"
#include "external/lodepng/lodepng.h"
#include "utils/getNumberLen.h"

std::vector<rbp::RectSize> App::getGlyphRectangles(const Glyphs &glyphs, const std::uint32_t additionalWidth, const std::uint32_t additionalHeight)
{
    std::vector<rbp::RectSize> result;
    for (const auto& kv : glyphs)
    {
        const auto& glyphInfo = kv.second;
        if (!glyphInfo.isEmpty())
            result.emplace_back(glyphInfo.width + additionalWidth, glyphInfo.height + additionalHeight, kv.first);
    }
    return result;
}

App::Glyphs App::collectGlyphInfo(const std::vector<ft::Font>& fonts, const std::set<std::uint32_t>& codes)
{
    Glyphs result;

    for (const auto& id : codes)
    {
        GlyphInfo glyphInfo;

        for (size_t i = 0; i < fonts.size(); ++i)
        {
            if (fonts[i].isGlyphProvided(id))
            {
                ft::Font::GlyphMetrics glyphMetrics = fonts[i].renderGlyph(nullptr, 0, 0, 0, 0, id, 0);
                glyphInfo.fontIndex = i;
                glyphInfo.width = glyphMetrics.width;
                glyphInfo.height = glyphMetrics.height;
                glyphInfo.xAdvance = glyphMetrics.horiAdvance;
                glyphInfo.xOffset = glyphMetrics.horiBearingX;
                glyphInfo.yOffset = fonts[i].ascent - glyphMetrics.horiBearingY;
                break;
            }
        }

        //TODO: add more checks for glyph.
        if (glyphInfo.fontIndex == -1)
            std::cout << "warning: glyph " << id << " not found." << std::endl;
        else
            result[id] = glyphInfo;
    }

    return result;
}

std::uint32_t App::arrangeGlyphs(Glyphs& glyphs, const Config& config)
{
    const auto additionalWidth = config.spacing.hor + config.padding.left + config.padding.right;
    const auto additionalHeight = config.spacing.ver + config.padding.up + config.padding.down;
    //TODO: check workAreaW,H
    const auto workAreaW = config.textureSize.w - config.spacing.hor;
    const auto workAreaH = config.textureSize.h - config.spacing.ver;

    auto glyphRectangles = getGlyphRectangles(glyphs, additionalWidth, additionalHeight);

    rbp::MaxRectsBinPack mrbp;
    std::uint32_t pageCount = 0;
    for (;;)
    {
        mrbp.Init(workAreaW, workAreaH);

        std::vector<rbp::Rect> arrangedRectangles;
        mrbp.Insert( glyphRectangles, arrangedRectangles, rbp::MaxRectsBinPack::RectBestAreaFit );
        if ( arrangedRectangles.empty() )
        {
            if ( !glyphRectangles.empty() )
                throw std::runtime_error("can not fit glyphs into texture");
            break;
        }

        for ( const auto& r: arrangedRectangles )
        {
            glyphs[r.tag].x = r.x + config.spacing.hor;
            glyphs[r.tag].y = r.y + config.spacing.ver;
            glyphs[r.tag].page = pageCount;
        }

        ++pageCount;
    }

    return pageCount;
}

void App::savePng(const std::string& fileName, const std::uint32_t* buffer, const std::uint32_t w, const std::uint32_t h, const bool withAlpha)
{
    std::vector<std::uint8_t> png;
    lodepng::State state;

    state.encoder.add_id = 0; // Don't add LodePNG version chunk to save more bytes
    state.encoder.auto_convert = 0;
    state.info_png.color.colortype = withAlpha ? LCT_RGBA : LCT_RGB;

    ///state.encoder.text_compression = 1; //Not needed because we don't add text chunks, but this demonstrates another optimization setting
    //state.encoder.zlibsettings.nicematch = 258; //Set this to the max possible, otherwise it can hurt compression
    //state.encoder.zlibsettings.lazymatching = 1; //Definitely use lazy matching for better compression
    //state.encoder.zlibsettings.windowsize = 32768; //Use maximum possible window size for best compression

    auto error = lodepng::encode(png, reinterpret_cast<const unsigned char*>(buffer), w, h, state);
    if (error)
        throw std::runtime_error("png encoder error " + std::to_string(error) + ": " + lodepng_error_text(error));

    error = lodepng::save_file(png, fileName);
    if (error)
        throw std::runtime_error("png save to file error " + std::to_string(error) + ": " + lodepng_error_text(error));
}

std::vector<std::string> App::renderTextures(const Glyphs& glyphs, const Config& config, const std::vector<ft::Font>& fonts, const std::uint32_t pageCount){
    std::vector<std::string> fileNames;

    //TODO: should we decrement pageCount before calculate?
    const auto pageNameDigits = getNumberLen(pageCount);

    for (std::uint32_t page = 0; page < pageCount; ++page)
    {
        std::vector<std::uint32_t> surface(config.textureSize.w * config.textureSize.h);
        memset(&surface[0], 0, surface.size() * sizeof(std::uint32_t));

        // Render every glyph
        //TODO: do not repeat same glyphs (with same index)
        for (const auto& kv: glyphs)
        {
            const auto& glyph = kv.second;
            if (glyph.page != page)
                continue;

            if (!glyph.isEmpty())
            {
                const auto x = glyph.x + config.padding.left;
                const auto y = glyph.y + config.padding.up;

                fonts[glyph.fontIndex].renderGlyph(&surface[0], config.textureSize.w, config.textureSize.h, x, y, kv.first, config.color.getBGR());
            }
        }

        if (!config.backgroundTransparent)
        {
            auto cur = surface.data();
            const auto end = &surface.back();

            const auto fgColor = config.color.getBGR();
            const auto bgColor = config.backgroundColor.getBGR();

            while (cur <= end)
            {
                const std::uint32_t a0 = (*cur) >> 24;
                const std::uint32_t a1 = 256 - a0;
                const std::uint32_t rb1 = (a1 * (bgColor & 0xFF00FF)) >> 8;
                const std::uint32_t rb2 = (a0 * (fgColor & 0xFF00FF)) >> 8;
                const std::uint32_t g1  = (a1 * (bgColor & 0x00FF00)) >> 8;
                const std::uint32_t g2  = (a0 * (fgColor & 0x00FF00)) >> 8;
                *cur =  ((rb1 | rb2) & 0xFF00FF) + ((g1 | g2) & 0x00FF00);
                ++cur;
            }
        }

        std::stringstream ss;
        ss << config.output << "_" << std::setfill ('0') << std::setw(pageNameDigits) << page << ".png";
        const auto fileName = ss.str();
        fileNames.push_back(extractFileName(fileName));

        savePng(fileName, &surface[0], config.textureSize.w, config.textureSize.h, config.backgroundTransparent);
    }

    return fileNames;
}

void App::writeFontInfoFile(const Glyphs& glyphs, const Config& config, const std::vector<ft::Font>& fonts, const std::vector<std::string>& fileNames)
{
    FontInfo f;

    f.info.face = fonts[0].getFamilyNameOr("unknown");
    f.info.size = config.fontSize;
    f.info.unicode = true;
    f.info.aa = 1;
    f.info.padding.up = static_cast<std::uint8_t>(config.padding.up);
    f.info.padding.right = static_cast<std::uint8_t>(config.padding.right);
    f.info.padding.down = static_cast<std::uint8_t>(config.padding.down);
    f.info.padding.left = static_cast<std::uint8_t>(config.padding.left);
    f.info.spacing.horizontal = static_cast<std::uint8_t>(config.spacing.hor);
    f.info.spacing.vertical = static_cast<std::uint8_t>(config.spacing.ver);

    f.common.lineHeight = static_cast<std::uint16_t>(fonts[0].lineskip);
    f.common.base = static_cast<std::uint16_t>(fonts[0].ascent);
    f.common.scaleW = static_cast<std::uint16_t>(config.textureSize.w);
    f.common.scaleH = static_cast<std::uint16_t>(config.textureSize.h);

    f.pages = fileNames;

    for (const auto kv: glyphs)
    {
        //TODO: page = 0 for empty glyphs.
        const auto &glyph = kv.second;
        FontInfo::Char c;
        c.id = kv.first;
        if (!glyph.isEmpty())
        {
            c.x = static_cast<std::uint16_t>(glyph.x);
            c.y = static_cast<std::uint16_t>(glyph.y);
            c.width = static_cast<std::uint16_t>(glyph.width + config.padding.left + config.padding.right);
            c.height = static_cast<std::uint16_t>(glyph.height + config.padding.up + config.padding.down);
            c.page = static_cast<std::uint8_t>(glyph.page);
            c.xoffset = static_cast<std::int16_t>(glyph.xOffset - config.padding.left);
            c.yoffset = static_cast<std::int16_t>(glyph.yOffset - config.padding.up);
        }
        c.xadvance = static_cast<std::int16_t>(glyph.xAdvance);
        c.chnl = 15;

        f.chars.push_back(c);
    }

    if (config.includeKerningPairs)
    {
        auto chars(config.chars);

        for (const auto& ch0 : config.chars)
        {
            for (const auto& ch1 : chars)
            {
                const auto k = static_cast<std::int16_t>(fonts[0].getKerning(ch0, ch1));
                if (k)
                {
                    FontInfo::Kerning kerning;
                    kerning.first = ch0;
                    kerning.second = ch1;
                    kerning.amount = k;
                    f.kernings.push_back(kerning);
                }
            }
        }
    }

    const auto dataFileName = config.output + ".fnt";
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

void App::execute(const int argc, char* argv[]) 
{
    const auto config = ProgramOptions::parseCommandLine(argc, argv);

    ft::Library library;

    std::vector<ft::Font> fonts;
    for (auto& f: config.fontFile)
        fonts.emplace_back(library, f, config.fontSize);

    auto glyphs = collectGlyphInfo(fonts, config.chars);
    const auto pageCount = arrangeGlyphs(glyphs, config);

    const auto fileNames = renderTextures(glyphs, config, fonts, pageCount);
    writeFontInfoFile(glyphs, config, fonts, fileNames);
}
