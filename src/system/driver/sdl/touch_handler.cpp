#include "touch_handler.h"

// IWYU pragma: no_include <__math/trigonometric_functions.h>

#include "texture.h"
#include <graphics/vga.h>

#include <SDL_Timer.h>
#include <SDL_error.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <utility>

namespace resl {

const char* const g_iconPath = "build_rail_icon.png";

TouchHandler::TouchHandler(SDL_Renderer* renderer, LongPressHandlerT handler)
    : m_handler(std::move(handler))
{
    m_context.icon = { renderer, g_iconPath };
    computePoints();
}

void TouchHandler::onPressStart(int x, int y)
{
    if (!m_context.pressed) {
        m_context.pressed = true;
        m_lastFrameTime = SDL_GetTicks64();
    }

    if (std::holds_alternative<WaitStage>(m_stage)) {
        if (x < g_r1)
            x = g_r1;
        else if (x > SCREEN_WIDTH - g_r1)
            x = SCREEN_WIDTH - g_r1;

        if (y < g_r1)
            y = g_r1;
        else if (y > SCREEN_HEIGHT - g_r1)
            y = SCREEN_HEIGHT - g_r1;

        moveTo(x, y);
        m_context.x = x;
        m_context.y = y;
    }
}

bool TouchHandler::onPressEnd()
{
    m_context.pressed = false;
    if (std::holds_alternative<FinishedStage>(m_stage)) {
        m_stage = WaitStage();
        return true;
    }
    return false;
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
            m_handler(m_context.x, m_context.y);
        m_stage = std::move(*newStage);
    }
}

std::optional<TouchHandler::StageT> TouchHandler::WaitStage::handle(
    SDL_Renderer*, int dTime, const AnimationContext& ctx)
{
    if (!ctx.pressed) {
        m_time = 0;
        return std::nullopt;
    }
    m_time += dTime;
    if (m_time >= g_animationDelayMs)
        return TimerStage();

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

    SDL_Rect dstRect = {
        ctx.x - 87 / 2, ctx.y - 53 / 2, 87, 53 // TODO constants
    };
    SDL_SetTextureAlphaMod(ctx.icon, static_cast<Uint8>(200 * nPoints / ctx.points.size()));
    SDL_RenderCopy(renderer, ctx.icon, nullptr, &dstRect);

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

    // TODO use constants
    const int width = 87 * scale;
    const int height = 53 * scale;
    SDL_Rect dstRect = {
        ctx.x - width / 2, ctx.y - height / 2, width, height
    };
    SDL_SetTextureAlphaMod(ctx.icon, static_cast<Uint8>(200 * (maxScaleTimeMs - scale) / maxScaleTimeMs));
    SDL_RenderCopy(renderer, ctx.icon, nullptr, &dstRect);

    if (m_time >= maxScaleTimeMs)
        return FinishedStage();

    return std::nullopt;
}

void TouchHandler::computePoints()
{
    static_assert(g_nSteps * 2 <= std::numeric_limits<std::uint8_t>::max());
    float angle = -M_PI / 2.0f;
    for (int step = 0; step < g_nSteps; ++step, angle += g_angleStep) {
        const float cosVal = std::cos(angle);
        const float sinVal = std::sin(angle);

        m_context.points[step * 2] = { cosVal * g_r1, sinVal * g_r1 };
        m_context.points[step * 2 + 1] = { cosVal * g_r2, sinVal * g_r2 };
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
    int dx = x - m_context.x;
    int dy = y - m_context.y;
    for (SDL_FPoint& p : m_context.points) {
        p.x += dx;
        p.y += dy;
    }
}

} // namespace resl
