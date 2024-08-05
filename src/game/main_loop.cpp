#include "main_loop.h"

#include "demo.h"
#include "drawing.h"
#include "entrance.h"
#include "header.h"
#include "header_field.h"
#include "init.h"
#include "io_status.h"
#include "melody.h"
#include "mouse/construction_mode.h"
#include "mouse/management_mode.h"
#include "mouse/mode.h"
#include "mouse/state.h"
#include "rail.h"
#include "rail_info.h"
#include "records.h"
#include "resources/entrance_rails.h"
#include "savefile/save_game.h"
#include "static_object.h"
#include "train.h"
#include <graphics/animation.h>
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/vga.h>
#include <system/driver/driver.h>
#include <system/keyboard.h>
#include <system/time.h>
#include <tasks/task.h>
#include <types/rectangle.h>
#include <ui/components/dialog.h>
#include <ui/components/draw_header.h>
#include <ui/components/status_bar.h>
#include <ui/game_over.h>
#include <ui/loading_screen.h>
#include <ui/main_menu.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>

namespace resl {

/* 1d7d:20ce */
Task g_taskGameMainLoop;

// Time elapsed since the beginning of the current game year.
// One game year corresponds to the value 2000 of this counter.
/* 262d:6f50 : 2 bytes */
static std::int16_t g_gameTime = 0;

//-----------------------------------------------------------------------------

/* 16a6:0750 */
static std::int16_t incrementGameTime(std::int16_t delta)
{
    g_gameTime += delta;
    if (g_gameTime < 2000)
        return 0;

    g_gameTime -= 2000;
    return g_headers[static_cast<int>(HeaderFieldId::Year)].value;
}

/* 15e8:0bbd */
static void pauseMenuShowSaveItem()
{
    g_dialogs[static_cast<int>(DialogType::Pause)].itemNames[1] = "Save";
}

/* 15e8:0bcf */
static void pauseMenuShowOkItem()
{
    g_dialogs[static_cast<int>(DialogType::Pause)].itemNames[1] = "  OK";
}

enum class PauseMenuAction {
    ContinueGame,
    ReturnToMainMenu
};

/* 16a6:0154 */
inline PauseMenuAction showPauseMenu()
{
    pauseMenuShowSaveItem();

    Rectangle dialogRect;
    bool needInit = true;
    for (;;) {
        mouse::g_state.mode->clearFn();
        if (needInit) {
            dialogRect = drawDialog(DialogType::Pause, 0);
            writeRecords();
            needInit = false;
        }

        /* 16a6:016f */
        g_lastKeyPressed = 0;
        std::int16_t item = handleDialog(DialogType::Pause);
        g_lastKeyPressed = 0;
        switch (item) {
        case -1:
            // Timeout
            break;

        case 0:
            /* 16a6:0195 */
            // [G]o
            vga::setVideoModeR0W1();
            graphics::copyFromShadowBuffer(dialogRect);
            vga::setVideoModeR0W2();
            scheduleAllTrainsRedrawing();
            mouse::g_state.mode->drawFn();
            enableTimer();
            spawnNewTrain();
            Driver::instance().mouse().setCursorVisibility(
                mouse::g_state.mode == &mouse::g_modeManagement);
            return PauseMenuAction::ContinueGame;

        case 1:
            /* 16a6:01b9 */
            // [S]ave
            if (saveGame() == IOStatus::NoError)
                pauseMenuShowOkItem();
            else
                alert("Error");
            needInit = true;
            break;

        case 2:
            /* 16a6:01d8 */
            // [B]ye
            enableTimer();
            return PauseMenuAction::ReturnToMainMenu;

        default:
            // unreachable
            std::abort();
        }
    }
}

/* 16a6:0001 */
Task taskGameMainLoop()
{
    g_gameTime = 0;

    // The original game uses uint8 here - VGA color code.
    // They alternate between 0 and 0x3F.
    std::uint32_t blinkingColor = 0xFFFFFF;

    bool isOddIter = false;

    stopTask(g_taskDemoAI);

    graphics::setVideoFrameOrigin(0, 0);
    graphics::shiftScreen(511);

    if (g_isDemoMode || !g_gameOver)
        showLoadingScreen();

    g_gameOver = false;

    for (;;) {
        /* 16a6:004c */

        disableTimer();

        createNewWorld();
        drawMainMenuBackground(350);

        setHeaderValues(0, 100, 1800, readLevel(), 350);
        drawDialog(DialogType::MainMenu, 350);

        graphics::animateScreenShifting();
        graphics::copyScreenBufferTo(0);
        graphics::setVideoFrameOrigin(0, 0);

        g_isDemoMode = false;

        mainMenu();

        if (g_isDemoMode) {
            stopTask(g_taskDemoAI);
            g_taskDemoAI = addTask(taskDemoAI());
        }

        bool needReturnToMainMenu = false;
        while (!needReturnToMainMenu) {
            resetTasks();

            g_gameTime = 0;
            mouse::g_state.mode->drawFn();
            if (g_headers[static_cast<int>(HeaderFieldId::Year)].value == 1800) {
                buildHouses(1800);
                if (g_entranceCount == 0)
                    spawnNewEntrance(g_entranceRails[g_entrances[0].entranceRailInfoIdx]);
            }
            g_lastKeyPressed = 0;
            enableTimer();

            bool needRestartGame = false;
            while (!needRestartGame) {
                /* 16a6:0126 */
                co_await sleep(50);

                if (g_lastKeyPressed) {
                    disableTimer();
                    if (g_isDemoMode) {
                        stopDemo();
                        enableTimer();
                        needReturnToMainMenu = true;
                        break;
                    } else {
                        PauseMenuAction menuRes = showPauseMenu();
                        if (menuRes == PauseMenuAction::ReturnToMainMenu) {
                            needReturnToMainMenu = true;
                            break;
                        }
                        continue;
                    }
                }

                updateStatusBar();
                accelerateTrains(5);
                vga::waitVerticalRetrace();

                blinkingColor ^= 0xFFFFFF;
                vga::setPaletteItem(Color::BWBlinking, blinkingColor);

                isOddIter = !isOddIter;
                if (!isOddIter) {
                    const std::int16_t probability =
                        (g_headers[static_cast<int>(HeaderFieldId::Level)].value / 2) * g_railRoadCount;
                    if ((rand() & 0xFFFF) < probability)
                        randomRailDamage();
                    continue;
                }

                tryRunWaitingTrains();

                if (mouse::g_state.mode == &mouse::g_modeConstruction)
                    mouse::g_state.mode->drawFn();

                std::int16_t newYear = incrementGameTime(100);
                if (!newYear)
                    continue;

                /* 16a6:024e */
                startHeaderFieldAnimation(HeaderFieldId::Year, 1);
                if (newYear & 1)
                    startHeaderFieldAnimation(HeaderFieldId::Money, -1);

                const std::int16_t waitingTrains = waitingTrainsCount();
                if (waitingTrains) {
                    char buf[60];
                    std::snprintf(buf, sizeof(buf), "%d,000 penalty for waiting trains",
                                  static_cast<int>(waitingTrains));
                    showStatusMessage(buf);
                    startHeaderFieldAnimation(HeaderFieldId::Money, -waitingTrains);
                }

                buildHouses(g_headers[static_cast<int>(HeaderFieldId::Year)].value + 1);
                scheduleAllTrainsRedrawing();

                switch (g_headers[static_cast<int>(HeaderFieldId::Year)].value) {
                case 1800:
                    if (g_entranceCount == 1)
                        spawnNewEntrance(g_entranceRails[g_entrances[1].entranceRailInfoIdx]);
                    break;
                case 1840:
                    if (g_entranceCount == 2)
                        spawnNewEntrance(g_entranceRails[g_entrances[2].entranceRailInfoIdx]);
                    break;
                case 1880:
                    if (g_entranceCount == 3)
                        spawnNewEntrance(g_entranceRails[g_entrances[3].entranceRailInfoIdx]);
                    break;
                case 1920:
                    if (g_entranceCount == 4)
                        spawnNewEntrance(g_entranceRails[g_entrances[4].entranceRailInfoIdx]);
                    break;
                case 1960:
                    if (g_entranceCount == 5)
                        spawnNewEntrance(g_entranceRails[g_entrances[5].entranceRailInfoIdx]);
                    break;
                case 2000:
                    /* 16a6:03d6 */
                    playHappy2000YearMelody();
                    if (!g_isDemoMode)
                        writeRecords();
                    createNewWorld();
                    alert("Happy New 2000 Year!");
                    g_headers[static_cast<int>(HeaderFieldId::Year)].value = 1800;
                    g_headers[static_cast<int>(HeaderFieldId::Level)].value++;
                    g_headers[static_cast<int>(HeaderFieldId::Money)].value += 10;
                    drawWorld();
                    graphics::animateScreenShifting();
                    graphics::copyScreenBufferTo(0);
                    graphics::setVideoFrameOrigin(0, 0);
                    needRestartGame = true;
                    break;
                };
            }
        }
    }
}

} // namespace resl
