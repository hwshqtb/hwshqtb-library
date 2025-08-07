#ifndef HWSHQTB__BIG_NUMBER__INTEGER_HPP
#define HWSHQTB__BIG_NUMBER__INTEGER_HPP

#include "natural.hpp"

namespace hwshqtb {
    namespace big_number {
        template <std::size_t Extent = std::numeric_limits<type>::digits>
        class integer {
            static_assert(Extent > 0, "");

        public:
            constexpr integer()noexcept:
                _signed(false), _number() {}
            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr integer(Integer v)noexcept:
                _signed(v < 0), _number(v < 0 ? -v : v) {}
            constexpr integer(const integer&) = default;
            constexpr integer(integer&& other)noexcept:
                _signed(std::exchange(other._signed, false)), _number(std::move(other._number)) {}
            ~integer() = default;

            constexpr integer& operator=(const integer&) = default;
            constexpr integer& operator=(integer&& other)noexcept {
                operator=(other);
                other.to_zero();
                return *this;
            }
            constexpr integer& to_quiet_NaN()noexcept {
                _signed = false;
                _number.to_NaN();
                return *this;
            }
            constexpr integer& to_signaling_NaN()noexcept {
                _signed = true;
                _number.to_NaN();
                return *this;
            }
            constexpr integer& to_positive_zero()noexcept {
                _signed = false;
                _number.to_zero();
                return *this;
            }
            constexpr integer& to_negative_zero()noexcept {
                _signed = true;
                _number.to_zero();
                return *this;
            }
            constexpr integer& to_positive_infinity()noexcept {
                _signed = false;
                _number.to_infinity();
                return *this;
            }
            constexpr integer& to_negative_infinity()noexcept {
                _signed = true;
                _number.to_infinity();
                return *this;
            }
            constexpr integer& to_max()noexcept {
                _signed = false;
                _number.to_max();
                return *this;
            }
            constexpr integer& to_min()noexcept {
                _signed = true;
                _number.to_max();
                return *this;
            }

            constexpr bool is_NaN()const noexcept {
                return _number.is_NaN();
            }
            constexpr bool is_quiet_NaN()const noexcept {
                return _number.is_NaN() && _signed == false;
            }
            constexpr bool is_signaling_NaN()const noexcept {
                return _number.is_NaN() && _signed == true;
            }
            constexpr bool is_zero()const noexcept {
                return _number.is_zero();
            }
            constexpr bool is_positive_zero()const noexcept {
                return _number.is_zero() && _signed == false;
            }
            constexpr bool is_negative_zero()const noexcept {
                return _number.is_zero() && _signed == true;
            }
            constexpr bool is_infinity()const noexcept {
                return _number.is_infinity();
            }
            constexpr bool is_positive_infinity()const noexcept {
                return _number.is_infinity() && _signed == false;
            }
            constexpr bool is_negative_infinity()const noexcept {
                return _number.is_infinity() && _signed == true;
            }
            static constexpr integer quiet_NaN()noexcept {
                integer result;
                return result.to_quiet_NaN();
            }
            static constexpr integer signaling_NaN()noexcept {
                integer result;
                return result.to_signaling_NaN();
            }
            static constexpr integer positive_zero()noexcept {
                integer result;
                return result.to_positive_zero();
            }
            static constexpr integer negative_zero()noexcept {
                integer result;
                return result.to_negative_zero();
            }
            static constexpr integer positive_infinity()noexcept {
                integer result;
                return result.to_positive_infinity();
            }
            static constexpr integer negative_infinity()noexcept {
                integer result;
                return result.to_negative_infinity();
            }
            static constexpr integer min()noexcept {
                integer result;
                return result.to_min();
            }
            static constexpr integer max()noexcept {
                integer result;
                return result.to_max();
            }

        private:
            bool _signed;
            natural<Extent> _number;

        };
    }
}

namespace std {
    template <size_t Extent>
    class numeric_limits<hwshqtb::big_number::integer<Extent>>: public numeric_limits<std::size_t> {
    public:
        static constexpr bool is_signed = true;
        static constexpr bool has_infinity = true;
        static constexpr bool has_quiet_NaN = true;
        static constexpr bool has_signaling_NaN = true;
        static constexpr bool is_modulo = false;
        static constexpr int digits = Extent * hwshqtb::big_number::lower_width;

        static constexpr hwshqtb::big_number::integer<Extent> min() noexcept {
            return hwshqtb::big_number::integer<Extent>::min();
        }
        static constexpr hwshqtb::big_number::integer<Extent> lowest() noexcept {
            return hwshqtb::big_number::integer<Extent>::min();
        }
        static constexpr hwshqtb::big_number::integer<Extent> max() noexcept {
            return hwshqtb::big_number::integer<Extent>::max();
        }
        static constexpr hwshqtb::big_number::integer<Extent> epsilon() noexcept {
            return hwshqtb::big_number::integer<Extent>::zero();
        }
        static constexpr hwshqtb::big_number::integer<Extent> round_error() noexcept {
            return hwshqtb::big_number::integer<Extent>::zero();
        }
        static constexpr hwshqtb::big_number::integer<Extent> infinity() noexcept {
            return hwshqtb::big_number::integer<Extent>::positive_infinity();
        }
        static constexpr hwshqtb::big_number::integer<Extent> quiet_NaN() noexcept {
            return hwshqtb::big_number::integer<Extent>::quiet_NaN();
        }
        static constexpr hwshqtb::big_number::integer<Extent> signaling_NaN() noexcept {
            return hwshqtb::big_number::integer<Extent>::signaling_NaN();
        }
        static constexpr hwshqtb::big_number::integer<Extent> denorm_min() noexcept {
            return hwshqtb::big_number::integer<Extent>::zero();
        }
    };
}

#endif