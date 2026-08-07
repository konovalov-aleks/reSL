#pragma once

namespace resl {

// Logs a Google Analytics 4 event
void trackEvent(const char* name, const char* key = nullptr, const char* value = nullptr);

} // namespace resl
