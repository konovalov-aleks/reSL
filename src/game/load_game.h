#pragma once

#include "io_status.h"

namespace resl {

/* 1400:009c */
IOStatus loadSavedGame(const char* fileName);

// Iterates save files cyclically.
// Returns an error code; 0 means success
/* 1400:0638 */
int findNextSaveFile();

} // namespace resl
