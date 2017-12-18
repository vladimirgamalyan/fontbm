#pragma once
#include <stdexcept>
#include "FtInclude.h"
#include "FtException.h"

namespace ft {

//WARNING! Not tested with multi objects.
class Library
{
public:
    Library() {
        FT_Error error = FT_Init_FreeType(&library);
        if (error)
            throw Exception("Couldn't init FreeType engine", error);
    }

    ~Library() {
        FT_Done_FreeType(library);
    }

    FT_Library library = nullptr;
};

}
