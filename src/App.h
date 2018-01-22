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
	static void execute(const int argc, char* argv[]);

private:
    typedef std::map<std::uint32_t, GlyphInfo> Glyphs;

    static std::vector<rbp::RectSize> getGlyphRectangles(const Glyphs& glyphs, std::uint32_t additionalWidth, std::uint32_t additionalHeight);
	static Glyphs collectGlyphInfo(ft::Font& font, const std::set<std::uint32_t>& codes);
	static std::uint32_t arrangeGlyphs(Glyphs& glyphs, const Config& config);
	static std::vector<std::string> renderTextures(const Glyphs& glyphs, const Config& config, ft::Font& font, std::uint32_t pageCount);
	static void savePng(const std::string& fileName, const std::uint32_t* buffer, std::uint32_t w, std::uint32_t h, bool withAlpha);
	static void writeFontInfoFile(const Glyphs& glyphs, const Config& config, ft::Font& font, const std::vector<std::string>& fileNames);
};
