#pragma once

#include "types/chunk.h"
#include "types/rectangle.h"

#include <cstdint>
#include <utility>

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
    char _padding;
    std::int16_t year;
    std::int16_t lastMovementTime;

    Carriage carriages[5];

    Location head;
    Location tail;
};

/* 262d:5f02 : 2640 bytes */
extern Train g_trains[20];

/* 262d:6fd2 : 2 bytes */
extern std::uint16_t g_collidedTrainsArrayLen;

/* 262d:7000 : 80 bytes */
extern std::pair<Carriage*, Carriage*> g_collidedTrainsArray[20];

/* 262d:6fd4 : 2 bytes */
extern std::uint16_t g_trainDrawingChainLen;

/* 262d:6fd6 : 40 bytes */
extern Carriage* g_trainDrawingChains[20];

/* 262d:6ffe : 1 byte */
extern bool g_needToRedrawTrains;

/* 262d:7050 : 800 bytes */
extern Rectangle g_carriagesBoundingBoxes[100];

//-----------------------------------------------------------------------------

/* 146b:0153 */
void initTrains();

/* 146b:018c */
void resetCarriages();

/* 18fa:000b */
void scheduleTrainsDrawing();

/* 19de:0841 */
void scheduleAllTrainsRedrawing();

/* 18fa:08d6 */
void drawTrainList(Carriage*);

/* 18fa:0b73 */
void drawTrains();

/* 132d:0002 */
void eraseTrain(const Train&);

/* 1a65:0256 */
Train* spawnServer(std::int16_t entranceIdx);

} // namespace resl
