#include "main_menu.h"

#include "components/dialog.h"
#include "components/draw_header.h"
#include "components/status_bar.h"
#include "manual.h"
#include <game/constants.h>
#include <game/demo.h>
#include <game/drawing.h>
#include <game/init.h>
#include <game/io_status.h>
#include <game/main_loop.h>
#include <game/melody.h>
#include <game/mouse/construction_mode.h>
#include <game/mouse/management_mode.h>
#include <game/mouse/state.h>
#include <game/player_name.h>
#include <game/records.h>
#include <game/savefile/load_game.h>
#include <game/static_object.h>
#include <game/train.h>
#include <graphics/animation.h>
#include <graphics/drawing.h>
#include <graphics/vga.h>
#include <system/buffer.h>
#include <system/exit.h>
#include <system/filesystem.h>
#include <system/keyboard.h>
#include <tasks/task.h>
#include <ui/components/button.h>
#include <ui/components/menu_button.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>

namespace resl {

/* 1d7d:2046 : 48 bytes*/
static const char g_monthNames[12][4] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/* 132d:03ea */
static void eraseArchiveMenu(std::int16_t yOffset)
{
    std::int16_t dstY = 47;
    std::int16_t srcY = 397;
    if (yOffset) {
        dstY = 397;
        srcY = 47;
    }
    graphics::copyRectangle(0, dstY, 0, srcY, 80, GAME_FIELD_HEIGHT);
    vga::setVideoModeR0W2();
}

enum class ArchiveMenuAction {
    ReturnToMainMenu,
    StartGame
};

/* 15e8:063e */
inline ArchiveMenuAction showArchiveMenu()
{
    for (bool showNextFile = true; showNextFile;) {
        showNextFile = false;

        if (int err = findNextSaveFile(); err) [[unlikely]] {
            alert("Can't open game files in current dir");
            eraseArchiveMenu(0);
            return ArchiveMenuAction::ReturnToMainMenu;
        }

        FileInfo file = lastSearchResult();
        g_lastKeyPressed = 0;

        IOStatus status = loadSavedGame(file.fileName);
        if (status != IOStatus::NoError) {
            alert("Load Error");
            continue;
        }
        char buf[80];
        // Date and time format:
        // https://www.stanislavs.org/helppc/file_attributes.html
        int year = (file.fileDate >> 9) + 80; // year since 1980
        if (year > 100)
            year -= 100;
        const std::string fileName =
            std::filesystem::path(file.fileName).filename().string();
        std::snprintf(buf, sizeof(buf),
                      "Player: %-18s Date: %02d-%3s-%02d  Time: %02d:%02d  File: %-s",
                      g_playerName,
                      file.fileDate & 0x1F,                           // day
                      g_monthNames[((file.fileDate >> 5) & 0xF) - 1], // month
                      year,
                      (file.fileTime >> 11) & 0x1F, // hours
                      (file.fileTime >> 5) & 0x3F,  // minutes
                      fileName.c_str());
        drawWorld();

        while (!showNextFile) {
            drawDialog(DialogType::Archive, 350);
            graphics::setVideoFrameOrigin(0, 350);
            graphics::copyScreenBufferTo(0);
            showStatusMessage(buf);
            graphics::setVideoFrameOrigin(0, 0);
            drawWorld();

            bool needRedrawDialog = false;
            while (!showNextFile && !needRedrawDialog) {
                switch (handleDialog(DialogType::Archive)) {
                case -1:
                    // Timeout
                    break;

                case 0:
                    /* 15e8:0785 */
                    // [V]iew
                    {
                        graphics::setVideoFrameOrigin(0, 350);
                        const Dialog& dialog = g_dialogs[static_cast<int>(DialogType::Archive)];
                        std::int16_t itemY = dialog.itemY[0];
                        if (itemY > 350)
                            itemY -= 350;
                        toggleButtonState(dialog.x, itemY);
                        // wait until the button is released
                        while (!(g_lastKeyCode & g_keyReleasedFlag)) {
                            // The original game uses busy-loop here (without sleep)
                            // I'm not so cruel :D
                            vga::waitVerticalRetrace();
                        }
                        graphics::setVideoFrameOrigin(0, 0);
                        g_lastKeyPressed = 0;
                    }
                    break;

                case 1:
                    /* 15e8:063e */
                    // [N]ext

                    // It looks like the original game uses just "goto" to the
                    // beginning of the function. But this is too ugly.
                    showNextFile = true;
                    break;

                case 2:
                    /* 15e8:07d6 */
                    // [G]o
                    graphics::setVideoFrameOrigin(0, 350);
                    eraseArchiveMenu(0);
                    graphics::setVideoFrameOrigin(0, 0);
                    fillGameFieldBackground(350);
                    drawFieldBackground(350);
                    mouse::setMode(mouse::g_modeManagement);
                    spawnNewTrain();
                    return ArchiveMenuAction::StartGame;

                case 3:
                    /* 15e8:0823 */
                    // [D] Delete
                    drawDialog(DialogType::Confirmation, 0);
                    g_lastKeyPressed = 0;
                    if (handleDialog(DialogType::Confirmation, 1) == 0) {
                        // Yes
                        std::remove(file.fileName);
                        showNextFile = true;
                    } else {
                        // No
                        needRedrawDialog = true;
                    }
                    break;

                case 4:
                    /* 15e8:085c */
                    // [B]ye
                    createNewWorld();
                    drawGameField(350);
                    setHeaderValues(0, 100, 1800, readLevel(), 350);
                    drawDialog(DialogType::MainMenu, 350);
                    graphics::setVideoFrameOrigin(0, 350);
                    graphics::copyScreenBufferTo(0);
                    graphics::setVideoFrameOrigin(0, 0);
                    return ArchiveMenuAction::ReturnToMainMenu;

                default:
                    // unreachable
                    std::abort();
                }
            }
        }
    }
    return ArchiveMenuAction::ReturnToMainMenu;
}

/* 15e8:04c3 */
void mainMenu()
{
    for (;;) {
        g_lastKeyPressed = 0;
        switch (handleDialog(DialogType::MainMenu)) {
        case 0:
            /* 15e8:04ea */
            // [M] Manual
            showManual();
            readBinaryFile("play.7", g_pageBuffer);
            drawGameField(350);
            setHeaderValues(0, 100, 1800, readLevel(), 350);
            drawDialog(DialogType::MainMenu, 350);
            graphics::animateScreenShifting();
            graphics::setVideoFrameOrigin(0, 350);
            graphics::copyScreenBufferTo(0);
            graphics::setVideoFrameOrigin(0, 0);
            break;

        case -1:
            /* 15e8:0575 */
            // No item selected - dialog was closed due to timeout.
            // In this case, run the demo.
            toggleButtonState(g_dialogs[static_cast<int>(DialogType::MainMenu)].x,
                              g_dialogs[static_cast<int>(DialogType::MainMenu)].itemY[1]);
            playEntitySwitchedSound(false);
            vga::waitForNRetraces(8);
            [[fallthrough]];

        case 1:
            /* 15e8:0590 */
            // [D]emo
            if (loadDemo() == IOStatus::NoError) {
                g_isDemoMode = true;
                drawWorld();
                MenuButton::draw(350);
                graphics::animateScreenShifting();
                graphics::copyScreenBufferTo(0);
                graphics::setVideoFrameOrigin(0, 0);
                fillGameFieldBackground(350);
                drawFieldBackground(350);
                mouse::setMode(mouse::g_modeManagement);
                return;
            }
            break;

        case 2:
            /* 15e8:05e8 */
            // [G]o!
            drawGameField(350);
            setHeaderValues(0, 100, 1800, readLevel(), 350);
            graphics::setVideoFrameOrigin(0, 350);
            graphics::copyScreenBufferTo(0);
            graphics::setVideoFrameOrigin(0, 0);
            mouse::setMode(mouse::g_modeConstruction);
            return;

        case 3:
            /* 15e8:063e */
            // [A] Archive
            if (showArchiveMenu() == ArchiveMenuAction::StartGame)
                return;
            break;

        case 4: {
            /* 15e8:08c7 */
            // [R] Records
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
            toggleButtonState(mainMenu.x, itemY);
            graphics::setVideoFrameOrigin(0, 0);
            break;
        }

        case 5:
            /* 15e8:0913 */
            // [B]ye
            exitWithMessage("Bye\n");

        default:
            // unreachable
            std::abort();
        };
    }
}

/* 132d:00b6 */
void drawMainMenuBackground(std::int16_t yOffset)
{
    readBinaryFile("play.7", g_pageBuffer);
    graphics::imageDot7(0, yOffset, SCREEN_WIDTH, SCREEN_HEIGHT, g_pageBuffer);
    drawStaticObjects(yOffset);
    drawCopyright(yOffset);
}

/* 16a6:065f */
void returnToMainMenu()
{
    vga::waitForNRetraces(40);
    [[maybe_unused]] bool ok = stopTask(g_taskGameMainLoop);
    assert(ok);
    g_taskGameMainLoop = addTask(taskGameMainLoop());
}

} // namespace resl
