#pragma once

#include "types/chunk.h"
#include "types/rectangle.h"
#include <tasks/task.h>

#include <array>
#include <cstdint>
#include <utility>

namespace resl {

enum CarriageType : std::uint8_t {
    Server = 0,
    AncientLocomotive = 1,
    SteamLocomotive = 2,
    Trolley = 3,
    DieselLocomotive = 4,
    ElectricLocomotive = 5,
    HighSpeedLocomotive = 6,

    AncientPassengerCarriage = 7,
    PassengerCarriage = 8,
    HighSpeedPassengerCarriage = 9,
    OpenFreightCarriage = 10,
    CoveredFreightCarriage = 11,
    PocketWagon = 12,
    TankWagon = 13,

    // special type - a crash marker
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
    std::uint8_t speed;
    std::uint8_t maxSpeed;
    std::uint8_t headCarriageIdx;
    std::uint8_t x_speed;
    char _padding;
    std::int16_t year;
    std::int16_t lastMovementTime;

    Carriage carriages[5];

    Location head;
    Location tail;
};

//-----------------------------------------------------------------------------

/* 262d:5f02 : 2640 bytes */
extern std::array<Train, 20> g_trains;

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

/* 16a6:08bb */
void tryRunWaitingTrains();

/* 1a65:0030 */
Task taskSpawnTrains();

/* 1a65:0256 */
Train* spawnServer(std::int16_t entranceIdx);

/* 16a6:06e0 */
void accelerateTrains(std::int16_t count);

/* 16a6:0893 */
std::int16_t waitingTrainsCount();

} // namespace resl
