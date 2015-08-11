#include <iostream>
#include <exception>
#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include "BmFontPrinter.h"
#include "BmFont.h"

using namespace SDL2pp;

int main(int /*argc*/, char** /*argv*/) try {
    SDL sdl(SDL_INIT_VIDEO);
    Window window("SDL2pp demo",
                  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                  640, 480,
                  SDL_WINDOW_RESIZABLE);

    Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);
    //Texture sprites(renderer, DATA_PATH "/M484SpaceSoldier.png");
    renderer.Clear();
    //renderer.Copy(sprites);
    renderer.Present();

    BmFont bmFont;

    BmFontPrinter bmFontPrinter(renderer);
    bmFontPrinter.print(SDL2pp::Point(0, 0), bmFont, "Hello, World!");

    SDL_Delay(5000);

    return 0;
} catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
}
