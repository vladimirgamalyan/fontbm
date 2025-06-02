#include "App.h"

#include <hb-ft.h> // HarfBuzz FreeType integration
#include <hb.h>

#include <iomanip>
#include <string>

#include "FontInfo.h"
#include "ProgramOptions.h"
#include "external/lodepng/lodepng.h"
#include "utils/extractFileName.h"
#include "utils/getNumberLen.h"

// TODO: read .bmfc files (BMFont configuration file)

std::set<std::uint32_t> App::collectAllChars(const ft::Font& font) {
    return font.collectChars();
}

std::vector<rbp::RectSize> App::getGlyphRectangles(const Glyphs &glyphs, const std::uint32_t additionalWidth, const std::uint32_t additionalHeight,
                                                   const Config &config)
{
    std::vector<rbp::RectSize> result;
    for (const auto &kv : glyphs)
    {
        const auto &glyphInfo = kv.second;
        if (!glyphInfo.isEmpty())
        {
            auto width = glyphInfo.width + additionalWidth;
            auto height = glyphInfo.height + additionalHeight;
            width = ((width + config.alignment.hor - 1) / config.alignment.hor) * config.alignment.hor;
            height = ((height + config.alignment.ver - 1) / config.alignment.ver) * config.alignment.ver;
            result.emplace_back(width, height, kv.first);
        }
    }
    return result;
}

std::set<std::tuple<std::uint32_t, std::uint32_t, bool>> App::shapeGlyphs(const ft::Font &font, const ft::Font &secondaryFont, const std::set<std::uint32_t> &utf32codes, bool tabularNumbers,
                                                                          bool slashedZero)
{

    std::vector<uint32_t> utf32codesVector;
    std::set<std::tuple<std::uint32_t, std::uint32_t, bool>> shaped_glyphs;

    for (const auto &id : utf32codes)
    {
        bool glyphIndexFound = false;
        // Handle numbers only for tabular case
        if (id >= 0x30 && id <= 0x39)
        {
            utf32codesVector.push_back(id);
            // Not good, we always assume we have numbers
            glyphIndexFound = true;
        }
        else
        {
            FT_UInt glyphIndex = FT_Get_Char_Index(font.face, id);
            if (glyphIndex == 0) 
            {
                if (secondaryFont.valid)
                {
                    glyphIndex = FT_Get_Char_Index(secondaryFont.face, id);
                    if (glyphIndex) 
                    {
                        shaped_glyphs.insert({glyphIndex, id, true});
                        glyphIndexFound = true;
                    }
                }
            } 
            else 
            {
                shaped_glyphs.insert({glyphIndex, id, false});
                glyphIndexFound = true;
            }
        }
        if (!glyphIndexFound) {
            std::cout << "warning: glyph " << id << " not found";
            if (id == 65279)
                std::cout << " (it looks like Unicode byte order mark (BOM))";
            std::cout << "." << std::endl;
        }
    }

    if (utf32codesVector.size()) 
    {
        hb_font_t *hb_font = hb_ft_font_create(font.face, nullptr);
        hb_buffer_t *hb_buffer = hb_buffer_create();

        for (auto id : utf32codesVector) 
        {
            uint32_t code = id;
            hb_buffer_add_utf32(hb_buffer, &code, 1, 0, -1);
        }

        hb_buffer_set_direction(hb_buffer, HB_DIRECTION_LTR);
        hb_buffer_set_script(hb_buffer, HB_SCRIPT_COMMON);
        hb_buffer_set_language(hb_buffer, hb_language_from_string("en", -1));

        hb_feature_t feature[3] = {};
        feature[0].tag = HB_TAG('t', 'n', 'u', 'm'); // Tag for Tabular Figures
        feature[0].value = tabularNumbers ? 1 : 0;   // 1 to enable, 0 to disable
        feature[0].start = 0;                        // Apply from the start of the buffer
        feature[0].end = (unsigned int)-1;           // Apply to the end of the buffer

        feature[1].tag = HB_TAG('z', 'e', 'r', 'o'); // Tag for slashed zeros
        feature[1].value = slashedZero ? 1 : 0;      // 1 to enable, 0 to disable
        feature[1].start = 0;                        // Apply from the start of the buffer
        feature[1].end = (unsigned int)-1;           // Apply to the end of the buffer

        feature[2].tag = HB_TAG('l', 'i', 'g', 'a'); // Tag for enabling ligatures
        feature[2].value = 0;                        // 1 to enable, 0 to disable
        feature[2].start = 0;                        // Apply from the start of the buffer
        feature[2].end = (unsigned int)-1;           // Apply to the end of the buffer

        hb_shape(hb_font, hb_buffer, &feature[0], 3);

        unsigned int glyph_count = 0;
        hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(hb_buffer, &glyph_count);

        for (unsigned int i = 0; i < glyph_count; i++)
        {
            hb_codepoint_t glyph_index = glyph_info[i].codepoint;
            shaped_glyphs.insert({glyph_index, utf32codesVector[i], false});
        }

        hb_buffer_destroy(hb_buffer);
        hb_font_destroy(hb_font);
    }

    return shaped_glyphs;
}

App::Glyphs App::collectGlyphInfo(const ft::Font &font, const ft::Font &secondaryFont, const std::set<std::uint32_t> &utf32codes, bool tabularNumbers, bool slashedZero)
{
    Glyphs result;

    const auto shaped_glyphs = shapeGlyphs(font, secondaryFont, utf32codes, tabularNumbers, slashedZero);
    for (const auto &id : shaped_glyphs)
    {
        if (std::get<0>(id))
        {
            GlyphInfo glyphInfo;
            ft::Font::GlyphMetrics glyphMetrics{};
            if (std::get<2>(id) && secondaryFont.valid) {
                glyphMetrics = secondaryFont.renderGlyph(nullptr, 0, 0, 0, 0, std::get<0>(id), 0);
            } else {
                glyphMetrics = font.renderGlyph(nullptr, 0, 0, 0, 0, std::get<0>(id), 0);
            }
            glyphInfo.utf32 = std::get<1>(id);
            glyphInfo.width = glyphMetrics.width;
            glyphInfo.height = glyphMetrics.height;
            glyphInfo.xAdvance = glyphMetrics.horiAdvance;
            glyphInfo.xOffset = glyphMetrics.horiBearingX;
            glyphInfo.yOffset = font.ascent - glyphMetrics.horiBearingY;
            glyphInfo.secondaryFont = std::get<2>(id);
            result[std::get<0>(id)] = glyphInfo;
        }
    }

    return result;
}

std::vector<Config::Size> App::arrangeGlyphs(Glyphs &glyphs, const Config &config)
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
        for (const auto &i : glyphRectangles)
            allGlyphSquare += static_cast<uint64_t>(i.width) * i.height;

        for (size_t i = 0; i < config.textureSizeList.size(); ++i)
        {
            const auto &ss = config.textureSizeList[i];

            // TODO: check workAreaW,H
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

        if (arrangedRectangles.empty())
        {
            if (!glyphRectangles.empty())
                throw std::runtime_error("can not fit glyphs into texture");
            break;
        }

        std::uint32_t maxX = 0;
        std::uint32_t maxY = 0;
        for (const auto &r : arrangedRectangles)
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

void App::savePng(const std::string &fileName, const std::uint32_t *buffer, const std::uint32_t w, const std::uint32_t h, const bool withAlpha)
{
    std::vector<std::uint8_t> png;
    lodepng::State state;

    state.encoder.add_id = 0; // Don't add LodePNG version chunk to save more bytes
    state.encoder.auto_convert = 0;
    state.info_png.color.colortype = withAlpha ? LCT_RGBA : LCT_RGB;
    state.encoder.zlibsettings.windowsize = 32768; // Use maximum possible window size for best compression

    auto error = lodepng::encode(png, reinterpret_cast<const unsigned char *>(buffer), w, h, state);
    if (error)
        throw std::runtime_error("png encoder error " + std::to_string(error) + ": " + lodepng_error_text(error));

    error = lodepng::save_file(png, fileName);
    if (error)
        throw std::runtime_error("png save to file error " + std::to_string(error) + ": " + lodepng_error_text(error));
}

std::vector<std::string> App::renderTextures(const Glyphs &glyphs, const Config &config, const ft::Font &font, const ft::Font &secondaryFont, const std::vector<Config::Size> &pages)
{
    std::vector<std::string> fileNames;
    if (pages.empty())
        return {};

    const auto pageNameDigits = getNumberLen(pages.size() - 1);

    for (std::uint32_t page = 0; page < pages.size(); ++page)
    {
        const Config::Size &s = pages[page];
        std::vector<std::uint32_t> surface(s.w * s.h, config.color.getBGR());

        // Render every glyph
        // TODO: do not repeat same glyphs (with same index)
        for (const auto &kv : glyphs)
        {
            const auto &glyph = kv.second;
            if (glyph.page != page)
                continue;

            if (!glyph.isEmpty())
            {
                const auto x = glyph.x + config.padding.left;
                const auto y = glyph.y + config.padding.up;

                if (glyph.secondaryFont && secondaryFont.valid) {
                    secondaryFont.renderGlyph(&surface[0], s.w, s.h, x, y, kv.first, config.color.getBGR());
                } else {
                    font.renderGlyph(&surface[0], s.w, s.h, x, y, kv.first, config.color.getBGR());
                }
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
                const std::uint32_t g1 = (a1 * (bgColor & 0x00FF00u)) >> 8u;
                const std::uint32_t g2 = (a0 * (fgColor & 0x00FF00u)) >> 8u;
                *cur = ((rb1 | rb2) & 0xFF00FFu) + ((g1 | g2) & 0x00FF00u);
                ++cur;
            }
        }

        std::stringstream ss;
        ss << config.output;
        if (config.textureNameSuffix != Config::TextureNameSuffix::None)
        {
            ss << "_";
            if (config.textureNameSuffix == Config::TextureNameSuffix::IndexAligned)
                ss << std::setfill('0') << std::setw(pageNameDigits);
            ss << page;
        }
        ss << ".png";
        const auto fileName = ss.str();
        fileNames.push_back(extractFileName(fileName));

        savePng(fileName, &surface[0], s.w, s.h, config.backgroundTransparent);
    }

    return fileNames;
}

void App::writeFontInfoFile(const Glyphs &glyphs, const Config &config, const ft::Font &font, const ft::Font &secondaryFont, const std::vector<std::string> &fileNames,
                            const std::vector<Config::Size> &pages)
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
    f.info.style = font.getStyleNameOr("unknown");
    f.info.size = -static_cast<std::int16_t>(config.fontSize);
    f.info.smooth = !config.monochrome;
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
    f.common.descent = static_cast<std::int16_t>(font.descent);
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

    std::vector<GlyphInfo> sortedGlyphs;
    sortedGlyphs.reserve(glyphs.size());
    for (const auto &kv : glyphs)
        sortedGlyphs.push_back(kv.second);
    std::sort(sortedGlyphs.begin(), sortedGlyphs.end(), [](const GlyphInfo &a, const GlyphInfo &b)
              { return a.utf32 < b.utf32; });

    // Official unicode characters with property White_Space = yes
    static const std::set<char32_t> white_space = {
        U'\u0009', // CHARACTER TABULATION (HT)
        U'\u000A', // LINE FEED (LF)
        U'\u000B', // LINE TABULATION (VT)
        U'\u000C', // FORM FEED (FF)
        U'\u000D', // CARRIAGE RETURN (CR)
        U'\u0020', // SPACE
        U'\u0085', // NEXT LINE (NEL)
        U'\u00A0', // NO‑BREAK SPACE
        U'\u1680', // OGHAM SPACE MARK
        U'\u2000', // EN QUAD
        U'\u2001', // EM QUAD
        U'\u2002', // EN SPACE
        U'\u2003', // EM SPACE
        U'\u2004', // THREE‑PER‑EM SPACE
        U'\u2005', // FOUR‑PER‑EM SPACE
        U'\u2006', // SIX‑PER‑EM SPACE
        U'\u2007', // FIGURE SPACE
        U'\u2008', // PUNCTUATION SPACE
        U'\u2009', // THIN SPACE
        U'\u200A', // HAIR SPACE
        U'\u2028', // LINE SEPARATOR
        U'\u2029', // PARAGRAPH SEPARATOR
        U'\u202F', // NARROW NO‑BREAK SPACE
        U'\u205F', // MEDIUM MATHEMATICAL SPACE
        U'\u3000'  // IDEOGRAPHIC SPACE
    };

    for (const auto &glyph : sortedGlyphs)
    {
        // TODO: page = 0 for empty glyphs.
        FontInfo::Char c;
        if (!glyph.isEmpty() || white_space.count(glyph.utf32) > 0)
        {
            c.id = static_cast<std::uint32_t>(glyph.utf32);
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
        auto chars = shapeGlyphs(font, secondaryFont, config.chars, config.tabularNumbers, config.slashedZero);

        ft::Font::KerningMode kerningMode = ft::Font::KerningMode::Basic;
        if (config.kerningPairs == Config::KerningPairs::Regular)
            kerningMode = ft::Font::KerningMode::Regular;
        if (config.kerningPairs == Config::KerningPairs::Extended)
            kerningMode = ft::Font::KerningMode::Extended;

        // Extended means we capture calculated kerning values if we can
        if (kerningMode == ft::Font::KerningMode::Extended)
        {

            size_t regularCount = 0;
            size_t specialCount = 0;
            size_t reshapeCount = 0;

            hb_font_t *hb_font = hb_ft_font_create(font.face, nullptr);
            int x_scale = 0;
            int y_scale = 0;
            hb_font_get_scale(hb_font, &x_scale, &y_scale);
            for (const auto &ch0 : glyphs)
            {
                for (const auto &ch1 : glyphs)
                {
                    // Sorry harfbuzz devs; I know this is the worst thing
                    // to do and will break in many ways. But it works for
                    // our use case.

                    // No kerning pairs if secondary font is involved
                    if ( std::get<1>(ch0).secondaryFont ||
                         std::get<1>(ch1).secondaryFont ) {
                        continue;
                    }

                    hb_codepoint_t codepoint_l = std::get<0>(ch0);
                    hb_codepoint_t codepoint_r = std::get<0>(ch1);
                    hb_codepoint_t utf32_l = std::get<1>(ch0).utf32;
                    hb_codepoint_t utf32_r = std::get<1>(ch1).utf32;

                    hb_buffer_t *hb_buffer = hb_buffer_create();
                    hb_buffer_set_direction(hb_buffer, HB_DIRECTION_LTR);
                    hb_buffer_set_script(hb_buffer, HB_SCRIPT_COMMON);
                    hb_buffer_set_language(hb_buffer, hb_language_from_string("en", -1));
                    hb_buffer_add_utf32(hb_buffer, &utf32_l, 1, 0, -1);
                    hb_buffer_add_utf32(hb_buffer, &utf32_r, 1, 0, -1);

                    hb_feature_t feature[3] = {};
                    feature[0].tag = HB_TAG('t', 'n', 'u', 'm');      // Tag for Tabular Figures
                    feature[0].value = config.tabularNumbers ? 1 : 0; // 1 to enable, 0 to disable
                    feature[0].start = 0;                             // Apply from the start of the buffer
                    feature[0].end = (unsigned int)-1;                // Apply to the end of the buffer

                    feature[1].tag = HB_TAG('z', 'e', 'r', 'o');   // Tag for slashed zeros
                    feature[1].value = config.slashedZero ? 1 : 0; // 1 to enable, 0 to disable
                    feature[1].start = 0;                          // Apply from the start of the buffer
                    feature[1].end = (unsigned int)-1;             // Apply to the end of the buffer

                    // Required otherwise we get tons of ligatures with modern fonts like SF-Pro
                    feature[2].tag = HB_TAG('l', 'i', 'g', 'a'); // Tag for enabling ligatures
                    feature[2].value = 0;                        // 1 to enable, 0 to disable
                    feature[2].start = 0;                        // Apply from the start of the buffer
                    feature[2].end = (unsigned int)-1;           // Apply to the end of the buffer

                    hb_shape(hb_font, hb_buffer, &feature[0], 3);

                    unsigned int glyph_count = 0;
                    hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(hb_buffer, &glyph_count);
                    // Make sure that hb_shape has not added glyphs
                    if (glyph_count != 2)
                    {
                        reshapeCount++;
                        hb_buffer_destroy(hb_buffer);
                        continue;
                    }

                    // Make sure that hb_shape has not changed glyphs on us.
                    if (glyph_info[0].codepoint != codepoint_l || glyph_info[1].codepoint != codepoint_r)
                    {
                        reshapeCount++;
                        hb_buffer_destroy(hb_buffer);
                        continue;
                    }

                    // Make sure that hb_shape has not added glyphs
                    hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(hb_buffer, &glyph_count);
                    if (glyph_count != 2)
                    {
                        reshapeCount++;
                        hb_buffer_destroy(hb_buffer);
                        continue;
                    }

                    // Convert back to pixel size
                    float advance = float(config.fontSize) * float(glyph_pos[0].x_advance) / float(x_scale);

                    // Convert back to integer pixels
                    // We use ceil/floor here to favor the original advance and reduce pairs.
                    int advanceInt = int(ceil(advance));
                    if ( advance > float(std::get<1>(ch0).xAdvance)) {
                        advanceInt = int(floor(advance));
                    }

                    // If we have something else than a regular advance and things look good,
                    // i.e. there has been no reshaping we can actually record it as a new 'kerning' value
                    if (advanceInt != std::get<1>(ch0).xAdvance)
                    {
                        FontInfo::Kerning kerning;
                        kerning.first = std::get<1>(ch0).utf32;
                        kerning.second = std::get<1>(ch1).utf32;
                        kerning.amount = advanceInt - std::get<1>(ch0).xAdvance;
                        f.kernings.push_back(kerning);
                        specialCount++;
                    }
                    else
                    {
                        regularCount++;
                    }
                    hb_buffer_destroy(hb_buffer);
                }
            }
            hb_font_destroy(hb_font);
            (void)specialCount;
            (void)regularCount;
            (void)reshapeCount;
            // printf("Regular advances %d, special advances %d, reshape events %d\n", int(regularCount), int(specialCount), int(reshapeCount));
        }
        else
        { // Don't do the old extended method using FT, the above will give way better results
            for (const auto &ch0 : glyphs)
            {
                for (const auto &ch1 : glyphs)
                {
                    // No kerning pairs if secondary font is involved
                    if ( std::get<1>(ch0).secondaryFont ||
                         std::get<1>(ch1).secondaryFont ) {
                        continue;
                    }

                    const auto k = static_cast<std::int16_t>(font.getKerning(std::get<1>(ch0).utf32, std::get<1>(ch1).utf32, kerningMode));
                    if (k)
                    {
                        FontInfo::Kerning kerning;
                        kerning.first = std::get<1>(ch0).utf32;
                        kerning.second = std::get<1>(ch1).utf32;
                        kerning.amount = k;
                        f.kernings.push_back(kerning);
                    }
                }
            }
        }
    }

    f.extraInfo = config.extraInfo;

    const auto dataFileName = config.output + ".fnt";
    switch (config.dataFormat)
    {
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

void App::execute(const int argc, char *argv[])
{
    const auto config = ProgramOptions::parseCommandLine(argc, argv);

    ft::Library library;
    if (config.verbose)
        std::cout << "freetype " << library.getVersionString() << "\n";

    ft::Font font(library, config.fontFile, config.fontSize, 0, config.monochrome, config.lightHinting, config.noHinting);
    ft::Font secondaryFont(library, config.secondaryFontFile, config.fontSize, 0, config.monochrome, config.lightHinting, config.noHinting);
    auto glyphs = collectGlyphInfo(font, secondaryFont, config.allChars ? collectAllChars(font) : config.chars, config.tabularNumbers, config.slashedZero);
    const auto pages = arrangeGlyphs(glyphs, config);
    if (config.useMaxTextureCount && pages.size() > config.maxTextureCount)
        throw std::runtime_error("too many generated textures (more than --max-texture-count)");

    const auto fileNames = renderTextures(glyphs, config, font, secondaryFont, pages);
    writeFontInfoFile(glyphs, config, font, secondaryFont, fileNames, pages);
}
