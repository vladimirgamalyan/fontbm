#include "FtLibrary.h"

namespace ft {

int Library::initialized = 0;

Library::Library()
{
    if (!initialized)
    {
        FT_Error error = FT_Init_FreeType(&library);
        if (error)
            throw Exception("Couldn't init FreeType engine", error);
    }

    ++initialized;
}

Library::~Library()
{
    if (initialized) {
        if (--initialized == 0) {
            FT_Done_FreeType(library);
            library = nullptr;
        }
    }
}
}
