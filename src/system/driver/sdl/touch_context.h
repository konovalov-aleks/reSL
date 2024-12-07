#pragma once

namespace resl {

enum class LongTouchAction {
    None,
    BuildRail,
    CallServer,
};

class TouchContextProvider {
public:
    virtual LongTouchAction recognizeTouchAction(int x, int y) const = 0;
    virtual bool isSwipeAllowed() const = 0;
};

} // namespace resl
