#pragma once

#include <SDL_render.h>

namespace resl {

class GridOverlay {
public:
    static GridOverlay& instance();

    void setVisibility(bool v) { m_active = v; }

private:
    GridOverlay();
    ~GridOverlay();
    GridOverlay(const GridOverlay&) = delete;
    GridOverlay& operator=(const GridOverlay&) = delete;

    void draw(SDL_Renderer*);
    SDL_Texture* texture(SDL_Renderer*);

    SDL_Texture* m_texture = nullptr;
    bool m_active = false;
};

} // namespace resl
