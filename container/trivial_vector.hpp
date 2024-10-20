#ifndef HWSHQTB__CONTAINER__TRIVIAL_VECTOR_HPP
#define HWSHQTB__CONTAINER__TRIVIAL_VECTOR_HPP

/*
*   2024-10-20
*       c++11 and above
*       change position (change module belonging to)
*       header-only except version control header `version.hpp`
*/

#include "../version.hpp"
#include <type_traits>
#include <memory>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <cstddef>
#include <iterator>

namespace hwshqtb {
    namespace container {
        template <typename T, class Allocator = std::allocator<T>>
        class trivial_vector {
            static_assert(std::is_trivial<T>::value, "T must be trivial type");

        public:
            using value_type = T;
            using allocator_type = Allocator;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            using reference = value_type&;
            using const_reference = const value_type&;
            using pointer = value_type*;
            using const_pointer = const value_type*;
            class iterator;
            class const_iterator;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        private:
            using allocator_traits = std::allocator_traits<allocator_type>;

            static constexpr double Growth_Rate = 2;

        public:
            HWSHQTB_CONSTEXPR20 trivial_vector()noexcept(noexcept(allocator_type())):
                _allocator(), _size(0), _capacity(0), _buffer(nullptr) {}
            HWSHQTB_CONSTEXPR20 explicit trivial_vector(const allocator_type& allocator)noexcept:
                _allocator(allocator), _size(0), _capacity(0), _buffer(nullptr) {}
            HWSHQTB_CONSTEXPR20 trivial_vector(size_type count, const value_type& value, const allocator_type& allocator = allocator_type()) :
                _allocator(allocator), _size(count), _capacity(count), _buffer(_allocate(_capacity)) {
                for (size_type i = 0; i < count; ++i)
                    *(_buffer + i) = value;
            }
            explicit trivial_vector(size_type count, const allocator_type& allocator = allocator_type()):
                _allocator(allocator), _size(count), _capacity(count), _buffer(_allocate(_capacity)) {}
            template <typename InputIt, typename std::enable_if<std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>::value, int>::type = 0>
            HWSHQTB_CONSTEXPR20 explicit trivial_vector(InputIt first, InputIt last, const allocator_type& allocator = allocator_type()):
                _allocator(allocator), _size(std::distance(first, last)), _capacity(_size), _buffer(_allocate(_capacity)) {
                pointer buffer = _buffer;
                for (size_type i = 0; i < _size; ++i)
                    *(_buffer + i) = *first++;
            }
            HWSHQTB_CONSTEXPR20 trivial_vector(const trivial_vector& other):
                _allocator(allocator_traits::select_on_container_copy_construction(other.get_allocator())), _size(other._size), _capacity(_size), _buffer(_allocate(_capacity)) {
                if (_capacity != 0)
                    std::memmove(_buffer, other._buffer, _size * sizeof(value_type));
            }
            HWSHQTB_CONSTEXPR20 trivial_vector(const trivial_vector& other, const allocator_type& allocator) :
                _allocator(allocator_traits::select_on_container_copy_construction(other.get_allocator())), _size(other._size), _capacity(_size), _buffer(_allocate(_capacity)) {
                if (_capacity != 0)
                    std::memmove(_buffer, other._buffer, _size * sizeof(value_type));
            }
            HWSHQTB_CONSTEXPR20 trivial_vector(trivial_vector&& other)noexcept:
                _allocator(std::move(other._allocator)), _size(exchange(other._size, 0)), _capacity(exchange(other._capacity, 0)), _buffer(exchange(other._buffer, nullptr)) {}
            HWSHQTB_CONSTEXPR20 trivial_vector(trivial_vector&& other, const allocator_type& allocator) :
                _allocator(allocator), _size(exchange(other._size(), 0)), _capacity(0), _buffer(nullptr) {
                if (_allocator != other._allocator) {
                    _capacity = _size;
                    _buffer = _allocate(_capacity);
                    std::memmove(_buffer, other._buffer, _size * sizeof(value_type));
                }
                else {
                    std::swap(_capacity, other._capacity);
                    std::swap(_buffer, other._buffer);
                }
            }
            HWSHQTB_CONSTEXPR20 trivial_vector(std::initializer_list<T> ilist, const allocator_type& allocator = allocator_type()):
                _allocator(allocator), _size(ilist.size()), _capacity(_size), _buffer(_allocate(_capacity)) {
                if (_capacity != 0)
                    std::memmove(_buffer, ilist.begin(), _size * sizeof(value_type));
            }

            HWSHQTB_CONSTEXPR20 ~trivial_vector() {
                if (_capacity != 0)
                    _deallocate(_buffer, _capacity);
            }

            HWSHQTB_CONSTEXPR20 trivial_vector& operator=(const trivial_vector& other) {
                bool check_propagate = false;
                if HWSHQTB_CONSTEXPR17(!allocator_traits::is_always_equal::value) {
                    if (_allocator != other._allocator) {
                        if (_buffer) {
                            _deallocate(_buffer, _capacity);
                            _buffer = nullptr;
                            _capacity = 0;
                        }
                        check_propagate = true;
                    }
                }
                if HWSHQTB_CONSTEXPR17(allocator_traits::propagate_on_container_copy_assignment::value)
                    _allocator = other._allocator;
                else if (check_propagate)
                    _allocator = other._allocator;
                if (_capacity < other._capacity)
                    _reallocate(other._capacity);
                std::memmove(_buffer, other._buffer, (_size = other._size) * sizeof(value_type));
                return *this;
            }
            HWSHQTB_CONSTEXPR20 trivial_vector& operator=(trivial_vector&& other) {
                bool check_propagate = false;
                if HWSHQTB_CONSTEXPR17(!allocator_traits::is_always_equal::value) {
                    if (_allocator != other._allocator) {
                        if (_buffer) {
                            _deallocate(_buffer, _capacity);
                            _buffer = nullptr;
                            _capacity = 0;
                        }
                        check_propagate = true;
                    }
                }
                if HWSHQTB_CONSTEXPR17(allocator_traits::propagate_on_container_copy_assignment::value)
                    _allocator = other._allocator;
                else if (check_propagate)
                    _allocator = other._allocator;

                if (!check_propagate)
                    _buffer = exchange(other._buffer, nullptr);
                else {
                    if (_capacity < other._capacity)
                        _reallocate(other._capacity);
                    std::memmove(_buffer, other._buffer, (_size = other._size) * sizeof(value_type));
                }
                _size = exchange(other._size, 0);
                _capacity = exchange(other._capacity, 0);
                return *this;
            }

            HWSHQTB_CONSTEXPR20 void assign(size_type count, const T& value) {
                if (_capacity < count) {
                    ~trivial_vector();
                    _buffer = _allocate(count);
                    _capacity = count;
                }
                _size = count;
                for (size_type i = 0; i < count; ++i) {
                    std::memmove(_buffer + i, std::addressof(value), sizeof(value_type));
                }
            }
            template <typename InputIt, typename std::enable_if<std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>::value, int>::type = 0>
            HWSHQTB_CONSTEXPR20 void assign(InputIt first, InputIt last) {
                size_type count = std::distance(first, last);
                if (_capacity < count) {
                    ~trivial_vector();
                    _buffer = _allocate(count);
                    _capacity = count;
                }
                _size = count;
                pointer buffer = _buffer;
                while (first != last) {
                    std::memmove(buffer++, std::addressof(*first++), sizeof(value_type));
                }
            }
            HWSHQTB_CONSTEXPR20 void assign(std::initializer_list<T> ilist) {
                size_type count = ilist.size();
                if (_capacity < count) {
                    ~trivial_vector();
                    _buffer = _allocate(count);
                    _capacity = count;
                }
                _size = count;
                std::memmove(_buffer, ilist.begin(), _size * sizeof(value_type));
            }

            HWSHQTB_CONSTEXPR20 allocator_type get_allocator()const noexcept {
                return _allocator;
            }

            HWSHQTB_CONSTEXPR20 reference at(size_type pos) {
                if (pos > _size) throw std::out_of_range("out_of_range");
                return _buffer[pos];
            }
            HWSHQTB_CONSTEXPR20 const_reference at(size_type pos)const {
                if (pos > _size) throw std::out_of_range("out_of_range");
                return _buffer[pos];
            }

            HWSHQTB_CONSTEXPR20 reference operator[](size_type pos) {
                return _buffer[pos];
            }
            HWSHQTB_CONSTEXPR20 const_reference operator[](size_type pos)const {
                return _buffer[pos];
            }

            HWSHQTB_CONSTEXPR20 reference front() {
                return *_buffer;
            }
            HWSHQTB_CONSTEXPR20 const_reference front()const {
                return *_buffer;
            }

            HWSHQTB_CONSTEXPR20 reference back() {
                return _buffer[_size - 1];
            }
            HWSHQTB_CONSTEXPR20 const_reference back()const {
                return _buffer[_size - 1];
            }

            HWSHQTB_CONSTEXPR20 pointer data()noexcept {
                return _buffer;
            }
            HWSHQTB_CONSTEXPR20 const_pointer data()const noexcept {
                return _buffer;
            }

            HWSHQTB_CONSTEXPR20 iterator begin()noexcept {
                return iterator(_buffer);
            }
            HWSHQTB_CONSTEXPR20 const_iterator begin()const noexcept {
                return const_iterator(_buffer);
            }
            HWSHQTB_CONSTEXPR20 const_iterator cbegin()const noexcept {
                return const_iterator(_buffer);
            }

            HWSHQTB_CONSTEXPR20 iterator end()noexcept {
                return iterator(_buffer + _size);
            }
            HWSHQTB_CONSTEXPR20 const_iterator end()const noexcept {
                return const_iterator(_buffer + _size);
            }
            HWSHQTB_CONSTEXPR20 const_iterator cend()const noexcept {
                return const_iterator(_buffer + _size);
            }

            HWSHQTB_CONSTEXPR20 reverse_iterator rbegin()noexcept {
                return iterator(iterator(_buffer + _size));
            }
            HWSHQTB_CONSTEXPR20 const_reverse_iterator rbegin()const noexcept {
                return const_iterator(const_iterator(_buffer + _size));
            }
            HWSHQTB_CONSTEXPR20 const_reverse_iterator crbegin()const noexcept {
                return const_iterator(const_iterator(_buffer + _size));
            }

            HWSHQTB_CONSTEXPR20 reverse_iterator rend()noexcept {
                return iterator(iterator(_buffer));
            }
            HWSHQTB_CONSTEXPR20 const_reverse_iterator rend()const noexcept {
                return const_iterator(const_iterator(_buffer));
            }
            HWSHQTB_CONSTEXPR20 const_reverse_iterator rcend()const noexcept {
                return const_iterator(const_iterator(_buffer));
            }

            HWSHQTB_CONSTEXPR20 bool empty() const noexcept {
                return _size == 0;
            }

            HWSHQTB_CONSTEXPR20 size_type size()const noexcept {
                return _size;
            }

            HWSHQTB_CONSTEXPR20 size_type max_size() const noexcept {
                return std::numeric_limits<difference_type>::max() / sizeof(value_type);
            }

            HWSHQTB_CONSTEXPR20 void reserve(size_type new_capacity) {
                if (new_capacity > max_size())
                    throw std::length_error("hwshqtb::container::trivial_vector<T, Allocator>::reserve() => std::length_error");
                if (new_capacity > _capacity) {
                    new_capacity = _get_new_capacity(new_capacity);
                    pointer new_buffer = allocator_traits::_allocate(_allocator, new_capacity);
                    if (_capacity != 0) {
                        std::memmove(new_buffer, _buffer, _size * sizeof(value_type));
                        _deallocate(_buffer, _capacity);
                    }
                    _capacity = new_capacity;
                    _buffer = new_buffer;
                }
            }

            HWSHQTB_CONSTEXPR20 size_type capacity() const noexcept {
                return _capacity;
            }

            HWSHQTB_CONSTEXPR20 void shrink_to_fit() {
                if (_size == 0) {
                    ~trivial_vector();
                    _capacity = 0;
                    _buffer = nullptr;
                }
                else if (_size < _capacity) {
                    pointer new_buffer = allocator_traits::_allocate(_allocator, _size);
                    std::memmove(new_buffer, _buffer, _size * sizeof(value_type));
                    _deallocate(_buffer, _capacity);
                    _capacity = _size;
                    _buffer = new_buffer;
                }
            }

            HWSHQTB_CONSTEXPR20 void clear() noexcept {
                _size = 0;
            }

            HWSHQTB_CONSTEXPR20 iterator insert(const_iterator pos, const T& value) {
                difference_type size = pos - cbegin();
                _renew_buffer_by_insert(size, 1);

                ++_size;
                std::memmove(_buffer + size, std::addressof(value), sizeof(value_type));
                return begin() + size;
            }
            HWSHQTB_CONSTEXPR20 iterator insert(const_iterator pos, T&& value) {
                difference_type size = pos - cbegin();
                _renew_buffer_by_insert(size, 1);

                ++_size;
                std::memmove(_buffer + size, std::addressof(value), sizeof(value_type));
                return begin() + size;
            }
            HWSHQTB_CONSTEXPR20 iterator insert(const_iterator pos, size_type count, const T& value) {
                if (count <= 0)
                    return pos;

                difference_type size = pos - cbegin();
                _renew_buffer_by_insert(size, count);

                _size += count;
                while (count--)
                    std::memmove(_buffer + size + count, std::addressof(value), sizeof(value_type));
                return begin() + size;
            }
            template <typename InputIt, typename std::enable_if<std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>::value, size_type>::type = 0>
            HWSHQTB_CONSTEXPR20 iterator insert(const_iterator pos, InputIt first, InputIt last) {
                difference_type count = std::distance(first, last);
                if (count <= 0)
                    return pos;

                difference_type size = pos - cbegin();
                _renew_buffer_by_insert(size, pos);

                _size += count;
                while (count--)
                    std::memmove(_buffer + size + count, std::addressof(*--last), sizeof(value_type));
                return begin() + size;
            }
            HWSHQTB_CONSTEXPR20 iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
                size_type count = ilist.size();
                if (count <= 0)
                    return pos;

                difference_type size = pos - cbegin();
                _renew_buffer_by_insert(size, pos);

                _size += count;
                std::memmove(_buffer + size, ilist.begin(), count * sizeof(value_type));
                return begin() + size;
            }

            template <class... Args>
            HWSHQTB_CONSTEXPR20 iterator emplace(const_iterator pos, Args&&... args) {
                difference_type size = pos - cbegin();
                _renew_buffer_by_insert(size, 1);

                ++_size;
                allocator_traits::construct(_allocator, _buffer + size, args...);
                return begin() + size;
            }

            HWSHQTB_CONSTEXPR20 iterator erase(const_iterator pos) {
                difference_type size = pos - cbegin();
                --_size;
                std::memmove(_buffer + size, _buffer + size + 1, (_size - size) * sizeof(value_type));
                return begin() + size;
            }
            HWSHQTB_CONSTEXPR20 iterator erase(const_iterator first, const_iterator last) {
                difference_type count = std::distance(first, last);
                if (count <= 0)
                    return begin() + (last - cbegin());
                difference_type size = first - cbegin();
                --_size;
                std::memmove(_buffer + size, _buffer + size + count, (_size + 1 - size - count) * sizeof(value_type));
                return begin() + size;
            }

            HWSHQTB_CONSTEXPR20 void push_back(const T& value) {
                _renew_buffer_by_insert(_size, 1);

                ++_size;
                std::memmove(_buffer + _size - 1, std::addressof(value), sizeof(value_type));
            }
            HWSHQTB_CONSTEXPR20 void push_back(T&& value) {
                _renew_buffer_by_insert(_size, 1);

                ++_size;
                std::memmove(_buffer + _size - 1, std::addressof(value), sizeof(value_type));
            }

            template <class... Args>
            HWSHQTB_CONSTEXPR20 reference emplace_back(Args&&... args) {
                _renew_buffer_by_insert(_size, 1);

                ++_size;
                allocator_traits::construct(_allocator, _buffer + _size - 1, args...);
                return back();
            }

            HWSHQTB_CONSTEXPR20 void pop_back() {
                if (_size) --_size;
            }

            HWSHQTB_CONSTEXPR20 void resize(size_type count) {
                if (count > _size) _renew_buffer_by_insert(_size, count - _size);
                _size = count;
            }
            HWSHQTB_CONSTEXPR20 void resize(size_type count, const value_type& value) {
                if (count > _size) _renew_buffer_by_insert(_size, count - _size);

                while (count > _size)
                    std::memmove(_buffer + _size++, std::addressof(value), sizeof(value_type));

                _size = count;
            }

            HWSHQTB_CONSTEXPR20 void swap(trivial_vector& other) {
                if (allocator_traits::propagate_on_container_swap::value) std::swap(_allocator, other._allocator);
                std::swap(_size, other._size);
                std::swap(_capacity, other._capacity);
                std::swap(_buffer, other._buffer);
            }

        private:
            allocator_type _allocator;
            size_type _size;
            size_type _capacity;
            pointer _buffer;

            HWSHQTB_CONSTEXPR20 pointer _allocate(size_type new_capacity) {
                return new_capacity == 0 ? nullptr : allocator_traits::_allocate(_allocator, new_capacity);
            }

            HWSHQTB_CONSTEXPR20 void _deallocate(pointer buffer, size_type capacity) {
                allocator_traits::deallocate(_allocator, buffer, capacity);
            }

            HWSHQTB_CONSTEXPR20 void _reallocate(size_type new_capacity) {
                new_capacity = _get_new_capacity(new_capacity);
                pointer new_buffer = _allocate(new_capacity);
                if (_capacity != 0) {
                    std::memmove(new_buffer, _buffer, _size * sizeof(value_type));
                    _deallocate(_buffer, _capacity);
                }
                _buffer = new_buffer;
                _capacity = new_capacity;
            }

            HWSHQTB_CONSTEXPR20 size_type _get_new_capacity(size_type new_capacity)const noexcept {
                if (_capacity == 0)
                    return new_capacity;
                size_type old_capacity = _capacity;
                while (old_capacity < new_capacity) {
                    if (old_capacity > (size_type)(max_size() / Growth_Rate))
                        old_capacity = max_size();
                    old_capacity *= Growth_Rate;
                }
                return old_capacity;
            }

            HWSHQTB_CONSTEXPR20 void _renew_buffer_by_insert(difference_type pos, difference_type count) {
                if (_size + count > _capacity) {
                    size_type new_capacity = _get_new_capacity(_size + count);
                    pointer new_buffer = allocator_traits::_allocate(_allocator, new_capacity);
                    if (_capacity != 0) {
                        std::memmove(new_buffer, _buffer, pos * sizeof(value_type));
                        std::memmove(new_buffer + pos + count, _buffer + pos, (_size - pos) * sizeof(value_type));
                        allocator_traits::deallocate(_allocator, _buffer, _capacity);
                    }
                    _buffer = new_buffer;
                    _capacity = new_capacity;
                }
                else std::memmove(_buffer + pos + count, _buffer + pos, (_size - pos) * sizeof(value_type));
            }
        };

        template <typename T, class Allocator>
        class trivial_vector<T, Allocator>::iterator {
            friend trivial_vector<T, Allocator>;
            friend typename trivial_vector<T, Allocator>::const_iterator;

        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using const_pointer = const T*;
            using reference = T&;
            using const_reference = const T&;

        public:
            HWSHQTB_CONSTEXPR20 iterator()noexcept:
                _pointer(nullptr) {}
            HWSHQTB_CONSTEXPR20 iterator(const iterator& other) = default;

        private:
            HWSHQTB_CONSTEXPR20 explicit iterator(pointer p)noexcept:
                _pointer(p) {}

        public:
            HWSHQTB_CONSTEXPR20 ~iterator() = default;

            HWSHQTB_CONSTEXPR20 reference operator*() {
                return *_pointer;
            }
            HWSHQTB_CONSTEXPR20 const_reference operator*()const {
                return *_pointer;
            }
            HWSHQTB_CONSTEXPR20 pointer operator->() {
                return _pointer;
            }
            HWSHQTB_CONSTEXPR20 const_pointer operator->()const {
                return _pointer;
            }

            HWSHQTB_CONSTEXPR20 iterator& operator++()noexcept {
                ++_pointer;
                return *this;
            }
            HWSHQTB_CONSTEXPR20 iterator operator++(int)noexcept {
                return iterator(_pointer++);
            }
            HWSHQTB_CONSTEXPR20 iterator& operator--()noexcept {
                --_pointer;
                return *this;
            }
            HWSHQTB_CONSTEXPR20 iterator operator--(int)noexcept {
                return iterator(_pointer--);
            }
            HWSHQTB_CONSTEXPR20 iterator& operator+=(const difference_type n)noexcept {
                _pointer += n;
                return *this;
            }
            HWSHQTB_CONSTEXPR20 iterator& operator-=(const difference_type n)noexcept {
                _pointer -= n;
                return *this;
            }
            HWSHQTB_CONSTEXPR20 difference_type operator-(const iterator& other)const noexcept {
                return _pointer - other._pointer;
            }
            HWSHQTB_CONSTEXPR20 reference operator[](const difference_type n)noexcept {
                return _pointer[n];
            }
            HWSHQTB_CONSTEXPR20 const_reference operator[](const difference_type n)const noexcept {
                return _pointer[n];
            }

            HWSHQTB_CONSTEXPR20 iterator operator+(const difference_type n)const noexcept {
                return iterator(_pointer + n);
            }
            HWSHQTB_CONSTEXPR20 iterator operator-(const difference_type n)const noexcept {
                return iterator(_pointer + n);
            }

            HWSHQTB_CONSTEXPR20 bool operator==(const iterator& other)const noexcept {
                return _pointer == other._pointer;
            }
            HWSHQTB_CONSTEXPR20 bool operator<(const iterator& other)const noexcept {
                return _pointer < other._pointer;
            }
            HWSHQTB_CONSTEXPR20 bool operator!=(const iterator& other)const noexcept {
                return _pointer != other._pointer;
            }
            HWSHQTB_CONSTEXPR20 bool operator>(const iterator& other)const noexcept {
                return _pointer > other._pointer;
            }
            HWSHQTB_CONSTEXPR20 bool operator<=(const iterator& other)const noexcept {
                return _pointer <= other._pointer;
            }
            HWSHQTB_CONSTEXPR20 bool operator>=(const iterator& other)const noexcept {
                return _pointer >= other._pointer;
            }

        private:
            pointer _pointer;

        };

        template <typename T, class Allocator>
        class trivial_vector<T, Allocator>::const_iterator {
            friend trivial_vector<T, Allocator>;

        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = const T*;
            using reference = const T&;

        public:
            HWSHQTB_CONSTEXPR20 const_iterator()noexcept:
                _pointer(nullptr) {}
            HWSHQTB_CONSTEXPR20 const_iterator(const const_iterator& other)noexcept:
                _pointer(other._pointer) {}
            HWSHQTB_CONSTEXPR20 const_iterator(const trivial_vector<T, Allocator>::iterator& other)noexcept:
                _pointer(other._pointer) {}

        private:
            HWSHQTB_CONSTEXPR20 explicit const_iterator(pointer p)noexcept:
                _pointer(p) {}

        public:
            HWSHQTB_CONSTEXPR20 ~const_iterator() = default;

            HWSHQTB_CONSTEXPR20 reference operator*()const {
                return *_pointer;
            }
            HWSHQTB_CONSTEXPR20 pointer operator->()const {
                return _pointer;
            }

            HWSHQTB_CONSTEXPR20 const_iterator& operator++()noexcept {
                ++_pointer;
                return *this;
            }
            HWSHQTB_CONSTEXPR20 const_iterator operator++(int)noexcept {
                return const_iterator(_pointer++);
            }
            HWSHQTB_CONSTEXPR20 const_iterator& operator--()noexcept {
                --_pointer;
                return *this;
            }
            HWSHQTB_CONSTEXPR20 const_iterator operator--(int)noexcept {
                return const_iterator(_pointer--);
            }
            HWSHQTB_CONSTEXPR20 const_iterator& operator+=(const difference_type n)noexcept {
                _pointer += n;
                return *this;
            }
            HWSHQTB_CONSTEXPR20 const_iterator& operator-=(const difference_type n)noexcept {
                _pointer -= n;
                return *this;
            }
            HWSHQTB_CONSTEXPR20 difference_type operator-(const const_iterator& other)const noexcept {
                return _pointer - other._pointer;
            }
            HWSHQTB_CONSTEXPR20 reference operator[](const difference_type n)const noexcept {
                return _pointer[n];
            }

            HWSHQTB_CONSTEXPR20 const_iterator operator+(const difference_type n)const noexcept {
                return const_iterator(_pointer + n);
            }
            HWSHQTB_CONSTEXPR20 const_iterator operator-(const difference_type n)const noexcept {
                return const_iterator(_pointer + n);
            }

            HWSHQTB_CONSTEXPR20 bool operator==(const const_iterator& other)const noexcept {
                return _pointer == other._pointer;
            }
            HWSHQTB_CONSTEXPR20 bool operator<(const const_iterator& other)const noexcept {
                return _pointer < other._pointer;
            }
            HWSHQTB_CONSTEXPR20 bool operator!=(const const_iterator& other)const noexcept {
                return _pointer != other._pointer;
            }
            HWSHQTB_CONSTEXPR20 bool operator>(const const_iterator& other)const noexcept {
                return _pointer > other._pointer;
            }
            HWSHQTB_CONSTEXPR20 bool operator<=(const const_iterator& other)const noexcept {
                return _pointer <= other._pointer;
            }
            HWSHQTB_CONSTEXPR20 bool operator>=(const const_iterator& other)const noexcept {
                return _pointer >= other._pointer;
            }

        private:
            pointer _pointer;

        };

        template <typename T, class Allocator>
        HWSHQTB_CONSTEXPR20 bool operator==(const trivial_vector<T, Allocator>& a, const trivial_vector<T, Allocator>& b) {
            if (a.size() != b.size()) return false;

            for (typename trivial_vector<T, Allocator>::size_type i = 0; i < a.size(); ++i)
                if (a[i] != b[i]) return false;

            return true;
        }
        template <typename T, class Allocator>
        HWSHQTB_CONSTEXPR20 bool operator!=(const trivial_vector<T, Allocator>& a, const trivial_vector<T, Allocator>& b) {
            if (a.size() != b.size()) return true;

            for (typename trivial_vector<T, Allocator>::size_type i = 0; i < a.size(); ++i)
                if (a[i] != b[i]) return true;

            return false;
        }
        template <typename T, class Allocator>
        HWSHQTB_CONSTEXPR20 bool operator<(const trivial_vector<T, Allocator>& a, const trivial_vector<T, Allocator>& b) {
            for (typename trivial_vector<T, Allocator>::size_type i = 0; i < a.size(); ++i)
                if (a[i] != b[i]) return a[i] < b[i];

            return a.size() < b.size();
        }
        template <typename T, class Allocator>
        HWSHQTB_CONSTEXPR20 bool operator<=(const trivial_vector<T, Allocator>& a, const trivial_vector<T, Allocator>& b) {
            for (typename trivial_vector<T, Allocator>::size_type i = 0; i < a.size(); ++i)
                if (a[i] != b[i]) return a[i] < b[i];

            return a.size() <= b.size();
        }
        template <typename T, class Allocator>
        HWSHQTB_CONSTEXPR20 bool operator>(const trivial_vector<T, Allocator>& a, const trivial_vector<T, Allocator>& b) {
            for (typename trivial_vector<T, Allocator>::size_type i = 0; i < a.size(); ++i)
                if (a[i] != b[i]) return a[i] > b[i];

            return a.size() > b.size();
        }
        template <typename T, class Allocator>
        HWSHQTB_CONSTEXPR20 bool operator>=(const trivial_vector<T, Allocator>& a, const trivial_vector<T, Allocator>& b) {
            for (typename trivial_vector<T, Allocator>::size_type i = 0; i < a.size(); ++i)
                if (a[i] != b[i]) return a[i] > b[i];

            return a.size() >= b.size();
        }
    }
}

namespace std {
    template <typename T, class Allocator>
    void swap(hwshqtb::container::trivial_vector<T, Allocator>& a, hwshqtb::container::trivial_vector<T, Allocator>& b) {
        a.swap(b);
    }
}

#endif