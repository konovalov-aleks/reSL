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
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <thread>

#ifndef NDEBUG
#   include <limits>
#endif

#ifdef __EMSCRIPTEN__
#   include <emscripten.h>
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

inline std::uint16_t mouseButtonState()
{
    Uint32 state = SDL_GetMouseState(nullptr, nullptr);
    std::uint16_t res = 0;

    if (state & SDL_BUTTON(SDL_BUTTON_LEFT))
        res |= MouseButton::MB_LEFT;
    if (state & SDL_BUTTON(SDL_BUTTON_RIGHT))
        res |= MouseButton::MB_RIGHT;
    if (state & SDL_BUTTON(SDL_BUTTON_MIDDLE))
        res |= MouseButton::MB_MIDDLE;
    return res;
}

Driver::Driver()
    : m_mouseButtonState(mouseButtonState())
{
}

void Driver::sleep(unsigned ms)
{
#ifdef __EMSCRIPTEN__

    // We have to return to the main loop to update the picture.
    // But this can be a relatively long operation, which is why on small delays
    // it should be done only when necessary.
    const unsigned timeToNextFrame = vga().timeToNextFrameMS();
    if (ms < timeToNextFrame) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        return;
    }

    using ClockT = std::chrono::steady_clock;
    const ClockT::time_point startTime = ClockT::now();
    if (timeToNextFrame)
        std::this_thread::sleep_for(std::chrono::milliseconds(timeToNextFrame));
    vga().flush();

    const auto timePassed =
        std::chrono::duration_cast<std::chrono::milliseconds>(ClockT::now() - startTime).count();

    const unsigned msToSleep = timePassed < ms ? ms - timePassed : 0;
    emscripten_sleep(msToSleep);

#else // __EMSCRIPTEN__

    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    vga().flush();

#endif // __EMSCRIPTEN__
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

inline MouseButton mouseButton(const SDL_MouseButtonEvent& e)
{
    switch (e.button) {
    case SDL_BUTTON_LEFT:
        return MouseButton::MB_LEFT;
    case SDL_BUTTON_RIGHT:
        return MouseButton::MB_RIGHT;
    case SDL_BUTTON_MIDDLE:
        return MouseButton::MB_MIDDLE;
    }
    return MouseButton::MB_NONE;
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

    const MouseButton btn = mouseButton(e);
    if (btn == MouseButton::MB_NONE) [[unlikely]]
        return;

    if (e.type == SDL_MOUSEBUTTONDOWN)
        m_mouseButtonState |= btn;
    else
        m_mouseButtonState &= (~btn);

    m_mouseHandler(mouseEventFlags, m_mouseButtonState, 0, 0);
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

    m_mouseHandler(0, m_mouseButtonState, dx, dy);
}

} // namespace resl
