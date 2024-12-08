#pragma once

#include <SDL_render.h>

namespace resl {

class Texture {
public:
    Texture() = default;
    Texture(SDL_Renderer*, const char*) noexcept;
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&&) noexcept;
    Texture& operator=(Texture&&) noexcept;

    operator SDL_Texture*() const noexcept { return m_texture; }

private:
    SDL_Texture* m_texture = nullptr;
};

} // namespace resl
