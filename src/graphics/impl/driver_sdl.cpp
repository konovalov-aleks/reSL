#include <graphics/driver.h>

#include <SDL.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

namespace resl {

static SDL_Window* g_window = NULL;
static SDL_Renderer* g_renderer = NULL;
static SDL_Texture* g_screen = NULL;
static char* g_screenPixels = NULL;
static int g_screenPixelsPitch = 0;

static void lockTexture()
{
    int err = SDL_LockTexture(g_screen, NULL, (void**)&g_screenPixels, &g_screenPixelsPitch);
    if (err) {
        std::fprintf(stderr, "Unable to lock the texture! SDL_Error: %s\n", SDL_GetError());
        std::exit(EXIT_FAILURE);
    }
}

static void unlockTexture() { SDL_UnlockTexture(g_screen); }

void graphics_init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        std::exit(EXIT_FAILURE);
    }

    int err = SDL_CreateWindowAndRenderer(
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI, &g_window,
        &g_renderer
    );
    if (err) {
        std::fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        graphics_close();
        std::exit(EXIT_FAILURE);
    }

    g_screen = SDL_CreateTexture(
        g_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, VIDEO_MEM_ROW_BYTES * 8,
        SCREEN_HEIGHT
    );
    if (!g_screen) {
        std::fprintf(stderr, "Unable to create SDL texture! SDL_Error: %s\n", SDL_GetError());
        graphics_close();
        std::exit(EXIT_FAILURE);
    }

    lockTexture();

    SDL_SetRenderTarget(g_renderer, NULL);
}

void graphics_close()
{
    if (g_screen)
        SDL_DestroyTexture(g_screen);
    if (g_renderer)
        SDL_DestroyRenderer(g_renderer);
    if (g_window)
        SDL_DestroyWindow(g_window);

    SDL_Quit();

    g_renderer = NULL;
    g_window = NULL;
    g_screen = NULL;
}

void graphics_update()
{
    unlockTexture();

    SDL_Rect srcRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    SDL_RenderCopy(g_renderer, g_screen, &srcRect, NULL);
    SDL_RenderPresent(g_renderer);

    lockTexture();
}

bool poll_event()
{
    int time = SDL_GetTicks();
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            return false;
    }
    if (time < 10)
        SDL_Delay(10 - time);
    return true;
}

uint8_t g_videoMask = 0;
uint8_t g_videoWriteMode = 2;
uint8_t g_videoRegMapMask = 0xF;

static const std::array<uint32_t, 16> g_palette = {
    0x55AA00, // Green
    0x000000, // Black
    0xAAAAAA, // Gray
    0x555555, // Dark gray
    0xFFFFFF, // White
    0xFFFF55, // Yellow
    0xAAAA00, // Brown
    0x0055FF, // Blue
    0x0055AA, // Dark blue
    0xFF5500, // Red
    0xAA5500, // Dark red
    0x1AFFFF, // Cyan
    0x00AAAA, // Dark cyan
    0x00FF00, // Light green
    0x00AA00, // Dark green
    0xFFFFFF  // White / erase
};

void writeVideoMem(unsigned memPtr, std::uint8_t color)
{
    unsigned offset = memPtr - VIDEO_MEM_START_ADDR;
    std::uint8_t mask = g_videoMask;
    int x0 = (offset * 8) % (VIDEO_MEM_ROW_BYTES * 8);
    int y = ((offset * 8) / (VIDEO_MEM_ROW_BYTES * 8)) % SCREEN_HEIGHT;
    std::uint32_t rgb = g_palette[color];
    if (g_videoWriteMode == 0) {
        // TODO rotate + modes
        mask = color;
    }
    for (int x = x0; x < x0 + 8; ++x) {
        if (g_videoWriteMode == 0) {
            std::memcpy(
                &rgb, &g_screenPixels[g_screenPixelsPitch * y + x * sizeof(rgb)], sizeof(rgb)
            );
            auto iter = std::find(g_palette.begin(), g_palette.end(), rgb);
            assert(iter != g_palette.end());
            std::uint8_t c = std::distance(g_palette.begin(), iter);
            c = (c & ~g_videoRegMapMask) | (((mask & 0x80) ? g_videoRegMapMask : 0));
            rgb = g_palette[c];
            std::memcpy(
                &g_screenPixels[g_screenPixelsPitch * y + x * sizeof(rgb)], &rgb, sizeof(rgb)
            );
        } else if (mask & 0x80)
            std::memcpy(
                &g_screenPixels[g_screenPixelsPitch * y + x * sizeof(rgb)], &rgb, sizeof(rgb)
            );
        mask <<= 1;
    }
}

} // namespace resl
