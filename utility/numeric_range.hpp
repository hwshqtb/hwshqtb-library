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
* 
*   2024-12-29
*       c++11 for sure
*       remove floating-type support
*       simplify implementation
*       some bug fix
* 
*   2024-12-30
*       add floating-type support
*       the iterator's type become random access iterator
*/

#include "../version.hpp"

namespace hwshqtb {
    namespace utility {
        template <typename T>
        class numeric_range {
            using Number = typename std::decay<T>::type;
            static_assert(std::numeric_limits<Number>::is_specialized, "numeric_range<T> => T must be NumericType");

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
            constexpr numeric_range():
                _begin(0), _step(1), _size(0) {}
            constexpr numeric_range(const Number& begin, const Number& end, const Number& step):
                _begin(begin),
                _step((end >= begin && step > 0 || end <= begin && step < 0) ? step : throw std::invalid_argument("hwshqtb::utility::numeric_range<T>::numeric_range(const Number&, const Number&, const Number&) => invalid argument")),
                _size(static_cast<size_type>(step < 0 ? ((begin - end) / -step + ((begin - end) % -step ? 1 : 0)) : ((end - begin) / step + ((end - begin) % step ? 1 : 0)))) {}
            constexpr numeric_range(const Number& end, const Number& step):
                _begin(0),
                _step((end >= 0 && step > 0 || end <= 0 && step < 0) ? step : throw std::invalid_argument("hwshqtb::utility::numeric_range<T>::numeric_range(const Number&, const Number&, const Number&) => invalid argument")),
                _size(static_cast<size_type>(step < 0 ? -end : end)) {}
            constexpr explicit numeric_range(const Number& end):
                _begin(0),
                _step(end >= 0 ? 1 : throw std::invalid_argument("hwshqtb::utility::numeric_range<T>::numeric_range(const Number&, const Number&, const Number&) => invalid argument")),
                _size(static_cast<size_type>(end)) {}
            constexpr numeric_range(const numeric_range&) = default;

            constexpr bool operator==(const numeric_range& other)const {
                return _begin == other._begin && _step == other._step && _size == other._size;
            }
            constexpr bool operator!=(const numeric_range& other)const {
                return _begin != other._begin || _step != other._step || _size != other._size;
            }

            constexpr iterator begin()const noexcept {
                return iterator(_begin, _step);
            }
            constexpr const_iterator cbegin()const noexcept {
                return begin();
            }
            constexpr iterator end()const noexcept {
                return iterator(_begin, _step, static_cast<difference_type>(_size));
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
                return _size == 0;
            }
            constexpr size_type size()const {
                return _size;
            }

        private:
            Number _begin, _step;
            size_type _size;

        };

        template <typename T>
        class numeric_range<T>::iterator {
            friend numeric_range<T>;

        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = const Number;
            using difference_type = std::ptrdiff_t;
            using pointer = void;
            using reference = value_type&;

        private:
            constexpr iterator(Number point, Number step, difference_type index = 0)noexcept:
                _point(point), _step(step), _index(index) {}

        public:
            constexpr iterator() = default;
            constexpr iterator(const iterator&) = default;

            constexpr reference operator*()const noexcept {
                return _point + _step * _index;
            }

            HWSHQTB_CONSTEXPR14 iterator& operator++()noexcept {
                ++_index;
                return *this;
            }
            HWSHQTB_CONSTEXPR14 iterator operator++(int)noexcept {
                iterator ret = *this;
                ++_index;
                return ret;
            }
            HWSHQTB_CONSTEXPR14 iterator& operator--()noexcept {
                --_index;
                return *this;
            }
            HWSHQTB_CONSTEXPR14 iterator operator--(int)noexcept {
                iterator ret = *this;
                --_index;
                return ret;
            }

            constexpr bool operator==(const iterator& other)const {
                return _index == other._index;
            }
            constexpr bool operator!=(const iterator& other)const {
                return _index != other._index;
            }
            constexpr bool operator<(const iterator& other)const {
                return _index < other._index;
            }
            constexpr bool operator>(const iterator& other)const {
                return _index > other._index;
            }
            constexpr bool operator<=(const iterator& other)const {
                return _index <= other._index;
            }
            constexpr bool operator>=(const iterator& other)const {
                return _index >= other._index;
            }

            HWSHQTB_CONSTEXPR14 iterator& operator+=(difference_type n)noexcept {
                _index += n;
                return *this;
            }
            HWSHQTB_CONSTEXPR14 iterator& operator-=(difference_type n)noexcept {
                _index -= n;
                return *this;
            }
            friend constexpr iterator operator+(difference_type n, const iterator& iter)noexcept {
                return iterator(iter._point, iter._step, iter._index + n);
            }
            constexpr iterator operator+(difference_type n)const noexcept {
                return iterator(_point, _step, _index + n);
            }
            constexpr iterator operator-(difference_type n)const noexcept {
                return iterator(_point, _step, _index - n);
            }
            constexpr difference_type operator-(const iterator& other)const noexcept {
                return _index - other._index;
            }
            constexpr reference operator[](difference_type n)const noexcept {
                return *(*this + n);
            }

        private:
            Number _point, _step;
            difference_type _index;

        };

        template <typename T1, typename T2, typename T3>
        constexpr numeric_range<typename std::common_type<typename std::decay<T1>::type, typename std::decay<T2>::type, typename std::decay<T3>::type>::type> make_numeric_range(T1&& t1, T2&& t2, T3&& t3)noexcept {
            return numeric_range<typename std::common_type<typename std::decay<T1>::type, typename std::decay<T2>::type, typename std::decay<T3>::type>::type>(std::forward<T1>(t1), std::forward<T2>(t2), std::forward<T3>(t3));
        }

        template <typename T1, typename T2>
        constexpr numeric_range<typename std::common_type<typename std::decay<T1>::type, typename std::decay<T2>::type>::type> make_numeric_range(T1&& t1, T2&& t2)noexcept {
            return numeric_range<typename std::common_type<typename std::decay<T1>::type, typename std::decay<T2>::type>::type>(std::forward<T1>(t1), std::forward<T2>(t2));
        }

        template <typename T>
        constexpr numeric_range<typename std::common_type<typename std::decay<T>::type>::type> make_numeric_range(T&& t)noexcept {
            return numeric_range<typename std::common_type<typename std::decay<T>::type>::type>(std::forward<T>(t));
        }

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

#endif