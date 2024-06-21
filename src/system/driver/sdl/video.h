#pragma once

#include <graphics/vga.h>

#include <SDL_render.h>
#include <SDL_video.h>

#include <array>
#include <chrono>
#include <cstdint>

namespace resl {

class VGAEmulation {
public:
    VGAEmulation();
    ~VGAEmulation();

    VGAEmulation(const VGAEmulation&) = delete;
    VGAEmulation& operator=(const VGAEmulation&) = delete;

    void flush();
    void waitVerticalRetrace();

    void setDebugMode(bool debug);

    [[nodiscard]] std::uint8_t read(vga::VideoMemPtr);
    void write(vga::VideoMemPtr, std::uint8_t color);

    void setWriteMask(std::uint8_t);

    [[nodiscard]] std::uint8_t writeMode();
    void setMode(std::uint8_t);

    void setPlaneMask(std::uint8_t);
    void setReadPlane(std::uint8_t);
    void setWriteOperation(vga::WriteOperation);

    void setPaletteItem(std::uint8_t idx, std::uint32_t rgb);

    unsigned timeToNextFrameMS() const;

private:
    static constexpr int s_FPS = 60;

    using ClockT = std::chrono::steady_clock;

    struct VGAState {
        std::uint8_t latches[4] = {};

        // 3CEh index 8
        std::uint8_t writeMask = 0;
        std::uint8_t writeMode = 2;

        vga::WriteOperation writeOperation = vga::WriteOperation::Copy;

        /* Read Mode
         0: Data is read from one of 4 bit planes depending on the Read Map
            Select Register (3CEh index 4).
         1: Data returned is a comparison between the 8 pixels occupying the
            read byte and the color in the Color Compare Register (3CEh
            index 2). A bit is set if the color of the corresponding pixel
            matches the register. */
        std::uint8_t readMode = 0;

        // 3CEh index  4  (R/W):  Graphics: Read Map Select Register
        // bit 0-1  Number of the plane Read Mode 0 will read from.
        std::uint8_t readPlane = 0;

        // 3C4h index 2 (Color Plane Write Enable Register)
        std::uint8_t regPlaneMask = 0xF;

        std::array<std::uint32_t, 16> palette;

        std::uint8_t mem[0x10000][4];
    };

    void init();
    void close();

    void lockTexture();
    void unlockTexture();

    void updateVideoMemory(unsigned);

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture* m_screen = nullptr;
    std::uint32_t* m_screenPixels = nullptr;
    int m_screenPixelsPitch = 0;

    int m_wndWidth = SCREEN_WIDTH;
    int m_wndHeight = SCREEN_HEIGHT;

    VGAState m_vgaState;

    ClockT::time_point m_nextFrameTime = {};

    bool m_dirty = true;
};

} // namespace resl
