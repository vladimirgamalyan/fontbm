#pragma once

#include <string>

inline std::string extractFileName(std::string s)
{
    const auto slash = s.find_last_of("\\/");
    if (slash != std::string::npos)
        s.erase(0, slash + 1);
    return s;
}
