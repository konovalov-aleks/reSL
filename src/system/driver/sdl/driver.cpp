#include "driver.h"

// IWYU pragma: no_include "system/driver/sdl/driver.h"

#include <system/mouse.h>

#include <SDL.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_mouse.h>

#include <cassert>
#include <cstdint>
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
        switch (e.type) {
        case SDL_QUIT:
            m_quit = true;
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            onMouseButtonEvent(e.button);
            break;
        }
    }
    return !m_quit;
}

inline std::uint16_t mousePressedFlags(const SDL_MouseButtonEvent& e)
{
    assert(e.type == SDL_MOUSEBUTTONDOWN);
    switch (e.button) {
    case SDL_BUTTON_LEFT:
        return MouseEvent::ME_LEFTPRESSED;
    case SDL_BUTTON_RIGHT:
        return MouseEvent::ME_RIGHTPRESSED;
    case SDL_BUTTON_MIDDLE:
        return MouseEvent::ME_CENTERPRESSED;
    }
    return 0;
}

inline std::uint16_t mouseReleasedFlags(const SDL_MouseButtonEvent& e)
{
    assert(e.type == SDL_MOUSEBUTTONUP);
    switch (e.button) {
    case SDL_BUTTON_LEFT:
        return MouseEvent::ME_LEFTRELEASED;
    case SDL_BUTTON_RIGHT:
        return MouseEvent::ME_RIGHTRELEASED;
    case SDL_BUTTON_MIDDLE:
        return MouseEvent::ME_CENTERRELEASED;
    }
    return 0;
}

void Driver::onMouseButtonEvent(const SDL_MouseButtonEvent& e)
{
    if (!m_mouseHandler) [[unlikely]]
        return;

    const std::uint16_t mouseEventFlags =
        e.type == SDL_MOUSEBUTTONDOWN ? mousePressedFlags(e)
                                      : mouseReleasedFlags(e);
    if (!mouseEventFlags) [[unlikely]]
        return;

    std::uint16_t mouseButtonState = 0;
    /* From the TurboC MOUSE/MOUSE.H header:
        271         BX = button state (bit 0 set if left button down, bit 1 set if right
        272                            button down and bit 2 set if center button down)
     */
    if (e.state & SDL_BUTTON(SDL_BUTTON_LEFT))
        mouseButtonState |= 1;
    if (e.state & SDL_BUTTON(SDL_BUTTON_RIGHT))
        mouseButtonState |= 2;
    if (e.state & SDL_BUTTON(SDL_BUTTON_MIDDLE))
        mouseButtonState |= 4;

    m_mouseHandler(mouseEventFlags, mouseButtonState, e.x, e.y);
}

} // namespace resl
