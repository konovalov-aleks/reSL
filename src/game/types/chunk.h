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
    std::uint16_t x;
    std::uint16_t y;
    std::uint8_t type;
    std::uint8_t x_probMinPathStep;
    std::uint8_t x_probMaxPathStep;
    std::int8_t semSlotIdByType[3];
    ChunkReference x_neighbours[2];
};

} // namespace resl
