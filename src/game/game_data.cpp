#include "game_data.h"

#include "types/chunk.h"
#include "types/rail_info.h"
#include "types/static_object.h"

namespace resl {

/* 262d:21d0 : 2 byte */
std::uint16_t chunksLoadedOffset;

/* 262d:21ce : 2 byte */
std::uint16_t entrancesLoadedOffset;

/* 1d7d:01e8 : 20 byte */
char playerName[20];

/* 262d:21da : 960 bytes */
StaticObject g_staticObjects[120];

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

/* 262d:6ef4 : 4 bytes */
ChunkReference x_unknownChunkConst;

} // namespace resl
