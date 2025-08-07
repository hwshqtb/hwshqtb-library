#ifndef HWSHQTB__BIG_NUMBER__BASE_HPP
#define HWSHQTB__BIG_NUMBER__BASE_HPP

#include "../version.hpp"

namespace hwshqtb {
    namespace big_number {
        constexpr const char* NaN_str[2] = {"nan", "NAN"};
        constexpr const char* infinity_str[2] = {"inf", "INF"};
        constexpr const char* hex_str[2] = {"0123456789abcdef", "0123456789ABCDEF"};
    }
}

#endif