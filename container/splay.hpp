#ifndef HWSHQTB__CONTAINER__SPLAY_HPP
#define HWSHQTB__CONTAINER__SPLAY_HPP

#include "../../version.hpp"
#include <memory>

namespace hwshqtb {
    namespace container {
        template <typename Value>
        struct splay_trait;

        template <typename Value, class Allocator>
        struct splay_node;

        template <typename Value, class Allocator>
        class splay_iterator;

        template <typename Value, class Allocator>
        class splay_const_iterator;

        template <typename Value, class Compare, class Allocator, bool Multiple>
        class splay;
    }

    namespace container {
        template <typename Key, typename Mapped>
        struct splay_trait<std::pair<const Key, Mapped>> {
            using key_type = Key;
            using mapped_type = Mapped;
            using value_type = std::pair<const Key, Mapped>;
            static constexpr bool is_set = false;
        };

        template <typename Key>
        struct splay_trait {
            using key_type = Key;
            using mapped_type = void;
            using value_type = Key;
            static constexpr bool is_set = true;
        };

        template <typename Value, class Allocator>
        struct splay_node {
            Value* value;
            std::size_t size = 1;
            splay_node* parent = nullptr;
            splay_node* left = nullptr;
            splay_node* right = nullptr;
            splay_node* less = nullptr;
            splay_node* greater = nullptr;

            template <typename... Ts>
            HWSHQTB_CONSTEXPR20 splay_node(Allocator& allocator, Ts&&... values):
                value(std::allocator_traits<Allocator>::allocate(allocator, 1)) {
                std::allocator_traits<Allocator>::construct(allocator, value, std::forward<Ts>(values)...);
            }

            HWSHQTB_CONSTEXPR20 std::size_t left_size()const {
                return this->left == nullptr ? 0 : this->left->size;
            }
            HWSHQTB_CONSTEXPR20 std::size_t right_size()const {
                return this->right == nullptr ? 0 : this->right->size;
            }
            HWSHQTB_CONSTEXPR20 void update_size() {
                size = left_size() + right_size() + 1;
            }
            bool is_left_son()const {
                return this == parent->left;
            }
            splay_node*& son(bool is_left) {
                return (is_left ? left : right);
            }
        };

        template <typename Value, class Allocator>
        class splay_iterator {
            friend splay_const_iterator<Value, Allocator>;
            template <typename, typename, typename, bool>
            friend class splay;

        private:
            HWSHQTB_CONSTEXPR20 splay_iterator(splay_node<Value, Allocator>* node, bool end):
                _node(node), _end(end) {};

        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = Value;
            using difference_type = std::ptrdiff_t;
            using pointer = Value*;
            using const_pointer = const Value*;
            using reference = Value&;
            using const_reference = const Value&;

            HWSHQTB_CONSTEXPR20 reference operator*() {
                return *_node->value;
            }
            HWSHQTB_CONSTEXPR20 const_reference operator*()const {
                return *_node->value;
            }
            HWSHQTB_CONSTEXPR20 pointer operator->() {
                return _node->value;
            }
            HWSHQTB_CONSTEXPR20 const_pointer operator->()const {
                return _node->value;
            }

            HWSHQTB_CONSTEXPR20 splay_iterator& operator++() {
                if (_node == nullptr) return *this;
                if (_end) return *this;
                if (_node->greater == nullptr) {
                    _end = true;
                    return *this;
                }
                _node = _node->greater;
                return *this;
            }
            HWSHQTB_CONSTEXPR20 splay_iterator& operator--() {
                if (_node == nullptr) return *this;
                if (_node->less == nullptr) return *this;
                if (_end) {
                    _end = false;
                    return *this;
                }
                _node = _node->less;
                return *this;
            }
            HWSHQTB_CONSTEXPR20 splay_iterator operator++(int)const {
                splay_iterator old = *this;
                ++*this;
                return old;
            }
            HWSHQTB_CONSTEXPR20 splay_iterator operator--(int)const {
                splay_iterator old = *this;
                --*this;
                return old;
            }

            HWSHQTB_CONSTEXPR20 bool operator==(const splay_iterator& other)const noexcept {
                return _end == other._end && _node == other._node;
            }
            HWSHQTB_CONSTEXPR20 bool operator!=(const splay_iterator& other)const noexcept {
                return _end != other._end || _node != other._node;
            }

        private:
            splay_node<Value, Allocator>* _node;
            bool _end;

        };

        template <typename Value, class Allocator>
        class splay_const_iterator {
            template <typename, typename, typename, bool>
            friend class splay;

        private:
            HWSHQTB_CONSTEXPR20 splay_const_iterator(splay_node<Value, Allocator>* node, bool end):
                _node(node), _end(end) {};

        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = const Value;
            using difference_type = std::ptrdiff_t;
            using pointer = const Value*;
            using reference = const Value&;

            HWSHQTB_CONSTEXPR20 splay_const_iterator(splay_iterator<Value, Allocator> iter):
                _node(iter._node), _end(iter._end) {};

            HWSHQTB_CONSTEXPR20 reference operator*()const {
                return *_node->value;
            }
            HWSHQTB_CONSTEXPR20 pointer operator->()const {
                return _node->value;
            }

            HWSHQTB_CONSTEXPR20 splay_const_iterator& operator++() {
                if (_node == nullptr) return *this;
                if (_end) return *this;
                if (_node->greater == nullptr) {
                    _end = true;
                    return *this;
                }
                _node = _node->greater;
                return *this;
            }
            HWSHQTB_CONSTEXPR20 splay_const_iterator& operator--() {
                if (_node == nullptr) return *this;
                if (_node->less == nullptr) return *this;
                if (_end) {
                    _end = false;
                    return *this;
                }
                _node = _node->less;
                return *this;
            }
            HWSHQTB_CONSTEXPR20 splay_const_iterator operator++(int)const {
                splay_iterator old = *this;
                ++*this;
                return old;
            }
            HWSHQTB_CONSTEXPR20 splay_const_iterator operator--(int)const {
                splay_iterator old = *this;
                --*this;
                return old;
            }

            HWSHQTB_CONSTEXPR20 bool operator==(const splay_const_iterator& other)const noexcept {
                return _end == other._end && _node == other._node;
            }
            HWSHQTB_CONSTEXPR20 bool operator!=(const splay_const_iterator& other)const noexcept {
                return _end != other._end || _node != other._node;
            }

        private:
            splay_node<Value, Allocator>* _node;
            bool _end;

        };

        template <typename Value, class Compare, class Allocator, bool Multiple>
        class splay {
            using tree_node = splay_node<Value, Allocator>;

        public:
            using key_type = typename splay_trait<Value>::key_type;
            using mapped_type = typename splay_trait<Value>::mapped_type;
            using value_type = typename splay_trait<Value>::value_type;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            using value_compare = Compare;
            using allocator_type = Allocator;

        private:
            using allocator_trait = std::allocator_traits<allocator_type>;
            using tree_node_allocator_type = typename allocator_trait::template rebind_alloc<tree_node>;
            using tree_node_allocator_trait = typename allocator_trait::template rebind_traits<tree_node>;

        public:
            using reference = value_type&;
            using const_reference = const value_type&;
            using pointer = value_type*;
            using const_pointer = const value_type*;
            using iterator = std::conditional_t<splay_trait<Value>::is_set, splay_const_iterator<Value, Allocator>, splay_iterator<Value, Allocator>>;
            using const_iterator = splay_const_iterator<Value, Allocator>;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;
            using insert_return_type = std::pair<iterator, bool>;

        public:
            constexpr splay(const Compare& compare = Compare(), const Allocator& allocator = Allocator())noexcept(std::is_nothrow_default_constructible_v<Compare>&& std::is_nothrow_default_constructible_v<Allocator>):
                _root(nullptr), _begin(nullptr), _end(nullptr), _compare(compare), _allocator(allocator), _tree_node_allocator(allocator) {}
            /*constexpr splay(std::initializer_list<value_type> vs, const Compare& compare = Compare(), const Allocator& allocator = Allocator()) :
                _root(nullptr), _compare(compare), _allocator(allocator), _tree_node_allocator(allocator) {
                tree_node* head = _root;
                for (const auto& v : vs)
                    head = construct_node(head, v, false);
                _root->left = build_from_double_link_line(_root->right, vs.size());
                _root->left->parent = _root;
                _root->right = nullptr;
                _root->size = vs.size() + 1;
                _root->update_height();
            }*/
            ~splay() {
                clear();
            }

            constexpr iterator begin() {
                return {_begin, empty()};
            }
            constexpr const_iterator begin()const {
                return {_begin, empty()};
            }
            constexpr const_iterator cbegin()const {
                return {_begin, empty()};
            }
            constexpr iterator end() {
                return {_end, true};
            }
            constexpr const_iterator end()const {
                return {_end, true};
            }
            constexpr const_iterator cend()const {
                return {_end, true};
            }
            constexpr reverse_iterator rbegin() {
                return std::make_reverse_iterator(end());
            }
            constexpr const_reverse_iterator rbegin()const {
                return std::make_reverse_iterator(end());
            }
            constexpr const_reverse_iterator crbegin()const {
                return std::make_reverse_iterator(cend());
            }
            constexpr reverse_iterator rend() {
                return std::make_reverse_iterator(begin());
            }
            constexpr const_reverse_iterator rend()const {
                return std::make_reverse_iterator(begin());
            }
            constexpr const_reverse_iterator crend()const {
                return std::make_reverse_iterator(cbegin());
            }

            constexpr bool empty()const noexcept {
                return _root == nullptr;
            }
            size_type size()const noexcept {
                if (_root == nullptr) return 0;
                return _root->size;
            }
            size_type max_size()const noexcept {
                return std::numeric_limits<size_type>::max();
            }

            void clear() {
                for (tree_node* node = _begin; node != _end;) {
                    node = node->greater;
                    destory_node(node->less);
                }
                if (_end) destory_node(_end);
            }
            insert_return_type insert(const value_type& value) {
                if (_root == nullptr) return {iterator(_begin = _end = _root = construct_node(nullptr, true, value), false), true};
                const auto fn = [this, &value](const value_type& v) -> bool {
                    return _compare(v, value);
                };
                auto _ = lower_bound_fn(fn);
                auto pos = _.first;
                auto rank = _.second;

                tree_node* result;
                if (rank == _root->size) result = construct_node(_end, false, value);
                else if (!Multiple && !_compare(*pos->value, value) && !_compare(value, *pos->value)) return {iterator{pos, false}, false};
                else if (rank == 0) result = construct_node(_begin, true, value);
                else if (pos->left == nullptr) result = construct_node(pos, true, value);
                else result = construct_node(pos->less, false, value);
                balance(result);
                return {{result, false}, true};
            }
            iterator operator[](size_type index) {
                tree_node* node = query_by_rank(_root, index);
                if (node) {
                    balance(node);
                    return iterator(node, false);
                }
                return end();
            }
            /*insert_return_type insert(value_type&& v) {
                if (_root->left_size() == 0) return {construct_node(_root, true, std::move(v)), true};
                const auto fn = [this, &v](const value_type& value) -> bool {
                    return _compare(value, v);
                };
                auto [pos, rank] = lower_bound(fn);

                tree_node* result;
                if (rank == _root->size - 1) result = construct_node(_root->less, false, std::move(v));
                else if (!Multiple && !_compare(pos->value, v) && !_compare(v, pos->value)) return {pos, false};
                else if (rank == 0) result = construct_node(pos, true, std::move(v));
                else if (pos->left == nullptr) result = construct_node(pos, true, std::move(v));
                else result = construct_node(pos->less, false, std::move(v));
                balance(result->parent);
                _root->update_size();
                _root->update_height();
                return {result, true};
            }*/
            /*template <class InputIt>
            std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, insert_return_type> insert_range(InputIt first, InputIt last) {

            }*/
            /*template <typename ...Ts>
            insert_return_type emplace(Ts&& ...values) {
                tree_node* node = construct_node(_root, true, std::forward<Ts>(values)...);
                if (_root->left_size() == 0) return {iterator(node), true};
                const auto fn = [this, data](const key_type& key) -> bool {
                    return _compare(key, value);
                };
                auto [pos, rank] = lower_bound(fn);

                tree_node* result;
                if (rank == _root->size - 1) result = construct_node(_root->less, false, data);
                else if (!Is_Multiple && !_compare(pos->value, value) && !_compare(value, pos->value)) {
                    destory_data(data);
                    return {pos, false};
                }
                else if (rank == 0) result = construct_node(pos, true, data);
                else if (pos->left == nullptr) result = construct_node(pos, true, data);
                else result = construct_node(pos->less, false, data);
                balance(result->parent);
                _root->update_size();
                _root->update_height();
                return {result, true};
            }*/
            iterator erase(const_iterator pos) {
                if (pos == end()) return end();
                tree_node* node = pos._node;
                balance(node);
                remove_node(node);
                iterator result(node, false);
                ++result;
                half_merge(node->left, _begin, node->less, node->right, node->greater, _end);
                _root = node->left;
                destory_node(node);
                return result;
            }
            /*iterator erase_range(const_iterator first, const_iterator last) {

            }*/

            iterator lower_bound(const value_type& value) {
                const auto fn = [this, &value](const value_type& v) -> bool {
                    return _compare(v, value);
                };
                auto result = lower_bound_fn(fn);
                if (result.first) {
                    balance(result.first);
                    return iterator(result.first, false);
                }
                return end();
            }
            iterator upper_bound(const value_type& value) {
                const auto fn = [this, &value](const value_type& v) -> bool {
                    return !_compare(value, v);
                };
                auto result = lower_bound_fn(fn);
                if (result.first) {
                    balance(result.first);
                    return iterator(result.first, false);
                }
                return end();
            }

            iterator find(const value_type& v)const {
                const auto fn = [this, &v](const key_type& key) -> bool {
                    return _compare(key, v);
                };
                auto result = lower_bound_fn(fn);
                if (result.first && !_compare(v, *result.first->value)) {
                    balance(result.first);
                    return iterator(result.first, false);
                }
                return end();
            }
            difference_type rank(const value_type& value)const {
                const auto fn = [this, &value](const value_type& v) -> bool {
                    return _compare(v, value);
                };
                auto result = lower_bound_fn(fn);
                if (result.first) balance(result.first);
                return result.second;
            }

            void half_merge(splay& source) {
                if (_root == nullptr) {
                    _root = exchange(source._root, nullptr);
                    _begin = exchange(source._begin, nullptr);
                    _end = exchange(source._end, nullptr);
                }
                if (!source._root) return;
                if (_compare(*source._end->value, *_end->value)) {
                    std::swap(_root, source._root);
                    std::swap(_begin, source._begin);
                    std::swap(_end, source._end);
                }
                half_merge(_root, _begin, _end, exchange(source._root, nullptr), exchange(source._begin, nullptr), exchange(source._end, nullptr));
            }

            splay split(const_iterator pos) {
                if (pos == end()) return {};
                tree_node* node = pos._node;
                balance(node);
                splay result(_compare, _allocator);
                result._begin = result._root = node;
                result._end = _end;
                _end = node->less;
                if (_root = node->left) {
                    _root->parent = nullptr;
                    _end->greater = nullptr;
                }
                node->left = nullptr;
                node->less = nullptr;
                return result;
            }

        private:
            tree_node* construct_default_node() {
                tree_node* node = tree_node_allocator_trait::allocate(_tree_node_allocator, 1);
                tree_node_allocator_trait::construct(_tree_node_allocator, node);
                return node;
            }
            template <typename ...Ts>
            tree_node* construct_node(tree_node* parent, bool isleft, Ts&& ...values) {
                tree_node* node = tree_node_allocator_trait::allocate(_tree_node_allocator, 1);
                tree_node_allocator_trait::construct(_tree_node_allocator, node, _allocator, std::forward<Ts>(values)...);
                node->parent = parent;
                if (parent) {
                    if (isleft) {
                        parent->left = node;
                        if (parent->less) parent->less->greater = node;
                        node->less = parent->less;
                        node->greater = parent;
                        parent->less = node;
                        if (parent == _begin) _begin = node;
                    }
                    else {
                        parent->right = node;
                        if (parent->greater) parent->greater->less = node;
                        node->greater = parent->greater;
                        node->less = parent;
                        parent->greater = node;
                        if (parent == _end) _end = node;
                    }
                    parent->update_size();
                }
                return node;
            }
            void destory_default_node(tree_node* node) {
                tree_node_allocator_trait::destroy(_tree_node_allocator, node);
                tree_node_allocator_trait::deallocate(_tree_node_allocator, node, 1);
            }
            void destory_node(tree_node* node) {
                allocator_trait::destroy(_allocator, node->value);
                allocator_trait::deallocate(_allocator, node->value, 1);
                destory_default_node(node);
            }
            void remove_node(tree_node* node) {
                if (node->parent)
                    (node->parent->left == node ? node->parent->left : node->parent->right) = nullptr;
                if (node->greater)
                    node->greater->less = node->less;
                if (node->less)
                    node->less->greater = node->greater;
                if (node == _begin)
                    _begin = _begin->greater;
                if (node == _end)
                    _end = _end->less;
                if (!_begin || !_end) _root = nullptr;
            }
            /*void destory_remove_node(tree_node* node) {
                remove_node(node);
                destory_node(node);
            }*/

            tree_node* build_from_double_link_line(tree_node*& head, size_type size) {
                if (size <= 0) return nullptr;
                tree_node* left = build_from_double_link_line(head, size / 2);
                tree_node* root = head;
                head = head->greater;
                root->left = left;
                root->right = build_from_double_link_line(head, size - size / 2 - 1);
                if (left != nullptr) left->parent = root;
                if (root->right != nullptr) root->right->parent = root;
                root->update_height();
                root->size = size;
                return root;
            }

            tree_node* extract(tree_node* node) {
                if (node->left == nullptr && node->right == nullptr) {
                    remove_node(node);
                    if (node->parent) {
                        node->parent->update_height();
                        node->parent->update_size();
                    }
                    return node;
                }
                if (node->left == nullptr) {
                    std::swap(node->value, node->right->value);
                    return extract(node->right);
                }
                if (node->right == nullptr) {
                    std::swap(node->value, node->left->value);
                    return extract(node->left);
                }
                if (node->left_height() > node->right_height()) {
                    std::swap(node->value, node->less->value);
                    return extract(node->less);
                }
                std::swap(node->value, node->greater->value);
                return extract(node->greater);
            }

            template <class Fn>
            std::pair<tree_node*, difference_type> lower_bound_fn(Fn&& fn)const {
                tree_node* now = _begin;
                tree_node* half = _root;
                difference_type rank = 0;
                size_type count = _root->size, step;
                while (count > 0) {
                    step = half->left_size();
                    if (fn(*half->value)) {
                        now = half->greater;
                        half = half->right;
                        count -= step + 1;
                        rank += step + 1;
                    }
                    else {
                        half = half->left;
                        count = step;
                    }
                }
                return {now, rank};
            }

            tree_node* query_by_rank(tree_node* node, size_type r) {
                if (node == nullptr || node->size <= r) return nullptr;
                if (node->left_size() == r) return node;
                if (node->left_size() > r) return query_by_rank(node->left, r);
                return query_by_rank(node->right, r - node->left_size() - 1);
            }

            void rotate(tree_node* son) {
                tree_node* node = son->parent; 
                tree_node* parent = node->parent;
                bool is_left = son->is_left_son();
                if (node->son(is_left) = son->son(!is_left)) son->son(!is_left)->parent = node;
                son->son(!is_left) = node;
                son->parent = parent;
                if (parent) parent->son(node->is_left_son()) = node;
                node->parent = son;
                node->update_size();
                son->update_size();
            }
            constexpr void balance(tree_node* now) {
                for (tree_node* parent = now->parent; parent = now->parent; rotate(now)) {
                    if (parent->parent) {
                        rotate(now->is_left_son() == parent->is_left_son() ? parent : now);
                    }
                }
                _root = now;
            }

            void half_merge(tree_node*& r1, tree_node*& begin1, tree_node*& end1, tree_node* r2, tree_node* begin2, tree_node* end2) {
                if (r1) {
                    r1 = r2;
                    begin1 = begin2;
                }
                else {
                    balance(end1);
                    end1->right = r2;
                    r2->parent = end1->right;
                    begin2->less = end1;
                    end1->greater = begin2;
                    r1 = end1;
                }
                end1 = end2;
            }

            tree_node* _root;
            tree_node* _begin;
            tree_node* _end;
            value_compare _compare;
            allocator_type _allocator;
            tree_node_allocator_type _tree_node_allocator;

        };
    }
}

#endif