#ifndef HWSHQTB__CONTIANER__DYNAMIC_BITSET_HPP
#define HWSHQTB__CONTIANER__DYNAMIC_BITSET_HPP

#include "../version.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <iostream>
#include <list>

namespace hwshqtb {
    namespace container {
        template <typename T = std::size_t, class Container = std::vector<T>>
        class dynamic_bitset {
        private:
            using base_type = T;

            static_assert(std::is_integral_v<base_type> && !std::numeric_limits<base_type>::is_signed, "");

            static constexpr base_type Block = std::numeric_limits<base_type>::digits;
            static constexpr base_type Mask = std::numeric_limits<base_type>::max();
            static constexpr base_type Bit = base_type(1);
            static constexpr base_type Zero = base_type(0);

        public:
            using container_type = Container;
            using value_type = bool;
            using allocator_type = typename container_type::allocator_type;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            class reference;
            using const_reference = bool;
            class iterator;
            class const_iterator;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;
            using pointer = iterator;
            using const_pointer = const_iterator;

            constexpr dynamic_bitset()noexcept:
                _memory(), _unused(0) {}
            constexpr dynamic_bitset(const allocator_type& allocator)noexcept(noexcept(Container(allocator))):
                _memory(allocator), _unused(0) {}
            constexpr explicit dynamic_bitset(size_type count, const allocator_type& allocator = allocator_type())noexcept(noexcept(Container(count, allocator))):
                _memory(count / Block + (count % Block ? 1 : 0), allocator), _unused((Block - count % Block) % Block) {}
            constexpr dynamic_bitset(size_type count, const value_type& value, const allocator_type& allocator = allocator_type())noexcept(noexcept(Container(count, Block, allocator))):
                _memory(count / Block + (count % Block ? 1 : 0), value ? Block : 0, allocator), _unused((Block - count % Block) % Block) {}
            template <typename T, std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>, int> = 0>
            constexpr dynamic_bitset(size_type size, T value, const allocator_type& allocator = allocator_type())noexcept(noexcept(Container(size, allocator))):
                dynamic_bitset(size, allocator) {
                for (auto x : *this) {
                    if (value == 0)
                        break;
                    x = value & Bit;
                    value >>= 1;
                }
            }
            template <class InputIt, std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, int> = 0>
            constexpr dynamic_bitset(InputIt first, InputIt last) {
                assign(first, last);
            }
            template <typename CharT, class Traits, class Allocator>
            constexpr dynamic_bitset(const std::basic_string<CharT, Traits, Allocator>& str, typename std::basic_string<CharT, Traits, Allocator>::size_type pos = 0, typename std::basic_string<CharT, Traits, Allocator>::size_type n = std::basic_string<CharT, Traits, Allocator>::npos, CharT zero = CharT('0'), CharT one = CharT('1')):
                dynamic_bitset(n == std::basic_string<CharT, Traits, Allocator>::npos ? str.size() - pos : n, 0) {
                assert(str.size() > pos);
                if (n == std::basic_string<CharT, Traits, Allocator>::npos)
                    n = str.size() - pos;
                typename std::basic_string<CharT, Traits, Allocator>::const_iterator iter = str.cbegin();
                std::advance(iter, pos);
                for (reverse_iterator x = rbegin() + (n > str.size() - pos ? n + pos - str.size() : 0); x != rend(); ++x) {
                    if (iter == str.cend()) break;
                    *x = std::char_traits<CharT>::eq(*iter, zero) ? false : (std::char_traits<CharT>::eq(*iter, one) ? true : throw std::invalid_argument(""));
                    ++iter;
                }
            }
            template <typename CharT, class Traits>
            constexpr dynamic_bitset(const std::basic_string_view<CharT, Traits>& str, std::size_t pos = 0, std::size_t n = std::size_t(-1), CharT zero = CharT('0'), CharT one = CharT('1')):
                dynamic_bitset(n == std::size_t(-1) ? str.size() - pos : n, 0) {
                if (n == std::size_t(-1))
                    n = str.size() - pos;
                typename std::basic_string_view<CharT, Traits>::const_iterator iter = str.cbegin();
                std::advance(iter, pos);
                for (reverse_iterator x = rbegin() + (n > str.size() - pos ? n + pos - str.size() : 0); x != rend(); ++x) {
                    if (iter == str.cend()) break;
                    *x = std::char_traits<CharT>::eq(*iter, zero) ? false : (std::char_traits<CharT>::eq(*iter, one) ? true : throw std::invalid_argument(""));
                    ++iter;
                }
            }
            template <typename CharT>
            constexpr dynamic_bitset(const CharT* str, std::size_t pos = 0, std::size_t n = std::size_t(-1), CharT zero = CharT('0'), CharT one = CharT('1')):
                dynamic_bitset(n == std::size_t(-1) ? std::char_traits<CharT>::length(str) - pos : n, 0) {
                size_type size = std::char_traits<CharT>::length(str);
                if (n == std::size_t(-1))
                    n = size - pos;
                for (reverse_iterator x = rbegin() + (n > size - pos ? n + pos - size : 0); x != rend(); ++x) {
                    if (pos == size) break;
                    *x = std::char_traits<CharT>::eq(str[pos], zero) ? false : (std::char_traits<CharT>::eq(str[pos], one) ? true : throw std::invalid_argument(""));
                    ++pos;
                }
            }
            constexpr dynamic_bitset(std::initializer_list<value_type> init, const allocator_type& allocator = allocator_type()):
                dynamic_bitset(init.size(), allocator) {
                typename std::initializer_list<value_type>::const_iterator iter = init.begin();
                for (auto& x : *this)
                    x = *iter++;
            }
            constexpr dynamic_bitset(const dynamic_bitset& other) = default;
            constexpr dynamic_bitset(const dynamic_bitset& other, const allocator_type& allocator)noexcept(noexcept(Container(other._memory, allocator))):
                _memory(other._memory, allocator), _unused(other._unused) {}
            constexpr dynamic_bitset(dynamic_bitset&& other) noexcept:
                _memory(std::move(other._memory)), _unused(std::exchange(other._unused, 0)) {}
            constexpr dynamic_bitset(dynamic_bitset&& other, const allocator_type& allocator)noexcept(noexcept(Container(std::move(other._memory), allocator))):
                _memory(std::move(other._memory), allocator), _unused(std::exchange(other._unused, 0)) {}
            constexpr ~dynamic_bitset() = default;

            constexpr dynamic_bitset& operator=(const dynamic_bitset& other) = default;
            constexpr dynamic_bitset& operator=(dynamic_bitset&& other)noexcept(noexcept(std::is_nothrow_move_assignable_v<Container>)) {
                _memory = std::move(other._memory);
                _unused = std::exchange(other._unused, 0);
                return *this;
            }
            constexpr dynamic_bitset& operator=(std::initializer_list<value_type> ilist) {
                assign(ilist);
                return *this;
            }
            constexpr void assign(size_type count, const value_type& value)noexcept(noexcept(Container::assign(count, value))) {
                _memory.assign(count / Block + (count % Block ? 1 : 0), value ? Block : 0);
                _unused = (Block - count % Block) % Block;
            }
            template <class InputIt, std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, int> = 0>
            constexpr void assign(InputIt first, InputIt last) {
                difference_type new_size = std::distance(first, last);
                resize(new_size);
                for (auto& x : *this)
                    x = *first++;
            }
            constexpr void assign(std::initializer_list<value_type> ilist) {
                resize(ilist.size());
                const value_type* first = ilist.begin();
                for (auto& x : *this)
                    x = *first++;
            }

            constexpr bool operator==(const dynamic_bitset& other)const noexcept {
                assert(_memory.size() == other._memory.size() && _unused == other._unused);
                if (size() == 0) return true;
                typename container_type::const_reverse_iterator iter = _memory.crbegin(), iter2 = other._memory.crbegin();
                if ((*iter++ << _unused) != (*iter2++ << _unused))
                    return false;
                while (iter != _memory.crend())
                    if (*iter++ != *iter2++)
                        return false;
                return true;
            }
            constexpr bool operator!=(const dynamic_bitset& other)const noexcept {
                return !operator==(other);
            }

            template <typename CharT = char, class Traits = std::char_traits<CharT>, class Allocator = std::allocator<CharT>>
            constexpr std::basic_string<CharT, Traits, Allocator> to_string(CharT zero = CharT('0'), CharT one = CharT('1'))const {
                std::basic_string<CharT, Traits, Allocator> result;
                result.reserve(_memory.size() * Block - _unused);
                for (const_reverse_iterator iter = crbegin(); iter != crend(); ++iter)
                    result.push_back(*iter ? one : zero);
                return result;
            }
            template <typename T, std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>, int> = 0>
            constexpr explicit operator T()const {
                T result{0};
                for (const_reverse_iterator x = crbegin(); x != crend(); ++x) {
                    T t = result * 2 + (*x ? 1 : 0);
                    if (result > t) throw std::overflow_error("");
                    else result = std::move(t);
                }
                return result;
            }

            constexpr reference operator[](size_type index)noexcept {
                typename container_type::iterator iter = _memory.begin();
                std::advance(iter, index / Block);
                return reference(Bit << (index % Block), iter);
            }
            constexpr const_reference operator[](size_type index)const {
                return (*const_cast<dynamic_bitset*>(this))[index];
            }
            constexpr reference at(size_type index) {
                if (index >= _memory.size() * Block - _unused)
                    throw std::out_of_range("");
                return operator[](index);
            }
            constexpr const_reference at(size_type index)const {
                if (index >= _memory.size() * Block - _unused)
                    throw std::out_of_range("");
                return operator[](index);
            }
            constexpr const_reference test(size_type index)const {
                return at(index);
            }
            constexpr bool all()const noexcept {
                typename container_type::const_iterator iter = _memory.cbegin(), end = --_memory.cend();
                while (iter != end)
                    if (*iter ^ Mask)
                        return false;
                if (_unused) {
                    return !((*iter ^ Mask) & ((Bit << (Block - _unused)) - 1));
                }
                else return !(*iter ^ Mask);
            }
            constexpr bool any()const noexcept {
                typename container_type::const_iterator iter = _memory.cbegin(), end = --_memory.cend();
                while (iter != end)
                    if (*iter)
                        return true;
                if (_unused) return *iter & ((Bit << (Block - _unused)) - 1);
                else return *iter;
            }
            constexpr bool none()const noexcept {
                typename container_type::const_iterator iter = _memory.cbegin(), end = --_memory.cend();
                while (iter != end)
                    if (*iter)
                        return false;
                if (_unused) {
                    return !(*iter & ((Bit << (Block - _unused)) - 1));
                }
                else return !*iter;
            }
            constexpr size_type count()const noexcept {
                size_type result = 0;
                for (auto x : *this)
                    result += (int)(bool)x;
                return result;
            }
            constexpr reference front()noexcept {
                return *begin();
            }
            constexpr const_reference front()const noexcept {
                return *cbegin();
            }
            constexpr reference back()noexcept {
                return *rbegin();
            }
            constexpr const_reference back()const noexcept {
                return *crbegin();
            }

            constexpr dynamic_bitset& operator&=(const dynamic_bitset& other)noexcept {
                assert(_memory.size() == other._memory.size() && _unused == other._unused);
                typename container_type::const_iterator iter = other._memory.cbegin();
                for (auto& x : _memory)
                    x &= *iter++;
                return *this;
            }
            constexpr dynamic_bitset& operator|=(const dynamic_bitset& other)noexcept {
                assert(_memory.size() == other._memory.size() && _unused == other._unused);
                typename container_type::const_iterator iter = other._memory.cbegin();
                for (auto& x : _memory)
                    x |= *iter++;
                return *this;
            }
            constexpr dynamic_bitset& operator^=(const dynamic_bitset& other)noexcept {
                assert(_memory.size() == other._memory.size() && _unused == other._unused);
                typename container_type::const_iterator iter = other._memory.cbegin();
                for (auto& x : _memory)
                    x |= *iter++;
                return *this;
            }
            constexpr dynamic_bitset& operator<<=(size_type pos)noexcept {
                size_type remaining = pos % Block, block = pos / Block;
                if (block) {
                    typename container_type::iterator iter = _memory.begin(), iter2 = iter;
                    std::advance(iter2, block);
                    while (iter2 != _memory.end())
                        *iter++ = *iter2++;
                    while (iter != _memory.end())
                        *iter++ = Zero;
                }
                if (remaining) {
                    typename container_type::iterator iter = _memory.begin(), iter2 = iter;
                    std::advance(iter2, 1);
                    while (iter2 != _memory.end()) {
                        *iter <<= remaining;
                        *iter++ |= *iter2++ >> (Block - remaining);
                    }
                    *iter <<= remaining;
                }
                return *this;
            }
            constexpr dynamic_bitset& operator>>=(size_type pos)noexcept {
                size_type remaining = pos % Block, block = pos / Block;
                if (block) {
                    typename container_type::reverse_iterator iter = _memory.rbegin(), iter2 = iter;
                    std::advance(iter2, block);
                    while (iter2 != _memory.rend())
                        *iter++ = *iter2++;
                    while (iter != _memory.rend())
                        *iter++ = Zero;
                }
                if (remaining) {
                    typename container_type::reverse_iterator iter = _memory.rbegin(), iter2 = iter;
                    std::advance(iter2, 1);
                    while (iter2 != _memory.rend()) {
                        *iter >>= remaining;
                        *iter++ |= *iter2++ << (Block - remaining);
                    }
                    *iter >>= remaining;
                }
                return *this;
            }
            constexpr dynamic_bitset& set(bool value = true)noexcept {
                if (value) _memory.assign(_memory.size(), Mask);
                else _memory.assign(_memory.size(), 0);
                return *this;
            }
            constexpr dynamic_bitset& set(size_type pos, bool value = true) {
                at(pos) = value;
                return *this;
            }
            constexpr dynamic_bitset& reset(bool value = false)noexcept {
                if (value) _memory.assign(_memory.size(), Mask);
                else _memory.assign(_memory.size(), 0);
                return *this;
            }
            constexpr dynamic_bitset& reset(size_type pos, bool value = false) {
                at(pos) = value;
                return *this;
            }
            constexpr dynamic_bitset& flip()noexcept {
                for (auto& x : _memory)
                    x = ~x;
                return *this;
            }
            constexpr dynamic_bitset& flip(size_type pos) {
                auto x = at(pos);
                x = ~x;
                return *this;
            }

            constexpr iterator begin()noexcept {
                return iterator(0, _memory.begin());
            }
            constexpr const_iterator begin()const noexcept {
                return cbegin();
            }
            constexpr const_iterator cbegin()const noexcept {
                return const_iterator(0, _memory.cbegin());
            }
            constexpr iterator end()noexcept {
                if (_unused == 0)
                    return iterator(0, _memory.end());
                else
                    return iterator(Block - _unused, --_memory.end());
            }
            constexpr const_iterator end()const noexcept {
                return cend();
            }
            constexpr const_iterator cend()const noexcept {
                if (_unused == 0)
                    return const_iterator(0, _memory.cend());
                else
                    return const_iterator(Block - _unused, --_memory.cend());
            }
            constexpr reverse_iterator rbegin()noexcept {
                return reverse_iterator(end());
            }
            constexpr const_reverse_iterator rbegin()const noexcept {
                return crbegin();
            }
            constexpr const_reverse_iterator crbegin()const noexcept {
                return const_reverse_iterator(cend());
            }
            constexpr reverse_iterator rend()noexcept {
                return reverse_iterator(begin());
            }
            constexpr const_reverse_iterator rend()const noexcept {
                return crend();
            }
            constexpr const_reverse_iterator crend()const noexcept {
                return const_reverse_iterator(cbegin());
            }

            constexpr bool empty()const noexcept {
                return _memory.empty();
            }
            constexpr size_type size()const noexcept {
                return _memory.size() * Block - _unused;
            }
            constexpr size_type max_size()const noexcept {
                return _memory.max_size() * Block;
            }
            constexpr void reserve(size_type capacity) {
                _memory.reserve(capacity / Block + (capacity % Block ? 1 : 0));
            }
            constexpr size_type capacity()const noexcept {
                return _memory.capacity() * Block;
            }

            constexpr void clear()noexcept {
                _memory.clear();
                _unused = 0;
            }
            constexpr void push_back(value_type value) {
                if (_unused) --_unused;
                else {
                    _memory.push_back({});
                    _unused = Block - 1;
                }
                *rbegin() = value;
            }
            constexpr void pop_back() {
                if (_unused == Block - 1) {
                    _memory.pop_back();
                    _unused = 0;
                }
                else ++_unused;
            }
            constexpr void resize(size_type new_size) {
                _memory.resize(new_size / Block + (new_size % Block ? 1 : 0));
                _unused = (Block - new_size % Block) % Block;
            }
            constexpr void swap(dynamic_bitset& other) {
                std::swap(_memory, other._memory);
                std::swap(_unused, other._unused);
            }

        private:
            container_type _memory;
            size_type _unused;

        };

        template <typename T, class Container>
        class dynamic_bitset<T, Container>::reference {
            friend dynamic_bitset;
            friend iterator;
            friend const_iterator;

        private:
            constexpr reference(base_type mask, typename container_type::iterator memory)noexcept:
                _mask(mask), _memory(memory) {};

        public:
            constexpr reference& operator=(bool x) {
                if (x)
                    *_memory |= _mask;
                else
                    *_memory &= Mask ^ _mask;
                return *this;
            }
            constexpr reference& operator=(const reference& x) {
                if (x)
                    *_memory |= _mask;
                else
                    *_memory &= Mask ^ _mask;
                return *this;
            }

            constexpr operator const_reference()const {
                return *_memory & _mask;
            }

        private:
            base_type _mask;
            typename container_type::iterator _memory;
        };

        template <typename T, class Container>
        class dynamic_bitset<T, Container>::iterator {
            friend dynamic_bitset;

        private:
            constexpr iterator(size_type index, typename container_type::iterator memory)noexcept:
                _index(index), _memory(memory) {};

        public:
            using difference_type = std::ptrdiff_t;
            using value_type = bool;
            using pointer = iterator;
            using reference = reference;
            using iterator_category = typename std::iterator_traits<typename container_type::iterator>::iterator_category;

            constexpr reference operator*()noexcept {
                return reference(Bit << _index, _memory);
            }
            constexpr const_reference operator*()const {
                return reference(Bit << _index, _memory);
            }

            constexpr bool operator==(const iterator& other)const noexcept {
                return _index == other._index && _memory == other._memory;
            }
            constexpr bool operator!=(const iterator& other)const noexcept {
                return _index != other._index || _memory != other._memory;
            }

            constexpr iterator& operator++()noexcept {
                if (++_index == Block) {
                    _index = 0;
                    ++_memory;
                }
                return *this;
            }
            constexpr iterator operator++(int)noexcept {
                iterator result = *this;
                operator++();
                return result;
            }
            constexpr iterator& operator--()noexcept {
                if (_index == 0) {
                    _index = Block - 1;
                    --_memory;
                }
                else --_index;
                return *this;
            }
            constexpr iterator operator--(int)noexcept {
                iterator result = *this;
                operator--();
                return result;
            }
            constexpr iterator& operator+=(difference_type diff)noexcept {
                if (diff < 0)
                    return operator-=(-diff);
                _index += diff;
                std::advance(_memory, _index / Block);
                _index %= Block;
                return *this;
            }
            constexpr iterator& operator-=(difference_type diff)noexcept {
                if (diff < 0)
                    return operator+=(-diff);
                if (_index < (size_type)diff) {
                    difference_type d = (difference_type)_index - diff;
                    _memory = std::prev(_memory, (-(d + 1)) / Block + 1);
                    _index = ((d + 1) % Block + Block) % Block;
                }
                else _index -= diff;
                return *this;
            }

            constexpr iterator operator+(difference_type diff)const noexcept {
                iterator result = *this;
                return result += diff;
            }
            constexpr iterator operator-(difference_type diff)const noexcept {
                iterator result = *this;
                return result -= diff;
            }

            constexpr difference_type operator-(const iterator& other)const noexcept {
                return _index - other._index + std::distance(_memory, other._memory) * Block;
            }

            constexpr operator const_iterator()const noexcept {
                return const_iterator(_index, _memory);
            }

        private:
            size_type _index;
            typename container_type::iterator _memory;

        };

        template <typename T, class Container>
        class dynamic_bitset<T, Container>::const_iterator {
            friend dynamic_bitset;
            friend iterator;

        private:
            constexpr const_iterator(size_type index, typename container_type::const_iterator memory)noexcept:
                _index(index), _memory(memory) {};

        public:
            using difference_type = std::ptrdiff_t;
            using value_type = bool;
            using pointer = const_iterator;
            using reference = bool;
            using iterator_category = typename std::iterator_traits<typename container_type::const_iterator>::iterator_category;

            constexpr reference operator*()const {
                return *_memory & (Bit << _index);
            }

            constexpr bool operator==(const const_iterator& other)const noexcept {
                return _index == other._index && _memory == other._memory;
            }
            constexpr bool operator!=(const const_iterator& other)const noexcept {
                return _index != other._index || _memory != other._memory;
            }

            constexpr const_iterator& operator++()noexcept {
                if (++_index == Block) {
                    _index = 0;
                    ++_memory;
                }
                return *this;
            }
            constexpr const_iterator operator++(int)noexcept {
                const_iterator result = *this;
                operator++();
                return result;
            }
            constexpr const_iterator& operator--()noexcept {
                if (_index == 0) {
                    _index = Block - 1;
                    --_memory;
                }
                else --_index;
                return *this;
            }
            constexpr const_iterator operator--(int)noexcept {
                const_iterator result = *this;
                operator--();
                return result;
            }
            constexpr const_iterator& operator+=(difference_type diff)noexcept {
                if (diff < 0)
                    return operator-=(-diff);
                _index += diff;
                std::advance(_memory, _index / Block);
                _index %= Block;
                return *this;
            }
            constexpr const_iterator& operator-=(difference_type diff)noexcept {
                if (diff < 0)
                    return operator+=(-diff);
                if (_index < (size_type)diff) {
                    difference_type d = (difference_type)_index - diff;
                    _memory = std::prev(_memory, (-(d + 1)) / Block + 1);
                    _index = ((d + 1) % Block + Block) % Block;
                }
                else _index -= diff;
                return *this;
            }

            constexpr const_iterator operator+(difference_type diff)const noexcept {
                const_iterator result = *this;
                return result += diff;
            }
            constexpr const_iterator operator-(difference_type diff)const noexcept {
                const_iterator result = *this;
                return result -= diff;
            }

            constexpr difference_type operator-(const const_iterator& other)const noexcept {
                return _index - other._index + std::distance(_memory, other._memory) * Block;
            }

        private:
            size_type _index;
            typename container_type::const_iterator _memory;

        };

        template <typename T, class Container>
        constexpr dynamic_bitset<T, Container> operator&(const dynamic_bitset<T, Container>& a, const dynamic_bitset<T, Container>& b) {
            dynamic_bitset<T, Container> result = a;
            result &= b;
            return result;
        }
        template <typename T, class Container>
        constexpr dynamic_bitset<T, Container> operator|(const dynamic_bitset<T, Container>& a, const dynamic_bitset<T, Container>& b) {
            dynamic_bitset<T, Container> result = a;
            result |= b;
            return result;
        }
        template <typename T, class Container>
        constexpr dynamic_bitset<T, Container> operator^(const dynamic_bitset<T, Container>& a, const dynamic_bitset<T, Container>& b) {
            dynamic_bitset<T, Container> result = a;
            result ^= b;
            return result;
        }
        template <typename T, class Container>
        constexpr dynamic_bitset<T, Container> operator~(const dynamic_bitset<T, Container>& a) {
            dynamic_bitset<T, Container> result = a;
            return result.flip();
        }
        template <typename T, class Container>
        constexpr dynamic_bitset<T, Container> operator<<(const dynamic_bitset<T, Container>& a, typename dynamic_bitset<T, Container>::size_type pos) {
            dynamic_bitset<T, Container> result = a;
            result <<= pos;
            return result;
        }
        template <typename T, class Container>
        constexpr dynamic_bitset<T, Container> operator>>(const dynamic_bitset<T, Container>& a, typename dynamic_bitset<T, Container>::size_type pos) {
            dynamic_bitset<T, Container> result = a;
            result >>= pos;
            return result;
        }

        template <typename CharT, class Traits, typename T, class Container>
        std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const dynamic_bitset<T, Container>& x) {
            const std::ctype<CharT>& ctype = std::use_facet<std::ctype<CharT>>(os.getloc());
            CharT zero = ctype.widen('0'), one = ctype.widen('1');
            os << x.to_string(zero, one);
            return os;
        }
        template <typename CharT, class Traits, typename T, class Container>
        std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, dynamic_bitset<T, Container>& x) {
            typename std::basic_istream<CharT, Traits>::sentry check_sentry(is);
            if (!check_sentry) return is;
            if (!x.size()) return is;

            CharT zero = is.widen('0'), one = is.widen('1');
            std::basic_string<CharT, Traits> digits;
            for (std::istreambuf_iterator<CharT, Traits> in(is), end; true; ++in) {
                if (in == end) break;
                CharT c = *in;
                if (Traits::eq(c, zero) || Traits::eq(c, one)) {
                    if (digits.size() == x.size()) break;
                    digits.push_back(c);
                }
                else break;
            }
            if (!digits.size())
                is.setstate(std::ios_base::failbit);
            else
                x = digits;
            return is;
        }

    }
}

namespace std {
    template <class Key>
    struct hash;

    template <typename T, class Container>
    struct hash<hwshqtb::container::dynamic_bitset<T, Container>> {
        size_t operator()(const hwshqtb::dynamic_bitset<T, Container>& x) {
            vector<bool> a(x.cbegin(), x.cend());
            return hash<std::vector<bool>>()(a);
        }
    };

    template <typename T, class Container>
    void swap(hwshqtb::container::dynamic_bitset<T, Container>& a, hwshqtb::container::dynamic_bitset<T, Container>& b) {
        a.swap(b);
    }

}

#endif