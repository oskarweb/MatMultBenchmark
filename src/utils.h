#pragma once

#include <ranges>
#include <iostream>

template<typename DataType, uint32_t Rows, uint32_t Columns>
class Matrix;

template<typename T>
concept ElementIterable =
    std::ranges::range<T> &&
    requires(T t)
    {
        { t.size() } -> std::same_as<std::size_t>;
    } &&
    std::ranges::range<std::ranges::range_value_t<T>> &&
    requires(std::ranges::range_value_t<T> elem)
    {
        { elem.size() } -> std::same_as<std::size_t>;
    };


template<typename DataType, uint32_t Rows, uint32_t Columns>
std::ostream &operator<<(std::ostream &os, const Matrix<DataType, Rows, Columns> &m) {
    std::ranges::for_each(m.get(), [&os](const auto &row) {
        for (auto element_it = row.begin(); element_it != row.end(); ++element_it) {
            os << *element_it;
            if (element_it != row.end() - 1)
                os << " ";
        }
        os << '\n';
    });
    return os;
}
