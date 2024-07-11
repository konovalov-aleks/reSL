#include "records.h"

// IWYU pragma: no_include <sys/fcntl.h>
// IWYU pragma: no_include <sys/_types/_seek_set.h>

#include "game_data.h"
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/text.h>
#include <system/buffer.h>
#include <system/filesystem.h>

#include <fcntl.h> // IWYU pragma: keep
#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>

namespace resl {

struct Record {
    std::uint16_t trains;
    std::uint16_t money;
    std::uint16_t year;
    std::uint16_t level;
    char reserved[8];
    char name[20];
};

static constexpr char g_recordsFileName[] = "results.tbl";

/* 174e:000c */
static int recordCompareByTrains(const void* a, const void* b)
{
    return reinterpret_cast<const Record*>(b)->trains < reinterpret_cast<const Record*>(a)->trains
        ? -1
        : 1;
}

/* 174e:0266 */
void showRecordsScreen()
{
    ssize_t bytesRead = readFromFile(g_recordsFileName, g_pageBuffer);
    if (bytesRead != 0 && bytesRead != -1) {
        // The data can contain an empty records
        // Normalize the representation - move all non-empty records to the head
        Record* records = (Record*)g_pageBuffer;
        std::size_t nRecords = 0;
        while (nRecords < 833 && records[nRecords].name[0])
            ++nRecords;

        std::size_t i = nRecords;
        while (++i < 833) {
            if (records[i].name[0])
                std::memcpy(&records[nRecords++], &records[i], sizeof(Record));
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
        for (std::uint16_t i = 0; i <= 9 && records[i].trains; ++i) {
            char buf[80];
            std::snprintf(
                buf, sizeof(buf), "%2d. %-14s %6u %4u,000 %6u %2u", i + 1, records[i].name,
                records[i].trains, records[i].money, records[i].year, records[i].level);
            drawTextSmall(114, (i + 2) * 12 + 370, buf, Color::Black);
        }
    }
}

/* 174e:0087 */
static std::uint16_t playerNameHash()
{
    std::uint16_t hash = 0;
    const char* c = g_playerName;
    for (std::int16_t i = 0; i < std::size(g_playerName) && *c; ++i, ++c)
        hash += *c << (i << 1);
    return hash % 833;
}

/* 174e:00cc */
void writeRecords()
{
    // TODO implement
}

/* 174e:052b */
std::int16_t readLevel()
{
    constexpr std::size_t levelFieldOffset = 7;
    constexpr std::size_t recordSize = 36;

    std::int16_t data[18];
    static_assert(sizeof(data) >= recordSize);

    int fd = open(g_recordsFileName, O_BINARY | O_RDONLY);
    if (fd == -1) [[unlikely]]
        data[levelFieldOffset] = 1;
    else {
        lseek(fd, playerNameHash() * recordSize, SEEK_SET);
        read(fd, data, recordSize);
        close(fd);
        if (data[levelFieldOffset] == 0)
            data[levelFieldOffset] = 1;
    }
    return data[levelFieldOffset];
}

} // namespace resl
