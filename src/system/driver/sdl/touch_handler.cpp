#include "touch_handler.h"

// IWYU pragma: no_include <__math/trigonometric_functions.h>

#include "texture.h"
#include <graphics/vga.h>

#include <SDL_Timer.h>
#include <SDL_error.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

namespace resl {

namespace {

    constexpr char g_iconBuildRailPath[] = "build_rail_icon.png";
    constexpr char g_iconCallServerPath[] = "call_server_icon.png";

    constexpr int g_r1 = 60;
    constexpr int g_r2 = 50;

    constexpr int g_iconWidth = 64;
    constexpr int g_iconHeight = 64;

    enum class IconPosition {
        Top,
        Left,
        Right
    };

    IconPosition iconPosition(int x, int y)
    {
        if (y - g_r1 - g_iconHeight >= 0)
            return IconPosition::Top;

        return x > SCREEN_WIDTH / 2 ? IconPosition::Left
                                    : IconPosition::Right;
    }

    std::pair<int, int> iconOffset(int x, int y)
    {
        switch (iconPosition(x, y)) {
        case IconPosition::Top:
            return { -g_iconWidth / 2, -g_r1 - g_iconHeight };
        case IconPosition::Left:
            return { -g_r1 - g_iconWidth, -g_iconHeight / 2 };
        case IconPosition::Right:
            return { g_r1, -g_iconHeight / 2 };
        };
    }

} // namespace

TouchHandler::TouchHandler(SDL_Renderer* renderer, LongPressHandlerT handler)
    : m_handler(std::move(handler))
{
    m_context.iconBuildRail = { renderer, g_iconBuildRailPath };
    m_context.iconCallServer = { renderer, g_iconCallServerPath };
    computePoints();
}

void TouchHandler::onPressStart(int x, int y)
{
    if (!m_context.pressed) {
        m_context.pressed = true;
        m_lastFrameTime = SDL_GetTicks64();
    }

    if (std::holds_alternative<WaitStage>(m_stage)) {
        moveTo(x, y);
        m_context.x = x;
        m_context.y = y;
    }
}

void TouchHandler::onPressEnd()
{
    if (m_handler && m_context.pressed && std::holds_alternative<WaitStage>(m_stage))
        m_handler(Action::Tap, m_context.x, m_context.y);
    m_context.pressed = false;
    if (std::holds_alternative<FinishedStage>(m_stage))
        m_stage = WaitStage();
}

void TouchHandler::onMove(int x, int y)
{
    const int dx = std::abs(x - m_context.x);
    const int dy = std::abs(y - m_context.y);
    const int distSqr = dx * dx + dy * dy;
    std::visit(
        [this, distSqr, x, y]<typename T>(T& s) {
            const bool allowSwipe =
                m_context.touchContextProvider && m_context.touchContextProvider->isSwipeAllowed();
            if constexpr (std::is_same_v<T, WaitStage>) {
                if (!allowSwipe)
                    return;

                if (distSqr >= g_minSwipeDistance * g_minSwipeDistance) {
                    static_assert(g_maxSwipeTimeMs >= WaitStage::g_animationDelayMs);
                    if (m_handler)
                        m_handler(Action::Swipe, x, y);
                    m_stage = FinishedStage();
                } else if (distSqr > g_maxTapDistance * g_maxTapDistance) {
                    static_assert(g_maxSwipeTimeMs >= WaitStage::g_animationDelayMs);
                    m_stage = SwipeStage { s.m_time };
                }
            } else if constexpr (std::is_same_v<T, TimerStage>)
                m_context.pressed = distSqr <= g_maxTapDistance * g_maxTapDistance;

            else if constexpr (std::is_same_v<T, SwipeStage>) {
                if (distSqr >= g_minSwipeDistance * g_minSwipeDistance) {
                    if (allowSwipe && m_handler)
                        m_handler(Action::Swipe, x, y);
                    m_stage = FinishedStage();
                }
            }
        },
        m_stage);
}

void TouchHandler::draw(SDL_Renderer* renderer)
{
    Uint64 time = SDL_GetTicks64();
    int dTime = static_cast<int>(time - m_lastFrameTime);
    m_lastFrameTime = time;

    std::optional<StageT> newStage =
        std::visit(
            [this, renderer, dTime](auto& stage) {
                return stage.handle(renderer, dTime, m_context);
            },
            m_stage);
    if (newStage) {
        if (m_handler && std::holds_alternative<FinishedStage>(*newStage))
            m_handler(Action::LongTap, m_context.x, m_context.y);
        m_stage = std::move(*newStage);
    }
}

void TouchHandler::setTouchContextProvider(TouchContextProvider* tcp)
{
    m_context.touchContextProvider = tcp;
}

std::optional<TouchHandler::StageT> TouchHandler::WaitStage::handle(
    SDL_Renderer*, int dTime, AnimationContext& ctx)
{
    const LongTouchAction longTouchAction = ctx.touchContextProvider
        ? ctx.touchContextProvider->recognizeTouchAction(ctx.x, ctx.y)
        : LongTouchAction::None;

    if (!ctx.pressed || longTouchAction == LongTouchAction::None) {
        m_time = 0;
        return std::nullopt;
    }
    m_time += dTime;
    if (m_time >= g_animationDelayMs) {
        ctx.curIcon = longTouchAction == LongTouchAction::BuildRail
            ? &ctx.iconBuildRail
            : &ctx.iconCallServer;
        return TimerStage();
    }
    return std::nullopt;
}

std::optional<TouchHandler::StageT> TouchHandler::TimerStage::handle(
    SDL_Renderer* renderer, int dTime, AnimationContext& ctx)
{
    if (ctx.pressed) {
        m_angle += (3 * M_PI / g_fillTimeMs) * dTime;
        if (m_angle > 3 * M_PI)
            return ConfirmationStage();
    } else {
        m_angle -= (3 * M_PI / g_clearTimeMs) * dTime;
        if (m_angle <= 0)
            return WaitStage();
    }

    int nPoints;
    int nIndices;
    if (m_angle - 2 * M_PI >= 0) {
        // entire circle
        nPoints = static_cast<int>(ctx.points.size());
        nIndices = static_cast<int>(ctx.indices.size());
    } else {
        const int steps = static_cast<int>(m_angle / g_angleStep);
        if (!steps)
            return std::nullopt;

        nPoints = (steps + 1) * 2;
        assert(nPoints <= static_cast<int>(ctx.points.size()));

        nIndices = steps * 6;
        assert(nIndices <= static_cast<int>(ctx.indices.size()));
    }

    fillColors(nPoints);

    int res = SDL_RenderGeometryRaw(
        renderer, nullptr,
        reinterpret_cast<const float*>(ctx.points.data()), sizeof(ctx.points[0]),
        m_colors.data(), sizeof(m_colors[0]),
        nullptr, 0, nPoints,
        ctx.indices.data(), nIndices, sizeof(ctx.indices[0]));
    if (res) [[unlikely]]
        std::cerr << "SDL_RenderGeometryRaw failed: " << SDL_GetError() << std::endl;

    const int adjX = adjustXForAnimation(ctx.x);
    const int adjY = adjustYForAnimation(ctx.y);
    const auto [iconXOffset, iconYOffset] = iconOffset(adjX, adjY);

    SDL_Rect dstRect = { adjX + iconXOffset, adjY + iconYOffset, g_iconWidth, g_iconHeight };
    assert(ctx.curIcon);
    SDL_SetTextureAlphaMod(*ctx.curIcon, static_cast<Uint8>(200 * nPoints / ctx.points.size()));
    SDL_RenderCopy(renderer, *ctx.curIcon, nullptr, &dstRect);

    return std::nullopt;
}

void TouchHandler::TimerStage::fillColors(int nPoints)
{
    assert(nPoints % 2 == 0);
    assert(static_cast<int>(m_colors.size()) >= nPoints * 2);

    for (int i = 0; i < nPoints; i += 2) {
        int dist = nPoints - i;
        if (m_angle > 2 * M_PI)
            dist = std::min<int>(dist + (m_angle - 2 * M_PI) * 15, nPoints);

        const int kStart = dist;
        const int kEnd = nPoints - dist;

        std::uint8_t r = (g_startColor.r * kStart + g_endColor.r * kEnd) / nPoints;
        std::uint8_t g = (g_startColor.g * kStart + g_endColor.g * kEnd) / nPoints;
        std::uint8_t b = (g_startColor.b * kStart + g_endColor.b * kEnd) / nPoints;
        std::uint8_t a = (g_startColor.a * kStart + g_endColor.a * kEnd) / nPoints;

        m_colors[i] = { r, g, b, a };
        m_colors[i + 1] = {
            static_cast<Uint8>(r * 4 / 5),
            static_cast<Uint8>(r * 4 / 5),
            static_cast<Uint8>(b * 4 / 5),
            a
        };
    }
}

std::optional<TouchHandler::StageT> TouchHandler::ConfirmationStage::handle(
    SDL_Renderer* renderer, int dTime, AnimationContext& ctx)
{
    // scale function: scale(x) = 1 - ((x - 100) ^ 2) / (200^2);

    constexpr float maxScaleTimeMs = 200;

    const int x = m_time - 100;
    const float scale = 1 - x * x / (maxScaleTimeMs * maxScaleTimeMs);
    m_time += dTime;

    const int width = g_iconWidth * scale;
    const int height = g_iconHeight * scale;
    const int adjX = adjustXForAnimation(ctx.x);
    const int adjY = adjustYForAnimation(ctx.y);
    const auto [iconXOffset, iconYOffset] = iconOffset(adjX, adjY);
    SDL_Rect dstRect = {
        static_cast<int>(adjX + iconXOffset * scale),
        static_cast<int>(adjY + iconYOffset * scale),
        width, height
    };
    assert(ctx.curIcon);
    SDL_SetTextureAlphaMod(*ctx.curIcon, static_cast<Uint8>(200 * (maxScaleTimeMs - scale) / maxScaleTimeMs));
    SDL_RenderCopy(renderer, *ctx.curIcon, nullptr, &dstRect);

    if (m_time >= maxScaleTimeMs)
        return FinishedStage();

    return std::nullopt;
}

std::optional<TouchHandler::StageT> TouchHandler::SwipeStage::handle(
    SDL_Renderer*, int dTime, const AnimationContext&)
{
    m_time += dTime;
    if (m_time > g_maxSwipeTimeMs)
        return FinishedStage();

    return std::nullopt;
}

void TouchHandler::computePoints()
{
    static_assert(g_nSteps * 2 <= std::numeric_limits<std::uint8_t>::max());
    float angle = -M_PI / 2.0f;

    m_context.x = g_r1;
    m_context.y = g_r1;
    for (int step = 0; step < g_nSteps; ++step, angle += g_angleStep) {
        const float cosVal = std::cos(angle);
        const float sinVal = std::sin(angle);

        m_context.points[step * 2] = {
            cosVal * g_r1 + m_context.x, sinVal * g_r1 + m_context.y
        };
        m_context.points[step * 2 + 1] = {
            cosVal * g_r2 + m_context.x, sinVal * g_r2 + m_context.y
        };
    }
    m_context.points[g_nSteps * 2] = m_context.points[0];
    m_context.points[g_nSteps * 2 + 1] = m_context.points[1];

    for (std::size_t i = 0; i < g_nSteps * 2; ++i) {
        m_context.indices[i * 3] = i;
        m_context.indices[i * 3 + 1] = i + 1;
        m_context.indices[i * 3 + 2] = i + 2;
    }
}

void TouchHandler::moveTo(int x, int y)
{
    x = adjustXForAnimation(x);
    y = adjustYForAnimation(y);
    const int oldX = adjustXForAnimation(m_context.x);
    const int oldY = adjustYForAnimation(m_context.y);

    const int dx = x - oldX;
    const int dy = y - oldY;
    for (SDL_FPoint& p : m_context.points) {
        p.x += dx;
        p.y += dy;
    }
}

int TouchHandler::adjustXForAnimation(int x)
{
    if (x < g_r1)
        return g_r1;
    if (x > SCREEN_WIDTH - g_r1)
        return SCREEN_WIDTH - g_r1;
    return x;
}

int TouchHandler::adjustYForAnimation(int y)
{
    if (y < g_r1)
        return g_r1;
    if (y > SCREEN_HEIGHT - g_r1)
        return SCREEN_HEIGHT - g_r1;
    return y;
}

} // namespace resl
