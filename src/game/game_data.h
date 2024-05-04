#pragma once

#include "types/chunk.h"
#include "types/entrance_info.h"
#include "types/header_field.h"
#include "types/rail_info.h"
#include "types/semaphore.h"
#include "types/switch.h"
#include "types/train.h"
#include "types/tree_info.h"

#include <cstdint>
#include <utility>

namespace resl {

/* 262d:21d0 : 2 byte */
extern std::uint16_t chunksLoadedOffset;

/* 262d:21ce : 2 byte */
extern std::uint16_t entrancesLoadedOffset;

/* 1d7d:01e8 : 20 byte */
extern char playerName[20];

/* 1d7d:00f0 : 0x48 bytes */
extern Headers g_headers;

/* 262d:5f00 : 2 bytes */
extern std::int16_t entranceCount;

/* 1d7d:01fc : 132 bytes */
extern EntranceInfo entrances[6];

/* 262d:21da : 960 bytes */
extern StaticObject g_staticObjects[120];

/* 262d:5f02 : 2640 bytes */
extern Train trains[20];

/* 262d:6952 : 2 bytes */
extern std::uint16_t g_railRoadCount;

/* 262d:5b02 : 900 bytes */
extern RailInfo g_railRoad[150];

/* 262d:259a : 11880 bytes */
extern Chunk g_chunks[11][11][6]; /* [tileX][tileY][railType] */

/* 262d:5e86 : 121 bytes */
extern std::uint8_t railroadTypeMasks[11][11];

/* 262d:6954 : 1440 bytes */
extern Chunk x_chunks_1[80];

/* 262d:21d4 : 2 bytes */
extern std::uint16_t g_nSwitches;

/* 262d:21d6 : 2 bytes */
extern std::uint16_t g_semaphoreCount;

/* 262d:6ef4 : 4 bytes */
extern ChunkReference x_unknownChunkConst;

/* 262d:6954 : 1440 bytes */
extern Switch g_switches[80];

/* 262d:6f9c : 48 bytes */
extern Semaphore x_newSemaphores[4];

/* 262d:6f60 : 48 bytes */
extern Semaphore x_erasedSemaphores[4];

/* 262d:6f90 : 2 bytes */
extern std::int16_t x_newSemaphoreCount;

/* 262d:6f92 : 2 bytes */
extern std::int16_t x_erasedSemaphoreCount;

/* 262d:58aa : 600 bytes */
extern Semaphore g_semaphores[50];


/* 262d:6fd2 : 2 bytes */
extern std::uint16_t g_orderArrayLen;

/* 262d:7000 : 80 bytes */
extern std::pair<const Carriage*, const Carriage*> x_orderArray[20];

/* 262d:6fd4 : 2 bytes */
extern std::uint16_t g_trainDrawingChainLen;

/* 262d:6fd6 : 40 bytes */
extern Carriage* g_trainDrawingChains[20];

} // namespace resl
