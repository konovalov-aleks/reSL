#include "video.h"

#include "graphics/vga.h"

#include <SDL_error.h>
#include <SDL_mouse.h>
#include <SDL_pixels.h>
#include <SDL_rect.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>

namespace resl {

// https://www.cs.utexas.edu/~lorenzo/corsi/439/ref/hardware/vgadoc/VGAREGS.TXT

static const char g_windowTitle[] = "reSL - reverse engineered ShortLine game";

static const std::array<std::uint32_t, 16> g_palette = {
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
    0xFFFFFF  // White / erase
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
    unlockTexture();

    SDL_Rect srcRect = { 0, 0, m_wndWidth, m_wndHeight };
    SDL_RenderCopy(m_renderer, m_screen, &srcRect, nullptr);
    SDL_RenderPresent(m_renderer);

    lockTexture();
}

void VGAEmulation::setDebugMode(bool debug)
{
    if (debug) {
        m_wndWidth = VIDEO_MEM_ROW_BYTES * 8;
        m_wndHeight = VIDEO_MEM_N_ROWS;
    } else {
        m_wndWidth = SCREEN_WIDTH;
        m_wndHeight = SCREEN_HEIGHT;
    }
    SDL_SetWindowSize(m_window, m_wndWidth, m_wndHeight);
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
    SDL_SetWindowTitle(m_window, g_windowTitle);
    SDL_WarpMouseInWindow(m_window, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    m_screen = SDL_CreateTexture(
        m_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING,
        VIDEO_MEM_ROW_BYTES * 8,
        VIDEO_MEM_N_ROWS);
    if (!m_screen) [[unlikely]] {
        std::cerr << "Unable to create SDL texture! SDL_Error: " << SDL_GetError() << std::endl;
        close();
        std::exit(EXIT_FAILURE);
    }

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
}

void VGAEmulation::unlockTexture()
{
    SDL_UnlockTexture(m_screen);
}

void VGAEmulation::write(VideoMemPtr memPtr, std::uint8_t color)
{
    unsigned offset = memPtr - VIDEO_MEM_START_ADDR;
    std::uint8_t mask = m_vgaState.writeMask;
    int x0 = (offset * 8) % (VIDEO_MEM_ROW_BYTES * 8);
    int y = (offset * 8) / (VIDEO_MEM_ROW_BYTES * 8);
    std::uint32_t rgb = 0;
    switch (m_vgaState.writeMode) {
    case 0:
        mask = color;
        break;
    case 1:
        mask = 0xFF;
        break;
    case 2:
        rgb = g_palette[color];
        break;
    };
    for (int x = x0; x < x0 + 8; ++x) {
        if (m_vgaState.writeMode == 1) {
            // read from latches
            std::uint8_t c = 0;
            const std::uint8_t bitMask = 1 << (7 - (x - x0));
            for (int plane = 0; plane < 4; ++plane)
                c |= ((m_vgaState.latches[plane] & bitMask) != 0) << plane;
            rgb = g_palette[c];
        }

        const auto getPixel = [this, &x, &y]() {
            std::uint32_t rgb;
            std::memcpy(
                &rgb, &m_screenPixels[m_screenPixelsPitch * y + x * sizeof(rgb)], sizeof(rgb));
            auto iter = std::find(g_palette.begin(), g_palette.end(), rgb);
            assert(iter != g_palette.end());
            return static_cast<std::uint8_t>(std::distance(g_palette.begin(), iter));
        };

        if (m_vgaState.writeMode == 0) {
            std::uint8_t c = getPixel();
            c = (c & ~m_vgaState.regMapMask) | (((mask & 0x80) ? m_vgaState.regMapMask : 0));
            rgb = g_palette[c];
            std::memcpy(
                &m_screenPixels[m_screenPixelsPitch * y + x * sizeof(rgb)], &rgb, sizeof(rgb));
        } else if (mask & 0x80) {
            std::uint8_t c = color;
            if (m_vgaState.writeOperation != WriteOperation::Copy) {
                std::uint8_t oldColor = getPixel();
                switch (m_vgaState.writeOperation) {
                case WriteOperation::Copy:
                    break;
                case WriteOperation::And:
                    c &= oldColor;
                    break;
                case WriteOperation::Or:
                    c |= oldColor;
                    break;
                case WriteOperation::Xor:
                    c ^= oldColor;
                    break;
                }
                rgb = g_palette[c];
            }
            std::memcpy(
                &m_screenPixels[m_screenPixelsPitch * y + x * sizeof(rgb)], &rgb, sizeof(rgb));
        }
        mask <<= 1;
    }
}

std::uint8_t VGAEmulation::read(VideoMemPtr memPtr)
{
    unsigned offset = memPtr - VIDEO_MEM_START_ADDR;
    int x0 = (offset * 8) % (VIDEO_MEM_ROW_BYTES * 8);
    int y = (offset * 8) / (VIDEO_MEM_ROW_BYTES * 8);

    const std::uint8_t planeMask = (1 << m_vgaState.readPlane);
    std::uint8_t res = 0;
    for (int plane = 0; plane < 4; ++plane)
        m_vgaState.latches[plane] = 0;
    for (int i = 0; i < 8; ++i) {
        std::uint32_t rgb = 0;
        std::memcpy(
            &rgb, &m_screenPixels[m_screenPixelsPitch * y + (x0 + i) * sizeof(rgb)], sizeof(rgb));
        auto iter = std::find(g_palette.begin(), g_palette.end(), rgb);
        assert(iter != g_palette.end());
        std::uint8_t c = std::distance(g_palette.begin(), iter);
        res |= ((c & planeMask) != 0) << (7 - i);
        for (int plane = 0; plane < 4; ++plane) {
            m_vgaState.latches[plane] |= (c & 1) << (7 - i);
            c >>= 1;
        }
    }
    return res;
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

void VGAEmulation::setMapMask(std::uint8_t mask)
{
    m_vgaState.regMapMask = mask;
}

void VGAEmulation::setReadPlane(std::uint8_t p)
{
    m_vgaState.readPlane = p;
}

void VGAEmulation::setWriteOperation(WriteOperation op)
{
    m_vgaState.writeOperation = op;
}

} // namespace resl
