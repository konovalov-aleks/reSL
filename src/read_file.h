#pragma once

#include <sys/types.h>

namespace resl {

/* 1abc:0005 */
ssize_t readFromFile(const char* fileName, void* pagePtr);

/* 1abc:0064 */
void readIfNotLoaded(const char* fileName, void* pagePtr);

} // namespace resl
