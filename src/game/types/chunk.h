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

} // namespace resl
