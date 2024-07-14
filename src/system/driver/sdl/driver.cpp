#include "driver.h"

// IWYU pragma: no_include "system/driver/sdl/driver.h"

#include <graphics/vga.h>
#include <system/keyboard.h>
#include <system/mouse.h>

#include <SDL.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_mouse.h>
#include <SDL_scancode.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <thread>

#ifndef NDEBUG
#   include <limits>
#endif

#ifdef __EMSCRIPTEN__
#   include <emscripten.h>
#endif

namespace resl {

Driver::SDLInit::SDLInit()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) [[unlikely]] {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    SDL_ShowCursor(false);
}

Driver::SDLInit::~SDLInit()
{
    SDL_Quit();
}

inline std::uint16_t mouseButtonState()
{
    Uint32 state = SDL_GetMouseState(nullptr, nullptr);
    std::uint16_t res = 0;

    if (state & SDL_BUTTON(SDL_BUTTON_LEFT))
        res |= MouseButton::MB_LEFT;
    if (state & SDL_BUTTON(SDL_BUTTON_RIGHT))
        res |= MouseButton::MB_RIGHT;
    if (state & SDL_BUTTON(SDL_BUTTON_MIDDLE))
        res |= MouseButton::MB_MIDDLE;
    return res;
}

Driver::Driver()
    : m_mouseButtonState(mouseButtonState())
{
}

void Driver::sleep(unsigned ms)
{
#ifdef __EMSCRIPTEN__

    // We have to return to the main loop to update the picture.
    // But this can be a relatively long operation, which is why on small delays
    // it should be done only when necessary.
    const unsigned timeToNextFrame = vga().timeToNextFrameMS();
    if (ms < timeToNextFrame) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        return;
    }

    using ClockT = std::chrono::steady_clock;
    const ClockT::time_point startTime = ClockT::now();
    if (timeToNextFrame)
        std::this_thread::sleep_for(std::chrono::milliseconds(timeToNextFrame));
    vga().flush();

    const auto timePassed =
        std::chrono::duration_cast<std::chrono::milliseconds>(ClockT::now() - startTime).count();

    const unsigned msToSleep = timePassed < ms ? ms - timePassed : 0;
    emscripten_sleep(msToSleep);

#else // __EMSCRIPTEN__

    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    vga().flush();

#endif // __EMSCRIPTEN__
}

void Driver::pollEvent()
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            std::exit(EXIT_SUCCESS);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            onMouseButtonEvent(e.button);
            break;
        case SDL_MOUSEMOTION:
            onMouseMove(e.motion);
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            onKeyboardEvent(e.key);
            break;
        }
    }
}

inline std::uint16_t mousePressedFlags(const SDL_MouseButtonEvent& e)
{
    assert(e.type == SDL_MOUSEBUTTONDOWN);
    switch (e.button) {
    case SDL_BUTTON_LEFT:
        return MouseEvent::ME_LEFTPRESSED;
    case SDL_BUTTON_RIGHT:
        return MouseEvent::ME_RIGHTPRESSED;
    case SDL_BUTTON_MIDDLE:
        return MouseEvent::ME_CENTERPRESSED;
    }
    return 0;
}

inline std::uint16_t mouseReleasedFlags(const SDL_MouseButtonEvent& e)
{
    assert(e.type == SDL_MOUSEBUTTONUP);
    switch (e.button) {
    case SDL_BUTTON_LEFT:
        return MouseEvent::ME_LEFTRELEASED;
    case SDL_BUTTON_RIGHT:
        return MouseEvent::ME_RIGHTRELEASED;
    case SDL_BUTTON_MIDDLE:
        return MouseEvent::ME_CENTERRELEASED;
    }
    return 0;
}

inline MouseButton mouseButton(const SDL_MouseButtonEvent& e)
{
    switch (e.button) {
    case SDL_BUTTON_LEFT:
        return MouseButton::MB_LEFT;
    case SDL_BUTTON_RIGHT:
        return MouseButton::MB_RIGHT;
    case SDL_BUTTON_MIDDLE:
        return MouseButton::MB_MIDDLE;
    }
    return MouseButton::MB_NONE;
}

void Driver::onMouseButtonEvent(const SDL_MouseButtonEvent& e)
{
    if (!m_mouseHandler) [[unlikely]]
        return;

    const std::uint16_t mouseEventFlags =
        e.type == SDL_MOUSEBUTTONDOWN ? mousePressedFlags(e)
                                      : mouseReleasedFlags(e);
    if (!mouseEventFlags) [[unlikely]]
        return;

    const MouseButton btn = mouseButton(e);
    if (btn == MouseButton::MB_NONE) [[unlikely]]
        return;

    if (e.type == SDL_MOUSEBUTTONDOWN)
        m_mouseButtonState |= btn;
    else
        m_mouseButtonState &= (~btn);

    m_mouseHandler(mouseEventFlags, m_mouseButtonState, 0, 0);
}

void Driver::onMouseMove(const SDL_MouseMotionEvent& e)
{
    if (!m_mouseHandler) [[unlikely]]
        return;

    // limit mouse movement if the debug graphics is active
    const Sint32 x = std::min(e.x, SCREEN_WIDTH);
    const Sint32 y = std::min(e.y, SCREEN_HEIGHT);

    // window is small => coordinates can't be large
    assert(x - m_lastCursorX <= std::numeric_limits<std::int16_t>::max());
    assert(x - m_lastCursorX >= std::numeric_limits<std::int16_t>::min());
    assert(y - m_lastCursorY <= std::numeric_limits<std::int16_t>::max());
    assert(y - m_lastCursorY >= std::numeric_limits<std::int16_t>::min());

    std::int16_t dx = static_cast<std::int16_t>(x - m_lastCursorX);
    std::int16_t dy = static_cast<std::int16_t>(y - m_lastCursorY);
    m_lastCursorX = x;
    m_lastCursorY = y;

    m_mouseHandler(0, m_mouseButtonState, dx, dy);
}

inline std::uint8_t scancodeToKeyCode(SDL_Scancode sc)
{
    /* http://www.techhelpmanual.com/57-keyboard_scan_codes.html

        01   1  Esc │12  18  E    │23  35  H      │34  52  . >    │45  69  NumLock
        02   2  1 ! │13  19  R    │24  36  J      │35  53  / ?    │46  70  ScrollLck
        03   3  2 @ │14  20  T    │25  37  K      │36  54  Shft(R)│47  71  Home [7]
        04   4  3 # │15  21  Y    │26  38  L      │37  55  * PrtSc│48  72  ↑    [8]
        05   5  4 $ │16  22  U    │27  39  ; :    │38  56  Alt    │49  73  PgUp [9]
        06   6  5 % │17  23  I    │28  40  " '    │39  57  space  │4a  74  K -
        07   7  6 ^ │18  24  O    │29  41  ` ~    │3a  58  CapsLck│4b  75  ←    [4]
        08   8  7 & │19  25  P    │2a  42  Shft(L)│3b  59  F1     │4c  76       [5]
        09   9  8 * │1a  26  [ {  │2b  43  \ |    │3c  60  F2     │4d  77  →    [6]
        0a  10  9 ( │1b  27  ] }  │2c  44  Z      │3d  61  F3     │4e  78  K +
        0b  11  0 ) │1c  28  Enter│2d  45  X      │3e  62  F4     │4f  79  End  [1]
        0c  12  - _ │1d  29  Ctrl │2e  46  C      │3f  63  F5     │50  80  ↓    [2]
        0d  13  + = │1e  30  A    │2f  47  V      │40  64  F6     │51  81  PgDn [3]
        0e  14  bksp│1f  31  S    │30  48  B      │41  65  F7     │52  82  Ins  [0]
        0f  15  Tab │20  32  D    │31  49  N      │42  66  F8     │53  83  Del  [.]
        10  16  Q   │21  33  F    │32  50  M      │43  67  F9     │
        11  17  W   │22  34  G    │33  51  , <    │44  68  F10    │

    */

    switch (sc) {
    case SDL_SCANCODE_ESCAPE: return 1;
    case SDL_SCANCODE_1: return 2;
    case SDL_SCANCODE_2: return 3;
    case SDL_SCANCODE_3: return 4;
    case SDL_SCANCODE_4: return 5;
    case SDL_SCANCODE_5: return 6;
    case SDL_SCANCODE_6: return 7;
    case SDL_SCANCODE_7: return 8;
    case SDL_SCANCODE_8: return 9;
    case SDL_SCANCODE_9: return 10;
    case SDL_SCANCODE_0: return 11;
    case SDL_SCANCODE_MINUS: return 12;
    case SDL_SCANCODE_EQUALS: return 13;
    case SDL_SCANCODE_BACKSPACE: return 14;

    case SDL_SCANCODE_TAB: return 15;
    case SDL_SCANCODE_Q: return 16;
    case SDL_SCANCODE_W: return 17;
    case SDL_SCANCODE_E: return 18;
    case SDL_SCANCODE_R: return 19;
    case SDL_SCANCODE_T: return 20;
    case SDL_SCANCODE_Y: return 21;
    case SDL_SCANCODE_U: return 22;
    case SDL_SCANCODE_I: return 23;
    case SDL_SCANCODE_O: return 24;
    case SDL_SCANCODE_P: return 25;
    case SDL_SCANCODE_LEFTBRACKET: return 26;
    case SDL_SCANCODE_RIGHTBRACKET: return 27;
    case SDL_SCANCODE_RETURN: return 28;
    case SDL_SCANCODE_LCTRL:
    case SDL_SCANCODE_RCTRL: return 29;

    case SDL_SCANCODE_A: return 30;
    case SDL_SCANCODE_S: return 31;
    case SDL_SCANCODE_D: return 32;
    case SDL_SCANCODE_F: return 33;
    case SDL_SCANCODE_G: return 34;
    case SDL_SCANCODE_H: return 35;
    case SDL_SCANCODE_J: return 36;
    case SDL_SCANCODE_K: return 37;
    case SDL_SCANCODE_L: return 38;
    case SDL_SCANCODE_SEMICOLON: return 39;
    case SDL_SCANCODE_APOSTROPHE: return 40;
    case SDL_SCANCODE_GRAVE: return 41;

    case SDL_SCANCODE_LSHIFT: return 42;
    case SDL_SCANCODE_BACKSLASH: return 43;
    case SDL_SCANCODE_Z: return 44;
    case SDL_SCANCODE_X: return 45;
    case SDL_SCANCODE_C: return 46;
    case SDL_SCANCODE_V: return 47;
    case SDL_SCANCODE_B: return 48;
    case SDL_SCANCODE_N: return 49;
    case SDL_SCANCODE_M: return 50;
    case SDL_SCANCODE_COMMA: return 51;
    case SDL_SCANCODE_PERIOD: return 52;
    case SDL_SCANCODE_SLASH: return 53;
    case SDL_SCANCODE_RSHIFT: return 54;

    // PrtSc : 55
    case SDL_SCANCODE_LALT:
    case SDL_SCANCODE_RALT: return 56;

    case SDL_SCANCODE_SPACE: return 57;
    case SDL_SCANCODE_CAPSLOCK: return 58;

    default: return 0;
    }
}

void Driver::onKeyboardEvent(const SDL_KeyboardEvent& e)
{
    if (!m_keyboardHandler) [[unlikely]]
        return;

    std::uint8_t keycode = scancodeToKeyCode(e.keysym.scancode);
    if (!keycode)
        return;

    if (e.type == SDL_KEYUP)
        keycode |= g_keyReleasedFlag;
    m_keyboardHandler(keycode);
}

} // namespace resl
