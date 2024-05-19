#include "header.h"

#include "draw_header.h"
#include "types/header_field.h"

#include <tasks/message_queue.h>
#include <tasks/task.h>

#include <cstddef>
#include <cstdint>

namespace resl {
namespace {

    struct HeaderValueChangeRequest {
        std::int8_t delta;
        HeaderFieldId fieldId;
        std::int16_t finalValue;
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

/* 16a6:067b */
static void onHeaderFieldMaxValueReached(HeaderField&)
{
    // TODO implement
}

/* 12c5:0008 */
Task taskHeaderFieldAnimation()
{
    // FIXME why not working?
    //    drawHeaderFieldFontTexture();

    for (;;) {
        HeaderValueChangeRequest req = co_await g_headerAnimationTaskQueue.pop();

        if (req.delta < 1)
            continue; // TODO implement

        HeaderField& hdrField = g_headers[static_cast<std::size_t>(req.fieldId)];
        for (; req.delta != 0; --req.delta) {
            if (hdrField.value + 1 == hdrField.valueLimit)
                onHeaderFieldMaxValueReached(hdrField);

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
    }
    co_return;
}

} // namespace resl
