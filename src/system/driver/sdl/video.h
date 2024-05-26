#pragma once

#include <graphics/vga.h>

#include <SDL.h>

#include <cstdint>

namespace resl {

class VGAEmulation {
public:
    VGAEmulation();
    ~VGAEmulation();

    VGAEmulation(const VGAEmulation&) = delete;
    VGAEmulation& operator=(const VGAEmulation&) = delete;

    void flush();
    void setDebugMode(bool debug);

    std::uint8_t read(VideoMemPtr);
    void write(VideoMemPtr, std::uint8_t color);

    void setWriteMask(std::uint8_t);

    std::uint8_t writeMode();
    void setMode(std::uint8_t);

    void setMapMask(std::uint8_t);
    void setReadPlane(std::uint8_t);
    void setWriteOperation(WriteOperation);

private:
    struct VGAState {
        std::uint8_t latches[4];

        std::uint8_t writeMask = 0;
        std::uint8_t writeMode = 2;

        WriteOperation writeOperation = WriteOperation::Copy;

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

        std::uint8_t regMapMask = 0xF;
    };

    void init();
    void close();

    void lockTexture();
    void unlockTexture();

    SDL_Window* m_window = NULL;
    SDL_Renderer* m_renderer = NULL;
    SDL_Texture* m_screen = NULL;
    char* m_screenPixels = NULL;
    int m_screenPixelsPitch = 0;

    int m_wndWidth = SCREEN_WIDTH;
    int m_wndHeight = SCREEN_HEIGHT;

    VGAState m_vgaState;
};

} // namespace resl
