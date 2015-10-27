#include "BmFont.h"

BmFont::BmFont(SDL2pp::Renderer& renderer, const std::string& fontFile) : renderer(renderer)
{
    fntInfo = FntInfo::loadFromFile(fontFile);
    for (auto& kv : fntInfo.pages)
        textures.emplace_back(SDL2pp::Texture(renderer, kv.second));
}

void BmFont::print(SDL2pp::Point pos, const std::string& text)
{
    for (auto c : text)
    {
        uint32_t code = static_cast<uint32_t>(c);
        if (fntInfo.characters.find(code) != fntInfo.characters.end())
        {
            FntInfo::Character& ch = fntInfo.characters[code];
            renderer.Copy(textures[ch.page],
                          SDL2pp::Rect(ch.x, ch.y, ch.w, ch.h),
                          pos + SDL2pp::Point(ch.xoffset, ch.yoffset));
            pos.x += ch.xadvance;
        }
    }
}
