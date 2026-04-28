#include "game_over.h"

#include "main_menu.h"
#include <game/demo.h>
#include <game/melody.h>
#include <game/records.h>
#include <graphics/drawing.h>
#include <system/filesystem.h>

#include <iostream>

namespace resl {

/* 262d:6f4f : 1 byte */
bool g_gameOver = false;

//-----------------------------------------------------------------------------

/* 16a6:0615 */
void gameOver()
{
    // In the original game, this file name is in lower case "gameover.7",
    // but the file in filesystem has an uppercase name "GAMEOVER.7".
    // This is not a problem for them, because DOS has a case-insensitive file system.
    // We use the identical names for better portability.
    std::span<std::byte> fileData = readBinaryFile("GAMEOVER.7");
    if (fileData.empty()) [[unlikely]]
        std::cerr << "unable to read file 'GAMEOVER.7'" << std::endl;
    else {
        graphics::imageDot7(184, 58, 284, 263,
                            reinterpret_cast<std::uint8_t*>(fileData.data()));
    }
    playGameOverMelody();

    if (!g_isDemoMode)
        writeRecords();

    g_gameOver = true;
    returnToMainMenu();
}

} // namespace resl
