#include "load_game.h"

// IWYU pragma: no_include <cwchar>

#include "common.h"
#include <game/entrance.h>
#include <game/header.h>
#include <game/header_field.h>
#include <game/init.h>
#include <game/player_name.h>
#include <game/rail.h>
#include <game/rail_info.h>
#include <game/semaphore.h>
#include <game/static_object.h>
#include <game/switch.h>
#include <game/train.h>
#include <graphics/color.h>
#include <system/filesystem.h>
#include <system/keyboard.h>
#include <system/time.h>
#include <types/rectangle.h>
#include <utility/endianness.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <ranges>
#include <string>
#include <type_traits>

namespace resl {

namespace {

    [[nodiscard]] bool validateEnum(Color c)
    {
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
            return true;
        }
        return false;
    }

    [[nodiscard]] bool validateEnum(StaticObjectKind s)
    {
        switch (s) {
        case StaticObjectKind::None:
        case StaticObjectKind::BuildingHouse:
        case StaticObjectKind::House:
        case StaticObjectKind::Tree:
            return true;
        }
        return false;
    }

    [[nodiscard]] bool validateEnum(CarriageType c)
    {
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
            return true;
        }
        return false;
    }

    template <typename T>
    class DataReader;

    class Reader {
    public:
        Reader(std::ifstream& f)
            : m_file(f)
        {
        }

        // Checks that the entire file was read successfully
        [[nodiscard]] bool isFinished() const noexcept
        {
            if (!ok()) [[unlikely]]
                return false;
            // Extra data at the end is a sign that the file is damaged.
            m_file.get();
            return m_file.eof();
        }

        [[nodiscard]] bool ok() const noexcept
        {
            return m_ok && m_file.good();
        }

        template <typename T>
        [[nodiscard]] T read()
        {
            std::optional<T> res = DataReader<T>::read(*this);
            if (!res) [[unlikely]] {
                m_ok = false;
                return {};
            }
            return *res;
        }

        void readBytes(char* dst, std::size_t n)
        {
            m_file.read(dst, n);
            if (!m_file || static_cast<std::size_t>(m_file.gcount()) != n) [[unlikely]]
                m_ok = false;
        }

        // The original game stores the pointers in the save file, but uses
        // them as handles (adjusts them after loading to get the correct
        // data pointers).
        // So, we have the same logic with only one difference:
        // we read 16 bits (the size of the pointer in the save file) and
        // store them into native pointer (that can be bigger than 16 bits).
        template <typename T>
        [[nodiscard]] T* readPtr()
        {
            std::uint16_t dosPtr = read<std::uint16_t>();
            return reinterpret_cast<T*>(dosPtr);
        }

        template <std::size_t N>
        void skipPadding()
        {
            char buf[N];
            readBytes(buf, N);
            if (!std::ranges::all_of(buf, [](char c) { return c == '\0'; })) [[unlikely]]
                m_ok = false;
        }

#ifndef NDEBUG
        // debug helper to make sure we have read the correct number of bytes
        // (just to make sure we read the same structure as the original game does)
        class [[nodiscard]] ExpectedBlockSize {
        public:
            ExpectedBlockSize(std::ifstream& f, std::ifstream::pos_type expected)
                : m_expected(expected)
                , m_startOffset(f.tellg())
                , m_file(f)
            {
            }

            ~ExpectedBlockSize()
            {
                if (m_file) {
                    std::ifstream::pos_type offset = m_file.tellg();
                    assert(offset - m_startOffset == m_expected);
                }
            }

        private:
            std::ifstream::pos_type m_expected;
            std::ifstream::pos_type m_startOffset;
            std::ifstream& m_file;
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
        std::ifstream& m_file;
        bool m_ok = true;
    };

    template <typename T>
    requires std::is_integral_v<T>
    class DataReader<T> {
    public:
        static std::optional<T> read(Reader& r)
        {
            T res;
            r.readBytes(reinterpret_cast<char*>(&res), sizeof(res));
            if (!r.ok()) [[unlikely]]
                return std::nullopt;
            return littleEndianToNative(res);
        }
    };

    template <>
    class DataReader<bool> {
    public:
        static std::optional<bool> read(Reader& r)
        {
            std::uint8_t v = r.read<std::uint8_t>();
            if (v != 0 && v != 1) [[unlikely]]
                return std::nullopt;
            return static_cast<bool>(v);
        }
    };

    template <typename T>
    requires std::is_enum_v<T>
    class DataReader<T> {
    public:
        static std::optional<T> read(Reader& r)
        {
            using U = std::underlying_type_t<T>;
            U val = r.read<U>();
            if (!validateEnum(static_cast<T>(val))) [[unlikely]]
                return std::nullopt;
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

    /* 262d:21d0 : 2 byte */
    std::uint16_t g_railsLoadedOffset;

    /* 262d:21ce : 2 byte */
    std::uint16_t g_entrancesLoadedOffset;

} // namespace

[[nodiscard]] static std::optional<Rail*> fixLoadedRailPtr(Rail* p)
{
    if (reinterpret_cast<std::uintptr_t>(p) < g_railsLoadedOffset ||
        reinterpret_cast<std::uintptr_t>(p) > g_railsLoadedOffset + fileRailArraySize) [[unlikely]]
        return std::nullopt;

    std::ptrdiff_t offset = reinterpret_cast<std::uintptr_t>(p) - g_railsLoadedOffset;
    if (offset % fileRailSize) [[unlikely]]
        return std::nullopt;

    return &g_rails[0][0][0] + (offset / fileRailSize);
}

/* 1400:0061 */
[[nodiscard]] static bool fixLoadedLocation(Location& l)
{
    if (!l.rail)
        return true;

    const std::uintptr_t c = reinterpret_cast<std::uintptr_t>(l.rail);
    if (c >= g_entrancesLoadedOffset + fileEntranceRailOffset &&
        c <= g_entrancesLoadedOffset + fileEntranceRailOffset + fileEntranceInfoSize * 5) {
        // obj is inside entrances array
        std::ptrdiff_t offset = c - (g_entrancesLoadedOffset + fileEntranceRailOffset);
        if (offset % fileEntranceInfoSize) [[unlikely]]
            return false;
        l.rail = &g_entrances[offset / fileEntranceInfoSize].rail;
    } else {
        // obj is inside objects array
        std::optional<Rail*> ptr = fixLoadedRailPtr(l.rail);
        if (!ptr) [[unlikely]]
            return false;
        l.rail = *ptr;
    }
    return true;
}

/* 1400:041c */
[[nodiscard]] static bool loadGameState(const char* fileName, char* switchStates, char* semaphoresAreRed)
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
            1) storing pointers in a file is a weird and dangerous idea
                (but they do it and they adjust these pointer after loading)
            2) different platforms have different pointer sizes, alignment
                of structures elements, byte order, etc.

       So, we will use portable approach instead - will read values field by
       field and convert the byte order from little endian to native
       representation.
    */
    std::ifstream file(fileName, std::ios::binary);
    if (!file) [[unlikely]]
        return false;

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
                    [[maybe_unused]] std::uint16_t unused1 = r.read<std::uint16_t>();
                    c.next = nullptr;

                    c.drawingPriority = r.read<std::int16_t>();

                    // This pointer is also not used after loading and
                    // contains a broken value.
                    [[maybe_unused]] std::uint16_t unused2 = r.read<std::uint16_t>();
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
    if (!r.ok()) [[unlikely]]
        return false;
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
    if (!r.ok()) [[unlikely]]
        return false;
    r.readBytes(switchStates, cnt);
    cnt = r.read<std::uint16_t>();
    r.readBytes(semaphoresAreRed, cnt);

    return r.isFinished();
}

/* 1400:009c */
[[nodiscard]] bool loadSavedGame(const char* fileName)
{
    char switchStatesBuf[100];
    char semaphoresIsRed[100];

    resetGameData();
    if (!loadGameState(fileName, switchStatesBuf, semaphoresIsRed)) [[unlikely]]
        return false;

    for (Train& train : g_trains) {
        if (train.isFreeSlot) {
            // clean the data for safety reasons - v0.93 wrote broken pointers here
            std::memset(&train, 0, sizeof(train));
            train.isFreeSlot = true;
        }
        if (!fixLoadedLocation(train.head) ||
            !fixLoadedLocation(train.tail)) [[unlikely]]
            return false;

        for (int i = 0; i < 5; ++i) {
            train.carriages[i].train = &train;
            if (!fixLoadedLocation(train.carriages[i].location)) [[unlikely]]
                return false;
        }
        train.lastMovementTime = getTime();
    }

    for (std::size_t i = 0; i < NormalEntranceCount; ++i) {
        std::optional<Rail*> ptr =
            fixLoadedRailPtr(g_entrances[i].rail.connections[0].rail);
        if (!ptr) [[unlikely]]
            return false;
        g_entrances[i].rail.connections[0].rail = *ptr;
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

    return true;
}

/* 1400:0638 */
int findNextSaveFile()
{
    static const std::filesystem::path searchPaths[] = {
#ifdef __EMSCRIPTEN__
        std::filesystem::path(g_persistentFolder) / "????????.??_",
#endif // __EMSCRIPTEN__
        "????????.??_",
    };

    static int g_searchStarted = false;
    static int g_curPath = 0;

    int err;
    if (!g_searchStarted) {
        const int initialPath = g_curPath;
        do {
            err = findFirst(searchPaths[g_curPath].generic_string().c_str(), 0);
            if (!err) {
                g_searchStarted = true;
                break;
            }
            g_curPath = (g_curPath + 1) % std::size(searchPaths);
        } while (g_curPath != initialPath);
    } else {
        err = findNext();
        if (err) {
            g_curPath = (g_curPath + 1) % std::size(searchPaths);
            g_searchStarted = false;
            err = findNextSaveFile();
        }
    }
    return err;
}

} // namespace resl
