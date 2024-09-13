#include "mouse.h"

#include "construction_mode.h"
#include "management_mode.h"
#include "mode.h"
#include "state.h"
#include <game/drawing.h>
#include <game/entrance.h>
#include <game/header.h>
#include <game/header_field.h>
#include <game/melody.h>
#include <game/rail.h>
#include <game/rail_info.h>
#include <game/resources/allowed_cursor_rail_types.h>
#include <game/road_construction.h>
#include <game/semaphore.h>
#include <game/switch.h>
#include <game/train.h>
#include <graphics/color.h>
#include <system/mouse.h>
#include <system/sound.h>
#include <tasks/message_queue.h>
#include <tasks/task.h>
#include <ui/components/status_bar.h>

#include <cassert>
#include <cstdint>

namespace resl {

/* 262d:6ef8 : 1 byte */
static std::uint8_t g_previousMouseButtonState = 0;

/* 1d7d:1c96 */
MessageQueue<MsgMouseEvent> g_mouseMsgQueue;

void handleMouseInput(const MouseEvent& me)
{
    // the original game uses a global vairable here but we have a different
    // coroutines implementation, and it makes sense to use local variable instead.
    MsgMouseEvent msg = { MouseAction::None, me.x(), me.y() };

    if (g_previousMouseButtonState)
        g_previousMouseButtonState = static_cast<std::uint8_t>(me.buttonState());
    else {
        if ((me.flags() & (ME_LEFTPRESSED | ME_RIGHTPRESSED)) &&
            me.buttonState() == (MouseButton::MB_LEFT | MouseButton::MB_RIGHT)) {

            msg.action = MouseAction::ToggleMouseMode;
            g_previousMouseButtonState = MouseButton::MB_LEFT;
        } else {
            if (me.flags() & ME_LEFTRELEASED) {
                // left button clicked
                if (mouse::g_state.mode == &mouse::g_modeManagement)
                    msg.action = MouseAction::MouseClick;
                else
                    msg.action = MouseAction::ToggleNextRailType;
            } else {
                if (me.flags() & ME_RIGHTRELEASED) {
                    // right button clicked
                    if (mouse::g_state.mode == &mouse::g_modeManagement)
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

        assert(mouse::g_state.mode);
        mouse::Mode& mode = *mouse::g_state.mode;
        bool positionChanged = false;
        if (e.action != MouseAction::BuildRails)
            positionChanged = mode.updatePosFn(e.x, e.y);

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
                        const std::int16_t switchIdx = static_cast<std::int16_t>(sw - g_switches);
                        eraseSwitch(switchIdx);
                        toggleSwitch(*sw);
                        drawSwitch(switchIdx, true);
                        scheduleAllTrainsRedrawing();
                        playSwitchSwitchedMelody();
                    }
                } else if (Semaphore* sem = findClosestSemaphore(mode.x, mode.y)) {
                    toggleSemaphore(*sem);
                    drawSemaphore(*sem, 0);
                    drawSemaphore(*sem, 350);
                    scheduleAllTrainsRedrawing();
                    playEntitySwitchedSound(sem->isRed);
                } else
                    playErrorMelody();
            }
            break;

        case MouseAction::ToggleMouseMode:
            mouse::toggleMode();
            break;

        case MouseAction::ToggleNextRailType:
            /* 14af:0595 */

            // if the user clicked on a new chunk, we just need to move cursor
            if (!positionChanged) {
                assert(mouse::g_state.mode);
                mouse::g_state.mode->clearFn();
                for (;;) {
                    RailInfo& rcs = mouse::g_railCursorState;
                    rcs.railType = (rcs.railType + 1) % 6;
                    const std::uint8_t curRailMask = 1 << rcs.railType;
                    const std::uint8_t allowedMask =
                        g_allowedRailCursorTypes[rcs.tileX][rcs.tileY];
                    if (curRailMask & allowedMask)
                        break;
                }
                mouse::g_state.mode->drawFn();
            }
            break;

        case MouseAction::BuildRails:
            /* 14af:04a5 */
            {
                RailInfo& rcs = mouse::g_railCursorState;
                const std::uint8_t newRail = 1 << rcs.railType;
                const std::uint8_t existing = g_railroadTypeMasks[rcs.tileX][rcs.tileY];
                if (newRail & existing) {
                    showStatusMessage("Track\'s already built over here");
                    playErrorMelody();
                } else {
                    if (checkRailWouldConflict(rcs.tileX, rcs.tileY, rcs.railType)) {
                        // the rail may conflict with both existing roads and rails of
                        // entrances that have not yet built
                        bool isTripleSwitch = checkRailWouldConflictWithExistingRoad(
                            rcs.tileX, rcs.tileY, rcs.railType);
                        const char* msg = isTripleSwitch
                            ? "No triple switch allowed"
                            : "Can't build for contradiction to General Construction Plan";
                        showStatusMessage(msg);
                        playErrorMelody();
                    } else {
                        beepSound(0);

                        rcs.year_8 = g_headers[static_cast<int>(HeaderFieldId::Year)].value - 8;
                        g_railConstructionMsgQueue.push(rcs);

                        mouse::g_state.mode->clearFn();
                        drawRail(rcs.tileX, rcs.tileY, rcs.railType, Color::White, 350);
                        mouse::g_state.mode->drawFn();
                    }
                }
            }
            break;

        case MouseAction::ToggleSound:
            /* 14af:0695 */
            // This branch looks unreachable in the original game because
            // the mouse handler (14af:0761) never sets this value.
            g_soundEnabled = !g_soundEnabled;
            if (!g_soundEnabled)
                playSingleClickSound();
            else {
                sound(2000);
                co_await sleep(20);
                nosound();
            }
            break;

        case MouseAction::None:
            break;
        }

        co_await sleep(1);
    }
}

} // namespace resl
