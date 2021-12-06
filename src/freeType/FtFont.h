#pragma once
#include <iostream>
#include <cmath>
#include "FtInclude.h"
#include "FtException.h"

/* Handy routines for converting from fixed point */
#define FT_FLOOR(X) (((X) & -64) / 64)
#define FT_CEIL(X)  ((((X) + 63) & -64) / 64)

/* Set and retrieve the font style */
#define TTF_STYLE_NORMAL        0x00
#define TTF_STYLE_BOLD          0x01
#define TTF_STYLE_ITALIC        0x02
#define TTF_STYLE_UNDERLINE     0x04
#define TTF_STYLE_STRIKETHROUGH 0x08

/* Handle a style only if the font does not already handle it */
#define TTF_HANDLE_STYLE_BOLD() ((style & TTF_STYLE_BOLD) && \
                                    !(face_style & TTF_STYLE_BOLD))
#define TTF_HANDLE_STYLE_ITALIC() ((style & TTF_STYLE_ITALIC) && \
                                      !(face_style & TTF_STYLE_ITALIC))
#define TTF_HANDLE_STYLE_UNDERLINE(font) ((font)->style & TTF_STYLE_UNDERLINE)
#define TTF_HANDLE_STYLE_STRIKETHROUGH(font) ((font)->style & TTF_STYLE_STRIKETHROUGH)

namespace ft {

class Font
{
public:

    struct GlyphMetrics
    {
        std::uint32_t width; // This is the width of the glyph image's bounding box. It is independent of the layout direction.
        std::uint32_t height; // This is the height of the glyph image's bounding box. It is independent of the layout direction.
        std::int32_t horiBearingX; // For horizontal text layouts, this is the horizontal distance from the current cursor position to the leftmost border of the glyph image's bounding box.
        std::int32_t horiBearingY; // For horizontal text layouts, this is the vertical distance from the current cursor position (on the baseline) to the topmost border of the glyph image's bounding box.
        std::int32_t horiAdvance; // For horizontal text layouts, this is the horizontal distance to increment the pen position when the glyph is drawn as part of a string of text.
        std::int32_t lsbDelta;
        std::int32_t rsbDelta;
    };

    Font(Library& library, const std::string& fontFile, int ptsize, const int faceIndex, const bool monochrome)
        : library(library), monochrome_(monochrome)
    {
        if (!library.library)
            throw std::runtime_error("Library is not initialized");

        auto error = FT_New_Face(library.library, fontFile.c_str(), faceIndex, &face);
        if (error == FT_Err_Unknown_File_Format)
            throw Exception("Unsupported font format", error);
        if (error)
            throw Exception("Couldn't load font file", error);

        if (!face->charmap)
        {
            FT_Done_Face(face);
            throw std::runtime_error("Font doesn't contain a Unicode charmap");
        }

        if (FT_IS_SCALABLE(face))
        {
            /* Set the character size and use default DPI (72) */
            error = FT_Set_Pixel_Sizes(face, ptsize, ptsize);
            if (error) {
                FT_Done_Face(face);
                throw Exception("Couldn't set font size", error);
            }

            /* Get the scalable font metrics for this font */
            const auto scale = face->size->metrics.y_scale;
            yMin = FT_FLOOR(FT_MulFix(face->bbox.yMin, scale));
            yMax  = FT_CEIL(FT_MulFix(face->bbox.yMax, scale));
            height  = FT_CEIL(FT_MulFix(face->height, scale));
        }
        else
        {
            /* Non-scalable font case.  ptsize determines which family
             * or series of fonts to grab from the non-scalable format.
             * It is not the point size of the font.
             * */
            if (ptsize >= face->num_fixed_sizes)
                ptsize = face->num_fixed_sizes - 1;
            font_size_family = ptsize;
            error = FT_Set_Pixel_Sizes( face,
                                    static_cast<FT_UInt>(face->available_sizes[ptsize].width),
                                    static_cast<FT_UInt>(face->available_sizes[ptsize].height ));
            //TODO: check error, free font

            /* With non-scalale fonts, Freetype2 likes to fill many of the
             * font metrics with the value of 0.  The size of the
             * non-scalable fonts must be determined differently
             * or sometimes cannot be determined.
             * */
            yMin = 0;
            yMax = face->available_sizes[ptsize].height;
            height = face->available_sizes[ptsize].height;
        }

        /* Initialize the font face style */
        face_style = TTF_STYLE_NORMAL;
        if (face->style_flags & FT_STYLE_FLAG_BOLD)
            face_style |= TTF_STYLE_BOLD;

        if (face->style_flags & FT_STYLE_FLAG_ITALIC)
            face_style |= TTF_STYLE_ITALIC;

        /* Set the default font style */
        style = face_style;
        outline = 0;
        kerning = 1;
        glyph_overhang = face->size->metrics.y_ppem / 10;
        /* x offset = cos(((90.0-12)/360)*2*M_PI), or 12 degree angle */
        glyph_italics = 0.207f;
        glyph_italics *= height;

        totalHeight = yMax - yMin;
    }

    ~Font()
    {
        FT_Done_Face(face);
    }

    GlyphMetrics renderGlyph(std::uint32_t* buffer, std::uint32_t surfaceW, std::uint32_t surfaceH, int x, int y,
            std::uint32_t ch, std::uint32_t color) const
    {
        const auto loadFlags = getLoadFlags();
        const auto error = FT_Load_Char(face, ch, loadFlags);
        if (error)
            throw std::runtime_error("Load glyph error");

        auto slot = face->glyph;
        const auto metrics = &slot->metrics;

        GlyphMetrics glyphMetrics;
        glyphMetrics.width = slot->bitmap.width;
        glyphMetrics.height = slot->bitmap.rows;
        glyphMetrics.horiBearingX = FT_FLOOR(metrics->horiBearingX);
        glyphMetrics.horiBearingY = FT_FLOOR(metrics->horiBearingY);
        glyphMetrics.horiAdvance = FT_CEIL(metrics->horiAdvance);
        glyphMetrics.lsbDelta = slot->lsb_delta;
        glyphMetrics.rsbDelta = slot->rsb_delta;

        if (buffer)
        {
            const auto dst_check = buffer + surfaceW * surfaceH;
            color &= 0xffffffu;

            for (std::uint32_t row = 0; row < glyphMetrics.height; ++row)
            {
                std::uint32_t *dst = buffer + (y + row) * surfaceW + x;
                const std::uint8_t *src = slot->bitmap.buffer + slot->bitmap.pitch * row;

                std::vector<std::uint8_t> unpacked;
                if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
                    unpacked.reserve(slot->bitmap.width);
                    for (int byte = 0; byte < slot->bitmap.pitch; ++byte)
                        for (std::uint8_t mask = 0x80; mask; mask = mask >> 1u)
                            unpacked.push_back(src[byte] & mask ? 0xff : 0x00);
                    src = unpacked.data();
                }

                for (auto col = glyphMetrics.width; col > 0 && dst < dst_check; --col)
                {
                    const std::uint32_t alpha = *src++;
                    *dst++ = color | (alpha << 24u);
                }
            }
        }

        return glyphMetrics;
    }

    int isGlyphProvided(FT_ULong ch) const
    {
        return FT_Get_Char_Index(face, ch) && !FT_Load_Char(face, ch, getLoadFlags());
    }

    int getKerning(const std::uint32_t left, const std::uint32_t right) const
    {
        if (!FT_HAS_KERNING(face))
            return 0;

        FT_Vector kerning;

        const auto indexLeft = FT_Get_Char_Index(face, left);
        const auto indexRight = FT_Get_Char_Index(face, right);

        const auto error = FT_Get_Kerning(face, indexLeft, indexRight, FT_KERNING_UNFITTED, &kerning);
        if (error)
            throw std::runtime_error("Couldn't find glyphs kerning");

        // X advance is already in pixels for bitmap fonts
        if (!FT_IS_SCALABLE(face))
            return static_cast<float>(kerning.x);

        float firstRsbDelta = renderGlyph(nullptr, 0, 0, 0, 0, left, 0).rsbDelta;
        float secondLsbDelta = renderGlyph(nullptr, 0, 0, 0, 0, right, 0).lsbDelta;

        return std::floor((secondLsbDelta - firstRsbDelta + static_cast<float>(kerning.x) + 32) / static_cast<float>(1 << 6));
    }

    void debugInfo() const
    {
        std::cout << "num_charmaps " << face->num_charmaps << std::endl;
        std::cout << "num_glyphs " << face->num_glyphs << std::endl;

        /*
            platform id         encoding id
            3                   1               Windows Unicode
            3                   0               Windows Symbol
            2                   1               ISO Unicode
            0                                   Apple Unicode
            Other constants can be found in file FT_TRUETYPE_IDS_H
         */

        for (auto i = 0; i < face->num_charmaps; i++) {
            const auto charmap = face->charmaps[i];
            std::cout << charmap->platform_id << ", " << charmap->encoding_id << std::endl;
        }

        const auto scale = face->size->metrics.y_scale;
        std::cout << "face->size->metrics.y_scale " << scale << " (" << face->size->metrics.y_scale / 64.0 << ")" << "\n";
        std::cout << "face->size->metrics.y_ppem " << face->size->metrics.x_ppem << "\n";
        std::cout << "face->bbox.yMax " << FT_CEIL(FT_MulFix(face->bbox.yMax, scale)) << "\n";
        std::cout << "face->bbox.yMin " << FT_FLOOR(FT_MulFix(face->bbox.yMin, scale)) << "\n";
        std::cout << "face->ascender " << FT_CEIL(FT_MulFix(face->ascender, scale)) << "\n";
        std::cout << "face->descender " << FT_FLOOR(FT_MulFix(face->descender, scale)) << "\n";
        std::cout << "face->height " << FT_CEIL(FT_MulFix(face->height, scale)) << "\n";

        FT_UInt gindex;
        FT_ULong charcode = FT_Get_First_Char(face, &gindex);
        std::int32_t maxHoriBearingY = 0;
        std::uint32_t glyphCount = 0;
        std::int32_t minY = 0;
        FT_ULong charcodeMaxHoriBearingY = 0;
        FT_ULong charcodeMinY = 0;
        while (gindex)
        {
            GlyphMetrics glyphMetrics = renderGlyph(nullptr, 0, 0, 0, 0, charcode, 0);
            if (glyphMetrics.horiBearingY > maxHoriBearingY) {
                maxHoriBearingY = glyphMetrics.horiBearingY;
                charcodeMaxHoriBearingY = charcode;
            }
            std::int32_t bottom = glyphMetrics.horiBearingY - glyphMetrics.height;
            if (bottom < minY) {
                charcodeMinY = charcode;
                minY = bottom;
            }
            ++glyphCount;

            charcode = FT_Get_Next_Char(face, charcode, &gindex);
        }
        std::cout << "maxHoriBearingY " << maxHoriBearingY << ", charcode " << charcodeMaxHoriBearingY << "\n";
        std::cout << "minY " << minY << ", charcode " << charcodeMinY << "\n";
        std::cout << "glyphCount " << glyphCount << "\n";
    }

    std::string getFamilyNameOr(const std::string& defaultName) const
    {
        if (!face->family_name)
            return defaultName;
        return std::string(face->family_name);
    }

    bool isBold() const
    {
        return (style & TTF_STYLE_BOLD) != 0;
    }

    bool isItalic() const
    {
        return (style & TTF_STYLE_ITALIC) != 0;
    }

    Library& library;
    FT_Face face = nullptr;
    int height;
    int yMax;
    int yMin;
    int totalHeight = 0;

    /* For non-scalable formats, we must remember which font index size */
    int font_size_family;

    /* The font style */
    int face_style;
    int style;
    int outline;
    bool monochrome_;

    /* Whether kerning is desired */
    int kerning;

    /* Extra width in glyph bounds for text styles */
    int glyph_overhang;
    float glyph_italics;

private:
    FT_Int32 getLoadFlags() const
    {
        FT_Int32 loadFlags = FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT;
        if (monochrome_)
            loadFlags |= FT_LOAD_MONOCHROME;
        return loadFlags;
    }
};

}
