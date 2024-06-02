#ifndef _HWSHQTB_ITERATOR_HPP
#define _HWSHQTB_ITERATOR_HPP

#include <cstddef>
#include <iterator>

namespace hwshqtb {
    template <class Derived,
        typename Type, typename Value, typename Difference, typename Pointer, typename Reference>
    class arraylike_iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = Value;
        using difference_type = Difference;
        using pointer = Pointer;
        using reference = Reference;

        constexpr arraylike_iterator()noexcept:
            _pointer(nullptr) {}
        constexpr explicit arraylike_iterator(Type pointer)noexcept:
            _pointer(pointer) {}

        constexpr reference operator*() {
            return *_pointer;
        }
        constexpr const reference operator*()const {
            return *_pointer;
        }
        constexpr pointer operator->() {
            return _pointer;
        }
        constexpr const pointer operator->()const {
            return _pointer;
        }

        constexpr Derived& operator++()noexcept {
            ++_pointer;
            return *static_cast<Derived*>(this);
        }
        constexpr Derived operator++(int)noexcept {
            return arraylike_iterator{_pointer++};
        }
        constexpr Derived& operator--()noexcept {
            --_pointer;
            return *static_cast<Derived*>(this);
        }
        constexpr Derived operator--(int)noexcept {
            return arraylike_iterator{_pointer--};
        }
        constexpr Derived& operator+=(const difference_type n)noexcept {
            _pointer += n;
            return *static_cast<Derived*>(this);
        }
        constexpr Derived& operator-=(const difference_type n)noexcept {
            _pointer -= n;
            return *static_cast<Derived*>(this);
        }
        constexpr difference_type operator-(const Derived& other)const noexcept {
            return _pointer - other._pointer;
        }
        constexpr reference operator[](const difference_type n)noexcept {
            return _pointer[n];
        }
        constexpr const reference operator[](const difference_type n)const noexcept {
            return _pointer[n];
        }

        constexpr Derived operator+(const difference_type n)const noexcept {
            return arraylike_iterator{_pointer + n};
        }
        constexpr Derived operator-(const difference_type n)const noexcept {
            return arraylike_iterator{_pointer + n};
        }

        constexpr bool operator==(const Derived& other)const noexcept {
            return _pointer == other._pointer;
        }
        constexpr bool operator<(const Derived& other)const noexcept {
            return _pointer < other._pointer;
        }
        constexpr bool operator!=(const Derived& other)const noexcept {
            return _pointer != other._pointer;
        }
        constexpr bool operator>(const Derived& other)const noexcept {
            return _pointer > other._pointer;
        }
        constexpr bool operator<=(const Derived& other)const noexcept {
            return _pointer <= other._pointer;
        }
        constexpr bool operator>=(const Derived& other)const noexcept {
            return _pointer >= other._pointer;
        }

    private:
        Type _pointer;

    };
}

#endif