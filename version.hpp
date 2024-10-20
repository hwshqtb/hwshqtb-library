#ifndef HWSHQTB__BASIC_HPP
#define HWSHQTB__BASIC_HPP

#if __cplusplus < 201103L // 98 (03)
#error only for c++11 and above
#endif

#if __cplusplus >= 201402L // 14
#define HWSHQTB_CONSTEXPR14 constexpr
#include <utility>
namespace hwshqtb {
    using std::exchange;
}
#else
#define HWSHQTB_CONSTEXPR14
#include <utility>
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
#else
#define HWSHQTB_CONSTEXPR17
#endif

#if __cplusplus >= 202002L // 20
#define HWSHQTB_CONSTEXPR20 constexpr
#else
#define HWSHQTB_CONSTEXPR20
#endif

#endif