#include "input_file.h"

#include <SDL_rwops.h>

#include <cassert>
#include <cstddef>
#include <utility>

namespace resl {

InputFile::InputFile(const char* fileName)
    : m_ops(SDL_RWFromFile(fileName, "rb"))
{
}

InputFile::~InputFile()
{
    close();
}

InputFile::InputFile(InputFile&& other) noexcept
    : m_ops(other.m_ops)
    , m_eof(other.m_eof)
{
    other.m_ops = nullptr;
}

InputFile& InputFile::operator=(InputFile&& other) noexcept
{
    std::swap(m_ops, other.m_ops);
    std::swap(m_eof, other.m_eof);
    return *this;
}

void InputFile::close() noexcept
{
    if (m_ops) {
        SDL_RWclose(m_ops);
        m_ops = nullptr;
    }
}

std::size_t InputFile::read(void* dst, std::size_t nBytes) noexcept
{
    if (!good()) [[unlikely]]
        return 0;

    std::size_t totalRead = 0;
    char* ptr = static_cast<char*>(dst);
    while (totalRead < nBytes) {
        std::size_t read = SDL_RWread(m_ops, ptr, 1, nBytes);
        if (read == 0) {
            m_eof = true;
            break;
        }
        assert(read <= nBytes);
        nBytes -= read;
        totalRead += read;
        ptr += read;
    }
    return totalRead;
}

char InputFile::get() noexcept
{
    char result;
    read(&result, sizeof(result));
    return result;
}

InputFile::pos_type InputFile::tellg() noexcept
{
    if (!m_ops) [[unlikely]]
        return 0;
    return SDL_RWtell(m_ops);
}

} // namespace resl
