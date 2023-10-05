#ifndef _HWSHQTB_NUMERIC_RANGE_HPP
#define _HWSHQTB_NUMERIC_RANGE_HPP

// 2023-10-1 done c++17

#include <limits>
#include <type_traits>
#include "iterator.hpp"
#include <stdexcept>
#include <cmath>

namespace hwshqtb {
    template <typename NUMBER>
    constexpr std::enable_if_t<std::numeric_limits<NUMBER>::is_specialized, bool> less(const NUMBER& a, const NUMBER& b) {
        return a - b < -::std::numeric_limits<NUMBER>::epsilon() * std::abs(a + b) / 2;
    }
    template <typename NUMBER>
    constexpr std::enable_if_t<std::numeric_limits<NUMBER>::is_specialized, bool> equal(const NUMBER& a, const NUMBER& b) {
        return std::abs(a - b) <= ::std::numeric_limits<NUMBER>::epsilon() * std::abs(a + b) / 2;
    }
    template <typename NUMBER>
    constexpr std::enable_if_t<std::numeric_limits<NUMBER>::is_specialized, bool> greater(const NUMBER& a, const NUMBER& b) {
        return a - b > ::std::numeric_limits<NUMBER>::epsilon() * std::abs(a + b) / 2;
    }

    namespace details {
        template <typename NUMBER>
        struct numeric_point_t {
            constexpr const NUMBER& operator*()const  {
                return _point;
            }
            constexpr const NUMBER* operator->()const {
                return &_point;
            }

            constexpr numeric_point_t& operator++() {
                _point += _step;
                return *this;
            }
            constexpr numeric_point_t operator++(int) {
                numeric_point_t ret = *this;
                _point += _step;
                return ret;
            }
            constexpr numeric_point_t& operator--() {
                _point -= _step;
                return *this;
            }
            constexpr numeric_point_t operator--(int) {
                numeric_point_t ret = *this;
                _point -= _step;
                return ret;
            }
            constexpr numeric_point_t& operator+=(const NUMBER n) {
                _point += _step * n;
                return *this;
            }
            constexpr numeric_point_t& operator-=(const NUMBER n) {
                _point -= _step * n;
                return *this;
            }
            constexpr ::std::ptrdiff_t operator-(const numeric_point_t& other)const {
                if (compare(_step, other._step) || compare(other._step, _step)) throw ::std::invalid_argument("invalid argument");
                return (_point - other._point) / _step;
            }
            constexpr const NUMBER& operator[](const ::std::ptrdiff_t n)const noexcept {
                return _point + n * _step;
            }

            constexpr numeric_point_t operator+(const ::std::ptrdiff_t n)const {
                return {_point + n * _step, _step};
            }
            constexpr numeric_point_t operator-(const ::std::ptrdiff_t n)const {
                return {_point - n * _step, _step};
            }

            constexpr bool operator==(const numeric_point_t& other)const {
                return !less(_point, other._point + _step) && greater(_point + _step, other._point);
            }
            constexpr bool operator<(const numeric_point_t& other)const {
                if (!equal(_step, other._step)) throw ::std::invalid_argument("invalid argument");
                return !greater(_point + _step, other._point);
            }
            constexpr bool operator!=(const numeric_point_t& other)const{
                if (!equal(_step, other._step)) throw ::std::invalid_argument("invalid argument");
                return !greater(_point + _step, other._point) || !less(_point, other._point + _step);
            }
            constexpr bool operator>(const numeric_point_t& other)const {
                if (!equal(_step, other._step)) throw ::std::invalid_argument("invalid argument");
                return !less(_point, other._point + _step);
            }
            constexpr bool operator<=(const numeric_point_t& other)const {
                if (!equal(_step, other._step)) throw ::std::invalid_argument("invalid argument");
                return less(_point, other._point + _step);
            }
            constexpr bool operator>=(const numeric_point_t& other)const {
                if (!equal(_step, other._step)) throw ::std::invalid_argument("invalid argument");
                return greater(_point + _step, other._point);
            }

            NUMBER _point, _step;
        };
    }

    template <typename NUMBER>
    class numeric_range_iterator:
        public arraylike_iterator<numeric_range_iterator<NUMBER>,
        details::numeric_point_t<NUMBER>, const NUMBER, std::ptrdiff_t, const NUMBER*, const NUMBER&> {

    public:
        constexpr numeric_range_iterator(const arraylike_iterator<numeric_range_iterator<NUMBER>,
            details::numeric_point_t<NUMBER>, const NUMBER, std::ptrdiff_t, const NUMBER*, const NUMBER&>& other)noexcept:
            arraylike_iterator<numeric_range_iterator<NUMBER>,
            details::numeric_point_t<NUMBER>, const NUMBER, std::ptrdiff_t, const NUMBER*, const NUMBER&>::arraylike_iterator(other) {}
    };

    template <typename T>
    class numeric_range {
        using NUMBER = std::decay_t<T>;
        static_assert(std::numeric_limits<NUMBER>::is_specialized, "NUMBER must be NumericType");

    public:
        using value_type = const NUMBER;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = reference;
        using pointer = value_type*;
        using const_pointer = pointer;
        using iterator = numeric_range_iterator<NUMBER>;
        using const_iterator = iterator;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = reverse_iterator;

    private:
        using original_iterator = arraylike_iterator<iterator, details::numeric_point_t<std::decay_t<NUMBER>>, value_type, difference_type, pointer, reference>;
        using const_original_iterator = original_iterator;

    public:
        constexpr numeric_range(const NUMBER& begin, const NUMBER& end, const NUMBER& step):
            _begin(begin), _end(NUMBER(std::ptrdiff_t((end - begin) / step)) * step + begin), _step(step) {
            if (!greater(_step, 0)) throw std::invalid_argument("invalid argument");
            if (greater(_begin, _end)) throw std::invalid_argument("invalid argument");
        }
        constexpr numeric_range(const NUMBER& end, const NUMBER& step) :
            _begin(), _end(NUMBER(std::ptrdiff_t((end) / step))* step), _step(step) {
            if (!greater(_step, 0)) throw std::invalid_argument("invalid argument");
            if (greater(0, _end)) throw std::invalid_argument("invalid argument");
        }
        constexpr numeric_range(const NUMBER& end) :
            _begin(), _end(NUMBER(std::ptrdiff_t((end)))), _step(1) {
            if (greater(0, _end)) throw std::invalid_argument("invalid argument");
        }

        constexpr reference at(const size_type pos)const {
            if (!compare(pos * _step, _end - _begin)) throw std::out_of_range("out of range");
            return _begin + pos * _step;
        }
        constexpr reference operator[](const size_type pos)const noexcept {
            return _begin + pos * _step;
        }
        constexpr reference front()const noexcept {
            return _begin;
        }
        constexpr reference back()const noexcept {
            return _end - _step;
        }

        constexpr iterator begin()const noexcept {
            return iterator(original_iterator(details::numeric_point_t<std::decay_t<NUMBER>>{_begin, _step}));
        }
        constexpr const_iterator cbegin()const noexcept {
            return iterator(original_iterator(details::numeric_point_t<std::decay_t<NUMBER>>{_begin, _step}));
        }
        constexpr iterator end()const noexcept {
            return iterator(original_iterator(details::numeric_point_t<std::decay_t<NUMBER>>{_end, _step}));
        }
        constexpr const_iterator cend()const noexcept {
            return iterator(original_iterator(details::numeric_point_t<std::decay_t<NUMBER>>{_end, _step}));
        }
        constexpr reverse_iterator rbegin()const noexcept {
            return reverse_iterator(iterator());
        }
        constexpr const_reverse_iterator crbegin()const noexcept {
            return const_reverse_iterator(cend());
        }
        constexpr reverse_iterator rend()const noexcept {
            return reverse_iterator(begin());
        }
        constexpr const_reverse_iterator crend()const noexcept {
            return const_reverse_iterator(cbegin());
        }

        constexpr bool operator==(const numeric_range& other)const {
            return !compare(_begin, other._begin) && !compare(other._begin, _begin) &&
                !compare(_end, other._end) && !compare(other._end, _end) &&
                !compare(_step, other._step) && !compare(other._step, _step);
        }
        constexpr bool operator!=(const numeric_range& other)const {
            return compare(_begin, other._begin) || compare(other._begin, _begin) ||
                compare(_end, other._end) || compare(other._end, _end) ||
                compare(_step, other._step) || !compare(other._step, _step);
        }

        constexpr bool empty()const noexcept {
            return compare(_end - _begin, _step);
        }
        constexpr size_type size()const {
            return (_end - _begin) / _step;
        }
        constexpr size_type max_size()const {
            return std::numeric_limits<size_type>::max();
        }

        constexpr void swap(numeric_range& other) {
            std::swap(_begin, other._begin);
            std::swap(_end, other._end);
            std::swap(_step, other._step);
        }

    private:
        NUMBER _begin, _end, _step;
    };

    template <typename T1, typename T2, typename T3>
    numeric_range(T1&&, T2&&, T3&&) -> numeric_range<std::common_type_t<std::decay_t<T1>, std::decay_t<T2>, std::decay_t<T3>>>;
    template <typename T1, typename T2>
    numeric_range(T1&&, T2&&) -> numeric_range<std::common_type_t<std::decay_t<T1>, std::decay_t<T2>>>;
    template <typename T>
    numeric_range(T&&) -> numeric_range<std::decay_t<T>>;
}

namespace std {
    template <typename T>
    constexpr void swap(hwshqtb::numeric_range<T>& a, hwshqtb::numeric_range<T>& b) {
        a.swap(b);
    }
}

#endif