#pragma once

#include <string>
#include <vector>
#include <sstream>

inline std::vector<std::string> splitStrByDelim(const std::string& str, const char delimiter)
{
    std::stringstream ss(str);
    std::vector<std::string> result;
    while (ss.good())
    {
        std::string substring;
        std::getline(ss, substring, delimiter);
        result.push_back(substring);
    }
    return result;
}
