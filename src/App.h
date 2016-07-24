#pragma once
#include <stdint.h>
#include <map>
#include <SDL2pp/SDL2pp.hh>
#include "maxRectsBinPack/MaxRectsBinPack.h"
#include "Config.h"
#include "GlyphInfo.h"

class App
{
public:
    void execute(int argc, char* argv[]);

private:
    typedef std::map<uint32_t, GlyphInfo> Glyphs;
    int getKerning(const SDL2pp::Font& font, uint32_t char0, uint32_t char1);
    void printGlyphData(const SDL2pp::Font& font, Uint16 ch);
    std::vector<rbp::RectSize> getSrcRects(const Glyphs &glyphs, int additionalWidth, int additionalHeight);
    Glyphs collectGlyphInfo(const SDL2pp::Font &font,
                            const std::set<uint32_t> &codes,
                            uint32_t maxTextureSizeX,
                            uint32_t maxTextureSizeY);
    SDL_Color makeSdlColor(Config::Color c, uint8_t a = 255);
    uint16_t arrangeGlyphs(Glyphs& glyphs, const Config& config);
    int getDigitCount(uint16_t x);
};
