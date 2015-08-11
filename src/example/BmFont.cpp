#include "BmFont.h"

BmFont::BmFont(const std::string &fontFile)
{

}

bool BmFont::isValid() const
{
    return false;
}

bool BmFont::isGlyphProvided(Uint16 ch) const
{
    return false;
}

SDL2pp::Rect BmFont::getGlyphRect(Uint16 ch) const
{
    return SDL2pp::Rect();
}

SDL2pp::Point BmFont::getGlyphOffset(Uint16 ch) const
{
    return SDL2pp::Point();
}

int BmFont::getKerning(Uint16 ch0, Uint16 ch1) const
{
    return 0;
}

SDL2pp::Surface BmFont::getSurface() const
{
    return SDL2pp::Surface(nullptr);
}
