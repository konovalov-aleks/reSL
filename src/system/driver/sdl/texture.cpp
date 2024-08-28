#include "texture.h"

#include <SDL_Image.h>

#include <cstdlib>
#include <iostream>
#include <utility>

namespace resl {

Texture::Texture(SDL_Renderer* renderer, const char* filePath) noexcept
    : m_texture(IMG_LoadTexture(renderer, filePath))
{
    if (!m_texture) [[unlikely]] {
        std::cerr << "Unable to load the image \"" << filePath << '"' << std::endl;
        std::abort();
    }
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
