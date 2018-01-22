#pragma once
#include "FtInclude.h"

namespace ft {

class Library
{
public:
    Library();
    Library(const Library&) = delete;
    Library& operator = (const Library&) = delete;
    Library(Library&&) = delete;
    Library& operator=(Library&&) = delete;
    ~Library();
    FT_Library library = nullptr;

private:
    static int initialized;
};

}
