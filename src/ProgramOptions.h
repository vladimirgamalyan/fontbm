#pragma once
#include "Config.h"

class ProgramOptions
{
public:
    Config parseCommandLine(int argc, char* argv[]) const;

    static std::set<std::uint32_t> parseCharsString(std::string str);
    static Config::Color parseColor(const std::string& str);
private:
    static void getCharsFromFile(const std::string& fileName, std::set<std::uint32_t>& result);
};
