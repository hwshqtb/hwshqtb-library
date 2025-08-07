#ifndef HWSHQTB__TREE__BST_HPP
#define HWSHQTB__TREE__BST_HPP

#include <type_traits>
#include <memory>
#include <iterator>

namespace hwshqtb {
    namespace tree {
        template <typename Key, typename Mapped>
        class binary_search_tree_data;

        template <typename Key, typename Mapped, class Derived>
        struct binary_search_tree_node;

        template <class Node>
        class binary_search_tree_iterator;

        template <class Node>
        class binary_search_tree_const_iterator;

        template <typename Key, typename Mapped, class Allocator>
        class node_handle;

        template <typename Key, typename Mapped, class Compare>
        class value_compare;
    }

    template <template <typename, typename, bool, typename, typename> class binary_seach_tree,
        typename Key, class Compare = std::less<Key>, class Allocator = std::allocator<Key>>
        using set = binary_seach_tree<Key, void, false, Compare, Allocator>;

    template <template <typename, typename, bool, typename, typename> class binary_seach_tree,
        typename Key, typename Mapped, class Compare = std::less<Key>, class Allocator = std::allocator<std::pair<Key, Mapped>>>
    using map = binary_seach_tree<Key, Mapped, false, Compare, Allocator>;

    template <template <typename, typename, bool, typename, typename> class binary_seach_tree,
        typename Key, class Compare = std::less<Key>, class Allocator = std::allocator<Key>>
        using multiset = binary_seach_tree<Key, void, true, Compare, Allocator>;

    template <template <typename, typename, bool, typename, typename> class binary_seach_tree,
        typename Key, typename Mapped, class Compare = std::less<Key>, class Allocator = std::allocator<std::pair<Key, Mapped>>>
    using multimap = binary_seach_tree<Key, Mapped, true, Compare, Allocator>;

    namespace tree {
        template <typename Key, typename Mapped>
        class binary_search_tree_data {
        public:
            static constexpr inline bool mapped_is_void = std::is_void_v<Mapped>;

            using key_type = const Key;
            using mapped_type = std::conditional_t<mapped_is_void, key_type, Mapped>;
            using value_type = std::conditional_t<mapped_is_void, key_type, std::pair<key_type, mapped_type>>;

            binary_search_tree_data(const value_type& value):
                _value(value) {}

            constexpr key_type& key()const {
                if constexpr (mapped_is_void)
                    return const_cast<binary_search_tree_data<Key, Mapped>*>(std::launder(this))->_value;
                else
                    return const_cast<binary_search_tree_data<Key, Mapped>*>(std::launder(this))->_value.first;
            }
            constexpr mapped_type& mapped()const {
                if constexpr (mapped_is_void)
                    return const_cast<binary_search_tree_data<Key, Mapped>*>(std::launder(this))->_value;
                else
                    return const_cast<binary_search_tree_data<Key, Mapped>*>(std::launder(this))->_value.second;
            }
            constexpr value_type& value()const {
                return const_cast<binary_search_tree_data<Key, Mapped>*>(std::launder(this))->_value;
            }

            static constexpr const key_type& key(const value_type& value) {
                if constexpr (mapped_is_void)
                    return value;
                else
                    return value.first;
            }

            template <class Allocator>
            static constexpr binary_search_tree_data* from_node_handle(node_handle<Key, Mapped, Allocator>& handle) {
                return std::exchange(handle._data, nullptr);
            }
            template <class Allocator>
            constexpr node_handle<Key, Mapped, Allocator> to_node_handle(const Allocator& allocator)const {
                return node_handle<Key, Mapped, Allocator>(const_cast<binary_search_tree_data*>(this), allocator);
            }
            template <class Allocator>
            constexpr void to_node_handle(node_handle<Key, Mapped, Allocator>& handle)const {
                handle._data = const_cast<binary_search_tree_data*>(this);
            }

        private:
            value_type _value;

        };

        template <typename Key, typename Mapped, class Derived>
        struct binary_search_tree_node {
            using base_type = binary_search_tree_node;
            using true_node = std::conditional_t<std::is_void_v<Derived>, binary_search_tree_node, Derived>;
            using data_type = binary_search_tree_data<Key, Mapped>;

            true_node* parent = nullptr;
            data_type* data = nullptr;
            std::size_t size = 1;
            true_node* left = nullptr;
            true_node* right = nullptr;
            true_node* lesser = static_cast<true_node*>(const_cast<base_type*>(this));
            true_node* greater = static_cast<true_node*>(const_cast<base_type*>(this));

            constexpr std::size_t left_size()const {
                return left == nullptr ? 0 : left->size;
            }
            constexpr std::size_t right_size()const {
                return right == nullptr ? 0 : right->size;
            }
            constexpr void update_size() {
                size = left_size() + right_size() + 1;
            }

            static constexpr true_node* from_iterator(binary_search_tree_iterator<true_node> iterator) {
                return iterator._node;
            }
            static constexpr true_node* from_const_iterator(binary_search_tree_const_iterator<true_node> iterator) {
                return const_cast<true_node*>(iterator._node);
            }
            constexpr binary_search_tree_iterator<true_node> to_iterator()const {
                true_node* node = static_cast<true_node*>(const_cast<base_type*>(this));
                return binary_search_tree_iterator<true_node>(node);
            }
            constexpr binary_search_tree_const_iterator<true_node> to_const_iterator()const {
                const true_node* node = static_cast<const true_node*>(this);
                return binary_search_tree_const_iterator<true_node>(node);
            }

        };

        template <class Node>
        class binary_search_tree_iterator {
            friend typename Node::base_type;
            friend binary_search_tree_const_iterator<Node>;

        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = typename std::remove_reference_t<decltype(*(std::declval<Node>().data))>::value_type;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type*;
            using const_pointer = const value_type*;
            using reference = value_type&;
            using const_reference = const value_type&;

        private:
            constexpr binary_search_tree_iterator(Node* node)noexcept:
                _node(node) {}

        public:
            constexpr reference operator*() {
                return _node->data->value();
            }
            constexpr const_reference operator*()const {
                return _node->data->value();
            }
            constexpr pointer operator->() {
                return std::addressof(_node->data->value());
            }
            constexpr const_pointer operator->()const {
                return std::addressof(_node->data->value());
            }

            constexpr binary_search_tree_iterator& operator++()noexcept {
                _node = _node->greater;
                return *this;
            }
            constexpr binary_search_tree_iterator operator++(int)noexcept {
                binary_search_tree_iterator old = *this;
                _node = _node->greater;
                return old;
            }
            constexpr binary_search_tree_iterator& operator--()noexcept {
                _node = _node->lesser;
                return *this;
            }
            constexpr binary_search_tree_iterator operator--(int)noexcept {
                binary_search_tree_iterator old = *this;
                _node = _node->lesser;
                return old;
            }

            constexpr bool operator==(const binary_search_tree_iterator& other)const noexcept {
                return _node == other._node;
            }
            constexpr bool operator!=(const binary_search_tree_iterator& other)const noexcept {
                return _node != other._node;
            }

        private:
            Node* _node;

        };

        template <class Node>
        class binary_search_tree_const_iterator {
            friend typename Node::base_type;

        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = typename std::remove_reference_t<decltype(*(std::declval<Node>().data))>::value_type;
            using difference_type = std::ptrdiff_t;
            using pointer = const value_type*;
            using reference = const value_type&;

        private:
            constexpr binary_search_tree_const_iterator(const Node* node)noexcept:
                _node(node) {}

        public:
            constexpr binary_search_tree_const_iterator(const binary_search_tree_iterator<Node>& other)noexcept:
                _node(other._node) {}

            constexpr reference operator*()const {
                return _node->data->value();
            }
            constexpr pointer operator->()const {
                return std::addressof(_node->data->value());
            }

            constexpr binary_search_tree_const_iterator& operator++()noexcept {
                _node = _node->greater;
                return *this;
            }
            constexpr binary_search_tree_const_iterator operator++(int)noexcept {
                binary_search_tree_const_iterator old = *this;
                _node = _node->greater;
                return old;
            }
            constexpr binary_search_tree_const_iterator& operator--()noexcept {
                _node = _node->lesser;
                return *this;
            }
            constexpr binary_search_tree_const_iterator operator--(int)noexcept {
                binary_search_tree_const_iterator old = *this;
                _node = _node->lesser;
                return old;
            }

            constexpr bool operator==(const binary_search_tree_const_iterator& other)const noexcept {
                return _node == other._node;
            }
            constexpr bool operator!=(const binary_search_tree_const_iterator& other)const noexcept {
                return _node != other._node;
            }

        private:
            const Node* _node;

        };

        template <typename Key, typename Mapped, class Allocator>
        class node_key_handle {
            friend node_handle<Key, Mapped, Allocator>;

        public:
            using data_type = binary_search_tree_data<Key, Mapped>;
            using key_type = typename data_type::key_type;
            using mapped_type = typename data_type::mapped_type;
            using allocator_type = Allocator;
            using allocator_traits = std::allocator_traits<Allocator>;

        private:
            node_key_handle(data_type* data, const Allocator& allocator):
                _data(data), _allocator(allocator) {}

        public:
            template <typename U>
            node_key_handle& operator=(U&& key) {
                if constexpr (data_type::mapped_is_void) {
                    allocator_traits::destroy(_allocator, _data);
                    allocator_traits::construct(_allocator, _data, std::forward<U>(key));
                }
                else {
                    mapped_type&& mapped = std::move(_data->mapped());
                    allocator_traits::destroy(_allocator, _data);
                    allocator_traits::construct(_allocator, _data, std::forward<U>(key), mapped);
                }
                return *this;
            }
            operator key_type& () {
                return _data->key();
            }

        private:
            data_type* _data;
            allocator_type _allocator;

        };

        template <typename Key, typename Mapped, class Allocator>
        class node_mapped_handle {
            friend node_handle<Key, Mapped, Allocator>;

        public:
            using data_type = binary_search_tree_data<Key, Mapped>;
            using mapped_type = typename data_type::mapped_type;
            using allocator_type = Allocator;
            using allocator_traits = std::allocator_traits<Allocator>;

        private:
            node_mapped_handle(data_type* data, const Allocator& allocator):
                _data(data), _allocator(allocator) {}

        public:
            template <typename U>
            node_mapped_handle& operator=(U&& mapped) {
                if constexpr (data_type::mapped_is_void) {
                    allocator_traits::destroy(_allocator, _data);
                    allocator_traits::construct(_allocator, _data, std::forward<U>(mapped));
                }
                else {
                    _data->mapped() = std::forward<U>(mapped);
                }
                return *this;
            }
            operator mapped_type& () {
                return _data->mapped();
            }

        private:
            data_type* _data;
            allocator_type _allocator;

        };

        template <typename Key, typename Mapped, class Allocator>
        class node_value_handle {
            friend node_handle<Key, Mapped, Allocator>;

        public:
            using data_type = binary_search_tree_data<Key, Mapped>;
            using value_type = typename data_type::value_type;
            using allocator_type = Allocator;
            using allocator_traits = std::allocator_traits<Allocator>;

        private:
            node_value_handle(data_type* data, const Allocator& allocator):
                _data(data), _allocator(allocator) {}

        public:
            template <typename U>
            node_value_handle& operator=(U&& value) {
                allocator_traits::destroy(_allocator, _data);
                allocator_traits::construct(_allocator, _data, std::forward<U>(value));
                return *this;
            }
            operator value_type& () {
                return _data->value();
            }

        private:
            data_type* _data;
            allocator_type _allocator;

        };

        template <typename Key, typename Mapped, class Allocator>
        class node_handle {
            friend binary_search_tree_data<Key, Mapped>;

        private:
            using data_type = binary_search_tree_data<Key, Mapped>;

        public:
            using key_type = node_key_handle<Key, Mapped, Allocator>;
            using mapped_type = node_mapped_handle<Key, Mapped, Allocator>;
            using value_type = node_value_handle<Key, Mapped, Allocator>;
            using allocator_type = Allocator;

        private:
            using allocator_traits = std::allocator_traits<allocator_type>;

        private:
            constexpr explicit node_handle(data_type* data, allocator_type allocator)noexcept(std::is_nothrow_default_constructible_v<allocator_type>):
                _data(data), _allocator(allocator) {}

        public:
            constexpr node_handle():
                _data(nullptr), _allocator() {}
            constexpr node_handle(node_handle&& other)noexcept(std::is_nothrow_move_constructible_v<allocator_type>):
                _data(std::exchange(other._data, nullptr)), _allocator(std::move(other._allocator)) {}
            node_handle(const node_handle& other) = delete;
            ~node_handle() {
                if (_data != nullptr) {
                    allocator_traits::destroy(_allocator, _data);
                    allocator_traits::deallocate(_allocator, _data, 1);
                }
            }

            constexpr node_handle& operator=(node_handle&& other) {
                if (_data.memory != nullptr) {
                    allocator_traits::destory(_allocator, _data.memory);
                    allocator_traits::deallocate(_allocator, _data.memory, 1);
                }
                if (allocator_traits::propagate_on_container_move_assignment || _data == nullptr)
                    _allocator(std::move(other._allocator));
                _data = std::exchange(other._data.memory, nullptr);
            }
            node_handle& operator=(const node_handle& other) = delete;

            bool empty()const noexcept {
                return _data.memory == nullptr;
            }
            explicit operator bool()const noexcept {
                return _data.memory != nullptr;
            }

            allocator_type get_allocator()const noexcept(std::is_nothrow_copy_constructible_v<allocator_type>) {
                return _allocator;
            }

            value_type value()const noexcept {
                return value_type(const_cast<node_handle*>(this)->_data, _allocator);
            }
            key_type key()const noexcept {
                return key_type(const_cast<node_handle*>(this)->_data, _allocator);
            }
            mapped_type mapped()const noexcept {
                return mapped_type(const_cast<node_handle*>(this)->_data, _allocator);
            }

            void swap(node_handle& other) {
                std::swap(_data.memory, other._data.memory);
                if constexpr (allocator_traits::propagate_on_container_swap)
                    std::swap(_allocator, other._allocator);
            }

        private:
            data_type* _data;
            allocator_type _allocator;

        };

        template <typename Key, typename Mapped, class Compare>
        class value_compare {
        public:
            constexpr value_compare(const Compare& c)noexcept(std::is_nothrow_copy_constructible_v<Compare>):
                _compare(c) {}

            constexpr bool operator()(const typename binary_search_tree_data<Key, Mapped>::value_type& a, const typename binary_search_tree_data<Key, Mapped>::value_type& b)const noexcept(noexcept(std::declval<Compare>()(std::declval<Key>(), std::declval<Key>()))) {
                return _compare(binary_search_tree_data<Key, Mapped>::key(a), binary_search_tree_data<Key, Mapped>::key(b));
            }

        private:
            Compare _compare;

        };
    }
}

#endif