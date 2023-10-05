#ifndef _HWSHQTB_ARRAY_HPP
#define _HWSHQTB_ARRAY_HPP

// 2023-9-30 done c++17

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include "../iterator/iterator.hpp"

namespace hwshqtb {
    template <typename T>
    class array_iterator:
        public arraylike_iterator<array_iterator<T>, T*, T, std::ptrdiff_t, T*, T&> {
    public:
        constexpr array_iterator(const arraylike_iterator<array_iterator<T>, T*, T, std::ptrdiff_t, T*, T&>& other)noexcept:
            arraylike_iterator<array_iterator<T>, T*, T, std::ptrdiff_t, T*, T&>::arraylike_iterator(other) {}
    };

    namespace details {
        template <typename T, ::std::size_t N>
        struct array_t {
            constexpr T* data()noexcept {
                return _data;
            }
            constexpr const T* data()const noexcept {
                return _data;
            }

            void swap(array_t& other)noexcept(std::is_swappable_v<T>) {
                for (std::size_t i = 0; i < N; ++i)
                    std::swap(_data[i], other._data[i]);
            }

            T _data[N];
        };

        template <typename T>
        struct array_t<T, 0> {
            constexpr T* data()noexcept {
                return nullptr;
            }
            constexpr const T* data()const noexcept {
                return nullptr;
            }

            void swap(array_t& other)noexcept {}
        };
    }

    template <typename T, std::size_t N>
    class array {
    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using iterator = array_iterator<value_type>;
        using const_iterator = array_iterator<const value_type>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    private:
        using original_iterator = arraylike_iterator<iterator, pointer, value_type, difference_type, pointer, reference>;
        using const_original_iterator = arraylike_iterator<const_iterator, const_pointer, const value_type, difference_type, const_pointer, const_reference>;

    public:
        constexpr reference at(const size_type pos) {
            if (pos >= N) throw std::out_of_range("out of range");
            return _data.data()[pos];
        }
        constexpr const_reference at(const size_type pos)const {
            if (pos >= N) throw std::out_of_range("out of range");
            return _data.data()[pos];
        }
        constexpr reference operator[](const size_type pos) {
            return _data.data()[pos];
        }
        constexpr const_reference operator[](const size_type pos)const {
            return _data.data()[pos];
        }
        constexpr reference front() {
            return _data.data()[0];
        }
        constexpr const_reference front(const size_type pos)const {
            return _data.data()[0];
        }
        constexpr reference back() {
            return _data.data()[N - 1];
        }
        constexpr const_reference back(const size_type pos)const {
            return _data.data()[N - 1];
        }
        constexpr pointer data()noexcept {
            return _data.data();
        }
        constexpr const_pointer data()const noexcept {
            return _data.data();
        }

        constexpr iterator begin()noexcept {
            return iterator(original_iterator(_data.data()));
        }
        constexpr const_iterator begin()const noexcept {
            return const_iterator(const_original_iterator(_data.data()));
        }
        constexpr const_iterator cbegin()const noexcept {
            return const_iterator(const_original_iterator(_data.data()));
        }
        constexpr iterator end()noexcept {
            return iterator(original_iterator(_data.data() + N));
        }
        constexpr const_iterator end()const noexcept {
            return const_iterator(const_original_iterator(_data.data() + N));
        }
        constexpr const_iterator cend()const noexcept {
            return const_iterator(const_original_iterator(_data.data() + N));
        }
        constexpr reverse_iterator rbegin()noexcept {
            return reverse_iterator(iterator(original_iterator(_data.data() + N)));
        }
        constexpr const_reverse_iterator rbegin()const noexcept {
            return const_reverse_iterator(const_iterator(const_original_iterator(_data.data() + N)));
        }
        constexpr const_reverse_iterator crbegin()const noexcept {
            return const_reverse_iterator(const_iterator(const_original_iterator(_data.data() + N)));
        }
        constexpr reverse_iterator rend()noexcept {
            return reverse_iterator(iterator(original_iterator(_data.data())));
        }
        constexpr const_reverse_iterator rend()const noexcept {
            return const_reverse_iterator(const_iterator(const_original_iterator(_data.data())));
        }
        constexpr const_reverse_iterator crend()const noexcept {
            return const_reverse_iterator(const_iterator(const_original_iterator(_data.data())));
        }

        constexpr bool empty()const noexcept {
            return N == 0;
        }
        constexpr size_type size()const noexcept {
            return N;
        }
        constexpr size_type max_size()const noexcept {
            return N;
        }

        void fill(const value_type& value) {
            for (size_type i = 0; i < N; ++i)
                _data.data()[i] = value;
        }
        void swap(array& other)noexcept(noexcept(_data.swap(other._data))) {
            _data.swap(other._data);
        }

        details::array_t<T, N> _data;
    };

    template <typename T, typename ...Ts>
    array(T, Ts...) -> array<std::enable_if_t<(std::is_same_v<T, Ts> && ...), T>, 1 + sizeof...(Ts)>;

    template <typename T, std::size_t N>
    constexpr bool operator==(const array<T, N>& a, const array<T, N>& b) {
        for (typename array<T, N>::size_type i = 0; i < N; ++i)
            if (a[i] != b[i]) return false;
        return true;
    }
    template <typename T, std::size_t N>
    constexpr bool operator<(const array<T, N>& a, const array<T, N>& b) {
        for (typename array<T, N>::size_type i = 0; i < N; ++i)
            if (a[i] >= b[i]) return false;
        return true;
    }
    template <typename T, std::size_t N>
    constexpr bool operator!=(const array<T, N>& a, const array<T, N>& b) {
        for (typename array<T, N>::size_type i = 0; i < N; ++i)
            if (a[i] == b[i]) return false;
        return true;
    }
    template <typename T, std::size_t N>
    constexpr bool operator>(const array<T, N>& a, const array<T, N>& b) {
        for (typename array<T, N>::size_type i = 0; i < N; ++i)
            if (a[i] <= b[i]) return false;
        return true;
    }
    template <typename T, std::size_t N>
    constexpr bool operator<=(const array<T, N>& a, const array<T, N>& b) {
        for (typename array<T, N>::size_type i = 0; i < N; ++i)
            if (a[i] > b[i]) return false;
        return true;
    }
    template <typename T, std::size_t N>
    constexpr bool operator>=(const array<T, N>& a, const array<T, N>& b) {
        for (typename array<T, N>::size_type i = 0; i < N; ++i)
            if (a[i] < b[i]) return false;
        return true;
    }

    namespace details {
        template <typename T, ::std::size_t N, ::std::size_t ...I>
        constexpr array<std::decay_t<T>, N> to_array(T(&a)[N], ::std::index_sequence<I...>) {
            return {{a[I]...}};
        }
    }
    template <typename T, std::size_t N>
    constexpr array<std::decay_t<T>, N> to_array(T(&a)[N]) {
        return details::to_array(a, std::make_index_sequence<N>());
    }
}

namespace std {
    template <size_t I, typename T, size_t N>
    constexpr enable_if_t<(I < N), T&> get(hwshqtb::array<T, N>& a)noexcept {
        return a[I];
    }
    template <size_t I, typename T, size_t N>
    constexpr enable_if_t<(I < N), const T&> get(const hwshqtb::array<T, N>& a)noexcept {
        return a[I];
    }
    template <size_t I, typename T, size_t N>
    constexpr enable_if_t<(I < N), T&&> get(hwshqtb::array<T, N>&& a)noexcept {
        return a[I];
    }
    template <size_t I, typename T, size_t N>
    constexpr enable_if_t<(I < N), const T&&> get(const hwshqtb::array<T, N>&& a)noexcept {
        return a[I];
    }

    template <typename T, size_t N>
    void swap(hwshqtb::array<T, N>& a, hwshqtb::array<T, N>& b) {
        a.swap(b);
    }

    template <typename T, size_t N>
    struct tuple_size<hwshqtb::array<T, N>>:
        integral_constant<size_t, N> {};

    template <size_t I, class T>
    struct tuple_element;
    template <size_t I, typename T, std::size_t N>
    struct tuple_element<I, hwshqtb::array<T, N>> {
        using type = T;
    };
}
#endif