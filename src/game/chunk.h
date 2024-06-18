#pragma once

#include <cstdint>

namespace resl {

struct Chunk;

inline static Chunk* const specialChunkPtr = reinterpret_cast<Chunk*>(1);

struct ChunkReference {
    Chunk* chunk;
    std::uint16_t slot;
};

struct Chunk {
    std::int16_t x;
    std::int16_t y;
    std::uint8_t type;
    std::uint8_t minPathStep;
    std::uint8_t maxPathStep;
    std::int8_t semSlotIdByDirection[3];
    ChunkReference x_neighbours[2];
};

//-----------------------------------------------------------------------------

/* 262d:259a : 11880 bytes */
extern Chunk g_chunks[11][11][6]; /* [tileX][tileY][railType] */

/* 262d:6954 : 1440 bytes */
extern Chunk x_chunks_1[80];

/* 262d:6ef4 : 4 bytes */
extern ChunkReference x_unknownChunkConst;

} // namespace resl
