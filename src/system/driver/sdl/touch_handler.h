#pragma once

#include "texture.h"
#include <graphics/vga.h>

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
    enum class Action {
        Tap,
        LongTap,
        Swipe
    };

    using LongPressHandlerT = std::function<void(Action, int x, int y)>;

    TouchHandler(SDL_Renderer*, LongPressHandlerT);

    void onPressStart(int x, int y);
    void onPressEnd();
    void onMove(int x, int y);

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

    static constexpr int g_maxTapDistance = SCREEN_HEIGHT / 5;
    static constexpr int g_minSwipeDistance = SCREEN_HEIGHT / 3;
    static constexpr int g_maxSwipeTimeMs = 500;

    struct WaitStage;
    class TimerStage;
    class ConfirmationStage;
    struct SwipeStage;
    class FinishedStage;

    using StageT = std::variant<
        WaitStage,
        TimerStage, ConfirmationStage, // long tap animation
        SwipeStage,
        FinishedStage>;

    struct AnimationContext {
        bool pressed = false;

        Texture icon;
        std::array<SDL_FPoint, (g_nSteps + 1) * 2> points;
        std::array<std::uint8_t, g_nSteps * 3 * 2> indices;

        int x;
        int y;
    };

    struct WaitStage {
        std::optional<StageT> handle(SDL_Renderer*, int dTime, const AnimationContext&);

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

    struct SwipeStage {
        std::optional<StageT> handle(SDL_Renderer*, int dTime, const AnimationContext&);

        int m_time;
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

    static int adjustXForAnimation(int);
    static int adjustYForAnimation(int);

    AnimationContext m_context;
    StageT m_stage = WaitStage();
    LongPressHandlerT m_handler;

    Uint64 m_lastFrameTime = 0;
};

} // namespace resl
