#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include "sdlSavePng/savepng.h"

int main(int /*argc*/, char** /*argv*/) try {

    SDL2pp::SDL sdl(SDL_INIT_VIDEO);
    SDL2pp::Window window("fontbm", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);
    SDL2pp::Renderer render(window, -1, SDL_RENDERER_ACCELERATED);

    SDL2pp::SDLTTF ttf;
    SDL2pp::Font font("./testdata/Vera.ttf", 32);

    SDL2pp::Surface surface = font.RenderText_Blended("Hello, world!", SDL_Color {255, 255, 255, 255} );
    std::cout << surface.GetSize() << std::endl;
    SDL_SavePNG(surface.Get(), "./testdata/output.png");

	return 0;
} catch (std::exception& e) {
	std::cerr << "Error: " << e.what() << std::endl;
	return 1;
} catch (...) {
	std::cerr << "Unknown error" << std::endl;
	return 1;
}
