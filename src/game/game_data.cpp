#include "game_data.h"

#include "types/chunk.h"
#include "types/rail_info.h"
#include "types/rectangle.h"
#include "types/semaphore.h"
#include "types/static_object.h"
#include "types/switch.h"
#include "types/train.h"

namespace resl {

/* 262d:21d0 : 2 byte */
std::uint16_t chunksLoadedOffset;

/* 262d:21ce : 2 byte */
std::uint16_t entrancesLoadedOffset;

/* 1d7d:01e8 : 20 byte */
char playerName[20];

/* 262d:21da : 960 bytes */
StaticObject g_staticObjects[120];

/* 262d:5f02 : 2640 bytes */
Train trains[20];

/* 262d:6952 : 2 bytes */
std::uint16_t g_railRoadCount;

/* 262d:5b02 : 900 bytes */
RailInfo g_railRoad[150];

/* 262d:259a : 11880 bytes */
Chunk g_chunks[11][11][6];

/* 262d:5e86 : 121 bytes */
std::uint8_t railroadTypeMasks[11][11];

/* 262d:6954 : 1440 bytes */
Chunk x_chunks_1[80];

/* 262d:21d4 : 2 bytes */
std::uint16_t g_nSwitches;

/* 262d:21d6 : 2 bytes */
std::uint16_t g_semaphoreCount;

/* 262d:6ef4 : 4 bytes */
ChunkReference x_unknownChunkConst;

/* 262d:6954 : 1440 bytes */
Switch g_switches[80];

/* 262d:6f9c : 48 bytes */
Semaphore x_newSemaphores[4];

/* 262d:6f60 : 48 bytes */
Semaphore x_erasedSemaphores[4];

/* 262d:6f90 : 2 bytes */
std::int16_t x_newSemaphoreCount;

/* 262d:6f92 : 2 bytes */
std::int16_t x_erasedSemaphoreCount;

/* 262d:58aa : 600 bytes */
Semaphore g_semaphores[50];

/* 262d:6fd2 : 2 bytes */
std::uint16_t g_collidedTrainsArrayLen;

/* 262d:7000 : 80 bytes */
std::pair<Carriage*, Carriage*> g_collidedTrainsArray[20];

/* 262d:6fd4 : 2 bytes */
std::uint16_t g_trainDrawingChainLen;

/* 262d:6fd6 : 40 bytes */
Carriage* g_trainDrawingChains[20];

/* 262d:6ffe : 1 byte */
bool g_needToRedrawTrains;

/* 262d:7050 : 800 bytes */
Rectangle g_carriagesBoundingBoxes[100];

} // namespace resl
