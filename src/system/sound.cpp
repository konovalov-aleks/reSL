#include "sound.h"

#include <cstdint>

namespace resl {

/* The original game uses sound/nosound functions from the standard "dos.h"
   TurboC header.
   These functions are not portable - their implementation is based on
   reading/writing PC Speaker I/O ports (0x61, 0x43, 0x42):
        https://wiki.osdev.org/PC_Speaker

   reSL uses portable SDL2 API instead.
*/

/* 1000:1a87 */
void sound(std::uint16_t frequency)
{
    // TODO implement
}

/* 1000:1ab3 */
void nosound()
{
    // TODO implement
}

} // namespace resl
