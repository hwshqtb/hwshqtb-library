#ifndef _HWSHQTB_MATRIX_HPP
#define	_HWSHQTB_MATRIX_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace hwshqtb {
    template <typename T>
    class matrix {
    public:
        struct position {
        public:
            std::size_t _p1;
            std::size_t _p2;
        };
        class line {
            friend class matrix;
        public:
            T& operator[](std::size_t pos) {
#ifndef NDEGBU
                if (!pos || pos > _length) throw std::out_of_range("");
#endif
                return _line[(pos - 1) * _step];
            }
            const T& operator[](std::size_t pos)const {
#ifndef NDEGBU
                if (!pos || pos > _length) throw std::out_of_range("");
#endif
                return _line[(pos - 1) * _step];
            }
            T& at(std::size_t pos) {
                if (!pos || pos > _length) throw std::out_of_range("");
                return _line[(pos - 1) * _step];
            }
            const T& at(std::size_t pos)const {
                if (!pos || pos > _length) throw std::out_of_range("");
                return _line[(pos - 1) * _step];
            }

        private:
            T* _line;
            std::size_t _step;
            std::size_t _length;
        };

    public:
        matrix():
            _row(0), _col(0), _matrix(nullptr) {}
        matrix(std::size_t row, std::size_t col):
            _row(row), _col(col), _matrix(new T[row * col]()) {}
        matrix(std::size_t row, std::size_t col, const T& val):
            _row(row), _col(col), _matrix(new T[row * col]) {
            for (std::size_t i = 0; i < _row * _col; ++i)
                _matrix[i] = val;
        }
        matrix(std::size_t diag, const T& val):
            _row(diag), _col(diag), _matrix(new T[diag * diag]()) {
            for (std::size_t i = 0; i < diag * diag; i += diag + 1)
                _matrix[i] = val;
        }
        matrix(const matrix& other):
            _row(other._row), _col(other._col), _matrix(new T[other._row * other.col]) {
            for (std::size_t i = 0; i < _row * _col; ++i)
                _matrix[i] = other._matrix[i];
        }
        matrix(matrix&& other)noexcept:
            _row(std::exchange(other._row, 0)), _col(std::exchange(other._col, 0)), _matrix(std::exchange(other._matrix, nullptr)) {}
        ~matrix() {
            _row = 0;
            _col = 0;
            if (_matrix != nullptr) {
                delete[] _matrix;
                _matrix = nullptr;
            }
        }

        matrix& operator=(const matrix& other) {
#ifndef NDEBUG
            if (this == &other) return *this;
#endif
            delete[] _matrix;
            _row = other._row;
            _col = other._col;
            _matrix = new T[_row * _col];
            for (std::size_t i = 0; i < _row * _col; ++i)
                _matrix[i] = other._matrix[i];
            return *this;
        }
        matrix& operator=(matrix&& other) {
#ifndef NDEBUG
            if (this == &other) return *this;
#endif
            _row = std::exchange(other._row, 0);
            _col = std::exchange(other._col, 0);
            if (_matrix != nullptr) delete[] _matrix;
            _matrix = std::exchange(other._matrix, nullptr);
            return *this;
        }

        T& operator[](const position& pos) {
#ifndef NDEBUG
            if (pos._p1 > _row || !pos._p1 || pos._p2 > _col || !pos._p2) throw std::out_of_range("");
#endif
            return _matrix[(pos._p1 - 1) * _col + pos._p2 - 1];
        }
        const T& operator[](const position& pos)const {
#ifndef NDEBUG
            if (pos._p1 > _row || !pos._p1 || pos._p2 > _col || !pos._p2) throw std::out_of_range("");
#endif
            return _matrix[(pos._p1 - 1) * _col + pos._p2 - 1];
        }
        T& at(const position& pos) {
            if (pos._p1 > _row || !pos._p1 || pos._p2 > _col || !pos._p2) throw std::out_of_range("");
            return _matrix[(pos._p1 - 1) * _col + pos._p2 - 1];
        }
        const T& at(const position& pos)const {
            if (pos._p1 > _row || !pos._p1 || pos._p2 > _col || !pos._p2) throw std::out_of_range("");
            return _matrix[(pos._p1 - 1) * _col + pos._p2 - 1];
        }
        line get_line(const position& pos)const {
#ifndef NDEBUG
            if (pos._p1 && pos._p2) throw std::out_of_range("");
#endif
            line ret;
            if (!pos._p1) ret._line = _matrix + pos._p2 - 1, ret._step = _row, ret._length = _col;
            else ret._line = _matrix + (pos._p1 - 1) * _col, ret._step = 1, ret._length = _row;
            return ret;
        }

        matrix& operator+=(const matrix& m) {
#ifndef NDEBUG
            if (_row != m._row || _col != m._col) std::invalid_argument("");
#endif
            for (std::size_t i = 0; i < _row * _col; ++i)
                _matrix[i] += m._matrix[i];
            return *this;
        }
        matrix& operator-=(const matrix& m) {
#ifndef NDEBUG
            if (_row != m._row || _col != m._col) std::invalid_argument("");
#endif
            for (std::size_t i = 0; i < _row * _col; ++i)
                _matrix[i] -= m._matrix[i];
            return *this;
        }
        matrix& operator*=(const matrix& m) {
#ifndef NDEBUG
            if (_col != m._row) std::invalid_argument("");
#endif
            (*this) = std::move(_mul(m));
            return *this;
        }
        matrix& operator/=(const matrix& m) {
#ifndef NDEBUG
            if (_col != m._col) std::invalid_argument("");
#endif
            (*this) = std::move(_mul(m.transpose()));
            return *this;
        }

        matrix transpose()const {
            matrix ret(_col, _row);
            for (std::size_t row = 1; row <= _col; ++row)
                for (std::size_t col = 1; col <= _row; ++col)
                    ret[{row, col}] = (*this)[{col, row}];
            return ret;
        }
        template <class F>
        matrix apply(F f)const {
            matrix ret(_row, _col);
            for (std::size_t i = 0; i < _row * _col; ++i)
                ret._matrix[i] = f(_matrix[i]);
            return ret;
        }

        std::size_t row()const {
            return _row;
        }
        std::size_t col()const {
            return _col;
        }

    private:
        matrix _mul(const matrix& m)const {
            matrix ret(_row, m._col);
            for (std::size_t row = 1; row <= _row; ++row)
                for (std::size_t col = 1; col <= m._col; ++col)
                    for (std::size_t i = 1; i <= _col; ++i)
                        ret[{row, col}] += (*this)[{row, i}] * m[{i, col}];
            return ret;
        }

    private:
        std::size_t _row;
        std::size_t _col;
        T* _matrix;

    };

}

#endif