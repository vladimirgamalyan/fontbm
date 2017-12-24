#pragma once
#include "Config.h"

class ProgramOptions
{
public:
    Config parseCommandLine(int argc, char* argv[]);

    std::set<uint32_t> parseCharsString(std::string str);
    Config::Color parseColor(const std::string& str);
private:
    void getCharsFromFile(const std::string& fileName, std::set<uint32_t>& result);
};
