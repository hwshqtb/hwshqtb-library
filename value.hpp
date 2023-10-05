#ifndef _HWSHQTB_VALUE_HPP
#define _HWSHQTB_VALUE_HPP

// 2023-10-2 done c++17

#include <type_traits>
#include <charconv>
#include <string>
#include <string_view>
#include <limits>

namespace hwshqtb {
    class value {
    public:
        template<std::size_t N>
        value(const char(&v)[N]):
            _value(v, v + N), _error_code() {}
        value(const std::string& v):
            _value(v), _error_code() {}
        value(std::string&& v):
            _value(std::move(v)), _error_code() {}
        template<typename T, typename = std::enable_if_t<std::numeric_limits<T>::is_specialized>>
        value(T&& v):
            _error_code() {
            to_chars(v);
        }

        value& operator=(const value& other) {
            _value.assign(other._value);
            _error_code = {};
            return *this;
        }
        value& operator=(value&& other) {
            _value.assign(std::move(other._value));
            _error_code = {};
            return *this;
        }
        template<std::size_t N>
        value& operator=(const char(&v)[N]) {
            _value.assign(v, v + N);
            _error_code = {};
            return *this;
        }
        value& operator=(const std::string& v) {
            _value.assign(v);
            _error_code = {};
            return *this;
        }
        value& operator=(std::string&& v) {
            _value.assign(std::move(v));
            _error_code = {};
            return *this;
        }
        template<typename T, typename = std::enable_if_t<std::numeric_limits<T>::is_specialized>>
        value& operator=(T&& v) {
            to_chars(std::forward<T>(v));
            return *this;
        }

        explicit operator std::string()const noexcept {
            return _value;
        }
        explicit operator std::string_view()const noexcept {
            return _value;
        }
        template<typename T, typename = std::enable_if_t<std::numeric_limits<T>::is_specialized>>
        explicit operator T()const {
            return from_chars<T>();
        }

        std::errc errc()const noexcept {
            return _error_code;
        }
        void clear_errc()const noexcept {
            _error_code = {};
        }

    private:
        template <typename T>
        void to_chars(T&& v) {
            using NUMBER = std::decay_t<T>;
            if constexpr (::std::is_same_v<NUMBER, bool>) {
                _error_code = {};
                _value.assign(v ? "true" : "false");
            }
            else if constexpr (::std::is_arithmetic_v<NUMBER>) {
                constexpr auto size = std::numeric_limits<NUMBER>::digits + std::numeric_limits<NUMBER>::is_signed - !std::numeric_limits<NUMBER>::is_integer;
                char memory[size] = {};
                const auto& [end, ec] = std::to_chars(memory, memory + size, v);
                _error_code = ec;
                _value.assign(memory, end);
            }
            else {
                _error_code = {};
                _value.assign((std::string)v);
            }
        }
        template <typename T, typename NUMBER = std::decay_t<T>>
        NUMBER from_chars()const {
            if constexpr (::std::is_same_v<NUMBER, bool>) {
                _error_code = {};
                if (_value == "true") return true;
                else {
                    if (_value != "false") _error_code = std::errc::result_out_of_range;
                    return false;
                }
            }
            else if constexpr (::std::is_arithmetic_v<NUMBER>) {
                NUMBER ret{};
                const auto& [end, ec] = std::from_chars(_value.c_str(), _value.c_str() + _value.size() + 1, ret);
                _error_code = ec;
                return ret;
            }
            else {
                _error_code = {};
                return (NUMBER)_value;
            }
        }

        std::string _value;
        mutable std::errc _error_code;
    };
}
#endif