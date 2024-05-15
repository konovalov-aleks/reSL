#pragma once

#include "chunk.h"
#include "rectangle.h"

#include <cstdint>

namespace resl {

enum CarriageType : std::uint8_t {
    Server = 0,
    AncientLocomotive = 1,
    PassengerCarriage = 7,
    FreightCarriage = 10,
    CrashedTrain = 14
};

struct Location {
    std::uint8_t pathStep;
    // TODO bool?
    std::uint8_t forwardDirection;
    Chunk* chunk;
};

struct Train;

// the special value for dstEntranceIdx for blinking trains
inline constexpr std::uint8_t blinkingTrainEntranceIdx = 8;

struct Carriage {
    Carriage* next;
    std::int16_t drawingPriority;
    Train* train;
    std::uint8_t dstEntranceIdx;
    CarriageType type;
    std::uint8_t direction;
    std::uint8_t x_direction;
    Location location;
    Rectangle rect;
};

struct Train {
    bool isFreeSlot;
    std::uint8_t carriageCnt;
    std::uint8_t drawingChainIdx;
    bool needToRedraw;
    bool x_needToMove;
    std::uint8_t x_acceleration;
    std::uint8_t x_maxSpeed;
    std::uint8_t x_headCarriageIdx;
    std::uint8_t x_speed;
    char unknown2;
    std::int16_t year;
    std::int16_t x_lastMovementTime;

    Carriage carriages[5];

    Location head;
    Location tail;
};

} // namespace resl
