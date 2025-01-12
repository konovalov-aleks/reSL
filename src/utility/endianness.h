#pragma once

#include <bit>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <type_traits>

namespace resl {

namespace details {

    template <typename T>
    concept arithmetic = std::is_arithmetic_v<T>;

    // Changes the byte order (little endian <=> big endian)
    template <arithmetic T>
    struct Endianness {
        static T Swap(T val)
        {
#if defined(__cpp_lib_byteswap) && __cpp_lib_byteswap == 202110L
            return std::byteswap(val);
#endif

            if constexpr (sizeof(T) == 1)
                return val;

// fast conversion using GCC extensions
#if defined __GNUC__ && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
            else if constexpr (sizeof(T) == 8)
                return std::bit_cast<T>(__builtin_bswap64(std::bit_cast<std::uint64_t>(val)));
            else if constexpr (sizeof(T) == 4)
                return std::bit_cast<T>(__builtin_bswap32(std::bit_cast<std::uint32_t>(val)));
            else if constexpr (sizeof(T) == 2) {
#   if (__GNUC__ == 4 && __GNUC_MINOR__ >= 8) || __GNUC__ > 4
                return std::bit_cast<T>(__builtin_bswap16(std::bit_cast<std::uint16_t>(val)));
#   else
                return std::bit_cast<T, std::uint16_t>(__builtin_bswap32(std::bit_cast<std::uint32_t>(val) << 16));
#   endif
            }

// MSVC extensions
#elif defined _MSC_VER
            if constexpr (sizeof(T) == 8) {
                static_assert(sizeof(unsigned __int64) == 8);
                return std::bit_cast<T>(_byteswap_uint64(std::bit_cast<unsigned __int64>(val)));
            } else if constexpr (sizeof(T) == 4) {
                static_assert(sizeof(unsigned long) == 4);
                return std::bit_cast<T>(_byteswap_ulong(std::bit_cast<unsigned long>(val)));
            } else if constexpr (sizeof(T) == 2) {
                static_assert(sizeof(unsigned short) == 2);
                return std::bit_cast<T>(_byteswap_ushort(std::bit_cast<unsigned short>(val)));
            }
#endif

            else {
                T result;
                const char* src = reinterpret_cast<char*>(&val);
                char* dst = reinterpret_cast<char*>(&result);
                for (std::size_t i = 0; i < sizeof(T); ++i)
                    dst[i] = src[sizeof(T) - i - 1];
                return result;
            }
        }
    };

} // namespace details

// Converts from the native representation to little endian
template <details::arithmetic T>
inline T nativeToLittleEndian(T n)
{
    if constexpr (std::endian::native == std::endian::big)
        return details::Endianness<T>::Swap(n);
    else if constexpr (std::endian::native == std::endian::little)
        return n;
    else
        static_assert(sizeof(T) == 0, "mixed-endian is not supported");
}

// Converts from the native representation to big endian
template <details::arithmetic T>
inline T NativeToBigEndian(T n)
{
    if constexpr (std::endian::native == std::endian::big)
        return n;
    else if constexpr (std::endian::native == std::endian::little)
        return details::Endianness<T>::Swap(n);
    else
        static_assert(sizeof(T) == 0, "mixed-endian is not supported");
}

// Converts from little endian to the native representation
template <details::arithmetic T>
inline T littleEndianToNative(T l)
{
    return nativeToLittleEndian(l);
}

// Converts from big endian to the native representation
template <typename T>
inline T bigEndianToNative(T b)
{
    return nativeToBigEndian(b);
}

} // namespace resl
