#pragma once

#include "texture.h"

#include <SDL_pixels.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_stdinc.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <optional>
#include <variant>

namespace resl {

class TouchHandler {
public:
    using LongPressHandlerT = std::function<void(int x, int y)>;

    TouchHandler(SDL_Renderer*, LongPressHandlerT);

    void onPressStart(int x, int y);
    // returns whether this keypress was processed
    bool onPressEnd();

    void draw(SDL_Renderer*);

private:
    static constexpr int g_r1 = 60;
    static constexpr int g_r2 = 55;

    static constexpr int g_nSteps = 32;
    static constexpr float g_angleStep = 2 * M_PI / g_nSteps;

    static constexpr SDL_Color g_startColor = { 0x55, 0x55, 0x55, 0xFF };
    static constexpr SDL_Color g_endColor = { 0xAA, 0xAA, 0xAA, 0x80 };

    static constexpr int g_fillTimeMs = 700;
    static constexpr int g_clearTimeMs = 400;

    class WaitStage;
    class TimerStage;
    class ConfirmationStage;
    class FinishedStage;

    using StageT = std::variant<WaitStage, TimerStage, ConfirmationStage, FinishedStage>;

    struct AnimationContext {
        bool pressed = false;

        Texture icon;
        std::array<SDL_FPoint, (g_nSteps + 1) * 2> points;
        std::array<std::uint8_t, g_nSteps * 3 * 2> indices;

        int x = 0;
        int y = 0;
    };

    class WaitStage {
    public:
        std::optional<StageT> handle(SDL_Renderer*, int dTime, const AnimationContext&);

    private:
        static constexpr int g_animationDelayMs = 200;
        int m_time = 0;
    };

    class TimerStage {
    public:
        std::optional<StageT> handle(SDL_Renderer*, int dTime, AnimationContext&);

    private:
        void fillColors(int nPoints);

        std::array<SDL_Color, (g_nSteps + 1) * 4> m_colors;
        float m_angle = 0;
    };

    class ConfirmationStage {
    public:
        std::optional<StageT> handle(SDL_Renderer*, int dTime, AnimationContext&);

    private:
        int m_time = 0;
    };

    class FinishedStage {
    public:
        std::optional<StageT> handle(SDL_Renderer*, int /*dTime*/, const AnimationContext&)
        {
            return std::nullopt;
        }
    };

    void computePoints();
    void moveTo(int x, int y);

    AnimationContext m_context;
    StageT m_stage = WaitStage();
    LongPressHandlerT m_handler;

    Uint64 m_lastFrameTime = 0;
};

} // namespace resl
