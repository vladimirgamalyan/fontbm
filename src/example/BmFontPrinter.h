#pragma once
#include <SDL2pp/SDL2pp.hh>
#include "BmFontInterface.h"

class BmFontPrinter
{
public:
    BmFontPrinter(SDL2pp::Renderer& renderer);
    void print(const SDL2pp::Point& pos, const BmFontInterface& font, const std::string& text);

private:
    SDL2pp::Renderer& renderer;
};
