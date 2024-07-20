#include "demo.h"

#include "game_data.h"
#include "io_status.h"
#include "main_menu.h"
#include "mouse/management_mode.h"
#include "mouse/mouse.h"
#include "mouse/mouse_mode.h"
#include "mouse/mouse_state.h"
#include "savefile/load_game.h"
#include "semaphore.h"
#include "switch.h"
#include <graphics/vga.h>
#include <system/random.h>
#include <system/time.h>
#include <tasks/message_queue.h>
#include <tasks/task.h>

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iterator>

namespace resl {

/* 1d7d:2114 : 14 bytes */
Task g_taskDemoAI;

//-----------------------------------------------------------------------------

/* 16a6:09f8 */
IOStatus loadDemo()
{
    /* 1d77:0000 : 16 bytes */
    static char g_demoFileName[16] = "demo_a";

    char origPlayerName[std::size(g_playerName)];
    std::strcpy(origPlayerName, g_playerName);

    IOStatus res = loadSavedGame(g_demoFileName);
    if (res != IOStatus::NoError) {
        g_demoFileName[5] = 'a';
        res = loadSavedGame(g_demoFileName);
    }

    if (g_demoFileName[5] == 'z')
        g_demoFileName[5] = 'a';
    else
        ++g_demoFileName[5];

    std::strcpy(g_playerName, origPlayerName);

    return res;
}

/* 16a6:0a7b */
void stopDemo()
{
    g_isDemoMode = false;
    stopTask(g_taskDemoAI);
}

/* 262d:21c0 : 6 bytes */
static MsgMouseEvent g_demoMouseMsg;

/* 1300:0273 */
static void moveCursorTowardsPoint(std::int16_t x, std::int16_t y)
{
    const mouse::MouseMode& m = *mouse::g_state.mode;
    g_demoMouseMsg.action = MouseAction::None;
    g_demoMouseMsg.cursorDX = (x - m.x) / 4;
    g_demoMouseMsg.cursorDY = (y - m.y) / 4;
    g_mouseMsgQueue.push(g_demoMouseMsg);
}

/* 1300:0200 */
static Task moveMouseCursor(std::int16_t x, std::int16_t y)
{
    for (std::int16_t i = 0; i <= 50; ++i) {
        assert(mouse::g_state.mode);
        const mouse::MouseMode& m = *mouse::g_state.mode;
        std::int16_t dx = std::abs(x - m.x);
        std::int16_t dy = std::abs(y - m.y);
        if (dx + dy <= 10) {
            g_demoMouseMsg.action = MouseAction::MouseClick;
            g_mouseMsgQueue.push(g_demoMouseMsg);
            co_return;
        }
        moveCursorTowardsPoint(x, y);
        co_await sleep(18);
    }
}

/* 1300:02b6 */
static void performMouseAction(MouseAction action)
{
    g_demoMouseMsg.action = action;
    g_mouseMsgQueue.push(g_demoMouseMsg);
}

/* 1300:0003 */
Task taskDemoAI()
{
    const TimeT startTime = getTime();
    for (;;) {
        if (getTime() - startTime > 6000) {
            returnToMainMenu();
            co_return;
        }
        std::int16_t time = getTime() & 7;
        if (mouse::g_state.mode == &mouse::g_modeManagement) {
            switch (time) {
            case 0:
                performMouseAction(MouseAction::ToggleMouseMode);
                break;

            case 1:
            case 2:
                break;

            case 3:
                if (g_semaphoreCount) {
                    const Semaphore& sem =
                        g_semaphores[genRandomNumber(g_semaphoreCount)];
                    co_await moveMouseCursor(sem.pixelX, sem.pixelY - 12);
                }
                break;

            default:
                if (g_nSwitches) {
                    const Switch& s = g_switches[genRandomNumber(g_nSwitches)];
                    co_await moveMouseCursor(s.x, s.y);
                }
                break;
            }
        } else {
            switch (time) {
            case 0:
            case 1: {
                std::int16_t x = genRandomNumber(SCREEN_WIDTH);
                // TODO make a constant for 287 (game field height)
                std::int16_t y = genRandomNumber(287);
                for (std::int16_t i = 0; i < 10; ++i) {
                    moveCursorTowardsPoint(x, y);
                    co_await sleep(15);
                }
            } break;

            case 2:
            case 3:
                performMouseAction(MouseAction::ToggleMouseMode);
                break;

            case 4:
            case 5: {
                performMouseAction(MouseAction::BuildRails);
                std::int16_t x = genRandomNumber(SCREEN_WIDTH);
                std::int16_t y = genRandomNumber(287) + 47;
                moveCursorTowardsPoint(x, y);
            } break;

            case 6:
            case 7:
                /* The original game has a dead code here (1300:0185):

                const RailInfo& ri = g_railRoad[genRandomNumber(g_railRoadCount)];
                const Rail& rail = g_rails[ri.tileX][ri.tileY][ri.railType];

                These variables are unused.

                In the construction mode, the pixel coordinates of the cursor
                are not stored  - only the tile position and relative
                coordinates within are stored.
                Probably, the authors of the original game wanted to choose a
                random location for a new road, but ran into trouble computing
                the current cursor position and left this code as is, unfinished.
                */
                performMouseAction(MouseAction::ToggleNextRailType);
                break;
            }
        }

        co_await sleep(100);
    }
}

} // namespace resl
