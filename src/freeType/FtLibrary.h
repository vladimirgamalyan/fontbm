#pragma once
#include "FtInclude.h"
#include <string>

namespace ft {

class Library
{
public:
    Library();
    ~Library();

    FT_Library library = nullptr;

    Library(const Library&) = delete;
    Library(Library&&) = delete;
    Library& operator = (const Library&) = delete;
    Library& operator=(Library&&) = delete;

    std::string getVersionString() const;

private:
    static int initialized;
};

}
