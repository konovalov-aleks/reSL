#include "driver.h"

#include "SDL.h"

#include <cstdlib>
#include <iostream>

namespace resl {

Driver::Driver()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS)) [[unlikely]] {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

Driver::~Driver()
{
    SDL_Quit();
}

bool Driver::pollEvent()
{
    SDL_Event e;
    while (!m_quit && SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            m_quit = true;
    }
    return !m_quit;
}

} // namespace resl
