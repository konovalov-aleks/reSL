#include "records.h"

#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/text.h>
#include <system/buffer.h>
#include <system/read_file.h>

#include <sys/types.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace resl {

struct Record {
    std::uint16_t trains;
    std::uint16_t money;
    std::uint16_t year;
    std::uint16_t level;
    char reserved[8];
    char name[20];
};

static int recordCompareByTrains(const void* a, const void* b)
{
    return reinterpret_cast<const Record*>(b)->trains < reinterpret_cast<const Record*>(a)->trains
        ? -1
        : 1;
}

/* 174e:0266 */
void showRecordsScreen()
{
    ssize_t bytesRead = readFromFile("results.tbl", g_pageBuffer);
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

} // namespace resl
