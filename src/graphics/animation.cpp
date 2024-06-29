#include "animation.h"

#include "vga.h"
#include <system/driver/driver.h>

#include <cstdint>

namespace resl::graphics {

/* 1b06:04fd */
void shiftScreen(std::int16_t yOffset)
{
    vga::waitVerticalRetrace();
    Driver::instance().vga().setLineCompareRegister(yOffset);
}

/* 1b06:042f */
void setVideoFrameOrigin(std::int16_t x, std::int16_t y)
{
    vga::waitVerticalRetrace();
    Driver::instance().vga().setFrameOrigin(x, y);
    vga::waitVerticalRetrace();
}

/* 17a7:00cd */
void animateScreenShifting()
{
    shiftScreen(1);
    setVideoFrameOrigin(0, 350);
    for (std::int16_t i = 0; i < 371; i += (i >> 4) + 2)
        shiftScreen(i);
}

} // namespace resl::graphics
