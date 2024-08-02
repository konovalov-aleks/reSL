#pragma once

#include <cstdint>

namespace resl {

int buttonHeight() noexcept;
int buttonWidth() noexcept;

void drawButton(std::int16_t x, std::int16_t y, const char* caption);

/* 15e8:047b */
void toggleButtonState(std::int16_t x, std::int16_t y);

} // namespace resl
