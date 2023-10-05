#ifndef _HWSHQTB_TRIVIAL_VECTOR_HPP
#define _HWSHQTB_TRIVIAL_VECTOR_HPP

#include <type_traits>
#include <iterator>
#include <memory>
#include <cstring>

namespace hwshqtb {
    namespace details {
        template <typename Iter, typename = void>
        struct is_iterator:
            public std::false_type {};
        template <typename Iter>
        struct is_iterator<Iter, std::void_t<typename std::iterator_traits<Iter>::iterator_category> >:
            public std::true_type {};
        template <class Iter>
        static constexpr bool is_iterator_v = is_iterator<Iter>::value;

        template <class Iter, typename = void>
        struct is_input_iterator:
            public std::false_type {};
        template <class Iter>
        struct is_input_iterator<Iter, std::void_t<std::enable_if_t<is_iterator_v<Iter> > > >:
            public std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<Iter>::iterator_category> {};
        template <class Iter>
        static constexpr bool is_input_iterator_v = is_input_iterator<Iter>::value;
    }
    template <typename T>
    class trivial_vector_iterator {
        static_assert(std::is_trivial_v<T>, "T must be TrivialType");

    public:
        typedef std::ptrdiff_t difference_type;
        typedef T value_type;
        typedef T* pointer;
        typedef T& reference;
        typedef std::random_access_iterator_tag iterator_category;
    private:
        typedef trivial_vector_iterator<T> self_type;

        trivial_vector_iterator(pointer _p):_ptr(_p) {}

        bool operator==(const self_type& other)const noexcept {
            return _ptr == other._ptr;
        }
        bool operator!=(const self_type& other)const noexcept {
            return _ptr != other._ptr;
        }

        bool operator<(const self_type& other)const noexcept {
            return _ptr < other._ptr;
        }
        bool operator>(const self_type& other)const noexcept {
            return _ptr > other._ptr;
        }
        bool operator<=(const self_type& other)const noexcept {
            return _ptr <= other._ptr;
        }
        bool operator>=(const self_type& other)const noexcept {
            return _ptr >= other._ptr;
        }

        reference operator*()noexcept {
            return *_ptr;
        }
        pointer operator->()noexcept {
            return _ptr;
        }

        self_type& operator++()noexcept {
            ++_ptr;
            return *this;
        }
        self_type operator++(int)noexcept {
            self_type i = *this;
            ++_ptr;
            return i;
        }
        self_type& operator--()noexcept {
            --_ptr;
            return *this;
        }
        self_type operator--(int)noexcept {
            self_type i = *this;
            --_ptr;
            return i;
        }
        self_type& operator+=(difference_type n)noexcept {
            _ptr += n;
            return *this;
        }
        self_type& operator-=(difference_type n)noexcept {
            _ptr -= n;
            return *this;
        }

        difference_type operator-(const self_type& other)const noexcept {
            return _ptr - other._ptr;
        }

        reference operator[](difference_type n)const noexcept {
            return *(_ptr + n);
        }

    private:
        pointer _ptr;
    };

    template <typename T>
    trivial_vector_iterator<T> operator+(const trivial_vector_iterator<T>& it, typename trivial_vector_iterator<T>::difference_type n)noexcept {
        trivial_vector_iterator<T> temp = it;
        return temp += n;
    }
    template <typename T>
    trivial_vector_iterator<T> operator+(typename trivial_vector_iterator<T>::difference_type n, const trivial_vector_iterator<T>& it)noexcept {
        trivial_vector_iterator<T> temp = it;
        return temp += n;
    }
    template <typename T>
    trivial_vector_iterator<T> operator-(const trivial_vector_iterator<T>& it, typename trivial_vector_iterator<T>::difference_type n)noexcept {
        trivial_vector_iterator<T> temp = it;
        return temp -= n;
    }

    template <typename T, class Allocator = std::allocator<T> >
    class trivial_vector {
        static_assert(std::is_trivial_v<T>, "T must be TrivialType");

    public:
        typedef T value_type;
        typedef Allocator allocator_type;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;
        typedef T& reference;
        typedef const T& const_reference;
        typedef T* pointer;
        typedef const T* const_pointer;
        typedef trivial_vector_iterator<T> iterator;
        typedef trivial_vector_iterator<const T> const_iterator;
        typedef std::reverse_iterator<trivial_vector_iterator<T> > reverse_iterator;
        typedef std::reverse_iterator<trivial_vector_iterator<const T> > const_reverse_iterator;
    private:
        typedef std::allocator_traits<Allocator> memory;
        typedef trivial_vector<T> self_type;

    public:
        trivial_vector()noexcept(noexcept(allocator_type())):
            _allocator(), _size(0), _capacity(0), _buffer(nullptr) {}
        explicit trivial_vector(const allocator_type& allocator)noexcept:
            _allocator(allocator), _size(0), _capacity(0), _buffer(nullptr) {}
        explicit trivial_vector(size_type count, const value_type& value, const allocator_type& allocator = allocator_type()):
            _allocator(allocator), _size(count), _capacity(count), _buffer(memory::allocate(_allocator, count)) {
            for (int i = 0; i < count; ++i) {
                memory::construct(_allocator, _buffer + i, value);
            }
        }
        explicit trivial_vector(size_type count, const allocator_type& allocator = allocator_type()):
            _allocator(allocator), _size(count), _capacity(count), _buffer(memory::allocate(_allocator, count)) {
            for (int i = 0; i < count; ++i) {
                memory::construct(_allocator, _buffer + i);
            }
        }
        template <typename InputIt, typename = std::enable_if_t<details::is_input_iterator_v<InputIt> > >
        explicit trivial_vector(InputIt first, InputIt last, const allocator_type& allocator = allocator_type()):
            _allocator(allocator), _size(std::distance(first, last)), _capacity(_size), _buffer(memory::allocate(_allocator, _size)) {
            pointer buffer = _buffer;
            while (first != last) {
                memory::construct(_allocator, buffer++, *first++);
            }
        }
        trivial_vector(const self_type& other):
            _allocator(memory::select_on_container_copy_construction(other.get_allocator())), _size(other._size), _capacity(_size), _buffer(memory::allocate(_allocator, _size)) {
            std::memcpy(_buffer, other._buffer, _size * sizeof(value_type));
        }
        trivial_vector(const self_type& other, const allocator_type& allocator):
            _allocator(allocator), _size(other._size), _capacity(_size), _buffer(memory::allocate(_allocator, _size)) {
            std::memcpy(_buffer, other._buffer, _size * sizeof(value_type));
        }
        trivial_vector(self_type&& other)noexcept:
            _allocator(std::exchange(other._allocator, {})), _size(std::exchange(other._size, 0)), _capacity(std::exchange(other._capacity, 0)), _buffer(std::exchange(other._buffer, nullptr)) {}
        trivial_vector(self_type&& other, const allocator_type& allocator):
            _allocator(allocator), _size(std::exchange(other._size, 0)), _capacity(std::exchange(other._capacity, 0)), _buffer(std::exchange(other._buffer, nullptr)) {}
        trivial_vector(std::initializer_list<T> init, const allocator_type& allocator = allocator_type()):
            _allocator(allocator), _size(init.size()), _capacity(_size), _buffer(memory::allocate(_allocator, _size)) {
            std::memcpy(_buffer, init.begin(), _size * sizeof(value_type));
        }

        ~trivial_vector() {
            if (_buffer != nullptr) {
                memory::deallocate(_allocator, _buffer, _capacity);
            }
        }

        self_type& operator=(const self_type& other)noexcept {
            if (_capacity != other._size) {
                ~trivial_vector();
            }
            std::memcpy(_buffer, other._buffer, (_size = other._size) * sizeof(value_type));
            return *this;
        }

        reference operator[](size_type i) {
            return _buffer[i];
        }

    private:
        allocator_type _allocator;
        size_type _size;
        size_type _capacity;
        pointer _buffer;

        //void reallocate() {
        //    // capacity *= 1.5
        //    _capacity += (_capacity >> 1);
        //    pointer p = std::allocator_traits<allocator_type>::allocate(_alloc, _capacity);
        //    std::memcpy(p, _buffer, _size * sizeof(value_type));
        //    delete _buffer;
        //    _buffer = p;
        //}
    };
}

#endif