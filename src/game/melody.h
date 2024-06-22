#pragma once

#include <cstdint>

namespace resl {

/* 1d7d:02c2 : 1 byte */
extern bool g_soundEnabled;

//-----------------------------------------------------------------------------

/* 19b2:0105 */
void playErrorMelody();

/* 19b2:024a */
void playFixRoadMelody();

/* 19b2:0047 */
void playSwitchSwitchedMelody();

/* 19b2:000f */
void playEntitySwitchedSound(bool turnOn);

/* 19b2:0128 */
void playScheduledTrainMelody(std::uint16_t freq1, std::uint16_t freq2, std::int16_t iterations);

/* 19b2:0077 */
void playTrainFinishedMelody(std::int16_t trainLen);

/* 19b2:0167 */
void beepSound(std::int16_t tone);

/* 19b2:018d */
void playSpawnedEntranceMelody();

} // namespace resl
