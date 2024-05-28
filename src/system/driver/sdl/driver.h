#pragma once

// IWYU pragma: private, include <system/driver/driver.h>
// IWYU pragma: friend "system/driver/sdl/.*"

// IWYU pragma: no_include "system/driver/sdl/audio.h"
// IWYU pragma: no_include "system/driver/sdl/video.h"

#include "audio.h" // IWYU pragma: export
#include "video.h" // IWYU pragma: export

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

    bool pollEvent();

private:
    Driver() = default;
    Driver(const Driver&) = delete;
    Driver& operator=(const Driver&) = delete;

    class SDLInit {
    public:
        SDLInit();
        ~SDLInit();
    };

    SDLInit m_sdl;

    VGAEmulation m_vga;
    AudioDriver m_audio;

    bool m_quit = false;
};

} // namespace resl
