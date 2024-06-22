#pragma once

#include "rail.h"
#include <graphics/color.h>

#include <cstdint>

namespace resl {

enum StaticObjectKind : std::uint8_t {
    None = 0,
    BuildingHouse = 1,
    House = 2,
    Tree = 3
};

struct StaticObject {
    std::int16_t x;
    std::int16_t y;
    StaticObjectKind kind;
    std::uint8_t type;
    Color color;
    std::uint8_t _yearsElapsed;
};

static_assert(sizeof(StaticObject) == 0x8);

//-----------------------------------------------------------------------------

/* 262d:21da : 960 bytes */
extern StaticObject g_staticObjects[120];

// "g_cuttingDownStaticObjectsByKind[type]" contains how many objects of a
// type "type" were destroyed to build a rail.
/* 262d:6f5a : 4 bytes */
extern std::uint8_t g_cuttingDownStaticObjectsByKind[4];

//-----------------------------------------------------------------------------

/* 1530:0229 */
void drawStaticObjects(std::int16_t yOffset);

/* 1530:0328 */
void eraseStaticObject(const StaticObject&, std::int16_t yOffset);

/* 17bf:04ac */
void destroyStaticObjectsForRailConstruction(const Rail&);

} // namespace resl
