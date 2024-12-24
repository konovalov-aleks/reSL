#pragma once

#include <SDL_rwops.h>

#include <cstddef>

namespace resl {

class InputFile {
public:
    using pos_type = Sint64;

    InputFile(const char* fileName);
    ~InputFile();

    InputFile(InputFile&&) noexcept;
    InputFile& operator=(InputFile&&) noexcept;

    void close() noexcept;

    std::size_t read(void*, std::size_t nBytes) noexcept;
    char get() noexcept;

    pos_type tellg() noexcept;

    explicit operator bool() const noexcept { return good(); }
    bool good() const noexcept { return m_ops && !eof(); }
    bool eof() const noexcept { return m_eof; }

private:
    SDL_RWops* m_ops = nullptr;
    bool m_eof = false;
};

} // namespace resl
