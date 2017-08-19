#pragma once
#include <exception>
#include "FtInclude.h"

namespace ft {

class Exception : public std::exception
{
public:
    Exception(const std::string&, FT_Error) {}
};

}
