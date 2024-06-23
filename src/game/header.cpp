#include "header.h"

#include "draw_header.h"
#include "game_over.h"
#include "status_bar.h"
#include "types/header_field.h"
#include <graphics/color.h>
#include <graphics/drawing.h>

#include <tasks/message_queue.h>
#include <tasks/task.h>

#include <cassert>
#include <cstdint>
#include <cstdio>

namespace resl {
namespace {

    struct HeaderValueChangeRequest {
        std::int8_t delta;
        HeaderFieldId fieldId;
        // if delta is zero, the value will be set without animation
        std::int16_t value;
    };

    /* The original game uses an own implementation of stack-based coroutines.
       I decided not to restore the sources of this mechanic, because it's
       non-portable and heavily dependent on X86 architecture.

       So, the address below contains a different data structure, but the
       meaning is roughly the same.

       1d7d:00de : 18 bytes */
    MessageQueue<HeaderValueChangeRequest> g_headerAnimationTaskQueue;

} // namespace

/* 1d7d:00f0 : 72 bytes */
Headers g_headers;

/* 12c5:031b */
void startHeaderFieldAnimation(HeaderFieldId fieldId, std::int16_t delta)
{
    HeaderValueChangeRequest request;
    request.delta = delta;
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
                if (drawing::getPixel(x + xOffset, y + yearFld.y) == Color::Black)
                    drawing::putPixel(x + xOffset, y + yearFld.y, Color::BWBlinking);
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
