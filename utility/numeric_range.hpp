#ifndef HWSHQTB__UTILITY__NUMERIC_RANGE_HPP
#define HWSHQTB__UTILITY__NUMERIC_RANGE_HPP

/*
*   2023-10-1
*       done c++17
* 
*   2024-5-31
*       the iterator's type should be bidirectional iterator
*       header-only
* 
*   2024-6-3
*       c++17 for sure
* 
*   2024-10-20
*       c++11 and above
*       change position (change module belonging to)
*       header-only except version control header `version.hpp`
*/

#include "../version.hpp"
#include <limits>
#include <type_traits>
#include <stdexcept>
#include <cmath>

namespace hwshqtb {
    namespace utility {
        namespace detail {
            template <typename Number>
            constexpr typename std::enable_if<std::numeric_limits<Number>::is_specialized, bool>::type less(const Number& a, const Number& b) {
                return a - b < -std::numeric_limits<Number>::epsilon() * std::abs(a + b) / 2;
            }
            template <typename Number>
            constexpr typename std::enable_if<std::numeric_limits<Number>::is_specialized, bool>::type equal(const Number& a, const Number& b) {
                return std::abs(a - b) <= std::numeric_limits<Number>::epsilon() * std::abs(a + b) / 2;
            }
            template <typename Number>
            constexpr typename std::enable_if<std::numeric_limits<Number>::is_specialized, bool>::type greater(const Number& a, const Number& b) {
                return a - b > std::numeric_limits<Number>::epsilon() * std::abs(a + b) / 2;
            }
        }

        template <typename T>
        class numeric_range {
            using Number = typename std::decay<T>::type;
            static_assert(std::numeric_limits<Number>::is_specialized, "Number must be NumericType");

        public:
            using value_type = const Number;
            using reference = value_type&;
            using const_reference = reference;
            using pointer = value_type*;
            using const_pointer = pointer;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            class iterator;
            using const_iterator = iterator;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = reverse_iterator;

        public:
            constexpr numeric_range() = default;
            HWSHQTB_CONSTEXPR14 numeric_range(const Number& begin, const Number& end, const Number& step):
                _begin(begin), _end(end), _step(step) {
                if (!detail::greater(_step, (Number)0)) throw std::invalid_argument("hwshqtb::utility::numeric_range<T>::numeric_range(begin, end, step) => invalid argument");
                if (detail::greater(_begin, _end)) throw std::invalid_argument("invalid argument");
            }
            HWSHQTB_CONSTEXPR14 numeric_range(const Number& end, const Number& step) :
                _begin(), _end(end), _step(step) {
                if (!detail::greater(_step, (Number)0)) throw std::invalid_argument("invalid argument");
                if (detail::greater((Number)0, _end)) throw std::invalid_argument("invalid argument");
            }
            HWSHQTB_CONSTEXPR14 numeric_range(const Number& end) :
                _begin(), _end(end), _step(1) {
                if (greater((Number)0, _end)) throw std::invalid_argument("invalid argument");
            }
            constexpr numeric_range(const numeric_range&) = default;

            HWSHQTB_CONSTEXPR20 ~numeric_range() = default;

            HWSHQTB_CONSTEXPR14 numeric_range& operator=(const numeric_range&) = default;

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

            constexpr iterator begin()const noexcept {
                return iterator(_begin, _step);
            }
            constexpr const_iterator cbegin()const noexcept {
                return begin();
            }
            constexpr iterator end()const noexcept {
                return iterator(_end, _step);
            }
            constexpr const_iterator cend()const noexcept {
                return end();
            }
            constexpr reverse_iterator rbegin()const noexcept {
                return reverse_iterator(end());
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

            constexpr bool empty()const noexcept {
                return compare(_end - _begin, _step);
            }
            constexpr size_type size()const {
                return (_end - _begin) / _step;
            }

        private:
            Number _begin, _end, _step;

        };

        template <typename T>
        class numeric_range<T>::iterator {
            friend numeric_range<T>;

        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = const Number;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;

        private:
            constexpr iterator(Number point, const Number step)noexcept:
                _point(point), _step(step) {}

        public:
            constexpr iterator() = default;
            constexpr iterator(const iterator&) = default;
            HWSHQTB_CONSTEXPR20 ~iterator() = default;

            HWSHQTB_CONSTEXPR14 iterator& operator=(const iterator&) = default;

            constexpr reference operator*()const noexcept {
                return _point;
            }
            constexpr pointer operator->()const noexcept {
                return std::addressof(_point);
            }

            HWSHQTB_CONSTEXPR14 iterator& operator++()noexcept {
                _point += _step;
                return *this;
            }
            HWSHQTB_CONSTEXPR14 iterator operator++(int)noexcept {
                iterator ret = *this;
                _point += _step;
                return ret;
            }
            HWSHQTB_CONSTEXPR14 iterator& operator--()noexcept {
                _point -= _step;
                return *this;
            }
            HWSHQTB_CONSTEXPR14 iterator operator--(int)noexcept {
                iterator ret = *this;
                _point -= _step;
                return ret;
            }

            HWSHQTB_CONSTEXPR14 bool operator==(const iterator& other)const {
                if (!detail::equal(_step, other._step)) throw std::invalid_argument("invalid argument");
                return detail::greater(other._point + _step, _point) && !detail::less(_point, other._point);
            }
            HWSHQTB_CONSTEXPR14 bool operator!=(const iterator& other)const {
                if (!detail::equal(_step, other._step)) throw std::invalid_argument("invalid argument");
                return !detail::greater(other._point + _step, _point) || detail::less(_point, other._point);
            }

            HWSHQTB_CONSTEXPR14 iterator& operator+=(difference_type n)noexcept {
                _point += _step * n;
                return *this;
            }
            HWSHQTB_CONSTEXPR14 iterator& operator-=(difference_type n)noexcept {
                _point -= _step * n;
                return *this;
            }
            constexpr iterator operator+(difference_type n)const noexcept {
                return {_point + n * _step, _step};
            }
            constexpr iterator operator-(difference_type n)const noexcept {
                return {_point - n * _step, _step};
            }

        private:
            Number _point, _step;

        };

#if __cplusplus >= 201703L
        template <typename T1, typename T2, typename T3>
        numeric_range(T1&&, T2&&, T3&&) -> numeric_range<std::common_type_t<std::decay_t<T1>, std::decay_t<T2>, std::decay_t<T3>>>;
        template <typename T1, typename T2>
        numeric_range(T1&&, T2&&) -> numeric_range<std::common_type_t<std::decay_t<T1>, std::decay_t<T2>>>;
        template <typename T>
        numeric_range(T&&) -> numeric_range<std::decay_t<T>>;
#endif
    }
}

namespace std {
    template <typename T>
    constexpr void swap(hwshqtb::utility::numeric_range<T>& a, hwshqtb::utility::numeric_range<T>& b) {
        a.swap(b);
    }

}

#endif