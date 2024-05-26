#pragma once

#include "audio.h"
#include "video.h"

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
