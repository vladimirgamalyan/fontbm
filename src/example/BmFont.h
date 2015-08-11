#pragma once
#include "BmFontInterface.h"

class BmFont : public BmFontInterface
{
public:
    BmFont(const std::string& fontFile);

    virtual bool isValid() const override;
    virtual bool isGlyphProvided(Uint16 ch) const override;
    virtual SDL2pp::Rect getGlyphRect(Uint16 ch) const override;
    virtual SDL2pp::Point getGlyphOffset(Uint16 ch) const override;
    virtual int getKerning(Uint16 ch0, Uint16 ch1) const override;
    virtual SDL2pp::Surface getSurface() const override;

private:

};
