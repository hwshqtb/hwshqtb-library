#ifndef _HWSHQTB_BIG_NUMBER_DATA_HPP
#define _HWSHQTB_BIG_NUMBER_DATA_HPP

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace hwshqtb {
    namespace number {
        template <std::size_t N>
        struct fixed_width_integer {
            using type = std::conditional_t<N == 1, std::uint8_t,
                std::conditional_t<N == 2, std::uint16_t,
                std::conditional_t<N == 4, std::uint32_t,
                std::uint64_t>>>;
        };

        using base_type = fixed_width_integer<sizeof(std::size_t)>::type;
        using base_half_type = fixed_width_integer<sizeof(std::size_t) / 2>::type;
        inline constexpr base_type lower_half = (base_half_type)-1, upper_half = ((base_type)-1) ^ lower_half;

        union data_t {
            constexpr data_t(base_type v)noexcept :
                _short{v & lower_half, v & upper_half} {}
            constexpr data_t(base_type* memory, base_type length)noexcept :
                _whole{memory, length} {}

            constexpr base_type& operator[](std::size_t i)const noexcept {
                if (i < 2) return _short[i];
                else return _whole._memory[i];
            }
            constexpr const base_type length()const noexcept {
                return 
            }

            mutable struct whole_t {
                base_type* _memory;
                base_type _length;
            }_whole;
            mutable base_type _short[2];
        };
    }
}

#endif