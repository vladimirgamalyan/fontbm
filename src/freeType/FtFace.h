#pragma once
#include <iostream>
#include "FtInclude.h"
#include "FtException.h"

namespace ft {

class Face
{
public:
    Face(Library& library, const std::string& fontFile, int ptsize, int faceIndex = 0) : library(library) {
        FT_Error error = FT_New_Face( library.library, fontFile.c_str(), faceIndex, &face );
        if ( error == FT_Err_Unknown_File_Format )
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
        if ( found ) {
            /* If this fails, continue using the default charmap */
            FT_Set_Charmap(face, found);
        }

        if ( FT_IS_SCALABLE(face) ) {
            /* Set the character size and use default DPI (72) */
            error = FT_Set_Char_Size(face, 0, ptsize * 64, 0, 0);
            if (error)
                //TODO: free font
                throw Exception("Couldn't set font size", error);
        } else {
            if ( ptsize >= face->num_fixed_sizes )
                ptsize = face->num_fixed_sizes - 1;
            error = FT_Set_Pixel_Sizes( face,
                                        static_cast<FT_UInt>(face->available_sizes[ptsize].width),
                                        static_cast<FT_UInt>(face->available_sizes[ptsize].height ));
            //TODO: check error, free font
        }


    }
    ~Face() {
        FT_Done_Face(face);
    }

    void debugInfo() {
        std::cout << "num_charmaps " << face->num_charmaps << std::endl;
        std::cout << "num_glyphs " << face->num_glyphs << std::endl;

        for (int i = 0; i < face->num_charmaps; i++) {
            FT_CharMap charmap = face->charmaps[i];
            std::cout << charmap->platform_id << ", " << charmap->encoding_id << std::endl;
        }
    }

    Library& library;
    FT_Face face;
};

}
