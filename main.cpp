//#include <iostream>
//#include <iomanip>
//#include "matrix.hpp"
//
//double f(double c) {
//    return c + 1;
//}
//
//constexpr std::size_t row = 2;
//constexpr std::size_t col = 10;
//
//template <typename T>
//void show_matrix_by_row(const hwshqtb::matrix<T>& m) {
//    for (std::size_t i = 1; i <= m.row(); ++i) {
//        hwshqtb::matrix<double>::line rows = m.get_line({i, 0});
//        for (std::size_t j = 1; j <= m.col(); ++j)
//            std::cout << std::setfill(' ') << std::setw(4) << rows[j];
//        std::cout << std::endl;
//    }
//    std::cout << std::endl;
//}
//
//template <typename T>
//void show_matrix_by_col(const hwshqtb::matrix<T>& m) {
//    for (std::size_t i = 1; i <= m.col(); ++i) {
//        hwshqtb::matrix<double>::line cols = m.get_line({0, i});
//        for (std::size_t j = 1; j <= m.row(); ++j)
//            std::cout << std::setfill(' ') << std::setw(4) << cols[j];
//        std::cout << std::endl;
//    }
//    std::cout << std::endl;
//}
//
//template <typename T>
//void show_matrix(const hwshqtb::matrix<T>& m) {
//    for (std::size_t i = 1; i <= m.col(); ++i) {
//        for (std::size_t j = 1; j <= m.row(); ++j)
//            std::cout << std::setfill(' ') << std::setw(4) << m[{i, j}];
//        std::cout << std::endl;
//    }
//    std::cout << std::endl;
//}
//
//int main() {
//    hwshqtb::matrix<double> m(5, 3.0);
//    show_matrix(m);
//    show_matrix_by_row(m);
//    show_matrix_by_col(m);
//    show_matrix(m.transpose());
//}
#include <iostream>
#include <string>
#include "big_number_data.hpp"
#include "array.hpp"

int main() {
    constexpr hwshqtb::number::data_t v{nullptr, 0};
    constexpr hwshqtb::array{v[0], v[1]};
}
