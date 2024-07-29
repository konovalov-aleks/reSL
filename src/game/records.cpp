#include "records.h"

// IWYU pragma: no_include <cwchar>
// IWYU pragma: no_include <sys/_types/_seek_set.h>

#include "game_data.h"
#include "header.h"
#include "types/header_field.h"
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/text.h>
#include <system/buffer.h>
#include <system/filesystem.h>
#include <utility/endianness.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>

namespace resl {

/*

The records file "results.tbl" is a hash table with a fixed capacity (833 slots).
The key is the player name, the hash table doesn't use any hash conflict
resolution algorithm => it will overwrite the other player's results if the
name hashes are equal.

Every item contain information about the player's best results in terms of
the number of finished trains and the duration of the game.

*/

struct RecordItem {
    std::uint16_t trains;
    std::uint16_t money;
    std::uint16_t year;
    std::uint16_t level;
};

// Best player results
struct Record {
    // The player's best results based on the number of finished trains.
    RecordItem byTrains;
    // The player's best results based on the game duration.
    RecordItem byYears;
    char playerName[20];
};

static constexpr char g_recordsFileName[] = "results.tbl";
static constexpr std::int16_t g_recordsTableCapacity = 833;
static constexpr std::size_t g_recordSize = 36;

static_assert(sizeof(Record) == g_recordSize);

// The original game is not portable - it works only on LE systems.
// reSL have to perform a byte order conversion to be able to work on BE systems.
static void convertByteOrderFileToNative(RecordItem& ri)
{
    ri.trains = littleEndianToNative(ri.trains);
    ri.money = littleEndianToNative(ri.money);
    ri.year = littleEndianToNative(ri.year);
    ri.level = littleEndianToNative(ri.level);
}

static void convertByteOrderFileToNative(Record& r)
{
    convertByteOrderFileToNative(r.byTrains);
    convertByteOrderFileToNative(r.byYears);
}

static void convertByteOrderNativeToFile(RecordItem& ri)
{
    ri.trains = nativeToLittleEndian(ri.trains);
    ri.money = nativeToLittleEndian(ri.money);
    ri.year = nativeToLittleEndian(ri.year);
    ri.level = nativeToLittleEndian(ri.level);
}

static void convertByteOrderNativeToFile(Record& r)
{
    convertByteOrderNativeToFile(r.byTrains);
    convertByteOrderNativeToFile(r.byYears);
}

/* 174e:000c */
static int recordCompareByTrains(const void* a, const void* b)
{
    assert(a && b);
    const Record* ra = reinterpret_cast<const Record*>(a);
    const Record* rb = reinterpret_cast<const Record*>(b);
    return rb->byTrains.trains < ra->byTrains.trains ? -1 : 1;
}

/* 174e:0027 */
static int recordCompareByLevelAndYear(const void* a, const void* b)
{
    assert(a && b);
    const RecordItem& ra = reinterpret_cast<const Record*>(a)->byYears;
    const RecordItem& rb = reinterpret_cast<const Record*>(b)->byYears;
    return rb.level < ra.level ||
            (rb.level == ra.level && rb.year < ra.year)
        ? -1
        : 1;
}

/* 174e:0266 */
void showRecordsScreen()
{
    std::size_t bytesRead = readBinaryFile(g_recordsFileName, g_pageBuffer);
    if (bytesRead) {
        // The data can contain an empty records
        // Normalize the representation - move all non-empty records to the head
        Record* records = (Record*)g_pageBuffer;
        std::size_t nRecords = 0;
        while (nRecords < g_recordsTableCapacity && *records[nRecords].playerName)
            ++nRecords;

        std::size_t i = nRecords;
        while (++i < g_recordsTableCapacity) {
            if (*records[i].playerName) {
                std::memcpy(&records[nRecords++], &records[i], sizeof(Record));
                convertByteOrderFileToNative(records[i]);
            }
        }

        /* fill entire area with red color */
        graphics::filledRectangle(0, 350, 80, 350, 0xFF, Color::Red);
        /* first block */
        graphics::dialogFrame(98, 367, 56, 151, Color::Gray);
        /* second block */
        graphics::dialogFrame(98, 545, 56, 151, Color::Gray);

        g_textSpacing = 2;
        drawText(153, 352, "* THE BEST DISPATCHERS *", Color::Black);
        drawTextSmall(114, 370,
                      " #  Name           Trains   Money   Year   Level", Color::Black);
        drawText(169, 530, "* THE BEST MANAGERS *", Color::Black);
        drawTextSmall(114, 548,
                      " #  Name        Level : Year   Money   Trains", Color::Black);
        std::qsort(records, nRecords, sizeof(Record), &recordCompareByTrains);
        for (std::uint16_t i = 0; i < 10 && records[i].byTrains.trains; ++i) {
            char buf[80];
            const RecordItem& ri = records[i].byTrains;
            std::snprintf(
                buf, sizeof(buf), "%2d. %-14s %6u %4u,000 %6u %2u", i + 1,
                records[i].playerName, static_cast<unsigned>(ri.trains),
                static_cast<unsigned>(ri.money), static_cast<unsigned>(ri.year),
                static_cast<unsigned>(ri.level));
            drawTextSmall(114, (i + 2) * 12 + 370, buf, Color::Black);
        }

        std::qsort(records, nRecords, sizeof(Record), &recordCompareByLevelAndYear);
        for (std::uint16_t i = 0; i < 10 && records[i].byYears.level; ++i) {
            char buf[80];
            const RecordItem& ri = records[i].byYears;
            std::snprintf(
                buf, sizeof(buf), "%2d. %-14s %2u : %4u %4u,000 %5u", i + 1,
                records[i].playerName, static_cast<unsigned>(ri.level), static_cast<unsigned>(ri.year),
                static_cast<unsigned>(ri.money), static_cast<unsigned>(ri.trains));
            drawTextSmall(114, (i + 2) * 12 + 548, buf, Color::Black);
        }
    }
}

/* 174e:0087 */
static std::uint16_t playerNameHash()
{
    std::uint16_t hash = 0;
    const char* c = g_playerName;
    constexpr std::int16_t playerNameLen =
        static_cast<std::int16_t>(std::size(g_playerName));
    for (std::int16_t i = 0; i < playerNameLen && *c; ++i, ++c)
        hash += *c << (i << 1);
    return hash % g_recordsTableCapacity;
}

/* 174e:0243 */
void fillRecordItem(RecordItem& ri)
{
    ri.trains = g_headers[static_cast<int>(HeaderFieldId::Trains)].value;
    ri.money = g_headers[static_cast<int>(HeaderFieldId::Money)].value;
    ri.year = g_headers[static_cast<int>(HeaderFieldId::Year)].value;
    ri.level = g_headers[static_cast<int>(HeaderFieldId::Level)].value;
}

/* 174e:00cc */
void writeRecords()
{
    /* 262d:6f52 : 8 bytes */
    static const RecordItem g_emptyRecordItem = {};

    std::FILE* file = fopen(g_recordsFileName, "r+");
    if (!file) {
        // initialize a new empty hash table
        file = fopen(g_recordsFileName, "w+");
        if (!file) [[unlikely]]
            return;

        Record r;
        r.byTrains = g_emptyRecordItem;
        r.byYears = g_emptyRecordItem;
        r.playerName[0] = '\0';

        for (std::int16_t i = 0; i < g_recordsTableCapacity; ++i)
            std::memcpy(&g_pageBuffer[i * g_recordSize], &r, g_recordSize);

        std::fwrite(g_pageBuffer, g_recordSize, g_recordsTableCapacity, file);
    }

    const std::uint16_t hash = playerNameHash();
    std::fseek(file, hash * g_recordSize, SEEK_SET);

    Record r;
    [[maybe_unused]] std::size_t nRecords = std::fread(&r, g_recordSize, 1, file);
    assert(nRecords == 1);
    convertByteOrderFileToNative(r);

    const std::int16_t curTrains =
        g_headers[static_cast<int>(HeaderFieldId::Trains)].value;

    bool needUpdate = r.byTrains.trains < curTrains;
    if (needUpdate)
        fillRecordItem(r.byTrains);

    const std::int16_t curLevel =
        g_headers[static_cast<int>(HeaderFieldId::Level)].value;
    const std::int16_t curYear =
        g_headers[static_cast<int>(HeaderFieldId::Year)].value;

    if (r.byYears.level < curLevel ||
        (r.byYears.level == curLevel && r.byYears.year < curYear)) {

        needUpdate = true;
        fillRecordItem(r.byYears);
    }

    if (needUpdate) {
        std::strcpy(r.playerName, g_playerName);
        std::fseek(file, -static_cast<long>(g_recordSize), SEEK_CUR);

        convertByteOrderNativeToFile(r);
        std::fwrite(&r, g_recordSize, 1, file);
    }

    std::fclose(file);
}

/* 174e:052b */
std::int16_t readLevel()
{
    constexpr std::size_t levelFieldOffset = 7;

    std::int16_t data[18];
    static_assert(sizeof(data) >= g_recordSize);

    std::FILE* file = std::fopen(g_recordsFileName, "rb");
    if (!file) [[unlikely]]
        data[levelFieldOffset] = 1;
    else {
        std::fseek(file, playerNameHash() * g_recordSize, SEEK_SET);
        std::fread(data, g_recordSize, 1, file);
        std::fclose(file);

        // The original game is not portable - it works only on LE systems.
        // reSL have to perform a byte order conversion to be able to work on BE systems.
        data[levelFieldOffset] = littleEndianToNative(data[levelFieldOffset]);

        if (data[levelFieldOffset] == 0)
            data[levelFieldOffset] = 1;
    }
    return data[levelFieldOffset];
}

} // namespace resl
