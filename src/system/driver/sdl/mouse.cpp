#include "mouse.h"

#include "system/driver/sdl/driver.h"
#include "system/driver/sdl/video.h"

#include <SDL_error.h>
#include <SDL_mouse.h>
#include <SDL_pixels.h>
#include <SDL_rect.h>
#include <SDL_surface.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>

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

} // namespace

MouseDriver::MouseDriver()
    : m_mouseConnected(hasMouse())
{
}

MouseDriver::~MouseDriver()
{
    if (m_cursorTexture)
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
    SDL_WarpMouseInWindow(Driver::instance().vga().m_window, x, y);
}

void MouseDriver::drawCursor(SDL_Renderer* renderer)
{
    /* It would be better to use SDL_CreateCursor / SDL_SetCursor to set
       a custom cursor. But in this case the cursor is drawn very blurry
       on high-dpi systems (like MacBook with retina display).
       So, we use custom cursor drawing instead.
     */
    if (!cursorVisible())
        return;

    if (!m_cursorTexture)
        m_cursorTexture = createTexture(renderer);

    int x, y;
    SDL_GetMouseState(&x, &y);
    SDL_Rect dstRect = { x, y, cursorW, cursorH };
    SDL_RenderCopy(renderer, m_cursorTexture, nullptr, &dstRect);
}

bool MouseDriver::hasMouse() const
{
    // https://discourse.libsdl.org/t/detect-if-the-mouse-is-available-on-the-current-platform/25081
    SDL_Cursor* c = SDL_GetDefaultCursor();
    if (c) {
        SDL_FreeCursor(c);
        return true;
    }
    return false;
}

} // namespace resl
