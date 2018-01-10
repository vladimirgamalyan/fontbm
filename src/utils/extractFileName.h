#pragma once

#include <string>

inline std::string extractFileName(const std::string& s)
{
    auto result(s);
    const auto slash = s.find_last_of("\\/");
    if (slash != std::string::npos)
        result.erase(0, slash + 1);
    return result;
}
