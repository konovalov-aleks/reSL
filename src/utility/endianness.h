#pragma once

#include <concepts>
#include <cstddef>
#include <cstring>
#include <type_traits>

namespace resl {

#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#   define RESL_BYTE_ORDER_BIG_ENDIAN
#else
#   define RESL_BYTE_ORDER_LITTLE_ENDIAN
#endif

namespace details {

    template <typename T>
    concept arithmetic = std::is_arithmetic_v<T>;

    template <std::integral DstT, std::integral SrcT>
    DstT bitCast(SrcT src)
    {
        static_assert(sizeof(DstT) == sizeof(SrcT));
        return static_cast<DstT>(src);
    }

    template <std::integral DstT, std::floating_point SrcT>
    DstT bitCast(SrcT src)
    {
        static_assert(sizeof(DstT) == sizeof(SrcT));
        DstT res;
        std::memcpy(&res, &src, sizeof(DstT));
        return res;
    }

    // Changes the byte order (little endian <=> big endian)
    template <arithmetic T>
    struct Endianness {
        static T Swap(T val)
        {
            if constexpr (sizeof(T) == 1)
                return val;

// fast conversion using GCC extensions
#if defined __GNUC__ && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
            else if constexpr (sizeof(T) == 8)
                return bitCast<T>(__builtin_bswap64(bitCast<std::uint64_t>(val)));
            else if constexpr (sizeof(T) == 4)
                return bitCast<T>(__builtin_bswap32(bitCast<std::uint32_t>(val)));
            else if constexpr (sizeof(T) == 2) {
#   if (__GNUC__ == 4 && __GNUC_MINOR__ >= 8) || __GNUC__ > 4
                return bitCast<T>(__builtin_bswap16(bitCast<std::uint16_t>(val)));
#   else
                return bitCast<T, std::uint16_t>(__builtin_bswap32(bitCast<std::uint32_t>(val) << 16));
#   endif
            }

// MSVC extensions
#elif defined _MSC_VER
            if constexpr (sizeof(T) == 8) {
                static_assert(sizeof(unsigned __int64) == 8);
                return bitCast<T>(_byteswap_uint64(bitCast<unsigned __int64>(val)));
            } else if constexpr (sizeof(T) == 4) {
                static_assert(sizeof(unsigned long) == 4);
                return bitCast<T>(_byteswap_ulong(bitCast<unsigned long>(val)));
            } else if constexpr (sizeof(T) == 2) {
                static_assert(sizeof(unsigned short) == 2);
                return bitCast<T>(_byteswap_ushort(bitCast<unsigned short>(val)));
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
#ifdef RESL_BYTE_ORDER_BIG_ENDIAN
    return details::Endianness<T>::Swap(n);
#else
    return n;
#endif
}

// Converts from the native representation to big endian
template <details::arithmetic T>
inline T NativeToBigEndian(T n)
{
#ifdef RESL_BYTE_ORDER_BIG_ENDIAN
    return n;
#else
    return details::Endianness<T>::Swap(n);
#endif
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
