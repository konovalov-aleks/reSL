#pragma once

// IWYU pragma: private, include <system/driver/driver.h>
// IWYU pragma: friend "system/driver/sdl/.*"

// IWYU pragma: no_include "system/driver/sdl/audio.h"
// IWYU pragma: no_include "system/driver/sdl/video.h"

#include "audio.h" // IWYU pragma: export
#include "video.h" // IWYU pragma: export

#include <graphics/vga.h>
#include <system/keyboard.h>
#include <system/mouse.h>

#include <SDL_events.h>
#include <SDL_stdinc.h>

#include <cstdint>

namespace resl {

class Driver {
public:
    using VGAAdaptor = VGAEmulation;

    static Driver& instance()
    {
        static Driver g_instance;
        return g_instance;
    }

    VGAEmulation& vga() { return m_vga; }
    AudioDriver& audio() { return m_audio; }

    void sleep(unsigned ms);

    void setMouseHandler(MouseHandler hdl) { m_mouseHandler = hdl; }
    void setKeyboardHandler(KeyboardHandler hdl) { m_keyboardHandler = hdl; }

    void pollEvent();

private:
    Driver();
    Driver(const Driver&) = delete;
    Driver& operator=(const Driver&) = delete;

    void onMouseButtonEvent(const SDL_MouseButtonEvent&);
    void onMouseMove(const SDL_MouseMotionEvent&);

    void onKeyboardEvent(const SDL_KeyboardEvent&);

    class SDLInit {
    public:
        SDLInit();
        ~SDLInit();
    };

    SDLInit m_sdl;

    VGAEmulation m_vga;
    AudioDriver m_audio;

    KeyboardHandler m_keyboardHandler = nullptr;

    MouseHandler m_mouseHandler = nullptr;
    Sint32 m_lastCursorX = LOGICAL_SCREEN_WIDTH / 2;
    Sint32 m_lastCursorY = LOGICAL_SCREEN_HEIGHT / 2;
    std::uint16_t m_mouseButtonState;
};

} // namespace resl
