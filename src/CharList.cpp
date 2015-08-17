#include "CharList.h"
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <functional>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <iostream>

/*
   12-21,23,25-43
    - one or more comma separated tokens
    - each token is number with optional second number after hyph
    - each number is 1 to 5 digits

 */

CharList parseCharListString(std::string str)
{
    CharList result;

    // remove whitespace characters
    str.erase(std::remove_if(str.begin(), str.end(), std::bind( std::isspace<char>, std::placeholders::_1, std::locale::classic() )), str.end());

    const boost::regex e("^\\d{1,5}(-\\d{1,5})?(,\\d{1,5}(-\\d{1,5})?)*$");
    if (!boost::regex_match(str, e))
        throw std::logic_error("invalid chars value");

    std::vector<std::string> ranges;
    boost::split(ranges, str, boost::is_any_of(","));
    for (auto range: ranges)
    {
        std::vector<std::string> minMaxStr;
        boost::split(minMaxStr, range, boost::is_any_of("-"));
        if (minMaxStr.size() == 1)
            minMaxStr.push_back(minMaxStr[0]);

        try
        {
            result.emplace_back(boost::lexical_cast<uint16_t>(minMaxStr[0]),
                                boost::lexical_cast<uint16_t>(minMaxStr[1]));
        }
        catch(boost::bad_lexical_cast &)
        {
            throw std::logic_error("incorrect chars value (out of range)");
        }
    }

    return result;
}

std::string charListToString(const CharList& charList)
{
    std::stringstream ss;
    for (size_t i = 0; i < charList.size(); ++i)
    {
        if (i)
            ss << ",";
        int min = charList[i].first;
        int max = charList[i].second;
        if (min == max)
            ss << min;
        else
            ss << min << "-" << max;
    }
    return ss.str();
}

void parseCharListString(std::string str, std::set<uint16_t>& result)
{
    CharList charList = parseCharListString(str);
    result.clear();
    for (auto range: charList)
    {
        for (uint16_t v = range.first; v < range.second; ++v)
            result.insert(v);
        result.insert(range.second);
    }
}

