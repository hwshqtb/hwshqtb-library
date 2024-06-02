#ifndef HWSHQTB_NUMERIC_RANGE_HPP
#define HWSHQTB_NUMERIC_RANGE_HPP

/*
    2023-10-1
        done c++17

    2024-5-31 
        the iterator's type should be bidirectional iterator
        header-only
*/

#include <limits>
#include <type_traits>
#include <stdexcept>
#include <cmath>

namespace hwshqtb {
    namespace details {
        template <typename Number>
        constexpr std::enable_if_t<std::numeric_limits<Number>::is_specialized, bool> less(const Number& a, const Number& b) {
            return a - b < -std::numeric_limits<Number>::epsilon() * std::abs(a + b) / 2;
        }
        template <typename Number>
        constexpr std::enable_if_t<std::numeric_limits<Number>::is_specialized, bool> equal(const Number& a, const Number& b) {
            return std::abs(a - b) <= std::numeric_limits<Number>::epsilon() * std::abs(a + b) / 2;
        }
        template <typename Number>
        constexpr std::enable_if_t<std::numeric_limits<Number>::is_specialized, bool> greater(const Number& a, const Number& b) {
            return a - b > std::numeric_limits<Number>::epsilon() * std::abs(a + b) / 2;
        }
    }

    template <typename Number>
    class numeric_range;

    template <typename Number>
    class numeric_range_iterator {
        friend numeric_range<Number>;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = const Number;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

    private:
        constexpr numeric_range_iterator(Number point, const Number step)noexcept:
            _point(point), _step(step) {}

    public:
        constexpr numeric_range_iterator(const numeric_range_iterator&) = default;
        constexpr ~numeric_range_iterator() = default;

        constexpr reference operator*()const noexcept {
            return _point;
        }
        constexpr pointer operator->()const noexcept {
            return std::addressof(_point);
        }

        constexpr numeric_range_iterator& operator++()noexcept {
            _point += _step;
            return *this;
        }
        constexpr numeric_range_iterator operator++(int)noexcept {
            numeric_range_iterator ret = *this;
            _point += _step;
            return ret;
        }
        constexpr numeric_range_iterator& operator--()noexcept {
            _point -= _step;
            return *this;
        }
        constexpr numeric_range_iterator operator--(int)noexcept {
            numeric_range_iterator ret = *this;
            _point -= _step;
            return ret;
        }

        constexpr bool operator==(const numeric_range_iterator& other)const {
            if (!details::equal(_step, other._step)) throw std::invalid_argument("invalid argument");
            return details::greater(other._point + _step, _point) && !details::less(_point, other._point);
        }
        constexpr bool operator!=(const numeric_range_iterator& other)const {
            if (!details::equal(_step, other._step)) throw std::invalid_argument("invalid argument");
            return !details::greater(other._point + _step, _point) || details::less(_point, other._point);
        }

        constexpr numeric_range_iterator& operator+=(difference_type n)noexcept {
            _point += _step * n;
            return *this;
        }
        constexpr numeric_range_iterator& operator-=(difference_type n)noexcept {
            _point -= _step * n;
            return *this;
        }
        constexpr numeric_range_iterator operator+(difference_type n)const noexcept {
            return {_point + n * _step, _step};
        }
        constexpr numeric_range_iterator operator-(difference_type n)const noexcept {
            return {_point - n * _step, _step};
        }

    private:
        Number _point;
        const Number _step;

    };

    template <typename T>
    class numeric_range {
        using Number = std::decay_t<T>;
        static_assert(std::numeric_limits<Number>::is_specialized, "Number must be NumericType");

    public:
        using value_type = const Number;
        using reference = value_type&;
        using const_reference = reference;
        using pointer = value_type*;
        using const_pointer = pointer;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using iterator = numeric_range_iterator<Number>;
        using const_iterator = iterator;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = reverse_iterator;

    public:
        constexpr numeric_range(const Number& begin, const Number& end, const Number& step):
            _begin(begin), _end(end), _step(step) {
            if (!greater(_step, (Number)0)) throw std::invalid_argument("invalid argument");
            if (greater(_begin, _end)) throw std::invalid_argument("invalid argument");
        }
        constexpr numeric_range(const Number& end, const Number& step) :
            _begin(), _end(end), _step(step) {
            if (!details::greater(_step, (Number)0)) throw std::invalid_argument("invalid argument");
            if (details::greater((Number)0, _end)) throw std::invalid_argument("invalid argument");
        }
        constexpr numeric_range(const Number& end) :
            _begin(), _end(end), _step(1) {
            if (greater((Number)0, _end)) throw std::invalid_argument("invalid argument");
        }
        constexpr numeric_range(const numeric_range&) = default;
        constexpr ~numeric_range() = default;

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