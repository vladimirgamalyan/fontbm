#pragma once
#include <SDL2pp/SDL2pp.hh>
#include "BmFontInterface.h"

class BmFontPrinter
{
public:
    void print(const SDL2pp::Point& pos, const std::string& text, const BmFontInterface& font);

private:

};
