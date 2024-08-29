#pragma once

#include <SDL_events.h>
#include <SDL_render.h>

#include "touch_handler.h"
#include <system/mouse.h>

#include <cstdint>

namespace resl {

class MouseDriver {
public:
    MouseDriver(SDL_Renderer*);
    ~MouseDriver();

    void setCursorVisibility(bool);
    bool cursorVisible() const noexcept;

    int cursorX() const { return m_cursorX; }
    int cursorY() const { return m_cursorY; }
    void setPosition(int x, int y);

    MouseHandler setHandler(MouseHandler hdl);

private:
    void onMouseButtonEvent(const SDL_MouseButtonEvent&);
    void onMouseMove(const SDL_MouseMotionEvent&);
    void onTouch(const SDL_TouchFingerEvent&);

    void handleTouchAction(TouchHandler::Action, int x, int y);

    void drawCursor(SDL_Renderer*);

    TouchHandler m_touchHandler;

    MouseHandler m_handler = nullptr;
    std::uint16_t m_mouseButtonState;

    int m_cursorX = 0;
    int m_cursorY = 0;

    SDL_Texture* m_cursorTexture;
    bool m_mouseConnected;
    bool m_cursorVisible = false;

    friend class Driver;
    friend class VGAEmulation;
};

} // namespace resl
