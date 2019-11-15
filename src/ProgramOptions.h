#pragma once
#include "Config.h"

class ProgramOptions
{
public:
    Config parseCommandLine(int argc, char* argv[]) const;

    std::set<std::uint32_t> parseCharsString(std::string str) const;
    Config::Color parseColor(const std::string& str) const;
private:
    void getCharsFromFile(const std::string& fileName, std::set<std::uint32_t>& result) const;
};
