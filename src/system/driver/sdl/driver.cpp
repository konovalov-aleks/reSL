#include "driver.h"

// IWYU pragma: no_include "system/driver/sdl/driver.h"

#include <SDL.h>
#include <SDL_error.h>
#include <SDL_events.h>

#include <cstdlib>
#include <iostream>

namespace resl {

Driver::SDLInit::SDLInit()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS)) [[unlikely]] {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

Driver::SDLInit::~SDLInit()
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
