#pragma once
#include <SDL2pp/SDL2pp.hh>

class BmFontInterface
{
public:
    virtual bool isValid() const = 0;
    virtual bool isGlyphProvided(Uint16 ch) const = 0;
    virtual SDL2pp::Rect getGlyphRect(Uint16 ch) const = 0;
    virtual SDL2pp::Point getGlyphOffset(Uint16 ch) const = 0;
    virtual int getKerning(Uint16 ch0, Uint16 ch1) const = 0;
    virtual SDL2pp::Surface getSurface() const = 0;
};
