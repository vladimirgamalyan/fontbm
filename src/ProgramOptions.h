#pragma once
#include "Config.h"

class ProgramOptions
{
public:
    Config parseCommandLine(int argc, const char* const argv[]);
private:
    std::set<uint32_t> parseCharsString(std::string str) const;
    std::set<uint32_t> getCharsFromFile(const boost::filesystem::path& f) const;
    Config::Color parseColor(const std::string& str) const;
};
