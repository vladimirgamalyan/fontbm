#include "App.h"
#include "ProgramOptions.h"
#include <string>
#include <iomanip>
#include "FontInfo.h"
#include "utils/extractFileName.h"
#include "external/lodepng/lodepng.h"
#include "utils/getNumberLen.h"

//TODO: read .bmfc files (BMFont configuration file)

std::vector<rbp::RectSize> App::getGlyphRectangles(const Glyphs &glyphs, const std::uint32_t additionalWidth, const std::uint32_t additionalHeight, const Config& config)
{
    std::vector<rbp::RectSize> result;
    for (const auto& kv : glyphs)
    {
        const auto& glyphInfo = kv.second;
        if (!glyphInfo.isEmpty()) {
            auto width = glyphInfo.width + additionalWidth;
            auto height = glyphInfo.height + additionalHeight;
            width = ((width + config.alignment.hor - 1) / config.alignment.hor) * config.alignment.hor;
            height = ((height + config.alignment.ver - 1) / config.alignment.ver) * config.alignment.ver;
            result.emplace_back(width, height, kv.first);
        }
    }
    return result;
}

App::Glyphs App::collectGlyphInfo(const ft::Font& font, const std::set<std::uint32_t>& codes)
{
    Glyphs result;

    for (const auto& id : codes)
    {
        GlyphInfo glyphInfo;

        if (font.isGlyphProvided(id))
        {
            ft::Font::GlyphMetrics glyphMetrics = font.renderGlyph(nullptr, 0, 0, 0, 0, id, 0);
            glyphInfo.width = glyphMetrics.width;
            glyphInfo.height = glyphMetrics.height;
            glyphInfo.xAdvance = glyphMetrics.horiAdvance;
            glyphInfo.xOffset = glyphMetrics.horiBearingX;
            glyphInfo.yOffset = font.ascent - glyphMetrics.horiBearingY;
            result[id] = glyphInfo;
        }
        else
        {
            std::cout << "warning: glyph " << id << " not found";
            if (id == 65279)
                std::cout << " (it looks like Unicode byte order mark (BOM))";
            std::cout << "." << std::endl;
        }
    }

    return result;
}

std::vector<Config::Size> App::arrangeGlyphs(Glyphs& glyphs, const Config& config)
{
    const auto additionalWidth = config.spacing.hor + config.padding.left + config.padding.right;
    const auto additionalHeight = config.spacing.ver + config.padding.up + config.padding.down;
    std::vector<Config::Size> result;

    auto glyphRectangles = getGlyphRectangles(glyphs, additionalWidth, additionalHeight, config);

    rbp::MaxRectsBinPack mrbp;

    for (;;)
    {
        std::vector<rbp::Rect> arrangedRectangles;
        auto glyphRectanglesCopy = glyphRectangles;
        Config::Size lastSize;

        uint64_t allGlyphSquare = 0;
        for (const auto& i : glyphRectangles)
            allGlyphSquare += static_cast<uint64_t>(i.width) * i.height;
        //std::cout << "allGlyphSquare: " << allGlyphSquare << "\n";

        //int tryCount = 0;
        for (size_t i = 0; i < config.textureSizeList.size(); ++i)
        {
            const auto& ss = config.textureSizeList[i];

            //TODO: check workAreaW,H
            const auto workAreaW = ss.w - config.spacing.hor;
            const auto workAreaH = ss.h - config.spacing.ver;

            uint64_t textureSquare = static_cast<uint64_t>(workAreaW) * workAreaH;
            if (textureSquare < allGlyphSquare && i + 1 < config.textureSizeList.size())
                continue;

            lastSize = ss;
            glyphRectangles = glyphRectanglesCopy;

            mrbp.Init(workAreaW, workAreaH);
            mrbp.Insert(glyphRectangles, arrangedRectangles, rbp::MaxRectsBinPack::RectBestAreaFit);

            if (glyphRectangles.empty())
                break;
        }

        //std::cout << "tryCount: " << tryCount << "\n";

        if (arrangedRectangles.empty())
        {
            if (!glyphRectangles.empty())
                throw std::runtime_error("can not fit glyphs into texture");
            break;
        }

        std::uint32_t maxX = 0;
        std::uint32_t maxY = 0;
        for (const auto& r: arrangedRectangles)
        {
            std::uint32_t x = r.x + config.spacing.hor;
            std::uint32_t y = r.y + config.spacing.ver;

            glyphs[r.tag].x = x;
            glyphs[r.tag].y = y;
            glyphs[r.tag].page = static_cast<std::uint32_t>(result.size());

            if (maxX < x + r.width)
                maxX = x + r.width;
            if (maxY < y + r.height)
                maxY = y + r.height;
        }
        if (config.cropTexturesWidth)
            lastSize.w = maxX;
        if (config.cropTexturesHeight)
            lastSize.h = maxY;

        result.push_back(lastSize);
    }

    return result;
}

void App::savePng(const std::string& fileName, const std::uint32_t* buffer, const std::uint32_t w, const std::uint32_t h, const bool withAlpha)
{
    std::vector<std::uint8_t> png;
    lodepng::State state;

    state.encoder.add_id = 0; // Don't add LodePNG version chunk to save more bytes
    state.encoder.auto_convert = 0;
    state.info_png.color.colortype = withAlpha ? LCT_RGBA : LCT_RGB;
    state.encoder.zlibsettings.windowsize = 32768; // Use maximum possible window size for best compression

    auto error = lodepng::encode(png, reinterpret_cast<const unsigned char*>(buffer), w, h, state);
    if (error)
        throw std::runtime_error("png encoder error " + std::to_string(error) + ": " + lodepng_error_text(error));

    error = lodepng::save_file(png, fileName);
    if (error)
        throw std::runtime_error("png save to file error " + std::to_string(error) + ": " + lodepng_error_text(error));
}

std::vector<std::string> App::renderTextures(const Glyphs& glyphs, const Config& config, const ft::Font& font, const std::vector<Config::Size>& pages)
{
    std::vector<std::string> fileNames;
    if (pages.empty())
        return {};

    const auto pageNameDigits = getNumberLen(pages.size() - 1);

    for (std::uint32_t page = 0; page < pages.size(); ++page)
    {
        const Config::Size& s = pages[page];
        std::vector<std::uint32_t> surface(s.w * s.h, config.color.getBGR());

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

                font.renderGlyph(&surface[0], s.w, s.h, x, y,
                        kv.first, config.color.getBGR());
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
                const std::uint32_t a0 = (*cur) >> 24u;
                const std::uint32_t a1 = 256 - a0;
                const std::uint32_t rb1 = (a1 * (bgColor & 0xFF00FFu)) >> 8u;
                const std::uint32_t rb2 = (a0 * (fgColor & 0xFF00FFu)) >> 8u;
                const std::uint32_t g1  = (a1 * (bgColor & 0x00FF00u)) >> 8u;
                const std::uint32_t g2  = (a0 * (fgColor & 0x00FF00u)) >> 8u;
                *cur =  ((rb1 | rb2) & 0xFF00FFu) + ((g1 | g2) & 0x00FF00u);
                ++cur;
            }
        }

        std::stringstream ss;
        ss << config.output;
        if (config.textureNameSuffix != Config::TextureNameSuffix::None)
        {
            ss << "_";
            if (config.textureNameSuffix == Config::TextureNameSuffix::IndexAligned)
                ss << std::setfill ('0') << std::setw(pageNameDigits);
            ss << page;
        }
        ss << ".png";
        const auto fileName = ss.str();
        fileNames.push_back(extractFileName(fileName));

        savePng(fileName, &surface[0], s.w, s.h, config.backgroundTransparent);
    }

    return fileNames;
}

void App::writeFontInfoFile(const Glyphs& glyphs, const Config& config, const ft::Font& font,
        const std::vector<std::string>& fileNames, const std::vector<Config::Size>& pages)
{
    if (!fileNames.empty())
        for (size_t i = 0; i < fileNames.size() - 1; ++i)
            for (size_t k = i + 1; k < fileNames.size(); ++k)
                if (fileNames[i] == fileNames[k])
                    throw std::runtime_error("textures have the same names");

    bool pagesHaveDifferentSize = false;
    if (pages.size() > 1)
    {
        for (size_t i = 1; i < pages.size(); ++i)
        {
            if (pages[0].w != pages[i].w || pages[0].h != pages[i].h)
            {
                pagesHaveDifferentSize = true;
                break;
            }
        }
    }

    FontInfo f;

    f.info.face = font.getFamilyNameOr("unknown");
    f.info.size = -static_cast<std::int16_t>(config.fontSize);
    f.info.smooth = config.monochrome;
    f.info.unicode = true;
    f.info.bold = font.isBold();
    f.info.italic = font.isItalic();
    f.info.stretchH = 100;
    f.info.aa = 1;
    f.info.padding.up = static_cast<std::uint8_t>(config.padding.up);
    f.info.padding.right = static_cast<std::uint8_t>(config.padding.right);
    f.info.padding.down = static_cast<std::uint8_t>(config.padding.down);
    f.info.padding.left = static_cast<std::uint8_t>(config.padding.left);
    f.info.spacing.horizontal = static_cast<std::uint8_t>(config.spacing.hor);
    f.info.spacing.vertical = static_cast<std::uint8_t>(config.spacing.ver);

    f.common.lineHeight = static_cast<std::uint16_t>(font.height);
    f.common.base = static_cast<std::uint16_t>(font.ascent);
    if (!pagesHaveDifferentSize && !pages.empty())
    {
        f.common.scaleW = static_cast<std::uint16_t>(pages.front().w);
        f.common.scaleH = static_cast<std::uint16_t>(pages.front().h);
    }
    f.common.alphaChnl = 0;
    f.common.redChnl = 4;
    f.common.greenChnl = 4;
    f.common.blueChnl = 4;
    f.common.totalHeight = static_cast<std::uint16_t>(font.totalHeight);

    f.pages = fileNames;

    for (const auto& kv: glyphs)
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

    if (config.kerningPairs != Config::KerningPairs::Disabled)
    {
        auto chars(config.chars);

        ft::Font::KerningMode kerningMode = ft::Font::KerningMode::Basic;
        if (config.kerningPairs == Config::KerningPairs::Regular)
            kerningMode = ft::Font::KerningMode::Regular;
        if (config.kerningPairs == Config::KerningPairs::Extended)
            kerningMode = ft::Font::KerningMode::Extended;

        for (const auto& ch0 : config.chars)
        {
            for (const auto& ch1 : chars)
            {
                const auto k = static_cast<std::int16_t>(font.getKerning(ch0, ch1, kerningMode));
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

    f.extraInfo = config.extraInfo;

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
        case Config::DataFormat::Cbor:
            f.writeToCborFile(dataFileName);
            break;
    }
}

void App::execute(const int argc, char* argv[])
{
    const auto config = ProgramOptions::parseCommandLine(argc, argv);

    ft::Library library;
    if (config.verbose)
        std::cout << "freetype " << library.getVersionString() << "\n";

    ft::Font font(library, config.fontFile, config.fontSize, 0, config.monochrome);

    auto glyphs = collectGlyphInfo(font, config.chars);
    const auto pages = arrangeGlyphs(glyphs, config);
    if (config.useMaxTextureCount && pages.size() > config.maxTextureCount)
        throw std::runtime_error("too many generated textures (more than --max-texture-count)");

    const auto fileNames = renderTextures(glyphs, config, font, pages);
    writeFontInfoFile(glyphs, config, font, fileNames, pages);
}
