#pragma once
#include <iostream>
#include "FtInclude.h"
#include "FtException.h"
#include <cmath>

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

class Face
{
public:
    Face(Library& library, const std::string& fontFile, int ptsize, int faceIndex = 0) : library(library) {
        if (!library.library)
            throw std::runtime_error("Library not initialized");

        FT_Error error = FT_New_Face(library.library, fontFile.c_str(), faceIndex, &face);
        if (error == FT_Err_Unknown_File_Format)
            throw Exception("Unsupported font format", error);
        if (error)
            throw Exception("Couldn't load font file", error);

        FT_CharMap found = 0;
        for (int i = 0; i < face->num_charmaps; i++) {
            FT_CharMap charmap = face->charmaps[i];
            if ((charmap->platform_id == 3 && charmap->encoding_id == 1) /* Windows Unicode */
                || (charmap->platform_id == 3 && charmap->encoding_id == 0) /* Windows Symbol */
                || (charmap->platform_id == 2 && charmap->encoding_id == 1) /* ISO Unicode */
                || (charmap->platform_id == 0)) { /* Apple Unicode */
                found = charmap;
                break;
            }
        }
        if (found) {
            /* If this fails, continue using the default charmap */
            FT_Set_Charmap(face, found);
        }

        if (FT_IS_SCALABLE(face)) {
            /* Set the character size and use default DPI (72) */
            error = FT_Set_Char_Size(face, 0, ptsize * 64, 0, 0);
            if (error) {
                FT_Done_Face(face);
                throw Exception("Couldn't set font size", error);
            }
            /* Get the scalable font metrics for this font */
            FT_Fixed scale = face->size->metrics.y_scale;
            ascent  = FT_CEIL(FT_MulFix(face->ascender, scale));
            descent = FT_CEIL(FT_MulFix(face->descender, scale));
            height  = ascent - descent + /* baseline */ 1;
            lineskip = FT_CEIL(FT_MulFix(face->height, scale));
            underline_offset = FT_FLOOR(FT_MulFix(face->underline_position, scale));
            underline_height = FT_FLOOR(FT_MulFix(face->underline_thickness, scale));
        } else {
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
            ascent = face->available_sizes[ptsize].height;
            descent = 0;
            height = face->available_sizes[ptsize].height;
            lineskip = FT_CEIL(ascent);
            underline_offset = FT_FLOOR(face->underline_position);
            underline_height = FT_FLOOR(face->underline_thickness);
        }

        if ( underline_height < 1 ) {
            underline_height = 1;
        }

        /* Initialize the font face style */
        face_style = TTF_STYLE_NORMAL;
        if ( face->style_flags & FT_STYLE_FLAG_BOLD ) {
            face_style |= TTF_STYLE_BOLD;
        }
        if ( face->style_flags & FT_STYLE_FLAG_ITALIC ) {
            face_style |= TTF_STYLE_ITALIC;
        }

        /* Set the default font style */
        style = face_style;
        outline = 0;
        kerning = 1;
        glyph_overhang = face->size->metrics.y_ppem / 10;
        /* x offset = cos(((90.0-12)/360)*2*M_PI), or 12 degree angle */
        glyph_italics = 0.207f;
        glyph_italics *= height;
    }

    ~Face() {
        FT_Done_Face(face);
    }

    void renderGlyph(uint32_t* buffer, uint32_t w, uint32_t h, int x, int y, uint32_t ch)
    {
        FT_Error error = FT_Load_Char(face, ch, FT_LOAD_RENDER);
        if (error)
            throw std::runtime_error("Load glyph error");

        FT_GlyphSlot slot = face->glyph;

        uint32_t* dst_check = buffer + w * h;
//        std::cout << (int)buffer << std::endl;
//        std::cout << (int)dst_check << std::endl;
//        std::cout << (int)(dst_check - buffer) << std::endl;
//        std::cout << "----------" << std::endl;


        int width = slot->bitmap.width;
//        if (outline <= 0 && width > glyph->maxx - glyph->minx) {
//            width = glyph->maxx - glyph->minx;
//        }

        uint32_t pixel = 0xff00; // (fg.r<<16)|(fg.g<<8)|fg.b;

        for ( unsigned int row = 0; row < slot->bitmap.rows; ++row ) {

            /* Make sure we don't go either over, or under the
             * limit */
//            if ( row + glyph->yoffset < 0 ) {
//                continue;
//            }
//            if ( row + glyph->yoffset >= textbuf->h ) {
//                continue;
//            }


//            Uint32 *dst = buffer +
//                  (row + glyph->yoffset) * textbuf->pitch / 4 +
//                  xstart + glyph->minx;

            Uint32 *dst = buffer + (y + row) * w + x;

            /* Added code to adjust src pointer for pixmaps to
             * account for pitch.
             * */
            uint8_t *src = slot->bitmap.buffer + slot->bitmap.pitch * row;

            for ( int col = width; col > 0 && dst < dst_check; --col) {
                Uint32 alpha = *src++;
                *dst++ = pixel | (alpha << 24);
            }
        }
    }

    int isGlyphProvided(FT_ULong ch) const
    {
        return FT_Get_Char_Index(face, ch);
    }

    int getKerning(uint32_t left, uint32_t right)
    {
        if (!FT_HAS_KERNING(face))
            return 0;

        FT_Vector delta;

        FT_UInt indexLeft = FT_Get_Char_Index(face, left);
        FT_UInt indexRight = FT_Get_Char_Index(face, right);

        FT_Error error = FT_Get_Kerning(face, indexLeft, indexRight, ft_kerning_default, &delta);
        if (error)
            throw std::runtime_error("Couldn't find glyph");
        return delta.x >> 6;
    }

    void GetGlyphMetrics(FT_ULong ch, int& minx_, int& maxx_, int& miny_, int& maxy_, int& advance_) const
    {
        FT_UInt index = FT_Get_Char_Index(face, ch);
        if (!index)
            throw std::runtime_error("Couldn't find glyph");

        FT_Int32 hinting = 0;
        FT_Error error = FT_Load_Glyph(face, index, FT_LOAD_DEFAULT | hinting);
        if (error)
            throw Exception("Error load glyph", error);

        /* Get our glyph shortcuts */
        FT_GlyphSlot glyph = face->glyph;
        FT_Glyph_Metrics* metrics = &glyph->metrics;
        //FT_Outline* outline = &glyph->outline;

        int minx;
        int maxx;
        int miny;
        int maxy;
        //int yoffset;
        int advance;

        if ( FT_IS_SCALABLE( face ) ) {
            /* Get the bounding box */
            minx = FT_FLOOR(metrics->horiBearingX);
            maxx = FT_CEIL(metrics->horiBearingX + metrics->width);
            maxy = FT_FLOOR(metrics->horiBearingY);
            miny = maxy - FT_CEIL(metrics->height);
            //yoffset = ascent - maxy;
            advance = FT_CEIL(metrics->horiAdvance);
        } else {
            /* Get the bounding box for non-scalable format.
             * Again, freetype2 fills in many of the font metrics
             * with the value of 0, so some of the values we
             * need must be calculated differently with certain
             * assumptions about non-scalable formats.
             * */
            minx = FT_FLOOR(metrics->horiBearingX);
            maxx = FT_CEIL(metrics->horiBearingX + metrics->width);
            maxy = FT_FLOOR(metrics->horiBearingY);
            miny = maxy - FT_CEIL(face->available_sizes[font_size_family].height);
            //yoffset = 0;
            advance = FT_CEIL(metrics->horiAdvance);
        }

        /* Adjust for bold and italic text */
        if ( TTF_HANDLE_STYLE_BOLD() ) {
            maxx += glyph_overhang;
        }
        if ( TTF_HANDLE_STYLE_ITALIC() ) {
            maxx += (int)std::ceil(glyph_italics);
        }

        minx_ = minx;
        maxx_ = maxx;
        if ( TTF_HANDLE_STYLE_BOLD() ) {
            maxx_ += glyph_overhang;
        }

        miny_ = miny;
        maxy_ = maxy;

        advance_ = advance;
        if ( TTF_HANDLE_STYLE_BOLD() ) {
            advance_ += glyph_overhang;
        }

    }

    void debugInfo() {
        std::cout << "num_charmaps " << face->num_charmaps << std::endl;
        std::cout << "num_glyphs " << face->num_glyphs << std::endl;

        for (int i = 0; i < face->num_charmaps; i++) {
            FT_CharMap charmap = face->charmaps[i];
            std::cout << charmap->platform_id << ", " << charmap->encoding_id << std::endl;
        }
    }

    std::string getFamilyName(const std::string& defaultName)
    {
        if (!face->family_name)
            return defaultName;
        return std::string(face->family_name);
    }

    Library& library;
    FT_Face face = nullptr;
    int height;
    int ascent;
    int descent;
    int lineskip;
    int underline_offset;
    int underline_height;

    /* For non-scalable formats, we must remember which font index size */
    int font_size_family;

    /* The font style */
    int face_style;
    int style;
    int outline;

    /* Whether kerning is desired */
    int kerning;


    /* Extra width in glyph bounds for text styles */
    int glyph_overhang;
    float glyph_italics;
};

}
