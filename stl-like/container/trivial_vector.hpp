#ifndef HWSHQTB__TRIVIAL_VECTOR_HPP
#define HWSHQTB__TRIVIAL_VECTOR_HPP

#include <type_traits>
#include <memory>
#include <cstring>
#include <limits>
#include "..\iterator\iterator.hpp"
#include <stdexcept>

namespace hwshqtb {
    template <typename T>
    class trivial_vector_iterator: public arraylike_iterator<trivial_vector_iterator<T>,
        T*, T, std::ptrdiff_t, T*, T&> {
    public:
        constexpr trivial_vector_iterator(const arraylike_iterator<trivial_vector_iterator<T>, T*, T, std::ptrdiff_t, T*, T&>& other)noexcept:
            arraylike_iterator<trivial_vector_iterator<T>, T*, T, std::ptrdiff_t, T*, T&>::arraylike_iterator(other) {}
    };

    template <typename T, class Allocator = std::allocator<T> >
    class trivial_vector {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivial type");

    public:
        using value_type = T;
        using allocator_type = Allocator;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using iterator = trivial_vector_iterator<value_type>;
        using const_iterator = trivial_vector_iterator<const value_type>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    private:
        using original_iterator = arraylike_iterator<iterator, pointer, value_type, difference_type, pointer, reference>;
        using const_original_iterator = arraylike_iterator<const_iterator, const_pointer, const value_type, difference_type, const_pointer, const_reference>;
        using alloc_traits = std::allocator_traits<allocator_type>;

        static constexpr double GROWTH_RATE = 2;

    public:
        trivial_vector()noexcept(noexcept(allocator_type())):
            _allocator(), _size(0), _capacity(0), _buffer(nullptr) {}
        explicit trivial_vector(const allocator_type& allocator)noexcept:
            _allocator(allocator), _size(0), _capacity(0), _buffer(nullptr) {}
        explicit trivial_vector(size_type count, const value_type& value, const allocator_type& allocator = allocator_type()):
            _allocator(allocator), _size(count), _capacity(count), _buffer(allocate(_capacity)) {
            for (int i = 0; i < count; ++i) {
                std::memmove(_buffer + i, std::addressof(value), sizeof(value_type));
            }
        }
        explicit trivial_vector(size_type count, const allocator_type& allocator = allocator_type()):
            _allocator(allocator), _size(count), _capacity(count), _buffer(allocate(_capacity)) {}
        template <typename InputIt, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>>>
        explicit trivial_vector(InputIt first, InputIt last, const allocator_type& allocator = allocator_type()):
            _allocator(allocator), _size(std::distance(first, last)), _capacity(_size), _buffer(allocate(_capacity)) {
            pointer buffer = _buffer;
            while (first != last) {
                std::memmove(buffer++, std::addressof(*first++), sizeof(value_type));
            }
        }
        trivial_vector(const trivial_vector& other):
            _allocator(other._allocator), _size(other._size), _capacity(_size), _buffer(allocate(_capacity)) {
            if (_capacity != 0) {
                std::memmove(_buffer, other._buffer, _size * sizeof(value_type));
            }
        }
        trivial_vector(const trivial_vector& other, const allocator_type& allocator):
            _allocator(allocator), _size(other._size), _capacity(_size), _buffer(allocate(_capacity)) {
            if (_capacity != 0) {
                std::memmove(_buffer, other._buffer, _size * sizeof(value_type));
            }
        }
        trivial_vector(trivial_vector&& other)noexcept:
            _allocator(std::exchange(other._allocator, {})), _size(std::exchange(other._size, 0)), _capacity(std::exchange(other._capacity, 0)), _buffer(std::exchange(other._buffer, nullptr)) {}
        trivial_vector(trivial_vector&& other, const allocator_type& allocator):
            _allocator(allocator), _size(std::exchange(other._size, 0)), _capacity(std::exchange(other._capacity, 0)), _buffer(std::exchange(other._buffer, nullptr)) {}
        trivial_vector(std::initializer_list<T> ilist, const allocator_type& allocator = allocator_type()):
            _allocator(allocator), _size(ilist.size()), _capacity(_size), _buffer(allocate(_capacity)) {
            if (_capacity != 0) {
                std::memmove(_buffer, ilist.begin(), _size * sizeof(value_type));
            }
        }

        ~trivial_vector() {
            if (_capacity != 0) {
                alloc_traits::deallocate(_allocator, _buffer, _capacity);
            }
        }

        trivial_vector& operator=(const trivial_vector& other) {
            if constexpr (alloc_traits::propagate_on_container_copy_assignment::value) {
                if (alloc_traits::is_always_equal::value || _allocator == other._allocator) {
                    _allocator = other._allocator;
                    if (_capacity != other._size) {
                        ~trivial_vector();
                        _buffer = alloc_traits::allocate(_allocator, _capacity = other._size);
                    }
                }
                else {
                    ~trivial_vector();
                    _allocator = other._allocator;
                    _buffer = alloc_traits::allocate(_allocator, _capacity = other._size);
                }
            }
            else {
                if (_capacity != other._size) {
                    ~trivial_vector();
                    _buffer = alloc_traits::allocate(_allocator, _capacity = other._size);
                }
            }
            std::memmove(_buffer, other._buffer, (_size = other._size) * sizeof(value_type));
            return *this;
        }
        trivial_vector& operator=(trivial_vector&& other) {
            if constexpr (alloc_traits::propagate_on_container_move_assignment::value) {
                ~trivial_vector();
                _allocator = std::move(other._allocator);
                _capacity = std::exchange(other._capacity, 0);
                _buffer = std::exchange(other._buffer, nullptr);
                _size = std::exchange(other._size, 0);
            }
            else {
                if (alloc_traits::is_always_equal::value || _allocator == other._allocator) {
                    ~trivial_vector();
                    _capacity = std::exchange(other._capacity, 0);
                    _buffer = std::exchange(other._buffer, nullptr);
                    _size = std::exchange(other._size, 0);
                }
                else {
                    ~trivial_vector();
                    _capacity = other._capacity;
                    _buffer = alloc_traits::allocate(_allocator, _capacity);
                    _size = std::exchange(other._size, 0);
                    if (other._capacity != 0) {
                        std::memmove(_buffer, other._buffer, _size * sizeof(value_type));
                        alloc_traits::deallocate(other._allocator, other._buffer, other._capacity);
                        other._capacity = 0;
                    }
                }
            }
            return *this;
        }

        void assign(size_type count, const T& value) {
            if (_capacity < count) {
                ~trivial_vector();
                _buffer = allocate(count);
                _capacity = count;
            }
            _size = count;
            for (int i = 0; i < count; ++i) {
                std::memmove(_buffer + i, std::addressof(value), sizeof(value_type));
            }
        }
        template <typename InputIt, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>>>
        void assign(InputIt first, InputIt last) {
            size_type count = std::distance(first, last);
            if (_capacity < count) {
                ~trivial_vector();
                _buffer = allocate(count);
                _capacity = count;
            }
            _size = count;
            pointer buffer = _buffer;
            while (first != last) {
                std::memmove(buffer++, std::addressof(*first++), sizeof(value_type));
            }
        }
        void assign(std::initializer_list<T> ilist) {
            size_type count = ilist.size();
            if (_capacity < count) {
                ~trivial_vector();
                _buffer = allocate(count);
                _capacity = count;
            }
            _size = count;
            std::memmove(_buffer, ilist.begin(), _size * sizeof(value_type));
        }

        allocator_type get_allocator()const noexcept {
            return _allocator;
        }

        reference at(size_type pos) {
            if (pos > _size) throw std::out_of_range("out_of_range");
            return _buffer[pos];
        }
        const_reference at(size_type pos)const {
            if (pos > _size) throw std::out_of_range("out_of_range");
            return _buffer[pos];
        }

        reference operator[](size_type pos) {
            return _buffer[pos];
        }
        const_reference operator[](size_type pos)const {
            return _buffer[pos];
        }

        reference front() {
            return *_buffer;
        }
        const_reference front()const {
            return *_buffer;
        }

        reference back() {
            return _buffer[_size - 1];
        }
        const_reference back()const {
            return _buffer[_size - 1];
        }

        pointer data()noexcept {
            return _buffer;
        }
        const_pointer data()const noexcept {
            return _buffer;
        }

        iterator begin()noexcept {
            return original_iterator(_buffer);
        }
        const_iterator begin()const noexcept {
            return const_original_iterator(_buffer);
        }
        const_iterator cbegin()const noexcept {
            return const_original_iterator(_buffer);
        }

        iterator end()noexcept {
            return original_iterator(_buffer + _size);
        }
        const_iterator end()const noexcept {
            return const_original_iterator(_buffer + _size);
        }
        const_iterator cend()const noexcept {
            return const_original_iterator(_buffer + _size);
        }

        reverse_iterator rbegin()noexcept {
            return iterator(original_iterator(_buffer + _size));
        }
        const_reverse_iterator rbegin()const noexcept {
            return const_iterator(const_original_iterator(_buffer + _size));
        }
        const_reverse_iterator crbegin()const noexcept {
            return const_iterator(const_original_iterator(_buffer + _size));
        }

        reverse_iterator rend()noexcept {
            return iterator(original_iterator(_buffer));
        }
        const_reverse_iterator rend()const noexcept {
            return const_iterator(const_original_iterator(_buffer));
        }
        const_reverse_iterator rcend()const noexcept {
            return const_iterator(const_original_iterator(_buffer));
        }

        bool empty() const noexcept {
            return _size == 0;
        }

        size_type size()const noexcept {
            return _size;
        }

        size_type max_size() const noexcept {
            return std::numeric_limits<difference_type>::max();
        }

        void reserve(size_type new_capacity) {
            if (new_capacity > _capacity) {
                new_capacity = get_new_capacity(new_capacity);
                pointer new_buffer = alloc_traits::allocate(_allocator, new_capacity);
                if (_capacity != 0) {
                    std::memmove(new_buffer, _buffer, _size * sizeof(value_type));
                    alloc_traits::deallocate(_allocator, _buffer, _capacity);
                }
                _capacity = new_capacity;
                _buffer = new_buffer;
            }
        }

        size_type capacity() const noexcept {
            return _capacity;
        }

        void shrink_to_fit() {
            if (_size == 0) {
                ~trivial_vector();
                _capacity = 0;
                _buffer = nullptr;
            }
            else if (_size < _capacity) {
                pointer new_buffer = alloc_traits::allocate(_allocator, _size);
                std::memmove(new_buffer, _buffer, _size * sizeof(value_type));
                alloc_traits::deallocate(_allocator, _buffer, _capacity);
                _capacity = _size;
                _buffer = new_buffer;
            }
        }

        void clear() noexcept {
            _size = 0;
        }

        iterator insert(const_iterator pos, const T& value) {
            difference_type size = pos - cbegin();
            renew_buffer_by_insert(size, 1);

            ++_size;
            std::memmove(_buffer + size, std::addressof(value), sizeof(value_type));
            return begin() + size;
        }
        iterator insert(const_iterator pos, size_type count, const T& value) {
            if (count <= 0)
                return pos;

            difference_type size = pos - cbegin();
            renew_buffer_by_insert(size, count);

            _size += count;
            while (count--)
                std::memmove(_buffer + size + count, std::addressof(value), sizeof(value_type));
            return begin() + size;
        }
        template <typename InputIt, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>>>
        iterator insert(const_iterator pos, InputIt first, InputIt last) {
            difference_type count = std::distance(first, last);
            if (count <= 0)
                return pos;

            difference_type size = pos - cbegin();
            renew_buffer_by_insert(size, pos);

            _size += count;
            while (count--)
                std::memmove(_buffer + size + count, std::addressof(*--last), sizeof(value_type));
            return begin() + size;
        }
        iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
            size_type count = ilist.size();
            if (count <= 0)
                return pos;

            difference_type size = pos - cbegin();
            renew_buffer_by_insert(size, pos);

            _size += count;
            std::memmove(_buffer + size, ilist.begin(), count * sizeof(value_type));
            return begin() + size;
        }

        template< class... Args >
        iterator emplace(const_iterator pos, Args&&... args) {
            difference_type size = pos - cbegin();
            renew_buffer_by_insert(size, 1);

            ++_size;
            alloc_traits::construct(_allocator, _buffer + size, args...);
            return begin() + size;
        }

        iterator erase(const_iterator pos) {
            difference_type size = pos - cbegin();
            --_size;
            std::memmove(_buffer + size, _buffer + size + 1, (_size - size) * sizeof(value_type));
            return begin() + size;
        }
        iterator erase(const_iterator first, const_iterator last) {
            difference_type count = std::distance(first, last);
            if (count <= 0)
                return begin() + (last - cbegin());
            difference_type size = first - cbegin();
            --_size;
            std::memmove(_buffer + size, _buffer + size + count, (_size + 1 - size - count) * sizeof(value_type));
            return begin() + size;
        }

        void push_back(const T& value) {
            renew_buffer_by_insert(_size, 1);

            ++_size;
            std::memmove(_buffer + _size - 1, std::addressof(value), sizeof(value_type));
        }

        template< class... Args >
        void emplace_back(Args&&... args) {
            renew_buffer_by_insert(_size, 1);

            ++_size;
            alloc_traits::construct(_allocator, _buffer + _size - 1, args...);
        }

        void pop_back() {
            if (_size) --_size;
        }

        void resize(size_type count) {
            if (count > _size) renew_buffer_by_insert(_size, count - _size);
            _size = count;
        }

        void resize(size_type count, const value_type& value) {
            if (count > _size) renew_buffer_by_insert(_size, count - _size);

            while (count > _size)
                std::memmove(_buffer + _size++, std::addressof(value), sizeof(value_type));

            _size = count;
        }

        void swap(trivial_vector& other) {
            if (alloc_traits::propagate_on_container_swap::value) std::swap(_allocator, other._allocator);
            std::swap(_size, other._size);
            std::swap(_capacity, other._capacity);
            std::swap(_buffer, other._buffer);
        }

    private:
        allocator_type _allocator;
        size_type _size;
        size_type _capacity;
        pointer _buffer;

        pointer allocate(size_type new_capacity) {
            return new_capacity == 0 ? nullptr : alloc_traits::allocate(_allocator, new_capacity);
        }
        size_type get_new_capacity(size_type new_capacity)const noexcept {
            if (_capacity == 0) {
                return new_capacity;
            }
            size_type _ = _capacity;
            while (_ < new_capacity) {
                if (_ >= (size_type)(std::numeric_limits<difference_type>::max() / GROWTH_RATE)) {
                    _ = std::numeric_limits<difference_type>::max();
                }
                _ *= GROWTH_RATE;
            }
            return _;
        }
        void renew_buffer_by_insert(difference_type pos, difference_type count) {
            if (_size + count > _capacity) {
                size_type new_capacity = get_new_capacity(_size + count);
                pointer new_buffer = alloc_traits::allocate(_allocator, new_capacity);
                if (_capacity != 0) {
                    std::memmove(new_buffer, _buffer, pos * sizeof(value_type));
                    std::memmove(new_buffer + pos + count, _buffer + pos, (_size - pos) * sizeof(value_type));
                    alloc_traits::deallocate(_allocator, _buffer, _capacity);
                }
                _buffer = new_buffer;
                _capacity = new_capacity;
            }
            else std::memmove(_buffer + pos + count, _buffer + pos, (_size - pos) * sizeof(value_type));
        }
    };

    template< class T, class Alloc >
    bool operator==(const trivial_vector<T, Alloc>& a, const trivial_vector<T, Alloc>& b) {
        if (a.size() != b.size()) return false;

        for (typename trivial_vector<T, Alloc>::size_type i = 0; i < a.size(); ++i)
            if (a[i] != b[i]) return false;

        return true;
    }
    template< class T, class Alloc >
    bool operator!=(const trivial_vector<T, Alloc>& a, const trivial_vector<T, Alloc>& b) {
        if (a.size() != b.size()) return true;

        for (typename trivial_vector<T, Alloc>::size_type i = 0; i < a.size(); ++i)
            if (a[i] != b[i]) return true;

        return false;
    }
    template< class T, class Alloc >
    bool operator<(const trivial_vector<T, Alloc>&a, const trivial_vector<T, Alloc>&b) {
        for (typename trivial_vector<T, Alloc>::size_type i = 0; i < a.size(); ++i)
            if (a[i] != b[i]) return a[i] < b[i];

        return a.size() < b.size();
    }
    template< class T, class Alloc >
    bool operator<=(const trivial_vector<T, Alloc>& a, const trivial_vector<T, Alloc>& b) {
        for (typename trivial_vector<T, Alloc>::size_type i = 0; i < a.size(); ++i)
            if (a[i] != b[i]) return a[i] < b[i];

        return a.size() <= b.size();
    }
    template< class T, class Alloc >
    bool operator>(const trivial_vector<T, Alloc>& a, const trivial_vector<T, Alloc>& b) {
        for (typename trivial_vector<T, Alloc>::size_type i = 0; i < a.size(); ++i)
            if (a[i] != b[i]) return a[i] > b[i];

        return a.size() > b.size();
    }
    template< class T, class Alloc >
    bool operator>=(const trivial_vector<T, Alloc>& a, const trivial_vector<T, Alloc>& b) {
        for (typename trivial_vector<T, Alloc>::size_type i = 0; i < a.size(); ++i)
            if (a[i] != b[i]) return a[i] > b[i];

        return a.size() >= b.size();
    }
}

namespace std {
    template< class T, class Alloc >
    void swap(hwshqtb::trivial_vector<T, Alloc>& a, hwshqtb::trivial_vector<T, Alloc>& b) {
        a.swap(b);
    }
}

#endif