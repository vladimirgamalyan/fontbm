#pragma once
#include <vector>
#include <string>
#include <stdint.h>
#include <set>

typedef std::pair<uint16_t , uint16_t> CharListItem;
typedef std::vector<CharListItem> CharList;
CharList parseCharListString(std::string str);
void parseCharListString(std::string str, std::set<uint16_t>& result);
std::string charListToString(const CharList& charList);

namespace  std
{
    __inline ostream& operator<< (ostream& ostr, const CharList& charList)
    {
        return ostr << charListToString(charList);
    }
}
