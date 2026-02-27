#pragma once

#include <graphics/vga.h>

#include <cstdint>

namespace resl {

// Width in pixels of the game field
constexpr std::int16_t GAME_FIELD_WIDTH = LOGICAL_SCREEN_WIDTH;

// Height in pixels of the game field
// (screen area excluding status bar and header)
constexpr std::int16_t GAME_FIELD_HEIGHT = 287;

} // namespace resl
