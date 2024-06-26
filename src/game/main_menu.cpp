#include "main_menu.h"

#include "dialog.h"
#include "draw_header.h"
#include "drawing.h"
#include "keyboard.h"
#include "melody.h"
#include "static_object.h"
#include "status_bar.h"
#include <graphics/drawing.h>
#include <graphics/vga.h>
#include <system/buffer.h>
#include <system/read_file.h>

#include <cstdint>
#include <iostream>

namespace resl {

/* 15e8:04c3 */
void mainMenu()
{
    g_lastKeyPressed = 0;
    switch (handleDialog(DialogType::MainMenu)) {
    case 0:
        /* 15e8:04ea */
        // [M]anual
        // TODO implement
        std::cout << "[M]anual" << std::endl;
        break;

    case -1:
        /* 15e8:0575 */
        // No item selected - dialog was closed due to timeout.
        // In this case, run the demo.
        highlightFirstDlgItemSymbol(g_dialogs[static_cast<int>(DialogType::MainMenu)].x,
                                    g_dialogs[static_cast<int>(DialogType::MainMenu)].itemY[1]);
        playEntitySwitchedSound(false);
        vga::waitForNRetraces(8);
        [[fallthrough]];

    case 1:
        /* 15e8:0590 */
        // [D]emo
        // TODO implement
        break;

    case 2: {
        /* 15e8:05e8 */
        // [G]o!
        drawGameField(350);
        // TODO
        std::int16_t level = 1; // readLevel();
        drawHeaderData(0, 100, 1800, level, 350);
        // TODO implement
        return;
    }
    };
    // TODO implement
}

/* 132d:00b6 */
void drawMainMenuBackground(std::int16_t yOffset)
{
    // The file name in the original game is lowercase:
    //  1d7d:0199 "play.7"
    // But DOS filesystem is case-insensitive => the fact that the file name on disk
    // and name in the code are in different case is not a problem there.
    // For portability, I use a name identical to the file name on disk.
    readFromFile("PLAY.7", g_pageBuffer);
    drawing::imageDot7(0, yOffset, SCREEN_WIDTH, SCREEN_HEIGHT, g_pageBuffer);
    drawStaticObjects(yOffset);
    drawCopyright(yOffset);
}

} // namespace resl
