#pragma once
#include <iostream>
#include <cmath>
#include "FtInclude.h"
#include "FtException.h"
#include "../utils/StringMaker.h"

#include "../external/lunasvg/include/lunasvg.h"
#include <freetype/otsvg.h>
#include <freetype/ftbbox.h>
#include <freetype/ftmodapi.h>

/* Handy routines for converting from fixed point */
#define FT_FLOOR(X)                 (((X) & -64) / 64)
#define FT_CEIL(X)                  ((((X) + 63) & -64) / 64)
#define FT_ROUND(_VAL)              ((float)(int)((_VAL) + 0.5f))
#define FT_COL32(R,G,B,A)           (((uint32_t)(A)<<24) | ((uint32_t)(B)<<16) | ((uint32_t)(G)<<8) | ((uint32_t)(R)<<0))
#define DE_MULTIPLY(color, alpha)   (uint32_t)(255.0f * (float)color / (float)alpha + 0.5f)
#define UNUSED(x)                   (void)(x)

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
            // height  = FT_CEIL(FT_MulFix(face->height, scale));
            height =  std::lround(static_cast<float>(face->size->metrics.height) / static_cast<float>(1 << 6));
            //height =  std::lround(FT_MulFix(face->height, scale) / static_cast<float>(1 << 6));
            ascent = FT_CEIL(FT_MulFix(face->ascender, scale));
            //ascent = std::lround(FT_MulFix(face->ascender, scale) / static_cast<float>(1 << 6));
            descent = FT_FLOOR(FT_MulFix(face->descender, scale));
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
            height = face->available_sizes[ptsize].height;
            yMax = height;
            yMin = 0;
            ascent = height;
            descent = 0;
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

        SVG_RendererHooks hooks = { lunasvgPortInit, lunasvgPortFree, lunasvgPortRender, lunasvgPortPresetSlot };
        error = FT_Property_Set(library.library, "ot-svg", "svg-hooks", &hooks);
        if (error)
            throw Exception("Couldn't set svg hooks", error);
    }

    ~Font()
    {
        FT_Done_Face(face);
    }

    GlyphMetrics renderGlyph(std::uint32_t* buffer, std::uint32_t surfaceW, std::uint32_t surfaceH, int x, int y,
            std::uint32_t ch, std::uint32_t color) const
    {
        FT_Int32 loadFlags = FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_COLOR;
        if (monochrome_)
            loadFlags |= FT_LOAD_MONOCHROME;

        const int error = FT_Load_Char(face, ch, loadFlags);

       if (error)
            throw std::runtime_error(StringMaker() << "Error Load glyph " << ch << " " << error);

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
                if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
                {
                    unpacked.reserve(slot->bitmap.width);
                    for (int byte = 0; byte < slot->bitmap.pitch; ++byte)
                        for (std::uint8_t mask = 0x80; mask; mask = mask >> 1u)
                            unpacked.push_back(src[byte] & mask ? 0xff : 0x00);
                    src = unpacked.data();
                }

                switch (slot->bitmap.pixel_mode)
                {
                    case FT_PIXEL_MODE_GRAY:
                    case FT_PIXEL_MODE_MONO:
                        for (auto col = glyphMetrics.width; col > 0 && dst < dst_check; --col)
                        {
                            const std::uint32_t a = *src++;
                            *dst++ =  color | (a << 24u);
                        }
                        break;
                    case FT_PIXEL_MODE_BGRA:
                        for (auto col = glyphMetrics.width; col > 0 && dst < dst_check; --col)
                        {
                            const std::uint32_t b = *src++;
                            const std::uint32_t g = *src++;
                            const std::uint32_t r = *src++;
                            const std::uint32_t a = *src++;
                            *dst++ = FT_COL32(DE_MULTIPLY(r, a), DE_MULTIPLY(g, a), DE_MULTIPLY(b, a), a);
                        }
                        break;
                    default:
                        throw std::runtime_error("Unknown color mode!");
                        break;
                }
            }
        }

        return glyphMetrics;
    }

    int isGlyphProvided(FT_ULong ch) const
    {
        return FT_Get_Char_Index(face, ch);
    }

    struct LunasvgPortState
    {
        FT_Error            err = FT_Err_Ok;
        lunasvg::Matrix     matrix;
        std::unique_ptr<lunasvg::Document> svg = nullptr;
    };

    static FT_Error lunasvgPortInit(FT_Pointer* _state)
    {
        *_state = new LunasvgPortState();
        return FT_Err_Ok;
    }

    static void lunasvgPortFree(FT_Pointer* _state)
    {
        delete(*(LunasvgPortState**)_state);
    }

    static FT_Error lunasvgPortRender(FT_GlyphSlot slot, FT_Pointer* _state)
    {
        LunasvgPortState* state = *(LunasvgPortState**)_state;
        // If there was an error while loading the svg in lunasvgPortPresetSlot(), the renderer hook still get called, so just returns the error.
        if (state->err != FT_Err_Ok)
            return state->err;
        // rows is height, pitch (or stride) equals to width * sizeof(int32)
        lunasvg::Bitmap bitmap((uint8_t*)slot->bitmap.buffer, slot->bitmap.width, slot->bitmap.rows, slot->bitmap.pitch);
        state->svg->setMatrix(state->svg->matrix().identity()); // Reset the svg matrix to the default value
        state->svg->render(bitmap, state->matrix);              // state->matrix is already scaled and translated
        state->err = FT_Err_Ok;
        return state->err;
    }

    static FT_Error lunasvgPortPresetSlot(FT_GlyphSlot slot, FT_Bool cache, FT_Pointer* _state)
    {
        UNUSED(cache);

        FT_SVG_Document   document = (FT_SVG_Document)slot->other;
        LunasvgPortState* state = *(LunasvgPortState**)_state;
        FT_Size_Metrics&  metrics = document->metrics;
        state->svg = lunasvg::Document::loadFromData((const char*)document->svg_document, document->svg_document_length);
        if (state->svg == nullptr)
        {
            state->err = FT_Err_Invalid_SVG_Document;
            return state->err;
        }
        lunasvg::Box box = state->svg->box();
        double scale = std::min(metrics.x_ppem / box.w, metrics.y_ppem / box.h);
        double xx = (double)document->transform.xx / (1 << 16);
        double xy = -(double)document->transform.xy / (1 << 16);
        double yx = -(double)document->transform.yx / (1 << 16);
        double yy = (double)document->transform.yy / (1 << 16);
        double x0 = (double)document->delta.x / 64 * box.w / metrics.x_ppem;
        double y0 = -(double)document->delta.y / 64 * box.h / metrics.y_ppem;
        // Scale and transform, we don't translate the svg yet
        state->matrix.identity();
        state->matrix.scale(scale, scale);
        state->matrix.transform(xx, xy, yx, yy, x0, y0);
        state->svg->setMatrix(state->matrix);
        // Pre-translate the matrix for the rendering step
        state->matrix.translate(-box.x, -box.y);
        // Get the box again after the transformation
        box = state->svg->box();
        // Calculate the bitmap size
        slot->bitmap_left = FT_Int(box.x);
        slot->bitmap_top = FT_Int(-box.y);
        slot->bitmap.rows = (unsigned int)(ceilf((float)box.h));
        slot->bitmap.width = (unsigned int)(ceilf((float)box.w));
        slot->bitmap.pitch = slot->bitmap.width * 4;
        slot->bitmap.pixel_mode = FT_PIXEL_MODE_BGRA;
        // Compute all the bearings and set them correctly. The outline is scaled already, we just need to use the bounding box.
        double metrics_width = box.w;
        double metrics_height = box.h;
        double horiBearingX = box.x;
        double horiBearingY = -box.y;
        double vertBearingX = slot->metrics.horiBearingX / 64.0 - slot->metrics.horiAdvance / 64.0 / 2.0;
        double vertBearingY = (slot->metrics.vertAdvance / 64.0 - slot->metrics.height / 64.0) / 2.0;
        slot->metrics.width = FT_Pos(FT_ROUND(metrics_width * 64.0));   // Using FT_ROUND() assume width and height are positive
        slot->metrics.height = FT_Pos(FT_ROUND(metrics_height * 64.0));
        slot->metrics.horiBearingX = FT_Pos(horiBearingX * 64);
        slot->metrics.horiBearingY = FT_Pos(horiBearingY * 64);
        slot->metrics.vertBearingX = FT_Pos(vertBearingX * 64);
        slot->metrics.vertBearingY = FT_Pos(vertBearingY * 64);
        if (slot->metrics.vertAdvance == 0)
            slot->metrics.vertAdvance = FT_Pos(metrics_height * 1.2 * 64.0);
        state->err = FT_Err_Ok;
        return state->err;
    }

    enum class KerningMode
    {
        Basic,
        Regular,
        Extended
    };

    int getKerning(const std::uint32_t left, const std::uint32_t right, KerningMode kerningMode) const
    {
        const auto indexLeft = FT_Get_Char_Index(face, left);
        const auto indexRight = FT_Get_Char_Index(face, right);

        FT_Vector k;
        k.x = 0;
        if (FT_HAS_KERNING(face))
        {
            const FT_UInt kernMode = kerningMode == KerningMode::Basic ? FT_KERNING_DEFAULT : FT_KERNING_UNFITTED;
            const auto error = FT_Get_Kerning(face, indexLeft, indexRight, kernMode, &k);
            if (error)
                throw std::runtime_error("Couldn't find glyphs kerning");
        }

        // X advance is already in pixels for bitmap fonts
        if (!FT_IS_SCALABLE(face))
            return static_cast<int>(k.x);

        const bool useRsbLsb = (kerningMode == KerningMode::Regular && k.x) || (kerningMode == KerningMode::Extended);

        const std::int32_t firstRsbDelta = useRsbLsb ? renderGlyph(nullptr, 0, 0, 0, 0, left, 0).rsbDelta : 0;
        const std::int32_t secondLsbDelta = useRsbLsb ? renderGlyph(nullptr, 0, 0, 0, 0, right, 0).lsbDelta : 0;

        return static_cast<int>(std::floor(static_cast<float>(secondLsbDelta - firstRsbDelta + k.x + 32) / 64.f));
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
        std::cout << "face->height " << FT_CEIL(FT_MulFix(face->height, scale)) << "\n";        // distance between lines
        std::cout << "face->height f " << static_cast<float>(FT_MulFix(face->height, scale)) / static_cast<float>(1 << 6) << "\n";
        std::cout << "face->size->metrics.height " << FT_CEIL(face->size->metrics.height) << "\n";

        std::cout << "metrics.height " << static_cast<float>(face->size->metrics.height) / static_cast<float>(1 << 6) << "\n"; // as in SFML getLineSpacing
        std::cout << "metrics.ascender " << static_cast<float>(face->size->metrics.ascender) / static_cast<float>(1 << 6) << "\n";
        std::cout << "metrics.descender " << static_cast<float>(face->size->metrics.descender) / static_cast<float>(1 << 6) << "\n";
        std::cout << "a " << static_cast<float>(FT_MulFix(face->ascender, scale)) / static_cast<float>(1 << 6) << "\n";

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
    int ascent;
    int descent;
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
};

}
