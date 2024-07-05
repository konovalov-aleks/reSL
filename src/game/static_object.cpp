#include "static_object.h"

#include "draw_header.h"
#include "drawing.h"
#include "rail.h"
#include "resources/glyph_empty_background.h"
#include "resources/movement_paths.h"
#include "resources/static_object_glyph.h"
#include "status_bar.h"
#include "types/rectangle.h"
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/glyph.h>
#include <graphics/vga.h>
#include <system/random.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace resl {

/* 262d:21da : 960 bytes */
StaticObject g_staticObjects[120];

/* 262d:6f5a : 4 bytes */
std::uint8_t g_cuttingDownStaticObjectsByKind[4];

//-----------------------------------------------------------------------------

/* 1530:0203 */
inline bool isInsideGameField(int x, int y)
{
    return y >= g_headerHeight && y < g_footerYPos && x >= 0 && x < SCREEN_WIDTH;
}

/* 1530:013e */
static void drawGrass(std::int16_t yOffset)
{
    const int seed = std::rand();
    std::srand(g_staticObjects[119].x);
    for (int i = 0; i < 25; ++i) {
        int x = genRandomNumber(640);
        int y = genRandomNumber(287) + g_headerHeight;
        for (int j = 0; j < 45; ++j) {
            if (isInsideGameField(x, y)) {
                Color c = graphics::getPixel(x, y + yOffset);
                if (c == Color::Green)
                    graphics::putPixel(x, y + yOffset, Color::Black);
            }
            x += symmetricRand(20);
            y += symmetricRand(20) / 4;
        }
    }
    std::srand(seed);
}

/* 1530:0229 */
void drawStaticObjects(std::int16_t yOffset)
{
    drawGrass(yOffset);

    g_glyphHeight = 16;
    for (StaticObject& obj : g_staticObjects) {
        if (obj.kind == StaticObjectKind::House) {
            g_glyphHeight = std::min(g_footerYPos - obj.y, 16);
            drawGlyphW16(g_houseGlyphs[obj.type].bg, obj.x, obj.y + yOffset, obj.color);
            drawGlyphW16(g_houseGlyphs[obj.type].fg, obj.x, obj.y + yOffset, Color::Black);
        } else if (obj.kind == StaticObjectKind::Tree) {
            g_glyphHeight = 16;
            drawGlyphW16(g_treeGlyphs[obj.type].bg, obj.x, obj.y + yOffset, obj.color);
            drawGlyphW16(g_treeGlyphs[obj.type].fg, obj.x, obj.y + yOffset, Color::Black);
        }
    }
}

/* 1530:0328 */
void eraseStaticObject(const StaticObject& obj, std::int16_t yOffset)
{
    g_glyphHeight = 16;
    drawGlyphW16(g_glyphEmptyBackground, obj.x, obj.y + yOffset, Color::Green);
}

/* 17bf:0b96 */
static bool needRemoveObjectToBuildRail(const StaticObject& obj, const Rail& r)
{
    if (std::abs(obj.x - r.x) > 176)
        return false;

    if (std::abs(obj.y - r.y) > 42)
        return false;

    const Path& path = g_movementPaths[r.type];
    for (std::uint16_t i = 0; i < path.size; ++i) {
        std::int16_t xCoord[4];
        std::int16_t yCoord[4];

        const std::int16_t xPos = r.x + path.data[i].dx;
        xCoord[0] = xPos - 4;
        xCoord[1] = xPos + 4;

        const std::int16_t yPos = r.y + path.data[i].dy;
        yCoord[0] = yPos + -2;
        yCoord[1] = yPos + 2;

        xCoord[2] = xCoord[0];
        xCoord[3] = xCoord[1];
        yCoord[2] = yCoord[1];
        yCoord[3] = yCoord[0];

        for (std::int16_t j = 0; j < 4; ++j) {
            if (obj.x <= xCoord[j] && obj.x + 16 >= xCoord[j] &&
                obj.y <= yCoord[j] && obj.y + 16 >= yCoord[j])
                return true;
        }
    }
    return false;
}

/* 17bf:04ac */
void destroyStaticObjectsForRailConstruction(const Rail& r)
{
    g_cuttingDownStaticObjectsByKind[static_cast<int>(StaticObjectKind::Tree)] = 0;
    g_cuttingDownStaticObjectsByKind[static_cast<int>(StaticObjectKind::House)] = 0;

    for (StaticObject& obj : g_staticObjects) {
        if (!needRemoveObjectToBuildRail(obj, r))
            continue;

        switch (obj.kind) {
        case StaticObjectKind::BuildingHouse:
            obj.kind = StaticObjectKind::None;
            break;

        case StaticObjectKind::House:
        case StaticObjectKind::Tree:
            ++g_cuttingDownStaticObjectsByKind[static_cast<int>(obj.kind)];
            obj.kind = StaticObjectKind::None;
            eraseStaticObject(obj, 350);
            if (obj.x < g_areaToRedraw.x1)
                g_areaToRedraw.x1 = obj.x;
            if (obj.x + 16 > g_areaToRedraw.x2)
                g_areaToRedraw.x2 = obj.x + 16;
            if (obj.y < g_areaToRedraw.y1)
                g_areaToRedraw.y1 = obj.y;
            if (obj.y + 16 > g_areaToRedraw.y2)
                g_areaToRedraw.y2 = obj.y + 16;
            break;

        case StaticObjectKind::None:
            break;
        }
    }
}

/* 1530:056f */
void buildHouses(std::int16_t year)
{
    // The original game uses a global variable here, but I don't see any
    // external reference to the variable.
    Rectangle boundingBox;
    setEmptyRectangle(boundingBox);

    bool created = false;
    for (StaticObject& obj : g_staticObjects) {
        if (obj.kind == StaticObjectKind::BuildingHouse && obj.creationYear == year - 1800) {
            obj.kind = StaticObjectKind::House;
            if (obj.x < boundingBox.x1)
                boundingBox.x1 = obj.x;
            if (obj.x + 16 > boundingBox.x2)
                boundingBox.x2 = obj.x + 16;
            if (obj.y < boundingBox.y1)
                boundingBox.y1 = obj.y;
            if (obj.y + 16 > boundingBox.y2)
                boundingBox.y2 = obj.y + 16;
            created = true;
        }
    }

    if (created) {
        drawStaticObjects(350);
        vga::setVideoModeR0W1();
        clampRectToGameFieldBoundaries(boundingBox);
        graphics::copyFromShadowBuffer(boundingBox);
        vga::setVideoModeR0W2();
    }
}

} // namespace resl
