#pragma once
#include "Config.h"

namespace helpers
{
    Config parseCommandLine(int argc, const char* const argv[]);
    std::set<uint32_t> parseCharsString(std::string str);
    std::set<uint32_t> getCharsFromFile(const boost::filesystem::path& f);
    Config::Color parseColor(const std::string& str);
}
