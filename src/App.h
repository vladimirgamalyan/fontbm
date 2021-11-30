#pragma once
#include <cstdint>
#include <map>
#include "external/maxRectsBinPack/MaxRectsBinPack.h"
#include "Config.h"
#include "GlyphInfo.h"
#include "freeType/FtLibrary.h"
#include "freeType/FtFont.h"

class App
{
public:
    static void execute(int argc, char* argv[]) ;

private:
    typedef std::map<std::uint32_t, GlyphInfo> Glyphs;

    static std::vector<rbp::RectSize> getGlyphRectangles(const Glyphs& glyphs, std::uint32_t additionalWidth, std::uint32_t additionalHeight) ;
    static Glyphs collectGlyphInfo(const std::vector<ft::Font>& fonts, const std::set<std::uint32_t>& codes, const Config& config) ;
    static std::uint32_t arrangeGlyphs(Glyphs& glyphs, const Config& config) ;
    static std::vector<std::string> renderTextures(const Glyphs& glyphs, const Config& config, const std::vector<ft::Font>& fonts, std::uint32_t pageCount) ;
    static void savePng(const std::string& fileName, const std::uint32_t* buffer, std::uint32_t w, std::uint32_t h, bool withAlpha) ;
    static void writeFontInfoFile(const Glyphs& glyphs, const Config& config, const std::vector<ft::Font>& fonts, const std::vector<std::string>& fileNames) ;
};
