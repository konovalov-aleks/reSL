#include "main_menu.h"

#include "dialog.h"
#include "draw_header.h"
#include "drawing.h"
#include "keyboard.h"
#include "load_game.h"
#include "melody.h"
#include "mouse/construction_mode.h"
#include "mouse/mouse_state.h"
#include "records.h"
#include "static_object.h"
#include "status_bar.h"
#include <graphics/animation.h>
#include <graphics/drawing.h>
#include <graphics/vga.h>
#include <system/buffer.h>
#include <system/exit.h>
#include <system/read_file.h>

#include <cstdint>
#include <iostream>

namespace resl {

/* 15e8:04c3 */
void mainMenu()
{
    for (;;) {
        g_lastKeyPressed = 0;
        switch (handleDialog(DialogType::MainMenu)) {
        case 0:
            /* 15e8:04ea */
            // [M]anual
            // TODO implement
            std::cout << "Sorry, manual is not implemented yet" << std::endl;
            playErrorMelody();
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
            std::cout << "Sorry, demo is not implemented yet" << std::endl;
            playErrorMelody();
            break;

        case 2: {
            /* 15e8:05e8 */
            // [G]o!
            drawGameField(350);
            std::int16_t level = readLevel();
            drawHeaderData(0, 100, 1800, level, 350);
            graphics::setVideoFrameOrigin(0, 350);
            graphics::flushScreenBuffer(0);
            graphics::setVideoFrameOrigin(0, 0);
            mouse::g_state.mode = &mouse::g_modeConstruction;
            return;
        }

        case 3:
            /* 15e8:063e */
            // [A]rchive
            // TODO implement
            std::cout << "Sorry, archive is not implemented yet" << std::endl;
            playErrorMelody();
            break;

        case 4: {
            /* 15e8:08c7 */
            // [R]ecords
            showRecordsScreen();
            graphics::animateScreenShifting();
            g_lastKeyPressed = 0;
            while (g_lastKeyPressed == 0) {
                // The original game has computation-intense loop here
                // (without waitVerticalRetrace call).
                vga::waitVerticalRetrace();
            }
            const Dialog& mainMenu = g_dialogs[static_cast<int>(DialogType::MainMenu)];
            std::int16_t itemY = mainMenu.itemY[4];
            if (itemY > 350)
                itemY -= 350;
            highlightFirstDlgItemSymbol(mainMenu.x, itemY);
            graphics::setVideoFrameOrigin(0, 0);
            break;
        }

        case 5:
            /* 15e8:0913 */
            // [B]ye
            exitWithMessage("Bye\n");

        default:
            playErrorMelody();
            break;
        };
        // TODO implement
    }
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
    graphics::imageDot7(0, yOffset, SCREEN_WIDTH, SCREEN_HEIGHT, g_pageBuffer);
    drawStaticObjects(yOffset);
    drawCopyright(yOffset);
}

} // namespace resl
