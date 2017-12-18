#pragma once
#include "Config.h"

namespace helpers
{
    Config parseCommandLine(int argc, char* argv[]);
    std::set<uint32_t> parseCharsString(std::string str);
    std::set<uint32_t> getCharsFromFile(const std::string& f);
    Config::Color parseColor(const std::string& str);
}
