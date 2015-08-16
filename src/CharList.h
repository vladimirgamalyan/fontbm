#pragma once
#include <vector>
#include <string>
#include <stdint.h>

typedef std::pair<uint16_t , uint16_t> CharListItem;
typedef std::vector<CharListItem> CharList;
CharList parseCharListString(std::string str);
std::string charListToString(const CharList& charList);
__inline std::ostream& operator<< (std::ostream& ostr, const CharList& charList)
{
    return ostr << charListToString(charList);
}
