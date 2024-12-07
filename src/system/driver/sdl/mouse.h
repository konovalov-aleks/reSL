#pragma once

#include <SDL_events.h>
#include <SDL_render.h>

#include "touch_context.h"
#include "touch_handler.h"
#include <system/mouse.h>

#include <cstdint>
#include <list>

namespace resl {

class MouseDriver {
    using HandlerStorageT = std::list<MouseHandler>;

public:
    class [[nodiscard]] HandlerHolder {
    public:
        HandlerHolder() noexcept;
        HandlerHolder(HandlerStorageT::iterator) noexcept;
        ~HandlerHolder();

        HandlerHolder(HandlerHolder&&) noexcept;
        HandlerHolder& operator=(HandlerHolder&&) noexcept;

        HandlerHolder(const HandlerHolder&) = delete;
        HandlerHolder& operator=(const HandlerHolder&) = delete;

    private:
        HandlerStorageT::iterator m_iter;
    };

    class [[nodiscard]] TouchContextProviderHolder {
    public:
        TouchContextProviderHolder(TouchHandler&);
        ~TouchContextProviderHolder();

        TouchContextProviderHolder(const TouchContextProviderHolder&) = delete;
        TouchContextProviderHolder(TouchContextProviderHolder&&) = delete;
        TouchContextProviderHolder& operator=(const TouchContextProviderHolder&) = delete;
        TouchContextProviderHolder& operator=(TouchContextProviderHolder&&) = delete;

    private:
        TouchHandler& m_touchHandler;
    };

    MouseDriver(SDL_Renderer*);
    ~MouseDriver();

    void setCursorVisibility(bool);
    bool cursorVisible() const noexcept;

    int cursorX() const { return m_cursorX; }
    int cursorY() const { return m_cursorY; }
    void setPosition(int x, int y);

    HandlerHolder addHandler(MouseHandler hdl);
    TouchContextProviderHolder setTouchContextProvider(TouchContextProvider&);

private:
    void handle(std::uint16_t flags, std::uint16_t btnState,
                std::int16_t x, std::int16_t y);

    void onMouseButtonEvent(const SDL_MouseButtonEvent&);
    void onMouseMove(const SDL_MouseMotionEvent&);
    void onTouch(const SDL_TouchFingerEvent&);

    void handleTouchAction(TouchHandler::Action, int x, int y);

    void drawCursor(SDL_Renderer*);

    TouchHandler m_touchHandler;

    HandlerStorageT m_handlers;
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
