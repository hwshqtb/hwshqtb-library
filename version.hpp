#ifndef HWSHQTB__VERSION_HPP
#define HWSHQTB__VERSION_HPP

#if __cplusplus < 201103L // 98 (03)
#error only for c++11 and above
#endif

#include <cstddef>
#include <cstdint>
#include <limits>
#include <cmath>
#include <type_traits>
#include <stdexcept>
#include <cassert>
#include <utility>
#include <cstring>
#include <iterator>

namespace hwshqtb {
    template <typename Arithmetic>
    constexpr bool isless(Arithmetic a, Arithmetic b) {
        return std::numeric_limits<Arithmetic>::is_integer ? a < b : std::isless(a, b);
    }

    template <typename Arithmetic>
    constexpr bool islessequal(Arithmetic a, Arithmetic b) {
        return std::numeric_limits<Arithmetic>::is_integer ? a <= b : std::islessequal(a, b);
    }

    template <typename Arithmetic>
    constexpr bool islessgreater(Arithmetic a, Arithmetic b) {
        return std::numeric_limits<Arithmetic>::is_integer ? (a < b || a > b) : std::islessgreater(a, b);
    }

    template <typename Arithmetic>
    constexpr bool isunordered(Arithmetic a, Arithmetic b) {
        return a != a || b != b;
    }

    template <typename Arithmetic>
    constexpr bool isgreater(Arithmetic a, Arithmetic b) {
        return isless(b, a);
    }

    template <typename Arithmetic>
    constexpr bool isgreaterequal(Arithmetic a, Arithmetic b) {
        return islessequal(b, a);
    }

    template <typename Arithmetic>
    constexpr bool isequal(Arithmetic a, Arithmetic b) {
        return !isunordered(a, b) && !islessequal(a, b);
    }

    template <std::size_t>
    struct int_type;
    template <std::size_t>
    struct uint_type;
    template <std::size_t N>
    using int_t = typename int_type<N>::type;
    template <std::size_t N>
    using uint_t = typename uint_type<N>::type;
    template <>
    struct int_type<8> {
        using type = std::int8_t;
    };
    template <>
    struct int_type<16> {
        using type = std::int16_t;
    };
    template <>
    struct int_type<32> {
        using type = std::int32_t;
    };
    template <>
    struct int_type<64> {
        using type = std::int64_t;
    };
    template <>
    struct uint_type<8> {
        using type = std::uint8_t;
    };
    template <>
    struct uint_type<16> {
        using type = std::uint16_t;
    };
    template <>
    struct uint_type<32> {
        using type = std::uint32_t;
    };
    template <>
    struct uint_type<64> {
        using type = std::uint64_t;
    };
}
#if defined _MSC_VER
#include <__msvc_int128.hpp>
namespace hwshqtb{
    template <>
    struct int_type<128> {
        using type = std::_Signed128;
    };
    template <>
    struct uint_type<128> {
        using type = std::_Unsigned128;
    };
}
#elif defined(__GNUC__) || defined(__clang__)
namespace hwshqtb {
    template <>
    struct int_type<128> {
        using type = __int128_t;
    };
    template <>
    struct uint_type<128> {
        using type = __uint128_t;
    };
}
#endif

#if __cplusplus >= 201402L // 14
#define HWSHQTB_CONSTEXPR14 constexpr
namespace hwshqtb {
    using std::exchange;
}
#else
#define HWSHQTB_CONSTEXPR14
namespace hwshqtb {
    template<class T, class U = T>
    T exchange(T& obj, U&& new_value) {
        T&& old_value(std::move(obj));
        obj = std::forward<U>(new_value);
        return old_value;
    }
}
#endif

#if __cplusplus >= 201703L // 17
#define HWSHQTB_CONSTEXPR17 constexpr
namespace hwshqtb {
    using std::in_place_t;
    using std::in_place;
}
#else
#define HWSHQTB_CONSTEXPR17
namespace hwshqtb {
    struct in_place_t {};
    constexpr in_place_t is_place;
}
#endif

#if __cplusplus >= 202002L // 20
#define HWSHQTB_CONSTEXPR20 constexpr
#else
#define HWSHQTB_CONSTEXPR20
#endif

#if __cplusplus >= 202302L
#define HWSHQTB_CONSTEXPR23 constexpr
#else
#define HWSHQTB_CONSTEXPR23
#endif

#endif