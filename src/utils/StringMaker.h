#pragma once
#include <sstream>

class StringMaker
{
public:
    operator std::string() const
    {
        return stream.str();
    }
    template<class T> StringMaker& operator<<(T const& VAR)
    {
        stream << VAR;
        return *this;
    }
protected:
    std::stringstream stream;
};
