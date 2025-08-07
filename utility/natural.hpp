#ifndef HWSHQTB__BIG_NUMBER__NATURAL_HPP
#define HWSHQTB__BIG_NUMBER__NATURAL_HPP

#include "base.hpp"
#include <iostream>
#include <array>
#include <list>

namespace hwshqtb {
    namespace big_number {
        template <typename T = std::size_t, class Container = std::array<T, 1024 / sizeof(T) / CHAR_BIT>>
        class natural {
        public:
            using container_type = Container;
            using value_type = typename container_type::value_type;
            using size_type = typename container_type::size_type;
            using reference = typename container_type::reference;
            using const_reference = typename container_type::const_reference;

        private:
            using value_type_property = std::numeric_limits<value_type>;

        public:
            static_assert(std::is_same<T, value_type>::value, "");
            static_assert(std::is_integral<value_type>::value && !value_type_property::is_signed, "");

            static constexpr value_type type_width = value_type_property::digits;
            static constexpr value_type lower_width = type_width / 2;
            static constexpr value_type upper_width = type_width - lower_width;
            static constexpr value_type base = (value_type)1 << lower_width;
            static constexpr value_type lower_half = ((value_type)1 << lower_width) - 1;
            static constexpr value_type upper_half = (((value_type)1 << upper_width) - 1) << lower_width;

            template <typename... Ts>
            HWSHQTB_CONSTEXPR14 natural(value_type v = 0, Ts&&... ts):
                _memory(std::forward<Ts>(ts)...), _length(0) {
                assign(v);
            }
            template <typename... Ts>
            HWSHQTB_CONSTEXPR14 natural(in_place_t, Ts&&... ts) :
                _memory(std::forward<Ts>(ts)...), _length(remove_zero(_memory.size())) {}
            constexpr natural(const natural&) = default;
            constexpr natural(natural&&) = default;

            template <typename Integer, typename... Ts, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            HWSHQTB_CONSTEXPR14 natural& operator=(Integer v)
            {
                assign(v);
                return *this;
            }
            constexpr natural& operator=(const natural&) = default;

            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            HWSHQTB_CONSTEXPR14 void assign(Integer v)noexcept {
                if (v < 0) to_inf();
                else if (v == 0) to_zero();
                else {
                    _length = 0;
                    while (v != 0 && _length < _memory.size()) {
                        _memory[_length++] = v & lower_half;
                        v = v / base;
                    }
                    if (v) to_inf();
                }
            }

            constexpr value_type operator[](std::size_t index)const noexcept {
                return index >= _length ? 0 : _memory[index];
            }

            constexpr bool is_NaN()const noexcept {
                return _length == 0;
            }
            constexpr bool is_inf()const noexcept {
                return _length == _memory.size() + 1;
            }
            constexpr bool is_zero()const noexcept {
                return _length == 1 && _memory[0] == 0;
            }

            constexpr natural& to_NaN()noexcept {
                _length = 0;
                return *this;
            }
            constexpr natural& to_zero()noexcept {
                _length = 1;
                _memory[0] = 0;
                return *this;
            }
            constexpr natural& to_inf()noexcept {
                _length = _memory.size() + 1;
                return *this;
            }
            constexpr natural& to_max()noexcept {
                _length = 0;
                while (_length < _memory.size())
                    _memory[_length++] = lower_half;
                return *this;
            }

            // \    0   NaN inf x
            // 0    0   NaN inf x
            // NaN  NaN NaN NaN NaN
            // inf  inf NaN inf x
            // x    x   NaN inf ?
            HWSHQTB_CONSTEXPR14 natural& operator+=(const natural& other)noexcept {
                if (is_NaN()) return *this;
                if (other.is_NaN()) return to_NaN();
                if (is_inf()) return *this;
                if (other.is_inf()) return to_inf();
                if (is_zero()) return operator=(other);
                if (other.is_zero()) return *this;
                _length = std::max(_length, other._length);
                value_type carry = 0;
                for (std::size_t i = 0; i < _length; ++i) {
                    carry = ((_memory[i] += other[i] + carry) & upper_half) >> lower_width;
                    _memory[i] &= lower_half;
                }
                if (carry) {
                    if (_length < _memory.size())
                        _memory[_length] = carry;
                    ++_length;
                }
                return *this;
            }
            // \    0   NaN inf x
            // 0    0   NaN inf inf
            // NaN  NaN NaN NaN NaN
            // inf  inf NaN NaN inf
            // x    x   NaN inf ?
            HWSHQTB_CONSTEXPR14 natural& operator-=(const natural& other)noexcept {
                if (is_NaN()) return *this;
                if (other.is_NaN() || (is_inf() && other.is_inf())) return to_NaN();
                if (is_inf()) return *this;
                if (other.is_zero()) return *this;
                if (other.is_inf() || (is_zero() && !other.is_zero()) || _length < other._length) return to_inf();
                value_type borrow = 0;
                for (std::size_t i = 0; i < _length; ++i) {
                    if (_memory[i] < borrow + other[i]) {
                        _memory[i] += base - borrow - other[i];
                        borrow = 1;
                    }
                    else {
                        _memory[i] -= borrow + other[i];
                        borrow = 0;
                    }
                }
                if (borrow == 1) to_inf();
                else remove_zero();
                return *this;
            }
            // \    0   NaN inf x
            // 0    0   NaN NaN 0
            // NaN  NaN NaN NaN NaN
            // inf  NaN NaN inf inf
            // x    0   NaN inf ?
            HWSHQTB_CONSTEXPR14 natural& operator*=(const natural& other)noexcept {
                if (is_NaN()) return *this;
                if (other.is_NaN() || (is_zero() && other.is_inf()) || (other.is_zero() && is_inf())) return to_NaN();
                if (is_zero()) return *this;
                if (other.is_zero()) return to_zero();
                if (is_inf()) return *this;
                if (other.is_inf() || _length + other._length >= _memory.size() + 2) return to_inf();
                natural x(*this);
                std::memset(_memory, 0, (_length + other._length - 1) * type_width);
                for (std::size_t i = 0; i < other._length; ++i) {
                    for (std::size_t j = 0; j < other._length; ++j) {
                        if ((_memory[i + j] += x[i] * other[j]) & upper_half) {
                            if (i + j + 1 == _memory.size()) return to_inf();
                            _memory[i + j + 1] += _memory[i + j] >> lower_width;
                            _memory[i + j] &= lower_half;
                        }
                    }
                }
                return *this;
            }
            // \    0   NaN inf x
            // 0    NaN NaN 0   0
            // NaN  NaN NaN NaN NaN
            // inf  NaN NaN NaN inf
            // x    NaN NaN 0   ?
            constexpr natural& operator/=(const natural& other)noexcept {
                if (is_NaN()) return *this;
                if (other.is_NaN() || other.is_zero() || (is_inf() && other.is_inf())) return to_NaN();
                if (is_zero()) return *this;
                if (other.is_inf() || operator<(other)) return to_zero();
                if (is_inf()) return *this;
                natural result;
                value_type v = other[other._length - 1] + 1;
                do {
                    value_type remainder = 0, index = _length, result_index = _length - other._length + 1;
                    while (remainder < v) {
                        remainder <<= lower_width;
                        remainder += _memory[--index];
                        if (--result_index == 0) break;
                    }
                    if (remainder < v) break;
                    value_type x = remainder / v;
                    result._memory[result_index] = result[result_index] + x;
                    natural mul_other(other);
                    mul_other.base_multiply(x);
                    mul_other.shift_left(result_index);
                    operator-=(mul_other);
                }
                while (true);
                if (*this > other)
                    ++result;
                *this = result;
                remove_zero();
                return *this;
            }
            // \    0   NaN inf x
            // 0    NaN NaN 0   0
            // NaN  NaN NaN NaN NaN
            // inf  NaN NaN NaN NaN
            // x    NaN NaN 0   ?
            constexpr natural& operator%=(const natural& other)noexcept {
                if (is_NaN()) return *this;
                if (other.is_NaN() || other.is_zero() || is_inf()) return to_NaN();
                if (is_zero() || other.is_inf() || operator<(other)) return *this;
                std::size_t i = _length + 1 - other._length;
                do {
                    --i;
                    natural x{_memory, i, _length};
                    while (x >= other)
                        x -= other;
                    for (std::size_t j = 0; j < x._length; ++j)
                        _memory[i + j] = x._memory[j];
                    _length = i + x._length;
                }
                while (i > 0);
                while (_length > 0 && _memory[_length - 1] == 0) --_length;
                return *this;
            }
            constexpr natural& operator&=(const natural& other)noexcept {
                if (is_NaN()) return *this;
                if (other.is_NaN()) return to_NaN();
                if (is_zero()) return *this;
                if (other.is_zero()) return to_zero();
                if (is_inf()) return *this;
                if (other.is_inf()) return to_inf();
                _length = std::max(_length, other._length);
                for (int i = 0; i < _length; ++i)
                    _memory[i] &= other[i];
                while (_length > 1 && _memory[_length - 1] == 0) --_length;
                return *this;
            }
            constexpr natural& operator|=(const natural& other)noexcept {
                if (is_NaN()) return *this;
                if (other.is_NaN()) return to_NaN();
                if (is_inf()) return *this;
                if (other.is_inf()) return to_inf();
                if (is_zero()) return operator=(other);
                if (other.is_zero()) return *this;
                _length = std::max(_length, other._length);
                for (int i = 0; i < _length; ++i)
                    _memory[i] |= other[i];
                while (_length > 1 && _memory[_length - 1] == 0) --_length;
                return *this;
            }
            constexpr natural& operator^=(const natural& other)noexcept {
                if (is_NaN() || is_inf()) return *this;
                if (other.is_NaN()) return to_NaN();
                if (other.is_inf()) return to_inf();
                if (is_zero()) return operator=(other).flip();
                _length = std::max(_length, other._length);
                for (int i = 0; i < _length; ++i)
                    _memory[i] ^= other[i];
                while (_length > 1 && _memory[_length - 1] == 0) --_length;
                return *this;
            }
            constexpr natural& operator<<=(const natural& other)noexcept {
                if (is_NaN()) return *this;
                if (other.is_NaN()) return to_NaN();
                if (is_zero() || other.is_zero() || is_inf()) return *this;
                if (other.is_inf()) return to_inf();
                natural x(other);
                value_type remaining = x.div(lower_width);
                value_type block = x;
                if (block) {
                    std::size_t index = _length + block - 1, index2 = _length - 1;
                    if (index >= _memory.size() + 1) return to_inf();
                    while (index != block - 1)
                        _memory[index--] = _memory[index2--];
                    ++index;
                    do _memory[--index] = 0;
                    while (index > 0);
                }
                if (remaining) {
                    std::size_t index = _length + block - 1;
                    if ((_memory[index] >> (lower_width - remaining)) && index == _memory.size()) return to_inf();
                    ++index;
                    do {
                        --index;
                        _memory[index + 1] |= (_memory[index] >> (lower_width - remaining));
                        _memory[index] = (_memory[index] << remaining) & lower_half;
                    }
                    while (index > 0);
                }
                _length = _memory[_length + block] ? _length + block : _length + block - 1;
                return *this;
            }
            constexpr natural& operator>>=(const natural& other)noexcept {
                if (is_NaN()) return *this;
                if (other.is_NaN()) return to_NaN();
                if (is_zero() || other.is_zero() || is_inf()) return *this;
                if (other.is_inf()) return to_zero();
                natural x(other);
                value_type remaining = x.div(lower_width);
                value_type block = x;
                if (block) {
                    std::size_t index = 0, index2 = block;
                    if (block >= _length) return to_zero();
                    while (index2 != _length)
                        _memory[index++] = _memory[index2++];
                    while (index != _length)
                        _memory[index++] = 0;
                }
                if (remaining) {
                    std::size_t index = 0;
                    if (_length == 1 && !(_memory[0] >> remaining)) return to_zero();
                    while (index < _length - block) {
                        _memory[index] >>= remaining;
                        _memory[index] |= _memory[index + 1] >> (lower_width - remaining);
                    }
                }
                _length = _memory[_length - block - 1] ? _length - block : _length - block - 1;
                if (_length == 0) _length = 1;
                return *this;
            }
            constexpr natural& filp()noexcept {
                if (is_NaN() || is_inf()) return *this;
                for (std::size_t i = 0; i < _length; ++i)
                    _memory[i] = (~_memory[i]) & lower_half;
                for (std::size_t i = _length; i < _memory.size(); ++i)
                    _memory[i] = lower_half;
                _length = _memory.size();
                while (_length > 1 && _memory[_length - 1] == 0) --_length;
                return *this;
            }
            constexpr natural div(const natural& other)noexcept {
                if (is_NaN() || other.is_NaN() || other.is_zero() || (is_inf() && other.is_inf())) {
                    to_NaN();
                    return NaN();
                }
                if (is_inf()) {
                    to_NaN();
                    return infinity();
                }
                if (is_zero() || other.is_inf() || operator<(other)) return zero();
                natural result;
                result._length = _length - other._length + 1;
                std::size_t i = _length - other._length + 1;
                do {
                    --i;
                    natural x{_memory, i, _length};
                    while (x > other) {
                        x -= other;
                        ++result._memory[i];
                    }
                    if (x == other) ++result._memory[i];
                    for (std::size_t j = 0; j < x._length; ++j)
                        _memory[i + j] = x._memory[j];
                    _length = i + x._length;
                }
                while (i > 0);
                while (_length > 0 && _memory[_length - 1] == 0) --_length;
                return result;
            }
            constexpr natural& operator++()noexcept {
                return operator+=(1);
            }
            constexpr natural& operator++(int)noexcept {
                natural result = *this;
                operator+=(1);
                return result;
            }
            constexpr natural& operator--()noexcept {
                return operator-=(1);
            }
            constexpr natural& operator--(int)noexcept {
                natural result = *this;
                operator-=(1);
                return result;
            }

            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr natural& operator+=(Integer other)noexcept {
                if (is_NaN() || is_inf()) return *this;
                if (is_zero()) return operator=(other);
                if (other == 0) return *this;
                if (other < 0) return operator-=(-other);
                std::common_type_t<value_type, Integer> v = other;
                value_type carry = 0;
                for (std::size_t i = 0; i < _length && (carry != 0 || v != 0); ++i) {
                    carry = (_memory[i] += carry + (v & lower_half)) >> lower_width;
                    _memory[i] &= lower_half;
                    v >>= lower_width;
                }
                while (_length < _memory.size() && (carry != 0 || v != 0)) {
                    carry = (_memory[_length] += carry + (v & lower_half)) >> lower_width;
                    _memory[_length++] &= lower_half;
                    v >>= lower_width;
                }
                if (carry != 0 || v != 0) return to_inf();
                return *this;
            }
            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr natural& operator-=(Integer other)noexcept {
                if (is_NaN() || is_inf() || other == 0) return *this;
                if (other < 0) return operator+=(-other);
                if (is_zero()) return to_inf();
                std::common_type_t<value_type, Integer> v = other;
                value_type borrow = 0;
                for (std::size_t i = 0; i < _length && (borrow != 0 || v != 0); ++i) {
                    value_type sub = v & lower_half;
                    v >>= lower_width;
                    if (_memory[i] < borrow + sub) {
                        _memory[i] += lower_half - borrow - sub;
                        borrow = 1;
                    }
                    else {
                        _memory[i] -= borrow - sub;
                        borrow = 0;
                    }
                }
                if (borrow != 0 || v != 0) return to_zero();
                while (_length > 0 && _memory[_length] == 0) --_length;
                return *this;
            }
            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr natural& operator*=(Integer other)noexcept {
                if (is_NaN()) return *this;
                if (other == 0 && is_inf()) return to_NaN();
                if (is_inf() || is_zero()) return *this;
                if (other == 0) return to_zero();
                if (other < 0) return to_inf();
                std::common_type_t<value_type, Integer> v = other;
                natural x(*this);
                to_zero();
                while (!is_inf() && v != 0) {
                    natural y(x);
                    y.multiply_base(v & lower_half);
                    operator+=(y);
                    v >>= lower_width;
                    if (v != 0)
                        x.shift_left(1);
                }
                return *this;
            }
            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr natural& operator/=(Integer other)noexcept {
                if (is_NaN()) return *this;
                if (other == 0) return to_NaN();
                if (is_inf() || is_zero()) return *this;
                if (other < 0) return to_inf();
                if (operator<(other)) return to_zero();
                if (other > base) return operator/=(natural(other));
                std::common_type_t<value_type, Integer> v = other;
                std::common_type_t<value_type, Integer> remaining = 0;
                std::size_t i = _length;
                do {
                    remaining = remaining * base + _memory[--i];
                    if (remaining >= v) {
                        _memory[i] = remaining / v;
                        remaining %= v;
                    }
                    else _memory[i] = 0;
                }
                while (i != 0);
                while (_length > 1 && _memory[_length - 1] == 0) --_length;
                return *this;
            }
            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr natural& operator%=(Integer other)noexcept {
                if (is_NaN()) return *this;
                if (other == 0 || is_inf()) return to_NaN();
                if (other < 0) return to_inf();
                if (is_zero() || operator<(other)) return *this;
                if (other > base) return operator%=(natural(other));
                std::common_type_t<value_type, Integer> v = other;
                std::common_type_t<value_type, Integer> remaining = 0;
                do remaining = (remaining * base + _memory[--_length]) % v;
                while (_length != 0);
                return operator=(remaining);
            }
            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr natural& operator&=(Integer other)noexcept {
                if (is_NaN()) return *this;
                if (is_zero()) return *this;
                if (other == 0) return to_zero();
                if (is_inf()) return *this;
                std::common_type_t<value_type, Integer> v = other;
                std::size_t i = 0;
                while (i < _length && v != 0) {
                    _memory[i++] &= v & lower_half;
                    v >>= lower_width;
                }
                while (_length > 1 && _memory[_length - 1] == 0) --_length;
                return *this;
            }
            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr natural& operator|=(Integer other)noexcept {
                if (is_NaN()) return *this;
                if (is_zero()) return *this = other;
                if (other == 0) return *this;
                if (is_inf()) return *this;
                std::common_type_t<value_type, Integer> v = other;
                std::size_t i = 0;
                while (i < _length && v != 0) {
                    _memory[i++] |= v & lower_half;
                    v >>= lower_width;
                }
                while (_length < _memory.size() && v != 0) {
                    _memory[_length++] = v & lower_half;
                    v >>= lower_width;
                }
                return *this;
            }
            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr natural& operator^=(Integer other)noexcept {
                if (is_NaN()) return *this;
                if (is_zero()) return *this = other;
                if (other == 0) return *this;
                if (is_inf()) return *this;
                std::common_type_t<value_type, Integer> v = other;
                std::size_t i = 0;
                while (i < _length && v != 0) {
                    _memory[i++] ^= v & lower_half;
                    v >>= lower_width;
                }
                while (_length < _memory.size() && v != 0) {
                    _memory[_length++] = v & lower_half;
                    v >>= lower_width;
                }
                while (_length > 1 && _memory[_length - 1] == 0) --_length;
                return *this;
            }
            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr natural& operator<<=(Integer other)noexcept {
                if (is_NaN()) return *this;
                if (is_zero() || other == 0 || is_inf()) return *this;
                if (other < 0) return to_inf();
                std::common_type_t<value_type, Integer> v = other;
                value_type remaining = v % lower_width;
                v /= lower_width;
                if (v)
                    shift_left(v);
                if (remaining) {
                    std::size_t index = _length - 1;
                    if ((_memory[index] >> (lower_width - remaining)) && index == _memory.size()) return to_inf();
                    ++index;
                    _memory[index] = 0;
                    do {
                        --index;
                        _memory[index + 1] |= (_memory[index] >> (lower_width - remaining));
                        _memory[index] = (_memory[index] << remaining) & lower_half;
                    }
                    while (index > 0);
                    _length = _memory[_length] ? _length : _length - 1;
                }
                return *this;
            }
            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr natural& operator>>=(Integer other)noexcept {
                if (is_NaN()) return *this;
                if (is_zero() || other == 0 || is_inf()) return *this;
                if (other < 0) return to_zero();
                std::common_type_t<value_type, Integer> v = other;
                value_type remaining = v % lower_width;
                v /= lower_width;
                if (v) {
                    std::size_t index = 0, index2 = v;
                    if (v >= _length) return to_zero();
                    while (index2 != _length)
                        _memory[index++] = _memory[index2++];
                    while (index != _length)
                        _memory[index++] = 0;
                }
                if (remaining) {
                    std::size_t index = 0;
                    if (_length == 1 && !(_memory[0] >> remaining)) return to_zero();
                    while (index < _length - v) {
                        _memory[index] >>= remaining;
                        _memory[index] |= _memory[index + 1] >> (lower_width - remaining);
                    }
                }
                _length = _memory[_length - v - 1] ? _length - v : _length - v - 1;
                if (_length == 0) _length = 1;
                return *this;
            }
            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr natural div(Integer other)noexcept {
                if (is_NaN() || other == 0) {
                    to_NaN();
                    return NaN();
                }
                if (is_inf()) {
                    to_NaN();
                    return infinity();
                }
                if (is_zero() || operator<(other)) return zero();
                if (other > base) return div(natural(other));
                std::common_type_t<value_type, Integer> v = other;
                std::common_type_t<value_type, Integer> remaining = 0;
                std::size_t i = _length;
                do {
                    remaining = remaining * base + _memory[--i];
                    if (remaining >= v) {
                        _memory[i] = remaining / v;
                        remaining %= v;
                    }
                    else _memory[i] = 0;
                }
                while (i != 0);
                while (_length > 1 && _memory[_length - 1] == 0) --_length;
                return natural(remaining);
            }

            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr explicit operator Integer()const noexcept {
                Integer result = 0;
                std::size_t i = _length;
                do result = result * base + _memory[--i];
                while (i != 0);
                return result;
            }

            constexpr bool operator==(const natural& other)const noexcept {
                if (is_NaN() || is_inf() || other.is_NaN() || other.is_inf()) return false;
                if (_length != other._length) return false;
                std::size_t i = _length;
                do {
                    --i;
                    if (_memory[i] != other[i]) return false;
                }
                while (i > 0);
                return true;
            }
            constexpr bool operator!=(const natural& other)const noexcept {
                if (is_NaN() || other.is_NaN()) return false;
                if (is_inf() || other.is_inf()) return true;
                if (_length != other._length) return true;
                std::size_t i = _length;
                do {
                    --i;
                    if (_memory[i] != other[i]) return true;
                }
                while (i > 0);
                return false;
            }
            constexpr bool operator<(const natural& other)const noexcept {
                if (is_NaN() || is_inf() || other.is_NaN() || other.is_inf()) return false;
                if (_length != other._length) return _length < other._length;
                std::size_t i = _length;
                do {
                    --i;
                    if (_memory[i] != other[i]) return _memory[i] < other[i];
                }
                while (i > 0);
                return false;
            }
            constexpr bool operator<=(const natural& other)const noexcept {
                if (is_NaN() || is_inf() || other.is_NaN() || other.is_inf()) return false;
                if (_length != other._length) return _length < other._length;
                std::size_t i = _length;
                do {
                    --i;
                    if (_memory[i] != other[i]) return _memory[i] < other[i];
                }
                while (i > 0);
                return true;
            }
            constexpr bool operator>(const natural& other)const noexcept {
                if (is_NaN() || is_inf() || other.is_NaN() || other.is_inf()) return false;
                if (_length != other._length) return _length > other._length;
                std::size_t i = _length;
                do {
                    --i;
                    if (_memory[i] != other[i]) return _memory[i] > other[i];
                }
                while (i > 0);
                return false;
            }
            constexpr bool operator>=(const natural& other)const noexcept {
                if (is_NaN() || is_inf() || other.is_NaN() || other.is_inf()) return false;
                if (_length != other._length) return _length > other._length;
                std::size_t i = _length;
                do {
                    --i;
                    if (_memory[i] != other[i]) return _memory[i] > other[i];
                }
                while (i > 0);
                return true;
            }

            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr bool operator==(Integer other)const noexcept {
                if (is_NaN() || is_inf()) return false;
                std::common_type_t<value_type, Integer> v = other;
                std::size_t i = 0;
                for (; i < _length && v != 0; ++i) {
                    if (_memory[i] != (v & lower_half)) return false;
                    v >>= lower_width;
                }
                return i == _length && v == 0;
            }
            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr bool operator!=(Integer other)const noexcept {
                if (is_NaN()) return false;
                if (is_inf()) return true;
                std::common_type_t<value_type, Integer> v = other;
                std::size_t i = 0;
                for (; i < _length && v != 0; ++i) {
                    if (_memory[i] != (v & lower_half)) return true;
                    v >>= lower_width;
                }
                return i != _length || v != 0;
            }
            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr bool operator<(Integer other)const noexcept {
                if (is_NaN() || is_inf()) return false;
                std::common_type_t<value_type, Integer> v = other;
                value_type state = 2;
                std::size_t i = 0;
                for (; i < _length && v != 0; ++i) {
                    if (_memory[i] != (v & lower_half)) state = _memory[i] < (v & lower_half);
                    v = v >> lower_width;
                }
                if (i == _length && v == 0) return state == 1;
                return i == _length;
            }
            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr bool operator<=(Integer other)const noexcept {
                if (is_NaN() || is_inf()) return false;
                std::common_type_t<value_type, Integer> v = other;
                value_type state = 2;
                std::size_t i = 0;
                for (; i < _length && v != 0; ++i) {
                    if (_memory[i] != (v & lower_half)) state = _memory[i] < (v & lower_half);
                    v >>= lower_width;
                }
                if (i == _length && v == 0) return state != 0;
                return i == _length;
            }
            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr bool operator>(Integer other)const noexcept {
                if (is_NaN() || is_inf()) return false;
                std::common_type_t<value_type, Integer> v = other;
                value_type state = 2;
                std::size_t i = 0;
                for (; i < _length && v != 0; ++i) {
                    if (_memory[i] != (v & lower_half)) state = _memory[i] > (v & lower_half);
                    v >>= lower_width;
                }
                if (i == _length && v == 0) return state == 1;
                return v == 0;
            }
            template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr bool operator>=(Integer other)const noexcept {
                if (is_NaN() || is_inf()) return false;
                std::common_type_t<value_type, Integer> v = other;
                value_type state = 2;
                std::size_t i = 0;
                for (; i < _length && v != 0; ++i) {
                    if (_memory[i] != (v & lower_half)) state = _memory[i] > (v & lower_half);
                    v >>= lower_width;
                }
                if (i == _length && v == 0) return state != 0;
                return v == 0;
            }

            /*template <typename CharT, class Traits = std::char_traits<CharT>>
            static constexpr natural from_c_str(const CharT* pointer, std::size_t N, int base = 10, Traits traits = {}) {
                natural result;
                if (N == 0)
                    return result;
                value_type x = 0;
                if (base == 0) {
                    if (*pointer == )
                }
                if (base == 2) {

                }
                return result;
            }
            template <typename CharT, class Traits>
            constexpr void to_c_str(CharT* pointer, std::size_t N, Traits traits) {
                return {};
            }*/
            template <typename CharT, class Traits>
            friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const natural& x) {
                //stage 0 check ostream
                typename std::basic_ostream<CharT, Traits>::sentry check_sentry(os);
                if (!check_sentry) return os;

                // stage 1
                std::ios_base::fmtflags basefield = (os.flags() & std::ios_base::basefield),
                    uppercase = (os.flags() & std::ios_base::uppercase),
                    showpos = (os.flags() & std::ios_base::showpos),
                    showbase = (os.flags() & std::ios_base::showbase);
                std::list<char> str;
                if (x.is_NaN())
                    for (std::size_t i = 0; i < 3; ++i)
                        str.push_back(NaN_str[uppercase ? 1 : 0][i]);
                else if (x.is_inf())
                    for (std::size_t i = 0; i < 3; ++i)
                        str.push_back(infinity_str[uppercase ? 1 : 0][i]);
                else if (x.is_zero()) {
                    if (basefield & std::ios_base::hex) str.insert(str.cend(), {'0', (uppercase ? 'X' : 'x'), '0'});
                    else str.push_back('0');
                }
                else {
                    if (basefield & std::ios_base::hex) {
                        std::size_t i = x._length;
                        do {
                            --i;
                            value_type t = x._memory[i] & lower_half;
                            for (std::size_t _ = 0; _ < lower_width / 4; ++_, t <<= 4)
                                str.push_back(hex_str[uppercase ? 1 : 0][(t & ((value_type)0xf << (lower_width - 4))) >> (lower_width - 4)]);
                        }
                        while (i > 0);
                        while (str.front() == '0') str.pop_front();
                        if (showbase) str.insert(str.cbegin(), {'0', (uppercase ? 'X' : 'x')});
                    }
                    else if (basefield & std::ios_base::oct) {
                        value_type t = 0;
                        for (std::size_t i = 0; i < x._length; ++i) {
                            t += (x._memory[i] << (i % 3 * lower_width % 3));
                            while (t >= 0x8) {
                                str.push_front((t & 0x7) + '0');
                                t >>= 3;
                            }
                        }
                        if (t) str.push_front((int)t + '0');
                        while (str.front() == '0') str.pop_front();
                        if (showbase) str.push_front('0');
                    }
                    else {
                        natural t = x;
                        while (!t.is_zero()) {
                            str.push_front((int)(t % 10) + '0');
                            t /= 10;
                        }
                    }
                }
                if (showpos && (basefield & std::ios_base::dec) != 0) str.push_front('+');

                //stage 2
                std::list<CharT> str2;
                typename std::list<CharT>::const_iterator special;
                const std::ctype<CharT>& ctype = std::use_facet<std::ctype<CharT>>(os.getloc());
                const std::numpunct<CharT>& numpunct = std::use_facet<std::numpunct<CharT>>(os.getloc());
                bool pd = true;
                for (char c : str) {
                    str2.push_back(ctype.widen(c));
                    bool t = true;
                    for (std::size_t i = 0; i < 16; ++i)
                        if (c == hex_str[uppercase ? 1 : 0][i])
                            t = false;
                    if (t) special = --str2.cend(), pd = false;
                }
                if (pd) special = str2.cbegin();
                std::size_t gap = 0;
                typename std::list<CharT>::const_iterator iter = str2.cend();
                CharT thousands_sep = numpunct.thousands_sep();
                for (char gapc : numpunct.grouping()) {
                    gap = gapc;
                    for (std::size_t i = 0; i < gap && iter != special; ++i)
                        --iter;
                    if (iter == special)
                        break;
                    iter = str2.insert(iter, thousands_sep);
                }
                while (gap) {
                    for (std::size_t i = 0; i < gap && iter != special; ++i)
                        --iter;
                    if (iter == special)
                        break;
                    iter = str2.insert(iter, thousands_sep);
                }

                //stage 3
                std::ios_base::fmtflags adjustfield = (os.flags() & (std::ios_base::adjustfield));
                std::size_t need_to_add = ((std::size_t)os.width() > str2.size() ? os.width() - str2.size() : 0);
                if (adjustfield == std::ios_base::left)
                    str2.insert(str2.cend(), need_to_add, os.fill());
                else if (adjustfield == std::ios_base::right)
                    str2.insert(str2.cbegin(), need_to_add, os.fill());
                else if (adjustfield == std::ios_base::internal)
                    str2.insert(++special, need_to_add, os.fill());
                else str2.insert(str2.cbegin(), need_to_add, os.fill());
                os.width(0);

                //stage 4
                std::ostreambuf_iterator<CharT, Traits> outer(os);
                for (auto c : str2)
                    *outer++ = c;

                return os;
            }
            template <typename CharT, class Traits>
            friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, natural& x) {
                // stage 0 check istream
                typename std::basic_istream<CharT, Traits>::sentry check_sentry(is);
                if (!check_sentry) return is;

                // stage 1
                std::ios_base::fmtflags basefield = (is.flags() & std::ios_base::basefield);

                // stage 2
                const std::ctype<CharT>& ctype = std::use_facet<std::ctype<CharT>>(is.getloc());
                const std::numpunct<CharT>& numpunct = std::use_facet<std::numpunct<CharT>>(is.getloc());
                std::list<std::size_t> thousands_seps;
                std::list<std::size_t> digits;
                const std::string& gaps = numpunct.grouping();
                bool has_plus = true, need_x = false;
                for (std::istreambuf_iterator<CharT, Traits> in(is), end; true; ++in) {
                    if (in == end) {
                        is.setstate(std::ios_base::eofbit);
                        break;
                    }
                    CharT c = *in;
                    if (Traits::eq(c, ctype.widen('+'))) {
                        if (digits.size() != 0 || has_plus)
                            break;
                        has_plus = true;
                    }
                    else if (Traits::eq(c, numpunct.decimal_point()))
                        break;
                    else if (Traits::eq(c, numpunct.thousands_sep())) {
                        if (x.is_NaN() || gaps.size() == 0)
                            break;
                        thousands_seps.push_front(digits.size());
                    }
                    else if (Traits::eq(c, ctype.widen('x')) || Traits::eq(c, ctype.widen('X'))) {
                        if (basefield == std::ios_base::hex && need_x)
                            digits.pop_front(), need_x = false;
                        else break;
                    }
                    else if (Traits::eq(c, ctype.widen('0'))) {
                        if (basefield == std::ios_base::hex) {
                            if (need_x) need_x = false;
                            else {
                                if (digits.size() == 0) need_x = true;
                                digits.push_front(0);
                            }
                        }
                        else {
                            digits.push_front(0);
                        }
                    }
                    else {
                        bool pd = true;
                        for (char to = '1'; to <= '7' && pd; ++to)
                            if (Traits::eq(c, ctype.widen(to))) {
                                digits.push_front(to - '0');
                                pd = false;
                            }
                        if (basefield != std::ios_base::oct)
                            for (char to = '8'; to <= '9' && pd; ++to)
                                if (Traits::eq(c, ctype.widen(to))) {
                                    digits.push_front(to - '0');
                                    pd = false;
                                }
                        if (basefield == std::ios_base::hex)
                            for (char to = 'a'; to <= 'f' && pd; ++to)
                                if (Traits::eq(c, ctype.widen(to)) || Traits::eq(c, ctype.widen(to + 'A' - 'a'))) {
                                    digits.push_front(to - 'a' + 10);
                                    pd = false;
                                }
                        if (pd) break;
                        else need_x = false;
                    }
                }

                //stage 3 via valid digits
                if (digits.size() == 0)
                    is.setstate(std::ios_base::failbit);
                natural v;
                if (basefield == std::ios_base::hex) {
                    std::size_t o = 0;
                    for (auto iter = digits.cbegin(); iter != digits.cend(); ++iter) {
                        v._memory[v._length - 1] |= (*iter << o);
                        o += 4;
                        if (o >= lower_width) {
                            o = 0;
                            ++v._length;
                        }
                    }
                }
                else if (basefield == std::ios_base::oct) {
                    std::size_t o = 0;
                    for (auto iter = digits.cbegin(); iter != digits.cend(); ++iter) {
                        v._memory[v._length - 1] |= (*iter << o);
                        o += 3;
                        if (o >= lower_width) {
                            v._memory[v._length - 1] &= lower_half;
                            o -= lower_width;
                            ++v._length;
                            v._memory[v._length - 1] |= (*iter & (((std::size_t)1 << o) - 1));
                        }
                    }
                }
                else {
                    for (auto iter = digits.crbegin(); iter != digits.crend(); ++iter) {
                        v *= 10;
                        v += *iter;
                    }
                }
                x = v;
                thousands_seps.push_front(digits.size());
                auto gap_iter = gaps.cbegin();
                for (auto before = thousands_seps.cbegin(), iter = ++thousands_seps.cbegin(); iter != thousands_seps.end(); ++before, ++iter, gap_iter = (++gap_iter == gaps.cend() ? --gap_iter : gap_iter))
                    if (*before - *iter != *gap_iter) {
                        is.setstate(std::ios_base::failbit);
                        break;
                    }

                return is;
            }

            static constexpr natural NaN()noexcept {
                natural result;
                return result.to_NaN();
            }
            static constexpr natural zero()noexcept {
                natural result;
                return result.to_zero();
            }
            static constexpr natural infinity()noexcept {
                natural result;
                return result.to_inf();
            }
            static constexpr natural max()noexcept {
                natural result;
                return result.to_max();
            }
            template <class URBG, class Uniform_Int_Distribution>
            static constexpr natural random(std::size_t length, URBG&& urbg, Uniform_Int_Distribution&& rd) {
                natural result;
                result._length = length / lower_width;
                for (std::size_t i = 0; i < result._length; ++i)
                    result._memory[i] = rd(urbg, std::decay_t<Uniform_Int_Distribution>::param_type(0, lower_half));
                std::size_t remaining = length % lower_width;
                if (remaining)
                    result._memory[result._length++] = rd(urbg, std::decay_t<Uniform_Int_Distribution>::param_type(0, ((value_type)1 << remaining) - 1));
                return result;
            }

        private:
            template <std::size_t N, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
            constexpr natural(const Integer(&vs)[N], std::size_t l = 0, std::size_t r = N):
                _length(0), _memory{} {
                while (l < r)
                    _memory[_length++] = vs[l++];
            }

            HWSHQTB_CONSTEXPR14 void remove_zero(std::size_t length) {
                while (length > 1 && _memory[length - 1] == 0)
                    --length;
                return length;
            }
            HWSHQTB_CONSTEXPR14 void remove_zero() {
                while (_length > 1 && _memory[_length - 1] == 0)
                    --_length;
            }

            HWSHQTB_CONSTEXPR14 void shift_left(std::size_t step) {
                if (_length + step > _memory.size())
                    to_inf();
                else {
                    std::memmove(_memory + step, _memory, _length * sizeof(value_type));
                    std::memset(_memory, 0, step * sizeof(value_type));
                    _length += step;
                }
            }
            HWSHQTB_CONSTEXPR14 void shift_right(std::size_t step) {
                if (step >= _length) to_zero();
                else {
                    std::memmove(_memory, _memory + step, _length * sizeof(value_type));
                    _length -= step;
                }
            }

            HWSHQTB_CONSTEXPR14 void base_multiply(value_type v) {
                value_type carry = 0;
                for (std::size_t i = 0; i < _length; ++i) {
                    carry = (_memory[i] = _memory[i] * v + carry) >> lower_width;
                    _memory[i] &= lower_half;
                }
                if (carry != 0) {
                    if (_length == _memory.size()) to_inf();
                    else _memory[_length++] = carry;
                }
            }
            HWSHQTB_CONSTEXPR14 value_type base_divide_by(value_type v) {
                value_type remainder = 0;
                for (std::size_t i = _length; i > 0; --i) {
                    remainder = (_memory[i - 1] += remainder << lower_width) % v;
                    _memory[i - 1] /= v;
                }
                remove_zero();
                return remainder;
            }

            container_type _memory;
            std::size_t _length;

        };

        constexpr natural<> operator""_n(unsigned long long v) {
            return natural<>(v);
        }

        template <typename T, class Container>
        struct ndiv_t {
            natural<T, Container> quot, rem;
        };

        template <typename T, class Container>
        constexpr natural<T, Container> operator+(const natural<T, Container>& x, const natural<T, Container>& v)noexcept {
            natural<T, Container> result = x;
            return result += v;
        }
        template <typename T, class Container>
        constexpr natural<T, Container> operator-(const natural<T, Container>& x, const natural<T, Container>& v)noexcept {
            natural<T, Container> result = x;
            return result -= v;
        }
        template <typename T, class Container>
        constexpr natural<T, Container> operator*(const natural<T, Container>& x, const natural<T, Container>& v)noexcept {
            natural<T, Container> result = x;
            return result *= v;
        }
        template <typename T, class Container>
        constexpr natural<T, Container> operator/(const natural<T, Container>& x, const natural<T, Container>& v)noexcept {
            natural<T, Container> result = x;
            return result /= v;
        }
        template <typename T, class Container>
        constexpr natural<T, Container> operator%(const natural<T, Container>& x, const natural<T, Container>& v)noexcept {
            natural<T, Container> result = x;
            return result %= v;
        }
        template <typename T, class Container>
        constexpr natural<T, Container> operator&(const natural<T, Container>& x, const natural<T, Container>& v)noexcept {
            natural<T, Container> result = x;
            return result &= v;
        }
        template <typename T, class Container>
        constexpr natural<T, Container> operator|(const natural<T, Container>& x, const natural<T, Container>& v)noexcept {
            natural<T, Container> result = x;
            return result |= v;
        }
        template <typename T, class Container>
        constexpr natural<T, Container> operator^(const natural<T, Container>& x, const natural<T, Container>& v)noexcept {
            natural<T, Container> result = x;
            return result ^= v;
        }
        template <typename T, class Container>
        constexpr natural<T, Container> operator<<(const natural<T, Container>& x, const natural<T, Container>& v)noexcept {
            natural<T, Container> result = x;
            return result <<= v;
        }
        template <typename T, class Container>
        constexpr natural<T, Container> operator>>(const natural<T, Container>& x, const natural<T, Container>& v)noexcept {
            natural<T, Container> result = x;
            return result >>= v;
        }
        template <typename T, class Container>
        constexpr natural<T, Container> operator~(const natural<T, Container>& x)noexcept {
            natural<T, Container> result = x;
            return result.filp();
        }
        template <typename T, class Container>
        constexpr natural<T, Container> operator+(const natural<T, Container>& x)noexcept {
            return x;
        }
        template <typename T, class Container>
        constexpr natural<T, Container> operator-(const natural<T, Container>& x)noexcept {
            if (x == 0) return natural<T, Container>::zero();
            return natural<T, Container>::infinity();
        }

        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator+(const natural<T, Container>& x, Integer v)noexcept {
            natural<T, Container> result = x;
            return result += v;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator+(Integer v, const natural<T, Container>& x)noexcept {
            natural<T, Container> result = x;
            return result += v;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator-(const natural<T, Container>& x, Integer v)noexcept {
            natural<T, Container> result = x;
            return result -= v;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator-(Integer v, const natural<T, Container>& x)noexcept {
            natural<T, Container> result = v;
            return result -= x;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator*(const natural<T, Container>& x, Integer v)noexcept {
            natural<T, Container> result = x;
            return result *= v;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator*(Integer v, const natural<T, Container>& x)noexcept {
            natural<T, Container> result = x;
            return result *= v;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator/(const natural<T, Container>& x, Integer v)noexcept {
            natural<T, Container> result = x;
            return result /= v;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator/(Integer v, const natural<T, Container>& x)noexcept {
            natural<T, Container> result = v;
            return result /= x;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator%(const natural<T, Container>& x, Integer v)noexcept {
            natural<T, Container> result = x;
            return result %= v;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator%(Integer v, const natural<T, Container>& x)noexcept {
            natural<T, Container> result = v;
            return result %= x;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator&(const natural<T, Container>& x, Integer v)noexcept {
            natural<T, Container> result = x;
            return result &= v;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator&(Integer v, const natural<T, Container>& x)noexcept {
            natural<T, Container> result = x;
            return result &= v;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator|(const natural<T, Container>& x, Integer v)noexcept {
            natural<T, Container> result = x;
            return result |= v;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator|(Integer v, const natural<T, Container>& x)noexcept {
            natural<T, Container> result = x;
            return result |= v;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator^(const natural<T, Container>& x, Integer v)noexcept {
            natural<T, Container> result = x;
            return result ^= v;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator^(Integer v, const natural<T, Container>& x)noexcept {
            natural<T, Container> result = x;
            return result ^= v;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator<<(const natural<T, Container>& x, Integer v)noexcept {
            natural<T, Container> result = x;
            return result <<= v;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator<<(Integer v, const natural<T, Container>& x)noexcept {
            natural<T, Container> result = v;
            return result <<= x;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator>>(const natural<T, Container>& x, Integer v)noexcept {
            natural<T, Container> result = x;
            return result >>= v;
        }
        template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
        constexpr natural<T, Container> operator>>(Integer v, const natural<T, Container>& x)noexcept {
            natural<T, Container> result = v;
            return result >>= x;
        }

    }
}

namespace std {
    template <typename T, class Container>
    constexpr hwshqtb::big_number::natural<T, Container> abs(const hwshqtb::big_number::natural<T, Container>& x)noexcept {
        return x;
    }
    template <typename T, class Container>
    constexpr hwshqtb::big_number::ndiv_t<T, Container> div(const hwshqtb::big_number::natural<T, Container>& x, const hwshqtb::big_number::natural<T, Container>& y)noexcept {
        hwshqtb::big_number::ndiv_t<T, Container> result;
        result.rem = x;
        result.quot = result.rem.div(y);
        return result;
    }
    template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
    constexpr hwshqtb::big_number::ndiv_t<T, Container> div(const hwshqtb::big_number::natural<T, Container>& x, Integer y)noexcept {
        hwshqtb::big_number::ndiv_t<T, Container> result;
        result.rem = x;
        result.quot = result.rem.div(y);
        return result;
    }
    template <typename T, class Container, typename Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
    constexpr hwshqtb::big_number::natural<T, Container> pow(const hwshqtb::big_number::natural<T, Container>& x, Integer v) {
        hwshqtb::big_number::natural<T, Container> result(0), base = x;
        while (v) {
            if (v & 1) result *= base;
            v >>= 1;
            base = base * base;
        }
        return result;
    }

}

#endif