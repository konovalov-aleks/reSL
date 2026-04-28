#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>

namespace resl {

#ifdef __EMSCRIPTEN__

static constexpr const char* const g_persistentFolder = "/persistent";

#endif // __EMSCRIPTEN__

// The original game uses a cumbersome structure DTA from DOS API with many
// useless fields:
//      https://www.stanislavs.org/helppc/int_21-4e.html
// This structure has the same meaning, but contain only useful fields.
struct FileInfo {
    // https://www.stanislavs.org/helppc/file_attributes.html
    std::uint16_t fileTime = 0;
    std::uint16_t fileDate = 0;
    std::filesystem::path filePath;
};

//-----------------------------------------------------------------------------

void initFS();

/* 1abc:0005 */
[[nodiscard]] std::span<std::byte> readBinaryFile(const char* fileName);

/* 1400:067f */
[[nodiscard]] std::span<std::byte> readTextFile(const char* fileName);

/* 12b1:0006 */
int findFirst(const char* pattern, std::uint8_t attrs);

/* 12b1:001d */
int findNext();

/* 1000:12a8 */
FileInfo lastSearchResult();

} // namespace resl
