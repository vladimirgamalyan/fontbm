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

        int min = boost::lexical_cast<int>(minMaxStr[0]);
        int max = boost::lexical_cast<int>(minMaxStr[1]);

        if (min > 65535)
            throw std::logic_error("incorrect chars value (out of range)");
        if (max > 65535)
            throw std::logic_error("incorrect chars value (out of range)");

        result.emplace_back(min, max);
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
