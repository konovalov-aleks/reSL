#include "vga.h"

#include "color.h"
#include <system/driver/driver.h>

#include <cassert>
#include <cstdint>

namespace resl::vga {

/* 1b06:026d */
void setVideoModeR0W0()
{
    Driver::instance().vga().setMode(0);
    Driver::instance().vga().setWriteMask(0xFF);
}

/* 1b06:0279 */
void setVideoModeR0W1()
{
    Driver::instance().vga().setMode(1);
    Driver::instance().vga().setWriteMask(0xFF);
}

/* 1b06:0285 */
void setVideoModeR0W2()
{
    Driver::instance().vga().setMode(2);
    Driver::instance().vga().setWriteMask(0xFF);
}

/* 1b06:02ec */
void setPaletteItem(Color c, std::uint32_t rgb)
{
    Driver::instance().vga().setPaletteItem(static_cast<std::uint8_t>(c), rgb);
}

/* 1b06:02a5 */
void setDataRotation(std::uint8_t rotation)
{
    assert((rotation & 7) == 0); // not implemented yet
    rotation >>= 3;
    assert(rotation < 4);
    Driver::instance().vga().setWriteOperation(static_cast<WriteOperation>(rotation));
}

/* 1b06:02b5 */
void videoChoosePlanes(std::uint8_t mask)
{
    Driver::instance().vga().setPlaneMask(mask);
}

/* 1b06:03fa */
void waitVerticalRetrace()
{
    Driver::instance().vga().waitVerticalRetrace();
}

/* 17a7:0106 */
void waitForNRetraces(std::int16_t n)
{
    for (int16_t i = 0; i < n; ++i)
        waitVerticalRetrace();
}

} // namespace resl::vga
