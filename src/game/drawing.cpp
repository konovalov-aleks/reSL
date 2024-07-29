#include "drawing.h"

#include "draw_header.h"
#include "graphics/drawing.h"
#include "header.h"
#include "impasse.h"
#include "mouse/management_mode.h"
#include "mouse/mouse_mode.h"
#include "mouse/mouse_state.h"
#include "rail.h"
#include "resources/chunk_bounding_boxes.h"
#include "resources/rail_glyph.h"
#include "resources/train_finished_exclamation_glyph.h"
#include "semaphore.h"
#include "static_object.h"
#include "status_bar.h"
#include "switch.h"
#include "train.h"
#include "types/header_field.h"
#include "types/rail_info.h"
#include "types/rectangle.h"
#include <graphics/color.h>
#include <graphics/glyph.h>
#include <graphics/vga.h>
#include <system/buffer.h>
#include <system/filesystem.h>

#include <cassert>
#include <cstdint>
#include <cstdlib>

#ifndef NDEBUG
#   include <iterator>
#endif

namespace resl {

/* 262d:6f94 : 8 bytes */
Rectangle g_areaToRedraw;

//-----------------------------------------------------------------------------

/* 17bf:0e1b */
void redrawScreenArea()
{
    vga::waitVerticalRetrace();
    if (mouse::g_state.mode == &mouse::g_modeManagement)
        mouse::g_state.mode->clearFn();

    vga::setVideoModeR0W1();
    graphics::copyFromShadowBuffer(g_areaToRedraw);
    vga::setVideoModeR0W2();

    if (mouse::g_state.mode == &mouse::g_modeManagement)
        mouse::g_state.mode->drawFn();
}

/* 17bf:0599 */
void scheduleRailRedrawing(const Rail& r)
{
    g_areaToRedraw = g_chunkBoundingBoxes[r.type];
    g_areaToRedraw.x1 += r.x;
    g_areaToRedraw.y1 += r.y;
    g_areaToRedraw.x2 += r.x;
    g_areaToRedraw.y2 += r.y;
}

/* 17bf:05dc */
void clampRectToGameFieldBoundaries(Rectangle& r)
{
    if (r.y1 < g_headerHeight)
        r.y1 = g_headerHeight;
    if (r.y2 > g_footerYPos)
        r.y2 = g_footerYPos;
    if (r.x1 < 0)
        r.x1 = 0;
    if (r.x2 > SCREEN_WIDTH)
        r.x2 = SCREEN_WIDTH;
}

/* 137c:006f */
void drawRailBg1(std::int16_t tileX, std::int16_t tileY,
                 std::int16_t railType, Color color, std::int16_t yOffset)
{
    assert(railType >= 0 && static_cast<std::size_t>(railType) <= std::size(railBackgrounds));
    RailGlyph* rg = railBackgrounds[railType].bg1;
    drawGlyphAlignX8(&rg->glyph, (tileX - tileY) * 88 + rg->dx + 320,
                     (tileX + tileY) * 21 + rg->dy + yOffset - 22, color);
}

/* 137c:00d2 */
void drawRailBg2(std::int16_t tileX, std::int16_t tileY,
                 std::int16_t railType, Color color, std::int16_t yOffset)
{
    assert(railType >= 0 && static_cast<std::size_t>(railType) <= std::size(railBackgrounds));
    RailGlyph* rg = railBackgrounds[railType].bg2;
    drawGlyphAlignX8(&rg->glyph, (tileX - tileY) * 88 + rg->dx + 320,
                     (tileX + tileY) * 21 + rg->dy + yOffset - 22, color);
}

/* 137c:000c */
void drawRail(std::int16_t tileX, std::int16_t tileY,
              std::int16_t railType, Color color, std::int16_t yOffset)
{
    assert(railType >= 0 && static_cast<std::size_t>(railType) <= std::size(railBackgrounds));
    RailGlyph* rg = railBackgrounds[railType].mainGlyph;
    drawGlyphAlignX8(&rg->glyph, (tileX - tileY) * 88 + rg->dx + 320,
                     (tileX + tileY) * 21 + rg->dy + yOffset - 22, color);
}

/* 17bf:0cd0 */
void drawFieldBackground(std::int16_t yOffset)
{
    for (const RailInfo* r = g_railRoad; r < g_railRoad + g_railRoadCount; ++r) {
        drawRailBg1(r->tileX, r->tileY, r->railType, Color::DarkGray, yOffset);
        drawRailBg2(r->tileX, r->tileY, r->railType, Color::Gray, yOffset);
    }
    for (const RailInfo* r = g_railRoad; r < g_railRoad + g_railRoadCount; ++r) {
        drawRail(r->tileX, r->tileY, r->railType, Color::Black, yOffset);
        drawImpasse(g_rails[r->tileX][r->tileY][r->railType], yOffset);
    }

    for (std::uint16_t i = 0; i < g_nSwitches; ++i)
        drawSwitch(i, false);

    drawStaticObjects(yOffset);

    for (std::uint16_t i = 0; i < g_semaphoreCount; ++i)
        drawSemaphore(g_semaphores[i], yOffset);

    scheduleAllTrainsRedrawing();
    drawHeaderFieldFontTexture();
}

/* 132d:004b */
void fillGameFieldBackground(std::int16_t yOffset)
{
    graphics::filledRectangle(0, 49 + yOffset, 80, 285, 0xFF, Color::Green);
}

/* 15e8:09c8 */
void drawWorld()
{
    fillGameFieldBackground(350);

    for (const RailInfo* r = g_railRoad; r < g_railRoad + g_railRoadCount; ++r) {
        drawRailBg1(r->tileX, r->tileY, r->railType, Color::DarkGray, 350);
        drawRailBg2(r->tileX, r->tileY, r->railType, Color::Gray, 350);
    }
    for (const RailInfo* r = g_railRoad; r < g_railRoad + g_railRoadCount; ++r) {
        drawRail(r->tileX, r->tileY, r->railType, Color::Black, 350);
        drawImpasse(g_rails[r->tileX][r->tileY][r->railType], 350);
    }

    for (std::uint16_t i = 0; i < g_nSwitches; ++i)
        drawSwitchNoBackup(i, 350);

    drawStaticObjects(350);

    for (std::uint16_t i = 0; i < g_semaphoreCount; ++i) {
        const Semaphore& s = g_semaphores[i];
        if (!s.isRightDirection)
            drawSemaphore(g_semaphores[i], 350);
    }

    scheduleTrainsDrawing();
    drawTrains();

    for (std::uint16_t i = 0; i < g_semaphoreCount; ++i) {
        const Semaphore& s = g_semaphores[i];
        if (s.isRightDirection)
            drawSemaphore(g_semaphores[i], 350);
    }

    drawHeaderBackground(350);
    drawHeaderData(g_headers[static_cast<std::size_t>(HeaderFieldId::Trains)].value,
                   g_headers[static_cast<std::size_t>(HeaderFieldId::Money)].value,
                   g_headers[static_cast<std::size_t>(HeaderFieldId::Year)].value,
                   g_headers[static_cast<std::size_t>(HeaderFieldId::Level)].value, 350);
    drawDispatchers(350);

    drawStatusBarWithCopyright(350);
}

/* 132d:01e2 */
void drawEraseTrainFinishedExclamation(std::int16_t entranceX, std::int16_t entranceY)
{
    // Uses XOR operation when drawing
    // => this allows us to use the same code to draw and erase the image

    std::int16_t x;
    const Glyph* glyph;
    if (entranceX < 320) {
        x = 8;
        glyph = &g_glyphTrainFinishedLeftEntrance;
    } else {
        x = 611;
        glyph = &g_glyphTrainFinishedRightEntrance;
    }

    vga::setDataRotation(0x18); // rotation = 0, mode = XOR
    drawGlyphAlignX8(glyph, x, entranceY - 9, Color::White);
    vga::setDataRotation(0); // default mode
}

/* 132d:00f9 */
void drawGameField(std::int16_t yOffset)
{
    readIfNotLoaded("play.7", g_pageBuffer);
    graphics::imageDot7(0, yOffset, 640, 350, g_pageBuffer);
    drawStaticObjects(yOffset);
    drawCopyright(yOffset);
}

} // namespace resl
