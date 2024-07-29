#include "mouse.h"

#include "construction_mode.h"
#include "management_mode.h"
#include "mouse_mode.h"
#include "mouse_state.h"
#include <game/drawing.h>
#include <game/entrance.h>
#include <game/header.h>
#include <game/melody.h>
#include <game/rail.h>
#include <game/resources/allowed_cursor_rail_types.h>
#include <game/road_construction.h>
#include <game/semaphore.h>
#include <game/status_bar.h>
#include <game/switch.h>
#include <game/train.h>
#include <game/types/header_field.h>
#include <game/types/rail_info.h>
#include <graphics/color.h>
#include <system/mouse.h>
#include <tasks/message_queue.h>
#include <tasks/task.h>

#include <cassert>
#include <cstdint>

namespace resl {

using namespace mouse; // FIXME

/* 262d:6ef8 : 1 byte */
static std::uint8_t g_previousMouseButtonState = 0;

/* 1d7d:1c96 */
MessageQueue<MsgMouseEvent> g_mouseMsgQueue;

/* 14af:0761 */
void handleMouseInput(std::uint16_t mouseEventFlags,
                      std::uint16_t mouseButtonState,
                      std::int16_t x, std::int16_t y)
{
    // the original game uses a global vairable here but we have a different
    // coroutines implementation, and it makes sense to use local variable instead.
    MsgMouseEvent msg = { MouseAction::None, x, y };

    if (g_previousMouseButtonState)
        g_previousMouseButtonState = static_cast<std::uint8_t>(mouseButtonState);
    else {
        // TODO make an enum for mouseButtonState
        if ((mouseEventFlags & (ME_LEFTPRESSED | ME_RIGHTPRESSED)) &&
            mouseButtonState == (MouseButton::MB_LEFT | MouseButton::MB_RIGHT)) {

            msg.action = MouseAction::ToggleMouseMode;
            g_previousMouseButtonState = MouseButton::MB_LEFT;
        } else {
            if (mouseEventFlags & ME_LEFTRELEASED) {
                // left button clicked
                if (g_state.mode == &g_modeManagement)
                    msg.action = MouseAction::MouseClick;
                else
                    msg.action = MouseAction::ToggleNextRailType;
            } else {
                if (mouseEventFlags & ME_RIGHTRELEASED) {
                    // right button clicked
                    if (g_state.mode == &g_modeManagement)
                        msg.action = MouseAction::CallServer;
                    else
                        msg.action = MouseAction::BuildRails;
                }
            }
        }
    }
    g_mouseMsgQueue.push(msg);
}

/* 14af:0320 */
Task taskMouseEventHandling()
{
    for (;;) {
        MsgMouseEvent e = co_await g_mouseMsgQueue.pop();

        assert(g_state.mode);
        MouseMode& mode = *g_state.mode;
        mode.updatePosFn(e.x, e.y);

        switch (e.action) {
        case MouseAction::CallServer:
            /* 14af:05e7 */
            {
                Entrance* entrance = findClosestEntrance(mode.x, mode.y);
                if (!entrance) {
                    showStatusMessage("Click CLOSER TO ENTRANCE to call server");
                    playErrorMelody();
                } else if (g_headers[static_cast<int>(HeaderFieldId::Money)].value < 2) {
                    showStatusMessage("No money to pay for server call");
                    playErrorMelody();
                } else {
                    const std::int16_t entranceIdx =
                        static_cast<std::int16_t>(entrance - &g_entrances[0]);
                    if (!entranceIsFree(entranceIdx)) {
                        showStatusMessage("Entrance is locked by train");
                        playErrorMelody();
                    } else {
                        if (spawnServer(entranceIdx))
                            spendMoney(2);
                        else {
                            showStatusMessage("Railnet OVERFLOW. Can't call the server");
                            playErrorMelody();
                        }
                    }
                }
            }
            break;

        case MouseAction::MouseClick:
            /* 14af:0364 */
            {
                if (Switch* sw = findClosestSwitch(mode.x, mode.y)) {
                    if (switchIsBusy(*sw)) {
                        showStatusMessage("Switch is locked by train");
                        playErrorMelody();
                    } else {
                        eraseArrowCursor();
                        const std::int16_t switchIdx = static_cast<std::int16_t>(sw - g_switches);
                        eraseSwitch(switchIdx);
                        toggleSwitch(*sw);
                        drawSwitch(switchIdx, true);
                        drawArrowCursor();
                        scheduleAllTrainsRedrawing();
                        playSwitchSwitchedMelody();
                    }
                } else if (Semaphore* sem = findClosestSemaphore(mode.x, mode.y)) {
                    eraseArrowCursor();
                    toggleSemaphore(*sem);
                    drawSemaphore(*sem, 0);
                    drawSemaphore(*sem, 350);
                    drawArrowCursor();
                    scheduleAllTrainsRedrawing();
                    playEntitySwitchedSound(sem->isRed);
                } else
                    playErrorMelody();
            }
            break;

        case MouseAction::ToggleMouseMode:
            /* 14af:0477 */
            if (mouse::g_state.mode == &mouse::g_modeManagement)
                setMouseMode(mouse::g_modeConstruction);
            else
                setMouseMode(mouse::g_modeManagement);
            break;

        case MouseAction::ToggleNextRailType:
            /* 14af:0595 */
            assert(mouse::g_state.mode);
            mouse::g_state.mode->clearFn();
            for (;;) {
                g_railCursorState.railType = (g_railCursorState.railType + 1) % 6;
                const std::uint8_t curRailMask = 1 << g_railCursorState.railType;
                const std::uint8_t allowedMask = g_allowedRailCursorTypes[g_railCursorState.tileX][g_railCursorState.tileY];
                if (curRailMask & allowedMask)
                    break;
            }
            mouse::g_state.mode->drawFn();
            break;

        case MouseAction::BuildRails:
            /* 14af:04a5 */
            {
                const std::uint8_t newRail = 1 << g_railCursorState.railType;
                const std::uint8_t existing = g_railroadTypeMasks[g_railCursorState.tileX][g_railCursorState.tileY];
                if (newRail & existing) {
                    showStatusMessage("Track\'s already built over here");
                    playErrorMelody();
                } else {
                    if (checkRailWouldConflict(g_railCursorState.tileX, g_railCursorState.tileY,
                                               g_railCursorState.railType)) {
                        // the rail may conflict with both existing roads and rails of
                        // entrances that have not yet built
                        bool isTripleSwitch =
                            checkRailWouldConflictWithExistingRoad(
                                g_railCursorState.tileX, g_railCursorState.tileY,
                                g_railCursorState.railType);
                        const char* msg = isTripleSwitch
                            ? "No triple switch allowed"
                            : "Can't build for contradiction to General Construction Plan";
                        showStatusMessage(msg);
                        playErrorMelody();
                    } else {
                        beepSound(0);

                        g_railCursorState.year_8 = g_headers[static_cast<int>(HeaderFieldId::Year)].value - 8;
                        g_railConstructionMsgQueue.push(g_railCursorState);

                        mouse::g_state.mode->clearFn();
                        drawRail(g_railCursorState.tileX, g_railCursorState.tileY, g_railCursorState.railType,
                                 Color::White, 350);
                        mouse::g_state.mode->drawFn();
                    }
                }
            }
            break;

        case MouseAction::None:
            // TODO implement
            /* 14af:0695 */
            break;
        }

        co_await sleep(1);
    }
    co_return;
}

} // namespace resl
