#include "load_game.h"

// IWYU pragma: no_include <sys/_types/_seek_set.h>
// IWYU pragma: no_include <sys/fcntl.h>

#include "entrance.h"
#include "game_data.h"
#include "header.h"
#include "init.h"
#include "io_status.h"
#include "rail.h"
#include "semaphore.h"
#include "static_object.h"
#include "switch.h"
#include "train.h"
#include "types/rail_info.h"
#include "types/rectangle.h"
#include <system/filesystem.h>
#include <system/time.h>

#include <fcntl.h> // IWYU pragma: keep
#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

#ifndef O_BINARY
#   define O_BINARY 0
#endif

namespace resl {

/* sizeof(EntranceInfo) can be different, because we have different size of
 * pointer */
constexpr std::size_t fileEntranceInfoSize = 0x16;
/* offsetof(EntranceInfo, obj) */
constexpr std::size_t fileEntranceInfoObjOffset = 4;
/* sizeof(Chunk) */
constexpr std::size_t fileChunkSize = 0x12;
/* sizeof(Chunk) */
constexpr std::size_t fileChunkArraySize = fileChunkSize * 6 * 10 * 11;

static Rail* fixLoadedRailPtr(Rail* p)
{
    assert(
        reinterpret_cast<std::intptr_t>(p) >= g_railsLoadedOffset &&
        reinterpret_cast<std::intptr_t>(p) <= g_railsLoadedOffset + fileChunkArraySize);
    std::ptrdiff_t offset = reinterpret_cast<std::intptr_t>(p) - g_railsLoadedOffset;
    assert(offset % fileChunkSize == 0);
    return &g_rails[0][0][0] + (offset / fileChunkSize);
}

/* 1400:0061 */
static void fixLoadedLocation(Location& l)
{
    if (!l.rail)
        return;

    const std::intptr_t c = reinterpret_cast<std::intptr_t>(l.rail);
    if (c >= g_entrancesLoadedOffset + fileEntranceInfoObjOffset &&
        c <= g_entrancesLoadedOffset + fileEntranceInfoObjOffset + fileEntranceInfoSize * 5) {
        // obj is inside entrances array
        std::ptrdiff_t offset = c - (g_entrancesLoadedOffset + fileEntranceInfoObjOffset);
        assert(offset % fileEntranceInfoSize == 0);
        l.rail = &g_entrances[offset / fileEntranceInfoSize].rail;
    } else {
        // obj is inside objects array
        l.rail = fixLoadedRailPtr(l.rail);
    }
}

static void checkRead(int fd, void* ptr, std::size_t len)
{
    ssize_t res = read(fd, ptr, len);
    assert(res == len);
}

template <typename T>
inline T* readPtr(int fd)
{
    std::uint16_t value;
    checkRead(fd, &value, sizeof(value));
    return reinterpret_cast<T*>(static_cast<std::uintptr_t>(value));
}

/* 1400:041c */
static void loadGameState(const char* fileName, void* switchStates, void* semaphoresIsRed)
{
    /* The original game just reads entire structures or even arrays or structures from a file.
       But obviously, this is not a portable approach:
            1) storing pointers in a file is a wierd and dangerous idea (but they do it and they adjast these pointer after loading)
            2) different platforms have different pointer sizes, alignment of structures elements,
               byte order, etc.

        TODO read field by field instead of reading entire structures/parts of structures
       */
    int fd = open(fileName, O_BINARY | O_RDONLY);
    if (fd == -1) [[unlikely]]
        ioStatus = OpenError;
    else {
        static_assert(
            sizeof(g_railsLoadedOffset) == 2 && sizeof(g_entrancesLoadedOffset) == 2 &&
            sizeof(g_playerName) == 0x14 && sizeof(g_headers) == 0x48 && sizeof(g_entranceCount) == 2 &&
            sizeof(g_staticObjects) == 0x3c0 && sizeof(g_railRoadCount) == 2 &&
            sizeof(g_railRoad[0]) == 6);

        checkRead(fd, &g_railsLoadedOffset, 2);
        checkRead(fd, &g_entrancesLoadedOffset, 2);
        checkRead(fd, g_playerName, 0x14);
        checkRead(fd, &g_headers, 0x48);
        checkRead(fd, &g_entranceCount, 2);

        if constexpr (sizeof(void*) == 2) {
            // static_assert(sizeof(entrances) == 0x84);
            checkRead(fd, g_entrances, 0x84);
        } else {
            off_t pos = lseek(fd, 0, SEEK_CUR);
            for (std::size_t i = 0; i < NormalEntranceCount; ++i) {
                checkRead(fd, &g_entrances[i], 4);
                checkRead(fd, &g_entrances[i].rail, 10);
                for (int j = 0; j < 2; ++j) {
                    g_entrances[i].rail.connections[j].rail = readPtr<Rail>(fd);
                    checkRead(fd, &g_entrances[i].rail.connections[j].slot, 2);
                }
            }
            off_t curPos = lseek(fd, 0, SEEK_CUR);
            assert(curPos - pos == 0x84);
        }

        checkRead(fd, g_staticObjects, 0x3c0);

        if constexpr (sizeof(void*) == 2) {
            // static_assert(sizeof(trains) == 0xa50);
            read(fd, &g_trains[0], 0xa50);
        } else {
            off_t pos = lseek(fd, 0, SEEK_CUR);
            for (Train& train : g_trains) {
                checkRead(fd, &train, 14);
                for (Carriage& c : train.carriages) {
                    c.next = readPtr<Carriage>(fd);
                    checkRead(fd, &c.drawingPriority, 2);
                    c.train = readPtr<Train>(fd);
                    checkRead(fd, &c.dstEntranceIdx, 4);
                    checkRead(fd, &c.location, 2);
                    c.location.rail = readPtr<Rail>(fd);
                    checkRead(fd, &c.rect, sizeof(Rectangle));
                }

                checkRead(fd, &train.head, 2);
                assert(train.head.forwardDirection >= 0 && train.head.forwardDirection <= 1);
                train.head.rail = readPtr<Rail>(fd);
                checkRead(fd, &train.tail, 2);
                assert(train.tail.forwardDirection >= 0 && train.tail.forwardDirection <= 1);
                train.tail.rail = readPtr<Rail>(fd);
            }
            off_t curPos = lseek(fd, 0, SEEK_CUR);
            assert(curPos - pos == 0xA50);
        }

        checkRead(fd, &g_railRoadCount, 2);
        checkRead(fd, g_railRoad, g_railRoadCount * 6);

        std::int16_t cnt;
        checkRead(fd, &cnt, 2);
        checkRead(fd, switchStates, cnt);
        checkRead(fd, &cnt, 2);
        checkRead(fd, semaphoresIsRed, cnt);

        fd = close(fd);
        if (fd) [[unlikely]]
            ioStatus = CloseError;
    }
}

/* 1c71:000f */
IOStatus x_smthRelatedToKeyboard(IOStatus errCode)
{
    // TODO research & implement
    return errCode;
}

/* 1400:009c */
IOStatus loadSavedGame(const char* fileName)
{
    char switchStatesBuf[100];
    char semaphoresIsRed[100];

    ioStatus = NoError;
    resetGameData();
    loadGameState(fileName, switchStatesBuf, semaphoresIsRed);

    if (ioStatus == NoError) {
        for (Train& train : g_trains) {
            fixLoadedLocation(train.head);
            fixLoadedLocation(train.tail);
            for (int i = 0; i < 5; ++i) {
                train.carriages[i].train = &train;
                fixLoadedLocation(train.carriages[i].location);
            }
            train.lastMovementTime = getTime();
        }

        for (std::size_t i = 0; i < NormalEntranceCount; ++i) {
            g_entrances[i].rail.connections[0].rail =
                fixLoadedRailPtr(g_entrances[i].rail.connections[0].rail);
            Rail& r = *g_entrances[i].rail.connections[0].rail;
            r.connections[isVisible(r)].rail = &g_entrances[i].rail;
        }
        for (RailInfo* road = g_railRoad; road < g_railRoad + g_railRoadCount; ++road) {
            createSwitches(*road);
            createSemaphores(*road);
        }
        for (int i = 0; i < g_nSwitches; ++i) {
            Switch& s = g_switches[i];
            if ((s.entry.rail < s.disabledPath.rail) != switchStatesBuf[i])
                toggleSwitch(s);
        }
        for (int i = 0; i < g_semaphoreCount; ++i)
            g_semaphores[i].isRed = semaphoresIsRed[i];
    }
    x_smthRelatedToKeyboard(ioStatus);
    return ioStatus;
}

/* 174e:052b */
std::int16_t readLevel()
{
    // TODO implement
    return 1;
}

/* 1400:0638 */
int findNextSaveFile()
{
    /* 1d7d:01a8 : 1 byte */
    static bool g_searchStarted = false;

    int err;
    if (!g_searchStarted) {
        err = findFirst("????????.??_", 0);
        if (!err)
            g_searchStarted = true;
    } else {
        err = findNext();
        if (err) {
            g_searchStarted = false;
            err = findNextSaveFile();
        }
    }
    return err;
}

} // namespace resl
