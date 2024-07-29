#include "load_game.h"

// IWYU pragma: no_include <cwchar>

#include "common.h"
#include <game/entrance.h>
#include <game/game_data.h>
#include <game/header.h>
#include <game/init.h>
#include <game/io_status.h>
#include <game/keyboard.h>
#include <game/rail.h>
#include <game/semaphore.h>
#include <game/static_object.h>
#include <game/switch.h>
#include <game/train.h>
#include <game/types/header_field.h>
#include <game/types/rail_info.h>
#include <game/types/rectangle.h>
#include <graphics/color.h>
#include <system/filesystem.h>
#include <system/time.h>
#include <utility/endianness.h>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <type_traits>

#ifndef NDEBUG
#   include <algorithm>
#   include <iterator>
#endif // !NDEBUG

namespace resl {

namespace {

    inline void validateEnum(Color c)
    {
#ifndef NDEBUG
        switch (c) {
        case Color::Green:
        case Color::Black:
        case Color::Gray:
        case Color::DarkGray:
        case Color::White:
        case Color::Yellow:
        case Color::Brown:
        case Color::Blue:
        case Color::DarkBlue:
        case Color::Red:
        case Color::DarkRed:
        case Color::Cyan:
        case Color::DarkCyan:
        case Color::LightGreen:
        case Color::DarkGreen:
        case Color::BWBlinking:
            return;
        }
        assert(false);
#else
        (void)c;
#endif
    }

    inline void validateEnum(StaticObjectKind s)
    {
#ifndef NDEBUG
        switch (s) {
        case StaticObjectKind::None:
        case StaticObjectKind::BuildingHouse:
        case StaticObjectKind::House:
        case StaticObjectKind::Tree:
            return;
        }
        assert(false);
#else
        (void)s;
#endif
    }

    inline void validateEnum(CarriageType c)
    {
#ifndef NDEBUG
        switch (c) {
        case CarriageType::Server:
        case CarriageType::AncientLocomotive:
        case CarriageType::SteamLocomotive:
        case CarriageType::Trolley:
        case CarriageType::DieselLocomotive:
        case CarriageType::ElectricLocomotive:
        case CarriageType::HighSpeedLocomotive:
        case CarriageType::AncientPassengerCarriage:
        case CarriageType::PassengerCarriage:
        case CarriageType::HighSpeedPassengerCarriage:
        case CarriageType::OpenFreightCarriage:
        case CarriageType::CoveredFreightCarriage:
        case CarriageType::PocketWagon:
        case CarriageType::TankWagon:
        case CarriageType::CrashedTrain:
            return;
        }
        assert(false);
#else
        (void)c;
#endif
    }

    template <typename T, typename Enable = void>
    class DataReader;

    class Reader {
    public:
        Reader(std::FILE* f)
            : m_file(f)
        {
        }

        template <typename T>
        T read()
        {
            return DataReader<T>::read(*this);
        }

        void readBytes(char* dst, std::size_t n)
        {
            [[maybe_unused]] std::size_t cnt = std::fread(dst, 1, n, m_file);
            // the original game also has no check if data was succesfully read
            assert(cnt == n);
        }

        // The original game stores the pointers in the save file, but uses
        // them as handles (adjusts them after loading to get the correct
        // data pointers).
        // So, we have the same logic with only one difference:
        // we read 16 bits (the size of the pointer in the save file) and
        // store them into native pointer (that can be bigger than 16 bits).
        template <typename T>
        T* readPtr()
        {
            std::uint16_t dosPtr = read<std::uint16_t>();
            return reinterpret_cast<T*>(dosPtr);
        }

        template <std::size_t N>
        void skipPadding()
        {
            char buf[N];
            readBytes(buf, N);
            assert(std::find_if(std::begin(buf), std::end(buf),
                                [](char c) { return c; }) == std::end(buf));
        }

#ifndef NDEBUG
        // debug helper to make sure we have read the correct number of bytes
        class [[nodiscard]] ExpectedBlockSize {
        public:
            ExpectedBlockSize(std::FILE* f, long expected)
                : m_expected(expected)
                , m_file(f)
            {
                m_startOffset = std::ftell(m_file);
            }

            ~ExpectedBlockSize()
            {
                long offset = std::ftell(m_file);
                assert(offset - m_startOffset == m_expected);
            }

        private:
            long m_expected;
            long m_startOffset;
            std::FILE* m_file;
        };

        ExpectedBlockSize expectedSize(long expected)
        {
            return ExpectedBlockSize(m_file, expected);
        }

#else // NDEBUG

        class [[maybe_unused]] ExpectedBlockSize { };
        ExpectedBlockSize expectedSize(long) { return {}; }

#endif // !NDEBUG

    private:
        std::FILE* m_file;
    };

    template <typename T>
    class DataReader<T, std::enable_if_t<std::is_integral_v<T>>> {
    public:
        static T read(Reader& r)
        {
            T res;
            r.readBytes(reinterpret_cast<char*>(&res), sizeof(res));
            return littleEndianToNative(res);
        }
    };

    template <>
    class DataReader<bool> {
    public:
        static bool read(Reader& r)
        {
            std::uint8_t v = r.read<std::uint8_t>();
            assert(v == 0 || v == 1);
            return static_cast<bool>(v);
        }
    };

    template <typename T>
    class DataReader<T, std::enable_if_t<std::is_enum_v<T>>> {
    public:
        static T read(Reader& r)
        {
            using U = std::underlying_type_t<T>;
            U val = r.read<U>();
            validateEnum(static_cast<T>(val));
            return static_cast<T>(val);
        }
    };

    template <>
    class DataReader<Location> {
    public:
        static Location read(Reader& r)
        {
            Location res;
            res.pathStep = r.read<std::uint8_t>();
            res.forwardDirection = r.read<std::uint8_t>();
            res.rail = r.readPtr<Rail>();
            return res;
        }
    };

} // namespace

static Rail* fixLoadedRailPtr(Rail* p)
{
    assert(
        reinterpret_cast<std::uintptr_t>(p) >= g_railsLoadedOffset &&
        reinterpret_cast<std::uintptr_t>(p) <= g_railsLoadedOffset + fileRailArraySize);
    std::ptrdiff_t offset = reinterpret_cast<std::uintptr_t>(p) - g_railsLoadedOffset;
    assert(offset % fileRailSize == 0);
    return &g_rails[0][0][0] + (offset / fileRailSize);
}

/* 1400:0061 */
static void fixLoadedLocation(Location& l)
{
    if (!l.rail)
        return;

    const std::uintptr_t c = reinterpret_cast<std::uintptr_t>(l.rail);
    if (c >= g_entrancesLoadedOffset + fileEntranceRailOffset &&
        c <= g_entrancesLoadedOffset + fileEntranceRailOffset + fileEntranceInfoSize * 5) {
        // obj is inside entrances array
        std::ptrdiff_t offset = c - (g_entrancesLoadedOffset + fileEntranceRailOffset);
        assert(offset % fileEntranceInfoSize == 0);
        l.rail = &g_entrances[offset / fileEntranceInfoSize].rail;
    } else {
        // obj is inside objects array
        l.rail = fixLoadedRailPtr(l.rail);
    }
}

/* 1400:041c */
static void loadGameState(const char* fileName, char* switchStates, char* semaphoresAreRed)
{
    /*
        The original game just reads entire structures or even arrays or
        structures from the file:

        >> read(fd, &g_railsLoadedOffset, 2);
        >> read(fd, &g_entrancesLoadedOffset, 2);
        >> read(fd, g_playerName, 0x14);
        >> read(fd, &g_headers, 0x48);
        >> read(fd, &g_entranceCount, 2);
        >> read(fd, g_entrances, 0x84);
        >> read(fd, g_staticObjects, 0x3c0);
        >> read(fd, &g_trains[0], 0xa50);
        >> read(fd, &g_railRoadCount, 2);
        >> read(fd, g_railRoad, g_railRoadCount * 6);

        >> std::int16_t cnt;
        >> read(fd, &cnt, 2);
        >> read(fd, switchStates, cnt);
        >> read(fd, &cnt, 2);
        >> read(fd, semaphoresIsRed, cnt);

        But obviously, this is not a portable solution:
            1) storing pointers in a file is a wierd and dangerous idea
                (but they do it and they adjast these pointer after loading)
            2) different platforms have different pointer sizes, alignment
                of structures elements, byte order, etc.

       So, we will use portable approach instead - will read values field by
       field and convert the byte order from little endian to native
       representation.
    */
    std::FILE* file = std::fopen(fileName, "rb");
    if (!file) [[unlikely]]
        g_ioStatus = IOStatus::OpenError;
    else {
        Reader r(file);

        {
            auto hdrSize = r.expectedSize(3830);
            {
                auto bs = r.expectedSize(24);
                g_railsLoadedOffset = r.read<std::uint16_t>();
                g_entrancesLoadedOffset = r.read<std::uint16_t>();
                r.readBytes(g_playerName, 20);
            }
            {
                auto bs = r.expectedSize(72);
                for (HeaderField& fld : g_headers) {
                    fld.x = r.read<std::int16_t>();
                    fld.y = r.read<std::int16_t>();
                    fld.valueLimit = r.read<std::int16_t>();
                    fld.nDigits = r.read<std::int8_t>();
                    fld.curAnimatingDigit = r.read<std::int8_t>();
                    fld.value = r.read<std::int16_t>();
                    fld.yScroll = r.read<std::int16_t>();
                    for (std::uint8_t& v : fld.digitValues)
                        v = r.read<std::int8_t>();
                }
            }
            {
                auto bs = r.expectedSize(134);
                g_entranceCount = r.read<std::int16_t>();
                for (std::size_t i = 0; i < NormalEntranceCount; ++i) {
                    Entrance& e = g_entrances[i];
                    e.bgColor = r.read<Color>();
                    e.fgColor = r.read<Color>();
                    e.entranceRailInfoIdx = r.read<std::uint8_t>();
                    e.waitingTrainsCount = r.read<std::uint8_t>();
                    e.rail.x = r.read<std::int16_t>();
                    e.rail.y = r.read<std::int16_t>();
                    e.rail.type = r.read<std::int8_t>();
                    e.rail.minPathStep = r.read<std::int8_t>();
                    e.rail.maxPathStep = r.read<std::int8_t>();
                    for (std::int8_t& s : e.rail.semSlotIdByDirection)
                        s = r.read<std::int8_t>();
                    for (RailConnection& rc : e.rail.connections) {
                        rc.rail = r.readPtr<Rail>();
                        rc.slot = r.read<std::uint16_t>();
                    }
                }
            }
            {
                auto bs = r.expectedSize(960);
                for (StaticObject& so : g_staticObjects) {
                    so.x = r.read<std::int16_t>();
                    so.y = r.read<std::int16_t>();
                    so.kind = r.read<StaticObjectKind>();
                    so.type = r.read<std::uint8_t>();
                    so.color = r.read<Color>();
                    so.creationYear = r.read<std::uint8_t>();
                }
            }
            {
                auto bs = r.expectedSize(2640);
                for (Train& t : g_trains) {
                    t.isFreeSlot = r.read<bool>();
                    t.carriageCnt = r.read<std::uint8_t>();
                    t.drawingChainIdx = r.read<std::uint8_t>();
                    t.needToRedraw = r.read<bool>();
                    t.isActualPosition = r.read<bool>();
                    t.speed = r.read<std::uint8_t>();
                    t.maxSpeed = r.read<std::uint8_t>();
                    t.headCarriageIdx = r.read<std::uint8_t>();
                    t.movementDebt = r.read<std::uint8_t>();
                    r.skipPadding<1>();
                    t.year = r.read<std::int16_t>();
                    t.lastMovementTime = r.read<std::int16_t>();
                    for (Carriage& c : t.carriages) {
                        // The value is a garbage (broken pointer). This is not
                        // a problem since the value will be overwritten before
                        // use, but we won't store it for better safety
                        r.read<std::uint16_t>();
                        c.next = nullptr;

                        c.drawingPriority = r.read<std::int16_t>();

                        // This pointer is also not used after loading and
                        // contains a broken value.
                        r.read<std::uint16_t>();
                        c.train = nullptr;

                        c.dstEntranceIdx = r.read<std::uint8_t>();
                        c.type = r.read<CarriageType>();
                        c.direction = r.read<std::uint8_t>();
                        c.x_direction = r.read<std::uint8_t>();
                        c.location = r.read<Location>();
                        c.rect.x1 = r.read<std::int16_t>();
                        c.rect.y1 = r.read<std::int16_t>();
                        c.rect.x2 = r.read<std::int16_t>();
                        c.rect.y2 = r.read<std::int16_t>();
                    }
                    t.head = r.read<Location>();
                    t.tail = r.read<Location>();
                }
            }
        }

        g_railRoadCount = r.read<std::uint16_t>();
        for (std::uint16_t i = 0; i < g_railRoadCount; ++i) {
            auto bs = r.expectedSize(6);
            RailInfo& ri = g_railRoad[i];
            ri.roadTypeMask = r.read<std::uint8_t>();
            ri.tileX = r.read<std::uint8_t>();
            ri.tileY = r.read<std::uint8_t>();
            ri.railType = r.read<std::uint8_t>();
            ri.year_8 = r.read<std::uint8_t>();
            r.skipPadding<1>();
        }

        std::uint16_t cnt = r.read<std::uint16_t>();
        r.readBytes(switchStates, cnt);
        cnt = r.read<std::uint16_t>();
        r.readBytes(semaphoresAreRed, cnt);

#ifndef NDEBUG
        // make sure we have read entire file
        char c;
        assert(std::fread(&c, 1, 1, file) == 0);
#endif // !NDEBUG

        if (std::fclose(file) != 0) [[unlikely]]
            g_ioStatus = IOStatus::CloseError;
    }
}

/* 1400:009c */
IOStatus loadSavedGame(const char* fileName)
{
    char switchStatesBuf[100];
    char semaphoresIsRed[100];

    g_ioStatus = IOStatus::NoError;
    resetGameData();
    loadGameState(fileName, switchStatesBuf, semaphoresIsRed);

    if (g_ioStatus == IOStatus::NoError) {
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
        for (RailInfo* r = g_railRoad; r < g_railRoad + g_railRoadCount; ++r) {
            connectRail(*r);
            updateSemaphores(*r);
        }
        for (std::uint16_t i = 0; i < g_nSwitches; ++i) {
            Switch& s = g_switches[i];
            if ((s.entry.rail < s.disabledPath.rail) != switchStatesBuf[i])
                toggleSwitch(s);
        }
        for (std::uint16_t i = 0; i < g_semaphoreCount; ++i)
            g_semaphores[i].isRed = semaphoresIsRed[i];
    }
    updateKeyboardLeds(static_cast<std::int16_t>(g_ioStatus));
    return g_ioStatus;
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
