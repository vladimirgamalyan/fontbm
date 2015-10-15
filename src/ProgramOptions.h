#pragma once
#include "Config.h"

class ProgramOptions
{
public:
    static Config parseCommandLine(int argc, char** argv);
private:
    static std::set<uint32_t> parseCharsString(std::string str);
    static std::set<uint32_t> getCharsFromFile(const boost::filesystem::path& f);
    static Config::Color parseColor(const std::string& str);
};
