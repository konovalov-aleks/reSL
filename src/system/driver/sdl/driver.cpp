#include "driver.h"

// IWYU pragma: no_include "system/driver/sdl/driver.h"

#include <graphics/vga.h>
#include <system/mouse.h>

#include <SDL.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_mouse.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>

#ifndef NDEBUG
#   include <limits>
#endif

namespace resl {

Driver::SDLInit::SDLInit()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS)) [[unlikely]] {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    SDL_ShowCursor(false);
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
        case SDL_MOUSEMOTION:
            onMouseMove(e.motion);
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

inline std::uint16_t mouseButtonState(Uint32 state)
{
    std::uint16_t res = 0;

    /* From the TurboC MOUSE/MOUSE.H header:
        271         BX = button state (bit 0 set if left button down, bit 1 set if right
        272                            button down and bit 2 set if center button down)
     */
    if (state & SDL_BUTTON(SDL_BUTTON_LEFT))
        res |= 1;
    if (state & SDL_BUTTON(SDL_BUTTON_RIGHT))
        res |= 2;
    if (state & SDL_BUTTON(SDL_BUTTON_MIDDLE))
        res |= 4;
    return res;
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

    m_mouseHandler(mouseEventFlags, mouseButtonState(e.state), 0, 0);
}

void Driver::onMouseMove(const SDL_MouseMotionEvent& e)
{
    if (!m_mouseHandler) [[unlikely]]
        return;

    // limit mouse movement if the debug graphics is active
    const Sint32 x = std::min(e.x, SCREEN_WIDTH);
    const Sint32 y = std::min(e.y, SCREEN_HEIGHT);

    // window is small => coordinates can't be large
    assert(x - m_lastCursorX <= std::numeric_limits<std::int16_t>::max());
    assert(x - m_lastCursorX >= std::numeric_limits<std::int16_t>::min());
    assert(y - m_lastCursorY <= std::numeric_limits<std::int16_t>::max());
    assert(y - m_lastCursorY >= std::numeric_limits<std::int16_t>::min());

    std::int16_t dx = static_cast<std::int16_t>(x - m_lastCursorX);
    std::int16_t dy = static_cast<std::int16_t>(y - m_lastCursorY);
    m_lastCursorX = x;
    m_lastCursorY = y;

    m_mouseHandler(0, mouseButtonState(e.state), dx, dy);
}

} // namespace resl
