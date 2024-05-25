#pragma once

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

    bool pollEvent();

private:
    Driver();
    ~Driver();

    Driver(const Driver&) = delete;
    Driver& operator=(const Driver&) = delete;

    VGAEmulation m_vga;
    bool m_quit = false;
};

} // namespace resl
