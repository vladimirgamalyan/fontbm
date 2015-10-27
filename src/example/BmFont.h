#pragma once
#include <SDL2pp/SDL2pp.hh>
#include "FntInfo.h"

class BmFont
{
public:
    BmFont(SDL2pp::Renderer& renderer, const std::string& fontFile);
    void print(SDL2pp::Point pos, const std::string& text);

private:
    SDL2pp::Renderer& renderer;
    FntInfo fntInfo;
    std::vector<SDL2pp::Texture> textures;
};
