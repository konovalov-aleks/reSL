#pragma once

#include <SDL_render.h>

namespace resl {

class MouseDriver {
public:
    MouseDriver();
    ~MouseDriver();

    void setCursorVisibility(bool);
    bool cursorVisible() const noexcept;

    void setPosition(int x, int y);

    void drawCursor(SDL_Renderer*);

private:
    bool hasMouse() const;

    SDL_Texture* m_cursorTexture = nullptr;
    bool m_mouseConnected;
    bool m_cursorVisible = false;
};

} // namespace resl
