#include "header.h"

#include "header_field.h"
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <ui/components/draw_header.h>
#include <ui/components/status_bar.h>
#include <ui/game_over.h>

#include <tasks/message_queue.h>
#include <tasks/task.h>

#include <cassert>
#include <cstdint>
#include <cstdio>

namespace resl {

/* 1d7d:00de : 18 bytes */
MessageQueue<HeaderValueChangeRequest> g_headerAnimationTaskQueue;

/* 1d7d:20dc */
Task g_taskHeaderFieldAnimation;

/* 1d7d:00f0 : 72 bytes */
Headers g_headers = {
    { .x = 16,  .y = 25, .valueLimit = -1,   .nDigits = 5 }, // trains
    { .x = 136, .y = 25, .valueLimit = -1,   .nDigits = 4 }, // money
    { .x = 272, .y = 25, .valueLimit = 2000, .nDigits = 4 }, // year
    { .x = 376, .y = 25, .valueLimit = 100,  .nDigits = 2 }  // level
};

/* 12c5:031b */
void startHeaderFieldAnimation(HeaderFieldId fieldId, std::int16_t delta)
{
    HeaderValueChangeRequest request;
    request.delta = static_cast<std::int8_t>(delta);
    request.fieldId = fieldId;
    g_headerAnimationTaskQueue.push(request);
}

/* 19de:0490 */
void spendMoney(std::int16_t delta)
{
    startHeaderFieldAnimation(HeaderFieldId::Money, -delta);
}

/* 132d:0420 */
static void makeYearFieldBlinking()
{
    const HeaderField& yearFld = g_headers[static_cast<int>(HeaderFieldId::Year)];
    for (std::int16_t digit = 0; digit < 4; ++digit) {
        const std::int16_t xOffset = yearFld.x + digit * 16;
        for (std::int16_t x = 0; x < 16; ++x) {
            for (std::int16_t y = 0; y < 16; ++y) {
                if (graphics::getPixel(x + xOffset, y + yearFld.y) == Color::Black)
                    graphics::putPixel(x + xOffset, y + yearFld.y, Color::BWBlinking);
            }
        }
    }
}

/* 16a6:067b */
static void onHeaderFieldLimitValueReached(const HeaderField& fld)
{
    HeaderFieldId fieldId = static_cast<HeaderFieldId>(&fld - &g_headers[0]);
    if (fieldId == HeaderFieldId::Money) {
        showStatusMessage("OUT OF MONEY");
        gameOver();
    } else if (fieldId == HeaderFieldId::Year)
        makeYearFieldBlinking();
}

/* 12c5:0008 */
Task taskHeaderFieldAnimation()
{
    drawHeaderFieldFontTexture();

    for (;;) {
        HeaderValueChangeRequest req = co_await g_headerAnimationTaskQueue.pop();
        HeaderField& hdrField = g_headers[static_cast<std::size_t>(req.fieldId)];

        if (req.delta == 0) {
            char buf[10];
            hdrField.value = req.value;
            std::snprintf(buf, sizeof(buf), "%5d", static_cast<int>(hdrField.value));
            for (int i = 0; i < 5; ++i) {
                if (buf[i] == ' ')
                    buf[i] = '0';
            }
            for (std::int8_t i = 0; i < hdrField.nDigits; ++i)
                hdrField.digitValues[i] = buf[4 - i] + '0';
            hdrField.yScroll = 0;
            hdrField.curAnimatingDigit = hdrField.nDigits - 1;
            drawHeaderField(hdrField);
        } else if (req.delta > 0) {
            for (; req.delta != 0; --req.delta) {
                if (hdrField.value + 1 == hdrField.valueLimit)
                    onHeaderFieldLimitValueReached(hdrField);

                hdrField.yScroll = 1;
                hdrField.curAnimatingDigit = 0;
                while (hdrField.curAnimatingDigit < hdrField.nDigits - 1 &&
                       hdrField.digitValues[hdrField.curAnimatingDigit] == 9)
                    ++hdrField.curAnimatingDigit;

                do {
                    drawHeaderField(hdrField);
                    co_await sleep(3);
                } while (++hdrField.yScroll <= 16);

                hdrField.yScroll = 0;
                for (std::int8_t i = 0; i < hdrField.curAnimatingDigit; ++i)
                    hdrField.digitValues[i] = 0;
                ++hdrField.digitValues[hdrField.curAnimatingDigit];
                ++hdrField.value;
            }
        } else {
            assert(req.delta < 0);
            for (; req.delta != 0; ++req.delta) {
                if (hdrField.value - 1 == hdrField.valueLimit)
                    onHeaderFieldLimitValueReached(hdrField);

                hdrField.yScroll = -1;
                hdrField.curAnimatingDigit = 0;
                while (hdrField.curAnimatingDigit < hdrField.nDigits - 1 &&
                       hdrField.digitValues[hdrField.curAnimatingDigit] == 0)
                    ++hdrField.curAnimatingDigit;

                do {
                    drawHeaderField(hdrField);
                    co_await sleep(2);
                } while (--hdrField.yScroll >= -16);

                hdrField.yScroll = 0;
                for (std::int8_t i = 0; i < hdrField.curAnimatingDigit; ++i)
                    hdrField.digitValues[i] = 9;
                --hdrField.digitValues[hdrField.curAnimatingDigit];
                --hdrField.value;
            }
        }
    }
    co_return;
}

} // namespace resl
