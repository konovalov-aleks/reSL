#include "melody.h"

#include <graphics/vga.h>
#include <system/sound.h>

namespace resl {

/* 1d7d:02c2 : 1 byte */
bool g_soundEnabled = true;

//-----------------------------------------------------------------------------

/* 19b2:0105 */
void playErrorMelody()
{
    if (!g_soundEnabled)
        return;

    sound(50);
    vga::waitForNRetraces(7);
    nosound();
}

/* 19b2:024a */
void playFixRoadMelody()
{
    if (!g_soundEnabled)
        return;

    for (std::int16_t i = 12; i >= 0; --i) {
        sound(i * 60 + 20);
        vga::waitVerticalRetrace();
    }
    nosound();
}

/* 19b2:0047 */
void playSwitchSwitchedMelody()
{
    if (!g_soundEnabled)
        return;

    for (std::int16_t i = 0; i < 3; ++i) {
        sound(i * 300 + 200);
        vga::waitVerticalRetrace();
    }
    nosound();
}

/* 19b2:000f */
void playEntitySwitchedSound(bool turnOn)
{
    if (!g_soundEnabled)
        return;

    std::int16_t freq = turnOn ? 3500 : 5000;
    sound(freq);
    for (int i = 0; i < 2; ++i)
        vga::waitVerticalRetrace();
    nosound();
}

/* 19b2:0128 */
void playScheduledTrainMelody(std::uint16_t freq1, std::uint16_t freq2, std::int16_t iterations)
{
    if (!g_soundEnabled)
        return;

    for (std::int16_t i = 0; i < iterations; ++i) {
        sound(freq1);
        vga::waitVerticalRetrace();
        sound(freq2);
        vga::waitVerticalRetrace();
    }
    nosound();
}

/* 19b2:0077 */
void playTrainFinishedMelody(std::int16_t trainLen)
{
    if (!g_soundEnabled)
        return;

    for (std::int16_t i = 0; i < trainLen; ++i) {
        sound(i * 200 + 1000);
        vga::waitVerticalRetrace();
        vga::waitVerticalRetrace();
    }
    nosound();
}

/* 19b2:0167 */
void beepSound(std::int16_t tone)
{
    if (!g_soundEnabled)
        return;

    sound((tone + 1) * 128);
    vga::waitVerticalRetrace();
    nosound();
}

/* 19b2:018d */
void playSpawnedEntranceMelody()
{
    for (std::int16_t tone = 7; tone >= 0; --tone) {
        beepSound(tone);
        vga::waitVerticalRetrace();
    }
}

/* 19b2:01e1 */
void playGameOverMelody()
{
    if (!g_soundEnabled)
        return;

    for (std::int16_t i = 20; i; --i) {
        sound(i * 100);
        for (std::int16_t j = 0; j < 5; ++j)
            vga::waitVerticalRetrace();
    }
    nosound();
}

/* 19b2:021a */
void playRailDamagedMelody()
{
    if (!g_soundEnabled)
        return;

    for (std::int16_t i = 0; i < 12; ++i) {
        sound(i * 60 + 20);
        vga::waitVerticalRetrace();
    }
    nosound();
}

} // namespace resl
