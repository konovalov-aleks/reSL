#include "melody.h"

#include <iostream>

namespace resl {

/* 19b2:0105 */
void playErrorMelody()
{
    // TODO implement
    std::cout << "🎶 error melody" << std::endl;
}

/* 19b2:024a */
void playFixRoadMelody()
{
    // TODO implement
    std::cout << "🎶 fix road melody" << std::endl;
}

/* 19b2:0047 */
void playSwitchSwitchedMelody()
{
    // TODO implement
    std::cout << "🎶 a switch has switched" << std::endl;
}

/* 19b2:000f */
void playEntitySwitchedSound(bool turnOn)
{
    // TODO implement
    std::cout << "🎶 an entity has switched (mode: " << turnOn << ')' << std::endl;
}

} // namespace resl
