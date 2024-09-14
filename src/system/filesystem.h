#pragma once

#include <cstddef>
#include <cstdint>

namespace resl {

// The original game uses a cumbersome structure DTA from DOS API with many
// useless fields:
//      https://www.stanislavs.org/helppc/int_21-4e.html
// This structure has the same meaning, but contain only useful fields.
struct FileInfo {
    // https://www.stanislavs.org/helppc/file_attributes.html
    std::uint16_t fileTime = 0;
    std::uint16_t fileDate = 0;
    const char* fileName;
};

//-----------------------------------------------------------------------------

/* 1abc:0005 */
std::size_t readBinaryFile(const char* fileName, void* pagePtr);

/* 1400:067f */
std::size_t readTextFile(const char* fileName);

/* 1abc:0064 */
void readIfNotLoaded(const char* fileName, void* pagePtr);

/* 12b1:0006 */
int findFirst(const char* pattern, std::uint8_t attrs);

/* 12b1:001d */
int findNext();

/* 1000:12a8 */
FileInfo lastSearchResult();

} // namespace resl
