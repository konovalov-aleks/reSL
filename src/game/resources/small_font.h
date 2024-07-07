#pragma once

#include <cstdint>

namespace resl {

// The original game uses a font from the VGA memory.
// This data was extracted by the "extract_vga_font" utility that is based on
// the code of the original game, which is responsible for searching for fonts
// in video memory.
/* C000:0900 : 3584 bytes */
extern const std::uint8_t g_font14Data[];

} // namespace resl
