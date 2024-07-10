#include "video.h"

#include "graphics/vga.h"
#include "system/driver/sdl/driver.h"

#include <SDL_blendmode.h>
#include <SDL_error.h>
#include <SDL_mouse.h>
#include <SDL_pixels.h>
#include <SDL_rect.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <compare>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

#ifdef __EMSCRIPTEN__
#   include <emscripten.h>
#else
#   include <thread>
#endif // __EMSCRIPTEN__

namespace resl {

// https://www.cs.utexas.edu/~lorenzo/corsi/439/ref/hardware/vgadoc/VGAREGS.TXT

static const char g_windowTitle[] = "reSL - reverse engineered ShortLine game";

VGAEmulation::VGAEmulation()
{
    init();
}

VGAEmulation::~VGAEmulation()
{
    close();
}

void VGAEmulation::flush()
{
    const ClockT::time_point now = ClockT::now();
    if (now < m_nextFrameTime)
        return;
    m_nextFrameTime = now + std::chrono::microseconds(1000000 / s_FPS);

    if (m_dirty) {
        m_dirty = false;

        unlockTexture();

        const bool isDebugGraphicsMode = m_wndWidth != SCREEN_WIDTH;
        if (isDebugGraphicsMode) [[unlikely]] {
            SDL_Rect srcRect = { 0, 0, m_wndWidth, m_wndHeight };
            SDL_RenderCopy(m_renderer, m_screen, &srcRect, nullptr);

            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_MUL);
            SDL_SetRenderDrawColor(m_renderer, 51, 204, 204, 255);
            SDL_Rect frameRect = {
                0, m_vgaState.yOrigin,
                m_wndWidth, std::min<int>(m_vgaState.overflowLineCompare, SCREEN_HEIGHT)
            };
            SDL_RenderDrawRect(m_renderer, &frameRect);

            if (m_vgaState.overflowLineCompare < SCREEN_HEIGHT) {
                SDL_SetRenderDrawColor(m_renderer, 153, 204, 0, 255);
                frameRect = {
                    0, 0,
                    m_wndWidth, SCREEN_HEIGHT - m_vgaState.overflowLineCompare
                };
                SDL_RenderDrawRect(m_renderer, &frameRect);
            }

        } else {
            if (m_vgaState.overflowLineCompare < SCREEN_HEIGHT) {
                SDL_Rect srcRect = {
                    0, m_vgaState.yOrigin,
                    SCREEN_WIDTH, m_vgaState.overflowLineCompare
                };
                SDL_Rect dstRect = {
                    0, 0,
                    SCREEN_WIDTH, m_vgaState.overflowLineCompare
                };
                SDL_RenderCopy(m_renderer, m_screen, &srcRect, &dstRect);

                dstRect = {
                    0, m_vgaState.overflowLineCompare,
                    SCREEN_WIDTH, SCREEN_HEIGHT - m_vgaState.overflowLineCompare
                };
                srcRect = {
                    0, 0,
                    SCREEN_WIDTH, SCREEN_HEIGHT - m_vgaState.overflowLineCompare
                };
                SDL_RenderCopy(m_renderer, m_screen, &srcRect, &dstRect);
            } else {
                SDL_Rect srcRect = { 0, m_vgaState.yOrigin, m_wndWidth, m_wndHeight };
                SDL_RenderCopy(m_renderer, m_screen, &srcRect, nullptr);
            }
        }
        SDL_RenderPresent(m_renderer);

        lockTexture();
    }

    // need to poll events to make the image appear
    Driver::instance().pollEvent();
}

void VGAEmulation::waitVerticalRetrace()
{
    const ClockT::time_point now = ClockT::now();
    if (now < m_nextFrameTime) {
#ifdef __EMSCRIPTEN__
        emscripten_sleep(
            std::chrono::duration_cast<std::chrono::milliseconds>(m_nextFrameTime - now).count());
#else
        std::this_thread::sleep_for(m_nextFrameTime - now);
#endif
    }
    flush();
}

void VGAEmulation::setDebugMode(bool debug)
{
    if (debug) {
        m_wndWidth = vga::VIDEO_MEM_ROW_BYTES * 8;
        m_wndHeight = vga::VIDEO_MEM_N_ROWS;
    } else {
        m_wndWidth = SCREEN_WIDTH;
        m_wndHeight = SCREEN_HEIGHT;
    }
    SDL_SetWindowSize(m_window, m_wndWidth, m_wndHeight);
    if (SDL_RenderSetLogicalSize(m_renderer, m_wndWidth, m_wndHeight) != 0) [[unlikely]] {
        std::cerr << "SDL_RenderSetLogicalSize failed: " << SDL_GetError() << std::endl;
        close();
        std::exit(EXIT_FAILURE);
    }
}

void VGAEmulation::init()
{
    int err = SDL_CreateWindowAndRenderer(
        m_wndWidth, m_wndHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI, &m_window,
        &m_renderer);
    if (err) [[unlikely]] {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        close();
        std::exit(EXIT_FAILURE);
    }
    if (SDL_RenderSetLogicalSize(m_renderer, m_wndWidth, m_wndHeight) != 0) [[unlikely]] {
        std::cerr << "SDL_RenderSetLogicalSize failed: " << SDL_GetError() << std::endl;
        close();
        std::exit(EXIT_FAILURE);
    }
    SDL_SetWindowTitle(m_window, g_windowTitle);
    SDL_WarpMouseInWindow(m_window, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    // roundUp(memSize / vgaRowBytes)
    constexpr int nRows = 1 + (sizeof(VGAState::mem) / sizeof(VGAState::mem[0]) - 1) / vga::VIDEO_MEM_ROW_BYTES;
    m_screen = SDL_CreateTexture(
        m_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING,
        vga::VIDEO_MEM_ROW_BYTES * 8, nRows);
    if (!m_screen) [[unlikely]] {
        std::cerr << "Unable to create SDL texture! SDL_Error: " << SDL_GetError() << std::endl;
        close();
        std::exit(EXIT_FAILURE);
    }

    m_vgaState.palette = {
        0x55AA00, // Green
        0x000000, // Black
        0xAAAAAA, // Gray
        0x555555, // Dark gray
        0xFFFFFF, // White
        0xFFFF55, // Yellow
        0xAAAA00, // Brown
        0x0055FF, // Blue
        0x0055AA, // Dark blue
        0xFF5500, // Red
        0xAA5500, // Dark red
        0x1AFFFF, // Cyan
        0x00AAAA, // Dark cyan
        0x00FF00, // Light green
        0x00AA00, // Dark green
        0x000000  // Blinking color (Black/White)
    };

    lockTexture();

    SDL_SetRenderTarget(m_renderer, nullptr);
}

void VGAEmulation::close()
{
    if (m_screen)
        SDL_DestroyTexture(m_screen);
    if (m_renderer)
        SDL_DestroyRenderer(m_renderer);
    if (m_window)
        SDL_DestroyWindow(m_window);

    m_renderer = nullptr;
    m_window = nullptr;
    m_screen = nullptr;
}

void VGAEmulation::lockTexture()
{
    int err = SDL_LockTexture(m_screen, nullptr, reinterpret_cast<void**>(&m_screenPixels), &m_screenPixelsPitch);
    if (err) [[unlikely]] {
        std::cerr << "Unable to lock the texture! SDL_Error: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    assert(m_screenPixels);
    assert(m_screenPixelsPitch);
    assert(m_screenPixelsPitch % sizeof(std::uint32_t) == 0);
    m_screenPixelsPitch /= sizeof(std::uint32_t);
}

void VGAEmulation::unlockTexture()
{
    SDL_UnlockTexture(m_screen);
#ifndef NDEBUG
    m_screenPixels = nullptr;
    m_screenPixelsPitch = 0;
#endif // !NDEBUG
}

void VGAEmulation::updateVideoMemory(unsigned srcByte)
{
    const int x = (srcByte * 8) % (vga::VIDEO_MEM_ROW_BYTES * 8);
    const int y = (srcByte * 8) / (vga::VIDEO_MEM_ROW_BYTES * 8);

    if (x >= m_wndWidth)
        return;

    std::uint32_t* dst = &m_screenPixels[x + y * m_screenPixelsPitch];
    for (int bit = 0; bit < 8; ++bit) {
        std::uint8_t color =
            (((m_vgaState.mem[srcByte][3] << bit) & 0x80) >> 4) |
            (((m_vgaState.mem[srcByte][2] << bit) & 0x80) >> 5) |
            (((m_vgaState.mem[srcByte][1] << bit) & 0x80) >> 6) |
            (((m_vgaState.mem[srcByte][0] << bit) & 0x80) >> 7);
        dst[bit] = m_vgaState.palette[color];
    }
    m_dirty = true;
}

void VGAEmulation::write(vga::VideoMemPtr memPtr, std::uint8_t color)
{
    unsigned offset = memPtr - vga::VIDEO_MEM_START_ADDR;

    bool changed = false;

    switch (m_vgaState.writeMode) {
    case 0:
        if (m_vgaState.regPlaneMask && m_vgaState.writeMask) {
            // rotation and logical operations are not used in SL
            // => not implemented here
            const std::uint8_t mask = m_vgaState.writeMask;
            for (int plane = 0; plane < 4; ++plane) {
                if (!(m_vgaState.regPlaneMask & (1 << plane)))
                    continue;
                std::uint8_t& v = m_vgaState.mem[offset][plane];
                v = (v & ~mask) | (mask & color);
            }
            changed = true;
        }
        break;
    case 1:
        std::memcpy(&m_vgaState.mem[offset], &m_vgaState.latches,
                    sizeof(m_vgaState.latches));
        changed = true;
        break;

    case 2:
        if (m_vgaState.writeMask) {
            const std::uint8_t mask = m_vgaState.writeMask;
            for (int plane = 0; plane < 4; ++plane) {
                std::uint8_t& v = m_vgaState.mem[offset][plane];

                std::uint8_t bitsToSet = mask * (color & 1);
                switch (m_vgaState.writeOperation) {
                case vga::WriteOperation::Copy:
                    break;
                case vga::WriteOperation::And:
                    bitsToSet &= v;
                    break;
                case vga::WriteOperation::Or:
                    bitsToSet |= v;
                    break;
                case vga::WriteOperation::Xor:
                    bitsToSet ^= v;
                    break;
                }
                v = (v & ~mask) | bitsToSet;
                color >>= 1;
            }
            changed = true;
        }
        break;
    };

    if (changed)
        updateVideoMemory(offset);
}

[[nodiscard]] std::uint8_t VGAEmulation::read(vga::VideoMemPtr memPtr)
{
    unsigned offset = memPtr - vga::VIDEO_MEM_START_ADDR;

    std::memcpy(&m_vgaState.latches, &m_vgaState.mem[offset], sizeof(m_vgaState.latches));

    // SL uses only read mode 0, so all other modes are not implemented
    assert(m_vgaState.readMode == 0);

    return m_vgaState.mem[offset][m_vgaState.readPlane];
}

void VGAEmulation::setWriteMask(std::uint8_t mask)
{
    m_vgaState.writeMask = mask;
}

std::uint8_t VGAEmulation::writeMode()
{
    return m_vgaState.writeMode;
}

void VGAEmulation::setMode(std::uint8_t mode)
{
    m_vgaState.writeMode = mode & 3;
    m_vgaState.readMode = (mode >> 2) & 1;
}

void VGAEmulation::setLineCompareRegister(std::uint16_t y)
{
    // line compare register has 9 bits
    y = y & 0x1FF;
    if (m_vgaState.overflowLineCompare != y) {
        m_vgaState.overflowLineCompare = y;
        m_dirty = true;
    }
}

void VGAEmulation::setFrameOrigin(std::int16_t x, std::int16_t y)
{
    assert(x == 0); // not supported
    if (m_vgaState.yOrigin != y) {
        m_vgaState.yOrigin = y;
        m_dirty = true;
    }
}

void VGAEmulation::setPlaneMask(std::uint8_t mask)
{
    m_vgaState.regPlaneMask = mask;
}

void VGAEmulation::setReadPlane(std::uint8_t p)
{
    m_vgaState.readPlane = p;
}

void VGAEmulation::setWriteOperation(vga::WriteOperation op)
{
    m_vgaState.writeOperation = op;
}

void VGAEmulation::setPaletteItem(std::uint8_t idx, std::uint32_t rgb)
{
    assert(idx < m_vgaState.palette.size());
    m_vgaState.palette[idx] = rgb;

    std::size_t srcByte = 0;
    std::uint32_t* dst = m_screenPixels;
    for (int y = 0; y < vga::VIDEO_MEM_N_ROWS; ++y) {
        const std::size_t s = srcByte;
        for (int x = 0; x <= m_wndWidth; x += 8) {
            for (int bit = 0; bit < 8; ++bit) {
                std::uint8_t color =
                    (((m_vgaState.mem[srcByte][3] << bit) & 0x80) >> 4) |
                    (((m_vgaState.mem[srcByte][2] << bit) & 0x80) >> 5) |
                    (((m_vgaState.mem[srcByte][1] << bit) & 0x80) >> 6) |
                    (((m_vgaState.mem[srcByte][0] << bit) & 0x80) >> 7);
                if (color == idx) {
                    m_dirty = true;
                    dst[x + bit] = rgb;
                }
            }
            ++srcByte;
        }
        dst += m_screenPixelsPitch;
        srcByte = s + vga::VIDEO_MEM_ROW_BYTES;
    }
}

unsigned VGAEmulation::timeToNextFrameMS() const
{
    const ClockT::time_point now = ClockT::now();
    if (now >= m_nextFrameTime)
        return 0;

    const auto dTime = m_nextFrameTime - now;
    return static_cast<unsigned>(
        std::chrono::duration_cast<std::chrono::milliseconds>(dTime).count());
}

} // namespace resl
