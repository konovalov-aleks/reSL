#pragma once

#include <cstdint>

namespace resl {

enum Color : std::uint8_t {
    Green = 0x0,      // #55AA00
    Black = 0x1,      // #000000
    Gray = 0x2,       // #AAAAAA
    DarkGray = 0x3,   // #555555
    White = 0x4,      // #FFFFFF
    Yellow = 0x5,     // #FFFF55
    Brown = 0x6,      // #AAAA00
    Blue = 0x7,       // #0055FF
    DarkBlue = 0x8,   // #0055AA
    Red = 0x9,        // #FF5500
    DarkRed = 0xA,    // #AA5500
    Cyan = 0xB,       // #1AFFFF
    DarkCyan = 0xC,   // #00AAAA
    LightGreen = 0xD, // #00FF00
    DarkGreen = 0xE,  // #00AA00
    BWBlinking = 0xF, // #000000 / #FFFFFF
};

} // namespace resl
