#include "save_game.h"

// IWYU pragma: no_include <cwchar>

#include "common.h"
#include <game/entrance.h>
#include <game/header.h>
#include <game/header_field.h>
#include <game/player_name.h>
#include <game/rail.h>
#include <game/rail_info.h>
#include <game/semaphore.h>
#include <game/static_object.h>
#include <game/switch.h>
#include <game/train.h>
#include <system/keyboard.h>
#include <system/random.h>
#include <types/rectangle.h>
#include <ui/components/status_bar.h>
#include <utility/endianness.h>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <limits>
#include <type_traits>

#ifdef __EMSCRIPTEN__
#   include <system/filesystem.h>

#   include <filesystem>
#   include <string>
#endif // __EMSCRIPTEN__

namespace resl {

namespace {

    // The original game writes Rail* pointers to a save file.
    // To be able to adjust pointers when loading, they also write the addresses
    // of the g_rails and g_entrances arrays.
    // But reSL is portable and can't write pointers (since they can be larger
    // than 2 bytes) => instead we will use these synthetic virtual offsets and
    // adjust pointer values we write to the file.
    static constexpr std::uint16_t g_railsVirtualOffset = 0x0E1F;
    static constexpr std::uint16_t g_entrancesVirtualOffset = 0xCAFE;

    static_assert(g_railsVirtualOffset + fileRailSize <= std::numeric_limits<std::uint16_t>::max());
    static_assert(g_railsVirtualOffset + fileRailSize < g_entrancesVirtualOffset);
    static_assert(g_entrancesVirtualOffset + fileEntrancesArraySize <= std::numeric_limits<std::uint16_t>::max());

    template <typename T>
    class DataWriter;

    class Writer {
    public:
#ifndef NDEBUG
        // Debug helper to make sure we have written the correct number of bytes.
        // It is only needed to make sure once again that the file will be
        // compatible with the original game.
        class [[nodiscard]] ExpectedBlockSize {
        public:
            ExpectedBlockSize(std::ofstream& f, std::ofstream::pos_type expected)
                : m_expected(expected)
                , m_startOffset(f.tellp())
                , m_file(f)
            {
            }

            ~ExpectedBlockSize()
            {
                if (m_file) {
                    std::ofstream::pos_type offset = m_file.tellp();
                    assert(offset - m_startOffset == m_expected);
                }
            }

        private:
            std::ofstream::pos_type m_expected;
            std::ofstream::pos_type m_startOffset;
            std::ofstream& m_file;
        };

        ExpectedBlockSize expectedSize(long expected)
        {
            return ExpectedBlockSize(m_file, expected);
        }

#else // NDEBUG

        class [[maybe_unused]] ExpectedBlockSize { };
        ExpectedBlockSize expectedSize(std::ofstream::pos_type) { return {}; }

#endif // !NDEBUG

        Writer(std::ofstream& f)
            : m_file(f)
        {
        }

        template <typename T>
        void write(T value)
        {
            DataWriter<T>::write(*this, value);
        }

        void writeBytes(const char* buf, std::size_t len)
        {
            m_file.write(buf, len);
        }

        bool finalize() const noexcept
        {
            m_file.flush();
            return m_file.good();
        }

    private:
        std::ofstream& m_file;
    };

    template <typename T>
    requires std::is_integral_v<T>
    class DataWriter<T> {
    public:
        static void write(Writer& w, T value)
        {
            T valueLE = nativeToLittleEndian(value);
            w.writeBytes(reinterpret_cast<const char*>(&valueLE), sizeof(valueLE));
        }
    };

    template <typename T>
    requires std::is_enum_v<T>
    class DataWriter<T> {
    public:
        static void write(Writer& w, T value)
        {
            w.write(static_cast<std::underlying_type_t<T>>(value));
        }
    };

    template <>
    class DataWriter<Location> {
    public:
        static void write(Writer& w, const Location& l)
        {
            auto bs = w.expectedSize(4);
            w.write(l.pathStep);
            w.write(l.forwardDirection);
            w.write(l.rail);
        }
    };

    template <>
    class DataWriter<Rail*> {
    public:
        static void write(Writer& w, Rail* ptr)
        {
            if (!ptr || ptr == g_disabledSwitchPath) {
                w.write(static_cast<std::uint16_t>(reinterpret_cast<std::uintptr_t>(ptr)));
                return;
            }

            const std::uintptr_t entranceArrOffset =
                reinterpret_cast<std::intptr_t>(g_entrances);

            const std::uintptr_t offset = reinterpret_cast<std::intptr_t>(ptr);
            bool isInsideEntranceArr =
                offset >= entranceArrOffset && offset < entranceArrOffset + sizeof(g_entrances);
            if (isInsideEntranceArr) {
                std::uintptr_t entranceOffset = offset - offsetof(Entrance, rail);
                assert((entranceOffset - entranceArrOffset) % sizeof(Entrance) == 0);
                std::size_t entranceIdx = (entranceOffset - entranceArrOffset) / sizeof(Entrance);
                w.write(static_cast<std::uint16_t>(
                    g_entrancesVirtualOffset + fileEntranceRailOffset + entranceIdx * fileEntranceInfoSize));
            } else {
                const std::uintptr_t railsArrOffset = reinterpret_cast<std::intptr_t>(g_rails);
                assert(offset >= railsArrOffset && offset < railsArrOffset + sizeof(g_rails));
                assert((offset - railsArrOffset) % sizeof(Rail) == 0);
                std::size_t railIdx = (offset - railsArrOffset) / sizeof(Rail);
                w.write(static_cast<std::uint16_t>(g_railsVirtualOffset + railIdx * fileRailSize));
            }
        }
    };

} // namespace

/* 1400:05a6 */
static void generateFileName(char* buf, std::size_t bufSize)
{
    std::snprintf(buf, bufSize, "%03d",
                  static_cast<int>(g_headers[static_cast<int>(HeaderFieldId::Money)].value));
    buf[3] = 'a' + genRandomNumber(26);
    buf[4] = 'a' + genRandomNumber(26);
    std::snprintf(buf + 5, bufSize - 5, "%03d",
                  static_cast<int>(g_headers[static_cast<int>(HeaderFieldId::Year)].value - 1000));
    buf[8] = '.';
    std::snprintf(buf + 9, bufSize - 9, "%02d",
                  static_cast<int>(g_headers[static_cast<int>(HeaderFieldId::Level)].value));
    buf[11] = '_';
    buf[12] = '\0';
}

/* 1400:0245 */
[[nodiscard]] static bool saveGameState(const char* fileName)
{
#ifdef __EMSCRIPTEN__
    const std::string fullPathStr =
        (std::filesystem::path(g_persistentFolder) / fileName).generic_string();
    const char* const fullPath = fullPathStr.c_str();
#else
    const char* const fullPath = fileName;
#endif

    std::ofstream file(fullPath, std::ios::binary);
    if (!file) [[unlikely]]
        return false;

    /* The original game just writes entire structures.
       But this approach is obviously non-portable for a bunch of reasons, e.g:
        * pointers may have different size depending on platform
          (ShortLine writes pointers to save files, kinda descriptors; and
          restores them when loading)
        * different platforms may have different alignment rules, different
          endianness, etc.

       Thus, reSL implements a portable approach - reads data field by field.

       So, the original code looked like this:

            // pointers to array heads to be able to restore pointers
            // inside data structures when loading
            write(fd, g_rails, 2);
            write(fd, g_entrances, 2);

            write(fd, g_playerName, 20);

            write(fd, &headers, 0x48);
            write(fd, &g_entranceCount, 2);
            write(fd, g_entrances, 0x84);
            write(fd, g_staticObjects, 0x3c0);
            write(fd, g_trains, 0xa50);
            write(fd, &g_railRoadCount, 2);
            write(fd, g_railRoad, g_railRoadCount * 6);

            bool data[200];
            for (std::int16_t i = 0; i < g_nSwitches; ++i)
                data[i] = &g_switches[i].entry < &g_switches[i].disabledPath;
            write(fd, &g_nSwitches, 2);
            writeFile(fd, data, g_nSwitches);

            for (std::int16_t i = 0; i < g_semaphoreCount; ++i)
                data[i] = &g_semaphores[i].isRed;
            write(fd, &g_semaphoreCount, 2);
            write(fd, data, g_semaphoreCount);
    */

    Writer w(file);

    {
        auto hdrSize = w.expectedSize(3830);

        w.write(g_railsVirtualOffset);
        w.write(g_entrancesVirtualOffset);

        static_assert(sizeof(g_playerName) == 20);
        w.writeBytes(g_playerName, 20);

        {
            auto bs = w.expectedSize(72);
            for (HeaderField& fld : g_headers) {
                w.write(fld.x);
                w.write(fld.y);
                w.write(fld.valueLimit);
                w.write(fld.nDigits);
                w.write(fld.curAnimatingDigit);
                w.write(fld.value);
                w.write(fld.yScroll);
                for (std::uint8_t v : fld.digitValues)
                    w.write<std::int8_t>(v);
            }
        }
        {
            auto bs = w.expectedSize(134);
            w.write<std::int16_t>(g_entranceCount);
            for (std::size_t i = 0; i < NormalEntranceCount; ++i) {
                const Entrance& e = g_entrances[i];
                w.write(e.bgColor);
                w.write(e.fgColor);
                w.write(e.entranceRailInfoIdx);
                w.write(e.waitingTrainsCount);
                w.write(e.rail.x);
                w.write(e.rail.y);
                w.write(e.rail.type);
                w.write(e.rail.minPathStep);
                w.write(e.rail.maxPathStep);
                for (std::int8_t s : e.rail.semSlotIdByDirection)
                    w.write(s);
                for (const RailConnection& rc : e.rail.connections) {
                    w.write(rc.rail); // pointer
                    w.write(rc.slot);
                }
            }
        }
        {
            auto bs = w.expectedSize(960);
            for (const StaticObject& so : g_staticObjects) {
                w.write(so.x);
                w.write(so.y);
                w.write(so.kind);
                w.write(so.type);
                w.write(so.color);
                w.write(so.creationYear);
            }
        }
        {
            auto bs = w.expectedSize(2640);
            for (const Train& t : g_trains) {
                w.write(t.isFreeSlot);
                w.write(t.carriageCnt);
                w.write(t.drawingChainIdx);
                w.write(t.needToRedraw);
                w.write(t.isActualPosition);
                w.write(t.speed);
                w.write(t.maxSpeed);
                w.write(t.headCarriageIdx);
                w.write(t.movementDebt);
                w.write<std::uint8_t>(0); // padding
                w.write(t.year);
                w.write(t.lastMovementTime);
                for (const Carriage& c : t.carriages) {
                    // Carriage* pointer. Ignored when loading.
                    w.write<std::uint16_t>(0);

                    w.write(c.drawingPriority);

                    // Train* pointer. Ignored when loading.
                    w.write<std::uint16_t>(0);

                    w.write(c.dstEntranceIdx);
                    w.write(c.type);
                    w.write(c.direction);
                    w.write(c.x_direction);
                    w.write(c.location);
                    w.write(c.rect.x1);
                    w.write(c.rect.y1);
                    w.write(c.rect.x2);
                    w.write(c.rect.y2);
                }
                w.write(t.head);
                w.write(t.tail);
            }
        }
    }

    w.write(g_railRoadCount);
    for (std::uint16_t i = 0; i < g_railRoadCount; ++i) {
        auto bs = w.expectedSize(6);
        const RailInfo& ri = g_railRoad[i];
        w.write(ri.roadTypeMask);
        w.write(ri.tileX);
        w.write(ri.tileY);
        w.write(ri.railType);
        w.write(ri.year_8);
        w.write<std::uint8_t>(0); // padding
    }

    char data[200];
    for (std::int16_t i = 0; i < g_nSwitches; ++i)
        data[i] = g_switches[i].entry.rail < g_switches[i].disabledPath.rail;
    w.write<std::uint16_t>(g_nSwitches);
    w.writeBytes(data, g_nSwitches);

    for (std::int16_t i = 0; i < g_semaphoreCount; ++i)
        data[i] = g_semaphores[i].isRed;
    w.write<std::uint16_t>(g_semaphoreCount);
    w.writeBytes(data, g_semaphoreCount);

    return w.finalize();
}

/* 1400:0004 */
[[nodiscard]] bool saveGame()
{
    char fileName[16];
    showStatusMessage("SAVING ...      ");
    generateFileName(fileName, sizeof(fileName));
    const bool res = saveGameState(fileName);
    showStatusMessage("SAVING ... DONE ");
    drawStatusBarWithCopyright(0);
    return res;
}

} // namespace resl
