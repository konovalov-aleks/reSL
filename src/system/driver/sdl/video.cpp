#include "video.h"

#include "graphics/vga.h"
#include "system/driver/sdl/driver.h"
#include "system/driver/sdl/mouse.h"

#include <SDL_blendmode.h>
#include <SDL_error.h>
#include <SDL_hints.h>
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
#include <iterator>
#include <limits>

#ifdef __EMSCRIPTEN__
#   include <emscripten.h>
#else
#   include <thread>
#endif // __EMSCRIPTEN__

namespace resl {

// https://www.cs.utexas.edu/~lorenzo/corsi/439/ref/hardware/vgadoc/VGAREGS.TXT

static const char g_windowTitle[] = "reSL - reverse engineered ShortLine game";

static constexpr Uint32 g_supportedPixelFormats[] = {
    SDL_PIXELFORMAT_ARGB8888,
    SDL_PIXELFORMAT_ABGR8888,
    SDL_PIXELFORMAT_RGBA8888,
    SDL_PIXELFORMAT_BGRA8888,
};

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

    if (updatePicture() || m_needRedraw) {
        m_needRedraw = false;

        SDL_SetRenderDrawColor(m_renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(m_renderer);
        SDL_SetRenderDrawColor(m_renderer, 0x55, 0xAA, 0x00, 0xFF);
        const SDL_Rect screenRect = { 0, 0, m_wndWidth, m_wndHeight };
        SDL_RenderFillRect(m_renderer, &screenRect);
        for (Overlay& ov : m_overlays) {
            const int yOffset = m_vgaState.overflowLineCompare < SCREEN_HEIGHT
                ? m_vgaState.overflowLineCompare
                : 0;
            ov(m_renderer, yOffset);
        }

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

        Driver::instance().mouse().drawCursor(m_renderer);
        SDL_RenderPresent(m_renderer);

        m_dirtyRect = {};
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

void VGAEmulation::setFullscreenMode(bool fullscreen)
{
    SDL_SetWindowFullscreen(m_window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void VGAEmulation::init()
{
#ifdef __APPLE__
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#endif // __APPLE__

    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");

    Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
#ifndef __EMSCRIPTEN__
    flags |= SDL_WINDOW_RESIZABLE;
#endif // !__EMSCRIPTEN__
    int err = SDL_CreateWindowAndRenderer(
        m_wndWidth, m_wndHeight, flags, &m_window, &m_renderer);
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

    m_pixelFormat = choosePixelFormat();
    std::cout << "Pixel format: " << SDL_GetPixelFormatName(m_pixelFormat)
              << std::endl;
    generatePalette();

    // roundUp(memSize / vgaRowBytes)
    constexpr int nRows =
        1 + (sizeof(VGAState::mem) / sizeof(VGAState::mem[0]) - 1) / vga::VIDEO_MEM_ROW_BYTES;
    m_screen = SDL_CreateTexture(
        m_renderer, m_pixelFormat, SDL_TEXTUREACCESS_STREAMING,
        vga::VIDEO_MEM_ROW_BYTES * 8, nRows);
    SDL_SetTextureBlendMode(m_screen, SDL_BLENDMODE_BLEND);
    if (!m_screen) [[unlikely]] {
        std::cerr << "Unable to create SDL texture! SDL_Error: " << SDL_GetError() << std::endl;
        close();
        std::exit(EXIT_FAILURE);
    }

    SDL_SetRenderTarget(m_renderer, nullptr);
}

void VGAEmulation::close()
{
    if (m_screen)
        SDL_DestroyTexture(m_screen);
    if (m_renderer)
        SDL_DestroyRenderer(m_renderer);

    m_renderer = nullptr;
    m_window = nullptr;
    m_screen = nullptr;
}

Uint32 VGAEmulation::choosePixelFormat()
{
    SDL_RendererInfo rInfo;
    if (SDL_GetRendererInfo(m_renderer, &rInfo)) [[unlikely]] {
        std::cerr << "WARNING: SDL_GetRendererInfo failed. SDL_Error: " << SDL_GetError()
                  << "\nWill use the default pixel format" << std::endl;
        return g_supportedPixelFormats[0];
    }

    std::cout << "Renderer: " << rInfo.name << "\nSupported pixel formats:\n";
    for (Uint32 i = 0; i < rInfo.num_texture_formats; ++i) {
        std::cout << '\t'
                  << SDL_GetPixelFormatName(rInfo.texture_formats[i])
                  << std::endl;
    }
    auto iter = std::find_first_of(
        rInfo.texture_formats, rInfo.texture_formats + rInfo.num_texture_formats,
        std::begin(g_supportedPixelFormats), std::end(g_supportedPixelFormats));
    if (iter != rInfo.texture_formats + rInfo.num_texture_formats)
        return *iter;

    std::cerr << "WARNING: no one preferred pixel format suits, "
                 "the default format will be used instead"
              << std::endl;
    return g_supportedPixelFormats[0];
}

void VGAEmulation::generatePalette()
{
    assert(std::find(
               std::begin(g_supportedPixelFormats),
               std::end(g_supportedPixelFormats),
               m_pixelFormat) != std::end(g_supportedPixelFormats));

    m_vgaState.palette = {
        0x0055AA00, // Green (transparent to be able to draw on the background)
        0xFF000000, // Black
        0xFFAAAAAA, // Gray
        0xFF555555, // Dark gray
        0xFFFFFFFF, // White
        0xFFFFFF55, // Yellow
        0xFFAAAA00, // Brown
        0xFF0055FF, // Blue
        0xFF0055AA, // Dark blue
        0xFFFF5500, // Red
        0xFFAA5500, // Dark red
        0xFF1AFFFF, // Cyan
        0xFF00AAAA, // Dark cyan
        0xFF00FF00, // Light green
        0xFF00AA00, // Dark green
        0xFFFFFFFF  // Blinking color (Black/White)
    };

    if (m_pixelFormat == SDL_PIXELFORMAT_ARGB8888)
        return;

    for (std::uint32_t& v : m_vgaState.palette)
        v = argbToPreferred(v);
}

std::uint32_t VGAEmulation::argbToPreferred(std::uint32_t argb) const
{
    if (m_pixelFormat == SDL_PIXELFORMAT_ARGB8888)
        return argb;

    std::uint32_t c = argb & 0xFFFFFF;
    if (m_pixelFormat == SDL_PIXELFORMAT_ABGR8888 ||
        m_pixelFormat == SDL_PIXELFORMAT_BGRA8888) {

        // RGB -> BGR
        c = (c & 0xFF) << 16 | (c & 0xFF00) | (c >> 16);
    }

    // add alpha value
    if (m_pixelFormat == SDL_PIXELFORMAT_ABGR8888)
        return (argb & 0xFF000000) | c;

    if (m_pixelFormat == SDL_PIXELFORMAT_RGBA8888 ||
        m_pixelFormat == SDL_PIXELFORMAT_BGRA8888) {

        return (c << 8) | (argb >> 24);
    }

    // unreachable
    std::cerr << "FATAL: Unsupported pixel format "
              << SDL_GetPixelFormatName(m_pixelFormat) << std::endl;
    std::abort();
}

bool VGAEmulation::updatePicture()
{
    SDL_Rect screenRect = { 0, 0, m_wndWidth, vga::VIDEO_MEM_N_ROWS };
    SDL_IntersectRect(&m_dirtyRect, &screenRect, &m_dirtyRect);
    if (SDL_RectEmpty(&m_dirtyRect))
        return false;

    std::uint32_t* dst = nullptr;
    int pitch = 0;
    int err = SDL_LockTexture(m_screen, &m_dirtyRect, reinterpret_cast<void**>(&dst), &pitch);
    if (err) [[unlikely]] {
        std::cerr << "Unable to lock the texture! SDL_Error: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    assert(dst);
    assert(pitch);
    assert(pitch % sizeof(std::uint32_t) == 0);
    pitch /= sizeof(std::uint32_t);

    std::size_t srcByte =
        m_dirtyRect.y * vga::VIDEO_MEM_ROW_BYTES + m_dirtyRect.x / 8;
    for (int y = 0; y < m_dirtyRect.h; ++y) {
        const std::size_t s = srcByte;
        for (int x = 0; x < m_dirtyRect.w; ++x) {
            const int bit = (x + m_dirtyRect.x) & 7;
            assert(srcByte < 0x10000);
            std::uint8_t color =
                (((m_vgaState.mem[srcByte][3] << bit) & 0x80) >> 4) |
                (((m_vgaState.mem[srcByte][2] << bit) & 0x80) >> 5) |
                (((m_vgaState.mem[srcByte][1] << bit) & 0x80) >> 6) |
                (((m_vgaState.mem[srcByte][0] << bit) & 0x80) >> 7);
            dst[x] = m_vgaState.palette[color];
            if (bit == 7)
                ++srcByte;
        }
        dst += pitch;
        srcByte = s + vga::VIDEO_MEM_ROW_BYTES;
    }

    SDL_UnlockTexture(m_screen);
    return true;
}

void VGAEmulation::updateVideoMemory(unsigned srcByte)
{
    const int x = (srcByte * 8) % (vga::VIDEO_MEM_ROW_BYTES * 8);
    const int y = (srcByte * 8) / (vga::VIDEO_MEM_ROW_BYTES * 8);

    if (x >= m_wndWidth)
        return;

    SDL_Rect r = { x, y, 8, 1 };
    SDL_UnionRect(&m_dirtyRect, &r, &m_dirtyRect);
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
        m_needRedraw = true;
    }
}

void VGAEmulation::setFrameOrigin([[maybe_unused]] std::int16_t x, std::int16_t y)
{
    assert(x == 0); // not supported
    if (m_vgaState.yOrigin != y) {
        m_vgaState.yOrigin = y;
        m_needRedraw = true;
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

void VGAEmulation::setPaletteItem(std::uint8_t idx, std::uint32_t argb)
{
    assert(idx < m_vgaState.palette.size());
    m_vgaState.palette[idx] = argbToPreferred(argb);

    int x1 = std::numeric_limits<int>::max();
    int x2 = std::numeric_limits<int>::min();
    int y1 = std::numeric_limits<int>::max();
    int y2 = std::numeric_limits<int>::min();

    std::size_t srcByte = 0;
    for (int y = 0; y < static_cast<int>(vga::VIDEO_MEM_N_ROWS); ++y) {
        const std::size_t s = srcByte;
        for (int x = 0; x <= m_wndWidth; x += 8) {
            for (int bit = 0; bit < 8; ++bit) {
                std::uint8_t color =
                    (((m_vgaState.mem[srcByte][3] << bit) & 0x80) >> 4) |
                    (((m_vgaState.mem[srcByte][2] << bit) & 0x80) >> 5) |
                    (((m_vgaState.mem[srcByte][1] << bit) & 0x80) >> 6) |
                    (((m_vgaState.mem[srcByte][0] << bit) & 0x80) >> 7);
                if (color == idx) {
                    int xx = x + bit;
                    x1 = std::min(x1, xx);
                    x2 = std::max(x2, xx);
                    y1 = std::min(y1, y);
                    y2 = std::max(y2, y);
                }
            }
            ++srcByte;
        }
        srcByte = s + vga::VIDEO_MEM_ROW_BYTES;
    }

    if (x2 >= x1) {
        assert(y2 >= y1);
        SDL_Rect r = { x1, y1, x2 - x1 + 1, y2 - y1 + 1 };
        SDL_UnionRect(&m_dirtyRect, &r, &m_dirtyRect);
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
