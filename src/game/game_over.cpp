#include "game_over.h"

#include "game_data.h"
#include "main_menu.h"
#include "melody.h"
#include "records.h"
#include <graphics/drawing.h>
#include <system/buffer.h>
#include <system/filesystem.h>

namespace resl {

/* 16a6:0615 */
void gameOver()
{
    // In the original game, this file name is in lower case "gameover.7",
    // but the file in filesystem has an uppercase name "GAMEOVER.7".
    // This is not a problem for them, because DOS has a case-insensitive file system.
    // We use the identical names for better portability.
    readBinaryFile("GAMEOVER.7", g_pageBuffer);
    graphics::imageDot7(184, 58, 284, 263, g_pageBuffer);
    playGameOverMelody();

    if (!g_isDemoMode)
        writeRecords();

    g_gameOver = true;
    returnToMainMenu();
}

} // namespace resl
