#pragma once

#include <cstdint>
#include <sys/types.h>

namespace resl {

// The original game uses a cumbersome structure DTA from DOS API with many
// useless fields:
//      https://www.stanislavs.org/helppc/int_21-4e.html
// This structure has the same meaning, but contain only usefull fields.
struct FileInfo {
    // https://www.stanislavs.org/helppc/file_attributes.html
    std::uint16_t fileTime;
    std::uint16_t fileDate;
    const char* fileName;
};

//-----------------------------------------------------------------------------

/* 1abc:0005 */
ssize_t readFromFile(const char* fileName, void* pagePtr);

/* 1abc:0064 */
void readIfNotLoaded(const char* fileName, void* pagePtr);

/* 12b1:0006 */
int findFirst(const char* pattern, std::uint8_t attrs);

/* 12b1:001d */
int findNext();

/* 1000:12a8 */
FileInfo lastSearchResult();

} // namespace resl
