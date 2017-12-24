#pragma once
#include <stdint.h>
#include <map>
#include "maxRectsBinPack/MaxRectsBinPack.h"
#include "Config.h"
#include "GlyphInfo.h"
#include "freeType/FtLibrary.h"
#include "freeType/FtFont.h"

class App
{
public:
    void execute(int argc, char* argv[]);

private:
    typedef std::map<uint32_t, GlyphInfo> Glyphs;
    std::vector<rbp::RectSize> getGlyphRectangles(const Glyphs& glyphs, int additionalWidth, int additionalHeight);
    Glyphs collectGlyphInfo(ft::Face& face,
                            const std::set<uint32_t> &codes,
                            uint32_t maxTextureSizeX,
                            uint32_t maxTextureSizeY);
    uint32_t arrangeGlyphs(Glyphs& glyphs, const Config& config);
    void savePng(const std::string& fileName, const uint32_t* buffer, uint32_t w, uint32_t h, bool withAlpha);
};
