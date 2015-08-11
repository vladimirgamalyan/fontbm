#include "BmFontPrinter.h"

BmFontPrinter::BmFontPrinter(SDL2pp::Renderer& renderer) : renderer(renderer)
{

}

void BmFontPrinter::print(const SDL2pp::Point& /*pos*/, const BmFontInterface& font, const std::string& /*text*/)
{
    if (!font.isValid())
        return;


}
