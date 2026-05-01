#include "texture.h"

#include <SDL3/SDL_surface.h>

#include <cstdlib>
#include <iostream>
#include <utility>

namespace resl {

Texture::Texture(SDL_Renderer* renderer, const char* filePath) noexcept
{
    SDL_Surface* surface = SDL_LoadPNG(filePath);
    if (!surface) [[unlikely]] {
        std::cerr << "Unable to load the image \"" << filePath << '"' << std::endl;
        std::abort();
    }

    m_texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!m_texture) [[unlikely]] {
        std::cerr << "Unable to create a texture object" << std::endl;
        std::abort();
    }
    SDL_DestroySurface(surface);
}

Texture::~Texture()
{
    if (m_texture)
        SDL_DestroyTexture(m_texture);
}

Texture::Texture(Texture&& other) noexcept
    : m_texture(other.m_texture)
{
    other.m_texture = nullptr;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    std::swap(m_texture, other.m_texture);
    return *this;
}

} // namespace resl
