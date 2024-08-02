#pragma once

#include "types/rectangle.h"

#include <cstdint>
#include <optional>

namespace resl {

enum class DialogType {
    MainMenu,
    Archive,
    Confirmation,
    Pause,
    Alert
};

struct Dialog {
    const char* title;
    const char* itemNames[7];
    std::int16_t x = 0;
    std::int16_t itemY[7] = {};
    std::int16_t itemWidth = 0;
};

//-----------------------------------------------------------------------------

/* 1d65:0000 : 240 bytes */
extern Dialog g_dialogs[5];

//-----------------------------------------------------------------------------

/* 15e8:047b */
void highlightFirstDlgItemSymbol(std::int16_t x, std::int16_t y);

/* 15e8:0003 */
Rectangle& drawDialog(DialogType, std::int16_t yOffset);

/* 15e8:03b7 */
std::int16_t handleDialog(DialogType, std::optional<std::int16_t> defaultChoice = std::nullopt);

/* 15e8:0947 */
void alert(const char*);

} // namespace resl
