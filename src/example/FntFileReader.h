#pragma once

#include <cstdint>
#include <map>
#include <vector>

class FntFileReader
{
public:
    struct Info
    {
        struct Padding
        {
            Padding() : top(0), right(0), bottom(0), left(0) {}
            int top;
            int right;
            int bottom;
            int left;
        };

        struct Character
        {
            int x;
            int y;
            int w;
            int h;
            int xoffset;
            int yoffset;
            int xadvance;
            int page;
        };

        Padding padding;
        std::map<uint32_t, Character> characters;
        std::map<int, std::string> pages;
    };

    static Info loadFromFile(const std::string& fileName);

private:
	static std::string getValueString(const std::string& line, const std::string& key);
	static std::string getQuotedValueString(const std::string& line, const std::string& key);
	static int getValueInt(const std::string& line, const std::string& key, int defaultVal = -1);
	static std::vector<int> getCsvInt(const std::string& str);
};
