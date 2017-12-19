#pragma once
#include <stdexcept>
#include "FtInclude.h"
#include "FtException.h"

namespace ft {

class Library
{
public:
    Library();
    ~Library();
    FT_Library library = nullptr;

private:
    static int initialized;
};

}
