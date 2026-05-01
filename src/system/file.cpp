#include "file.h"

#include <SDL_rwops.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <utility>

namespace resl {

File::File(const char* fileName, const char* mode) noexcept
{
    open(fileName, mode);
}

File::~File()
{
    close();
}

File::File(File&& other) noexcept
    : m_ops(other.m_ops)
    , m_eofBit(other.m_eofBit)
    , m_failBit(other.m_failBit)
    , m_badBit(other.m_badBit)
{
    other.m_ops = nullptr;
}

File& File::operator=(File&& other) noexcept
{
    std::swap(m_ops, other.m_ops);
    m_eofBit = other.m_eofBit;
    m_failBit = other.m_failBit;
    m_badBit = other.m_badBit;
    other.close();
    return *this;
}

void File::open(const char* fileName, const char* mode) noexcept
{
    close();
    m_ops = SDL_RWFromFile(fileName, mode);
    m_failBit = !m_ops;
}

void File::close() noexcept
{
    if (m_ops) {
        SDL_RWclose(m_ops);
        m_ops = nullptr;
    }
    clear();
}

void File::clear() noexcept
{
    m_eofBit = false;
    m_failBit = false;
    m_badBit = false;
}

std::size_t File::read(void* dst, std::size_t nBytes) noexcept
{
    if (fail()) [[unlikely]]
        return 0;

    std::size_t totalRead = 0;
    char* ptr = static_cast<char*>(dst);
    while (totalRead < nBytes) {
        std::size_t read = SDL_RWread(m_ops, ptr, 1, nBytes);
        if (read == 0) {
            m_eofBit = true;
            m_failBit = true;
            break;
        }
        assert(read <= nBytes);
        nBytes -= read;
        totalRead += read;
        ptr += read;
    }
    return totalRead;
}

void File::ignore(std::size_t nBytes) noexcept
{
    char buf[64];
    while (good() && nBytes) {
        std::size_t n = read(buf, std::min(sizeof(buf), nBytes));
        nBytes -= n;
    }
}

std::size_t File::write(const void* data, std::size_t nBytes) noexcept
{
    if (fail()) [[unlikely]]
        return 0;

    const std::size_t written =
        SDL_RWwrite(m_ops, data, 1, nBytes);
    if (written != nBytes) [[unlikely]]
        m_badBit = true;
    return written;
}

bool File::seek(pos_type pos) noexcept
{
    if (fail()) [[unlikely]]
        return false;

    m_eofBit = false;
    Sint64 res = SDL_RWseek(m_ops, pos, RW_SEEK_SET);
    if (res != pos) [[unlikely]] {
        m_failBit = true;
        return false;
    }
    return true;
}

File::pos_type File::tell() const noexcept
{
    if (fail()) [[unlikely]]
        return static_cast<pos_type>(-1);
    return SDL_RWtell(m_ops);
}

} // namespace resl
