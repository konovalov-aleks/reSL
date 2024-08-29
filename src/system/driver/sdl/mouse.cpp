#include "mouse.h"

#include "system/driver/sdl/driver.h"
#include "system/driver/sdl/video.h"
#include "touch_handler.h"
#include <graphics/vga.h>
#include <system/mouse.h>

#include <SDL_error.h>
#include <SDL_mouse.h>
#include <SDL_pixels.h>
#include <SDL_rect.h>
#include <SDL_stdinc.h>
#include <SDL_surface.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>

#ifndef NDEBUG
#   include <limits>
#endif

namespace resl {
namespace {

    /* 1d7d:2582 : 32 bytes */
    const std::uint8_t g_cursorGlyph1[] = {
        0x00, 0xc0, 0x00, 0xa0, 0x00, 0x90, 0x00, 0x88, 0x00, 0x84, 0x00, 0x82,
        0x00, 0x81, 0x80, 0x80, 0x40, 0x80, 0x20, 0x80, 0xe0, 0x81, 0x00, 0xb9,
        0x00, 0xc9, 0x80, 0x04, 0x80, 0x04, 0x80, 0x03
    };

    /* 1d7d:25a2 : 32 bytes */
    const std::uint8_t g_cursorGlyph2[] = {
        0x00, 0x00, 0x00, 0x40, 0x00, 0x60, 0x00, 0x70, 0x00, 0x78, 0x00, 0x7c,
        0x00, 0x7e, 0x00, 0x7f, 0x80, 0x7f, 0xc0, 0x7f, 0x00, 0x7c, 0x00, 0x46,
        0x00, 0x06, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00
    };

    constexpr int cursorW = 16;
    constexpr int cursorH = 16;

    SDL_Texture* createTexture(SDL_Renderer* renderer)
    {
        constexpr int wBytes = cursorW / 8;
        static_assert(std::size(g_cursorGlyph1) == wBytes * cursorH);
        static_assert(std::size(g_cursorGlyph2) == wBytes * cursorH);

        std::uint32_t data[cursorW * cursorH];

        for (int y = 0; y < cursorH; ++y) {
            for (int x = 0; x < wBytes; ++x) {
                const int srcOffset = y * wBytes + (wBytes - x - 1);
                std::uint8_t black = g_cursorGlyph1[srcOffset];
                std::uint8_t white = g_cursorGlyph2[srcOffset];

                const int dstOffset = y * cursorW + x * 8;
                for (int bit = 0; bit < 8; ++bit) {
                    std::uint32_t color = 0;
                    if (white & 0x80)
                        color = 0xFFFFFFFF;
                    else if (black & 0x80)
                        color = 0xFF000000;
                    data[dstOffset + bit] = color;
                    black <<= 1;
                    white <<= 1;
                }
            }
        }
        SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(
            data, cursorW, cursorH, sizeof(std::uint32_t) * 8,
            cursorW * sizeof(std::uint32_t),
            SDL_PIXELFORMAT_ARGB8888);
        if (!surf) [[unlikely]] {
            std::cerr << "SDL_CreateSurfaceFrom failed: "
                      << SDL_GetError() << std::endl;
            std::abort();
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf);
        if (!texture) [[unlikely]] {
            std::cerr << "SDL_CreateTextureFromSurface failed: "
                      << SDL_GetError() << std::endl;
            std::abort();
        }
        SDL_FreeSurface(surf);

        return texture;
    }

    std::uint16_t mouseButtonState()
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

    bool hasMouse()
    {
        // https://discourse.libsdl.org/t/detect-if-the-mouse-is-available-on-the-current-platform/25081
        SDL_Cursor* c = SDL_GetDefaultCursor();
        if (c) {
            SDL_FreeCursor(c);
            return true;
        }
        return false;
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

    std::uint16_t mousePressedFlags(const SDL_MouseButtonEvent& e)
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

    std::uint16_t mouseReleasedFlags(const SDL_MouseButtonEvent& e)
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

} // namespace

MouseDriver::MouseDriver(SDL_Renderer* renderer)
    : m_touchHandler(
          renderer,
          [this](TouchHandler::Action a, int x, int y) {
              handleTouchAction(a, x, y);
          })
    , m_mouseButtonState(mouseButtonState())
    , m_cursorTexture(createTexture(renderer))
    , m_mouseConnected(hasMouse())
{
}

MouseDriver::~MouseDriver()
{
    SDL_DestroyTexture(m_cursorTexture);
}

void MouseDriver::setCursorVisibility(bool visible)
{
    m_cursorVisible = visible;
}

bool MouseDriver::cursorVisible() const noexcept
{
    return m_mouseConnected && m_cursorVisible;
}

void MouseDriver::setPosition(int x, int y)
{
    // SDL_WarpMouseInWindow returns void, but it may fail in some cases
    // (e.g. this doesn't work in WASM environment).
    // Thus, we manually issue a motion event (otherwise the demo mode might be broken).
    SDL_MouseMotionEvent e = {};
    e.x = x;
    e.y = y;
    onMouseMove(e);
}

MouseHandler MouseDriver::setHandler(MouseHandler hdl)
{
    std::swap(hdl, m_handler);
    return hdl;
}

void MouseDriver::drawCursor(SDL_Renderer* renderer)
{
    if (!m_mouseConnected) {
        m_touchHandler.draw(renderer);
        Driver::instance().vga().requestScreenUpdate(); // TODO check if the animation is visible
        return;
    }

    /* It would be better to use SDL_CreateCursor / SDL_SetCursor to set
       a custom cursor. But in this case the cursor is drawn very blurry
       on high-dpi systems (like MacBook with retina display).
       So, we use custom cursor drawing instead.
     */
    if (!cursorVisible())
        return;

    assert(m_cursorTexture);

    SDL_Rect dstRect = { m_cursorX, m_cursorY, cursorW, cursorH };
    SDL_RenderCopy(renderer, m_cursorTexture, nullptr, &dstRect);
}

void MouseDriver::onMouseButtonEvent(const SDL_MouseButtonEvent& e)
{
    m_mouseConnected = true;
    if (!m_handler) [[unlikely]]
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

    m_handler(mouseEventFlags, m_mouseButtonState, e.x, e.y);
}

void MouseDriver::onMouseMove(const SDL_MouseMotionEvent& e)
{
    m_mouseConnected = true;
    if (!m_handler) [[unlikely]]
        return;

    if (cursorVisible())
        Driver::instance().vga().requestScreenUpdate();

    // limit mouse movement if the debug graphics is active
    const Sint32 x = std::min(e.x, SCREEN_WIDTH);
    const Sint32 y = std::min(e.y, SCREEN_HEIGHT);

    // window is small => coordinates can't be large
    assert(x <= std::numeric_limits<std::int16_t>::max());
    assert(x >= std::numeric_limits<std::int16_t>::min());
    assert(y <= std::numeric_limits<std::int16_t>::max());
    assert(y >= std::numeric_limits<std::int16_t>::min());

    m_handler(0, m_mouseButtonState, x, y);

    m_cursorX = static_cast<int>(x);
    m_cursorY = static_cast<int>(y);
}

void MouseDriver::onTouch(const SDL_TouchFingerEvent& e)
{
    m_mouseConnected = false;
    const int x = static_cast<int>(e.x * SCREEN_WIDTH);
    const int y = static_cast<int>(e.y * SCREEN_HEIGHT);
    switch (e.type) {
    case SDL_FINGERMOTION:
        m_touchHandler.onMove(x, y);
        break;
    case SDL_FINGERDOWN: {
        m_touchHandler.onPressStart(x, y);
        break;
    }
    case SDL_FINGERUP:
        m_touchHandler.onPressEnd();
        break;
    [[unlikely]] default:
        std::cerr << "Unexpected touch event, type = " << e.type << std::endl;
    }
}

void MouseDriver::handleTouchAction(TouchHandler::Action action, int x, int y)
{
    if (!m_handler) [[unlikely]]
        return;

    switch (action) {
    case TouchHandler::Action::Tap:
        m_handler(ME_LEFTRELEASED, 0, x, y);
        break;
    case TouchHandler::Action::LongTap:
        m_handler(ME_RIGHTRELEASED, 0, x, y);
        break;
    case TouchHandler::Action::Swipe:
        m_handler(
            ME_LEFTPRESSED | ME_RIGHTPRESSED,
            MouseButton::MB_LEFT | MouseButton::MB_RIGHT, x, y);
        m_handler(ME_LEFTRELEASED | ME_RIGHTRELEASED, 0, x, y);
        break;
    }
}

} // namespace resl
