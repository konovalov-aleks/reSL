#pragma once

#include <SDL_rwops.h>

#include <cstddef>

namespace resl {

class File {
public:
    using pos_type = Sint64;

    File() = default;
    File(const char* fileName, const char* mode) noexcept;
    ~File();

    File(File&&) noexcept;
    File& operator=(File&&) noexcept;

    void open(const char* fileName, const char* mode) noexcept;
    void close() noexcept;

    bool isOpen() const noexcept { return m_ops; }

    void clear() noexcept;

    std::size_t read(void*, std::size_t nBytes) noexcept;
    void ignore(std::size_t nBytes) noexcept;

    std::size_t write(const void*, std::size_t nBytes) noexcept;

    bool seek(pos_type) noexcept;
    [[nodiscard]] pos_type tell() const noexcept;

    explicit operator bool() const noexcept { return !fail(); }
    bool good() const noexcept { return isOpen() && !m_eofBit && !m_failBit && !m_badBit; };
    bool fail() const noexcept { return !isOpen() || m_failBit || m_badBit; }
    bool eof() const noexcept { return m_eofBit; }

private:
    SDL_RWops* m_ops = nullptr;
    bool m_eofBit : 1 = false;
    bool m_failBit : 1 = false;
    bool m_badBit : 1 = false;
};

} // namespace resl
