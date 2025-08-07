#ifndef HWSHQTB__CONTAINER__ORDERED_MAP_HPP
#define HWSHQTB__CONTAINER__ORDERED_MAP_HPP

/*
*   2025-07-07
*       first untested version
* 
*   2025-08-07
*       bug fix
*/

// #include "../version.hpp"
#include <stdexcept>
#include <list>
#include <map>

namespace hwshqtb {
    namespace container {
        template <typename Key, typename T,
            class Container = std::list<std::pair<const Key, T>>,
            class Map = std::map<Key, typename Container::iterator>>
            class ordered_map {
            public:
                using key_type = Key;
                using mapped_type = T;
                using value_type = std::pair<const Key, T>;
                using size_type = typename Container::size_type;
                using difference_type = typename Container::difference_type;
                using allocator_type = typename Container::allocator_type;
                using reference = typename Container::reference;
                using const_reference = typename Container::const_reference;
                using pointer = typename Container::pointer;
                using const_pointer = typename Container::const_pointer;
                using iterator = typename Container::iterator;
                using const_iterator = typename Container::const_iterator;
                using reverse_iterator = typename Container::reverse_iterator;
                using const_reverse_iterator = typename Container::const_reverse_iterator;
                using container_type = Container;

                ordered_map() = default;
                ordered_map(const ordered_map& other): _container(other._container), _map(other._map) {
                    for (auto iter = _container.begin(); iter != _container.end(); ++iter)
                        _map[iter->first] = iter;
                }
                ordered_map(ordered_map&& other) noexcept: _container(std::move(other._container)), _map(std::move(other._map)) {}
                explicit ordered_map(const container_type& container): _container(container) {
                    for (auto it = _container.begin(); it != _container.end(); ++it) {
                        _map[it->first] = it;
                    }
                }
                explicit ordered_map(container_type&& container) noexcept: _container(std::move(container)) {
                    for (auto it = _container.begin(); it != _container.end(); ++it) {
                        _map[it->first] = it;
                    }
                }
                template <typename InputIt>
                ordered_map(InputIt first, InputIt last, const allocator_type& allocator = allocator_type())
                    : _container(allocator) {
                    for (auto it = first; it != last; ++it) {
                        insert(*it);
                    }
                }
                ordered_map(std::initializer_list<value_type> ilist)
                    : _container(ilist) {
                    for (const auto& value : ilist) {
                        _map[value.first] = --_container.end();
                    }
                }
                template <class Alloc>
                explicit ordered_map(const Alloc& allocator): _container(allocator), _map(allocator) {}
                template <class Alloc>
                ordered_map(const ordered_map& other, const Alloc& allocator)
                    : _container(other._container, allocator), _map(other._map, allocator) {
                    for (auto it = _container.begin(); it != _container.end(); ++it) {
                        _map[it->first] = it;
                    }
                }
                template <class Alloc>
                ordered_map(ordered_map&& other, const Alloc& allocator) noexcept
                    : _container(std::move(other._container), allocator), _map(std::move(other._map), allocator) {
                    for (auto it = _container.begin(); it != _container.end(); ++it) {
                        _map[it->first] = it;
                    }
                }
                template <class Alloc>
                ordered_map(const container_type& container, const Alloc& allocator) : _container(container, allocator), _map(allocator) {
                    for (auto it = _container.begin(); it != _container.end(); ++it) {
                        _map[it->first] = it;
                    }
                }
                template <class Alloc>
                ordered_map(container_type&& container, const Alloc& allocator) noexcept
                    : _container(std::move(container), allocator), _map(allocator) {
                    for (auto it = _container.begin(); it != _container.end(); ++it) {
                        _map[it->first] = it;
                    }
                }
                template <typename InputIt, class Alloc>
                ordered_map(InputIt first, InputIt last, const Alloc& allocator)
                    : _container(allocator) {
                    for (auto it = first; it != last; ++it) {
                        insert(*it);
                    }
                }
                template <class Alloc>
                ordered_map(std::initializer_list<value_type> ilist, const Alloc& allocator)
                    : _container(ilist, allocator), _map(allocator) {
                    for (const auto& value : ilist) {
                        _map[value.first] = --_container.end();
                    }
                }

                ~ordered_map() = default;

                ordered_map& operator=(const ordered_map& other) {
                    if (this != &other) {
                        _container = other._container;
                        _map = other._map;
                        for (auto iter = _container.begin(); iter != _container.end(); ++iter)
                            _map[iter->first] = iter;
                    }
                    return *this;
                }
                ordered_map& operator=(ordered_map&& other) noexcept {
                    if (this != &other) {
                        _container = std::move(other._container);
                        _map = std::move(other._map);
                    }
                    return *this;
                }
                ordered_map& operator=(std::initializer_list<value_type> ilist) {
                    clear();
                    insert(ilist);
                    return *this;
                }

                allocator_type get_allocator() const noexcept {
                    return _container.get_allocator();
                }

                mapped_type& operator[](const key_type& key) {
                    auto it = _map.find(key);
                    if (it == _map.end()) {
                        _container.emplace_back(key, mapped_type{});
                        _map[key] = --_container.end();
                        return (--_container.end())->second;
                    }
                    return it->second->second;
                }

                const mapped_type& operator[](const key_type& key)const {
                    auto it = _map.find(key);
                    return it->second->second;
                }

                mapped_type& at(const key_type& key) {
                    auto it = _map.find(key);
                    if (it == _map.end()) {
                        throw std::out_of_range("Key not found");
                    }
                    return it->second->second;
                }
                const mapped_type& at(const key_type& key) const {
                    auto it = _map.find(key);
                    if (it == _map.end()) {
                        throw std::out_of_range("Key not found");
                    }
                    return it->second->second;
                }

                bool empty() const noexcept {
                    return _container.empty();
                }

                size_type size() const noexcept {
                    return _container.size();
                }

                size_type max_size() const noexcept {
                    return _container.max_size();
                }

                void clear() {
                    _container.clear();
                    _map.clear();
                }

                void insert(const value_type& value) {
                    auto it = _map.find(value.first);
                    if (it == _map.end()) {
                        _container.push_back(value);
                        _map[_container.back().first] = --_container.end();
                    }
                }
                void insert(value_type&& value) {
                    auto it = _map.find(value.first);
                    if (it == _map.end()) {
                        _container.push_back(std::move(value));
                        _map[_container.back().first] = --_container.end();
                    }
                }
                template <typename P>
                typename std::enable_if<std::is_constructible<value_type, P>::value&& std::is_same<typename std::decay<P>::type, value_type>::value>::type insert(P&& value) {
                    auto it = _map.find(value.first);
                    if (it == _map.end()) {
                        _container.push_back(std::forward<P>(value));
                        _map[_container.back().first] = --_container.end();
                    }
                }
                iterator insert(const_iterator hint, const value_type& value) {
                    auto it = _map.find(value.first);
                    if (it == _map.end()) {
                        _container.insert(hint, value);
                        auto new_it = --_container.end();
                        _map[_container.back().first] = new_it;
                        return new_it;
                    }
                    return it->second;
                }
                iterator insert(const_iterator hint, value_type&& value) {
                    auto it = _map.find(value.first);
                    if (it == _map.end()) {
                        _container.insert(hint, std::move(value));
                        auto new_it = --_container.end();
                        _map[_container.back().first] = new_it;
                        return new_it;
                    }
                    return it->second;
                }
                template <typename P>
                typename std::enable_if<std::is_constructible<value_type, P>::value&& std::is_same<typename std::decay<P>::type, value_type>::value, iterator>::type insert(const_iterator hint, P&& value) {
                    auto it = _map.find(value.first);
                    if (it == _map.end()) {
                        _container.insert(hint, std::forward<P>(value));
                        auto new_it = --_container.end();
                        _map[_container.back().first] = new_it;
                        return new_it;
                    }
                    return it->second;
                }
                template <typename InputIt>
                void insert(InputIt first, InputIt last) {
                    for (auto it = first; it != last; ++it) {
                        insert(*it);
                    }
                }
                void insert(std::initializer_list<value_type> ilist) {
                    for (const auto& value : ilist) {
                        insert(value);
                    }
                }

                template <typename M>
                std::pair<iterator, bool> insert_or_assign(const key_type& key, M&& value) {
                    auto it = _map.find(key);
                    if (it == _map.end()) {
                        _container.emplace_back(key, std::forward<M>(value));
                        auto new_it = --_container.end();
                        _map[key] = new_it;
                        return {new_it, true};
                    }
                    else {
                        it->second->second = std::forward<M>(value);
                        return {it->second, false};
                    }
                }
                template <typename M>
                std::pair<iterator, bool> insert_or_assign(key_type&& key, M&& value) {
                    auto it = _map.find(key);
                    if (it == _map.end()) {
                        _container.emplace_back(std::move(key), std::forward<M>(value));
                        auto new_it = --_container.end();
                        _map[new_it->first] = new_it;
                        return {new_it, true};
                    }
                    else {
                        it->second->second = std::forward<M>(value);
                        return {it->second, false};
                    }
                }
                template <typename M>
                iterator insert_or_assign(const_iterator hint, const key_type& key, M&& value) {
                    auto it = _map.find(key);
                    if (it == _map.end()) {
                        _container.insert(hint, std::make_pair(key, std::forward<M>(value)));
                        auto new_it = --_container.end();
                        _map[key] = new_it;
                        return new_it;
                    }
                    else {
                        it->second->second = std::forward<M>(value);
                        return it->second;
                    }
                }
                template <typename M>
                iterator insert_or_assign(const_iterator hint, key_type&& key, M&& value) {
                    auto it = _map.find(key);
                    if (it == _map.end()) {
                        _container.insert(hint, std::make_pair(std::move(key), std::forward<M>(value)));
                        auto new_it = --_container.end();
                        _map[new_it->first] = new_it;
                        return new_it;
                    }
                    else {
                        it->second->second = std::forward<M>(value);
                        return it->second;
                    }
                }

                template <typename... Args>
                std::pair<iterator, bool> emplace(Args&&... args) {
                    auto it = _map.find(key_type(std::forward<Args>(args)...));
                    if (it == _map.end()) {
                        _container.emplace_back(std::forward<Args>(args)...);
                        auto new_it = --_container.end();
                        _map[new_it->first] = new_it;
                        return {new_it, true};
                    }
                    return {it->second, false};
                }
                template <typename... Args>
                iterator emplace_hint(const_iterator hint, Args&&... args) {
                    auto it = _map.find(key_type(std::forward<Args>(args)...));
                    if (it == _map.end()) {
                        _container.emplace(hint, std::forward<Args>(args)...);
                        auto new_it = --_container.end();
                        _map[new_it->first] = new_it;
                        return new_it;
                    }
                    return it->second;
                }

                template <typename... Args>
                std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args) {
                    auto it = _map.find(key);
                    if (it == _map.end()) {
                        _container.emplace_back(key, std::forward<Args>(args)...);
                        auto new_it = --_container.end();
                        _map[key] = new_it;
                        return {new_it, true};
                    }
                    return {it->second, false};
                }
                template <typename... Args>
                std::pair<iterator, bool> try_emplace(key_type&& key, Args&&... args) {
                    auto it = _map.find(key);
                    if (it == _map.end()) {
                        _container.emplace_back(std::move(key), std::forward<Args>(args)...);
                        auto new_it = --_container.end();
                        _map[new_it->first] = new_it;
                        return {new_it, true};
                    }
                    return {it->second, false};
                }
                template <typename... Args>
                iterator try_emplace(const_iterator hint, const key_type& key, Args&&... args) {
                    auto it = _map.find(key);
                    if (it == _map.end()) {
                        _container.emplace(hint, key, std::forward<Args>(args)...);
                        auto new_it = --_container.end();
                        _map[key] = new_it;
                        return new_it;
                    }
                    return it->second;
                }
                template <typename... Args>
                iterator try_emplace(const_iterator hint, key_type&& key, Args&&... args) {
                    auto it = _map.find(key);
                    if (it == _map.end()) {
                        _container.emplace(hint, std::move(key), std::forward<Args>(args)...);
                        auto new_it = --_container.end();
                        _map[new_it->first] = new_it;
                        return new_it;
                    }
                    return it->second;
                }

                void erase(iterator pos) {
                    if (pos != _container.end()) {
                        _map.erase(pos->first);
                        _container.erase(pos);
                    }
                }
                void erase(const_iterator pos) {
                    if (pos != _container.end()) {
                        _map.erase(pos->first);
                        _container.erase(pos);
                    }
                }
                void erase(const_iterator first, const_iterator last) {
                    for (auto it = first; it != last; ++it) {
                        _map.erase(it->first);
                    }
                    _container.erase(first, last);
                }
                void erase(const key_type& key) {
                    auto it = _map.find(key);
                    if (it != _map.end()) {
                        _container.erase(it->second);
                        _map.erase(it);
                    }
                }
                template <typename K>
                void erase(K&& key) {
                    auto it = _map.find(std::forward<K>(key));
                    if (it != _map.end()) {
                        _container.erase(it->second);
                        _map.erase(it);
                    }
                }

                void swap(ordered_map& other) noexcept {
                    std::swap(_container, other._container);
                    std::swap(_map, other._map);
                }

                iterator begin() noexcept {
                    return _container.begin();
                }
                const_iterator begin() const noexcept {
                    return _container.begin();
                }
                const_iterator cbegin() const noexcept {
                    return _container.cbegin();
                }

                iterator end() noexcept {
                    return _container.end();
                }
                const_iterator end() const noexcept {
                    return _container.end();
                }
                const_iterator cend() const noexcept {
                    return _container.cend();
                }

                reverse_iterator rbegin() noexcept {
                    return _container.rbegin();
                }
                const_reverse_iterator rbegin() const noexcept {
                    return _container.rbegin();
                }
                const_reverse_iterator crbegin() const noexcept {
                    return _container.crbegin();
                }
                reverse_iterator rend() noexcept {
                    return _container.rend();
                }
                const_reverse_iterator rend() const noexcept {
                    return _container.rend();
                }
                const_reverse_iterator crend() const noexcept {
                    return _container.crend();
                }

                size_type count(const key_type& key) const {
                    return _map.count(key);
                }
                template <typename K>
                size_type count(const K& key) const {
                    return _map.count(key);
                }

                iterator find(const key_type& key) {
                    auto it = _map.find(key);
                    if (it != _map.end()) {
                        return it->second;
                    }
                    return _container.end();
                }
                const_iterator find(const key_type& key) const {
                    auto it = _map.find(key);
                    if (it != _map.end()) {
                        return it->second;
                    }
                    return _container.end();
                }
                template <typename K>
                iterator find(const K& key) {
                    auto it = _map.find(key);
                    if (it != _map.end()) {
                        return it->second;
                    }
                    return _container.end();
                }
                template <typename K>
                const_iterator find(const K& key) const {
                    auto it = _map.find(key);
                    if (it != _map.end()) {
                        return it->second;
                    }
                    return _container.end();
                }

                std::pair<iterator, iterator> equal_range(const key_type& key) {
                    auto it = _map.find(key);
                    if (it != _map.end()) {
                        return {it->second, it->second + 1};
                    }
                    return {_container.end(), _container.end()};
                }
                std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const {
                    auto it = _map.find(key);
                    if (it != _map.end()) {
                        return {it->second, it->second + 1};
                    }
                    return {_container.end(), _container.end()};
                }
                template <typename K>
                std::pair<iterator, iterator> equal_range(const K& key) {
                    auto it = _map.find(key);
                    if (it != _map.end()) {
                        return {it->second, it->second + 1};
                    }
                    return {_container.end(), _container.end()};
                }
                template <typename K>
                std::pair<const_iterator, const_iterator> equal_range(const K& key) const {
                    auto it = _map.find(key);
                    if (it != _map.end()) {
                        return {it->second, it->second + 1};
                    }
                    return {_container.end(), _container.end()};
                }

            private:
                Container _container;
                Map _map;

        };

        template <typename Key, typename T, class Container, class Map>
        bool operator==(const ordered_map<Key, T, Container, Map>& lhs, const ordered_map<Key, T, Container, Map>& rhs) {
            return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
        }

        template <typename Key, typename T, class Container, class Map>
        bool operator!=(const ordered_map<Key, T, Container, Map>& lhs, const ordered_map<Key, T, Container, Map>& rhs) {
            return !(lhs == rhs);
        }
    }
}

#endif