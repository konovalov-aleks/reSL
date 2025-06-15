#include "field_tile_grid_overlay.h"

#include "constants.h"
#include <graphics/vga.h>
#include <system/driver/driver.h>
#include <ui/components/draw_header.h>

#include <SDL_blendmode.h>
#include <SDL_error.h>
#include <SDL_rect.h>

#include <cstdlib>
#include <iostream>

namespace resl {

GridOverlay& GridOverlay::instance()
{
    static GridOverlay overlay;
    return overlay;
}

GridOverlay::GridOverlay()
{
    Driver::instance().vga().addOverlay(
        [this](SDL_Renderer* r, int yOffset) {
            draw(r, yOffset);
        });
}

GridOverlay::~GridOverlay()
{
    if (m_texture)
        SDL_DestroyTexture(m_texture);
}

SDL_Texture* GridOverlay::texture(SDL_Renderer* renderer)
{
    if (!m_texture) {
        m_texture = SDL_CreateTexture(renderer,
                                      Driver::instance().vga().preferredPixelFormat(),
                                      SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
        if (!m_texture) [[unlikely]] {
            std::cerr << "Unable to create a texture. SDL error: "
                      << SDL_GetError() << std::endl;
            std::abort();
        }
        if (SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND)) [[unlikely]] {
            std::cerr << "SDL_SetTextureBlendMode failed. "
                      << SDL_GetError() << std::endl;
            std::abort();
        }

        SDL_Texture* oldTarget = SDL_GetRenderTarget(renderer);
        if (SDL_SetRenderTarget(renderer, m_texture)) [[unlikely]] {
            std::cerr << "SDL_SetRenderTarget failed. "
                      << SDL_GetError() << std::endl;
            std::abort();
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        SDL_Rect clipRect = {
            10, g_headerHeight + 30,
            GAME_FIELD_WIDTH - 20, GAME_FIELD_HEIGHT - 40
        };
        SDL_RenderSetClipRect(renderer, &clipRect);

        SDL_SetRenderDrawColor(renderer, 0x64, 0x8B, 0x3C, 0xFF);
        for (int tileX1 = 0; tileX1 < 11; ++tileX1) {
            const int tileX2 = tileX1;
            const int tileY1 = 0;
            const int tileY2 = 10;

            const int x1 = (tileX1 - tileY1) * 88 + 320;
            const int y1 = (tileX1 + tileY1) * 21 - 22;
            const int x2 = (tileX2 - tileY2) * 88 + 320;
            const int y2 = (tileX2 + tileY2) * 21 - 22;
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
        for (int tileY1 = 0; tileY1 < 11; ++tileY1) {
            const int tileY2 = tileY1;
            const int tileX1 = 0;
            const int tileX2 = 10;

            const int x1 = (tileX1 - tileY1) * 88 + 320;
            const int y1 = (tileX1 + tileY1) * 21 - 22;
            const int x2 = (tileX2 - tileY2) * 88 + 320;
            const int y2 = (tileX2 + tileY2) * 21 - 22;
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
        for (int tileX = 0; tileX < 11; ++tileX) {
            for (int tileY = 0; tileY < 11; ++tileY) {
                const int x = (tileX - tileY) * 88 + 320;
                const int y = (tileX + tileY) * 21 - 22;
                SDL_Rect rect = { x - 1, y - 1, 3, 3 };
                SDL_RenderDrawRect(renderer, &rect);
            }
        }

        SDL_RenderSetClipRect(renderer, nullptr);
        if (SDL_SetRenderTarget(renderer, oldTarget)) [[unlikely]] {
            std::cerr << "SDL_SetRenderTarget failed. "
                      << SDL_GetError() << std::endl;
            std::abort();
        }
    }
    return m_texture;
}

void GridOverlay::draw(SDL_Renderer* renderer, int yOffset)
{
    if (!m_active)
        return;

    SDL_Rect dstRect = {
        0, yOffset,
        SCREEN_WIDTH, SCREEN_HEIGHT
    };
    SDL_RenderCopy(renderer, texture(renderer), nullptr, &dstRect);
}

} // namespace resl
