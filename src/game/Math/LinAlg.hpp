//
//  LinAlg.hpp
//  Project256
//
//  Created by Andreas Stahl on 13.03.23.
//

#pragma once

template<int Rows, int Columns>
struct RowFirstIndexing;

template<int Rows, int Columns>
struct ColumnFirstIndexing {
    using transposed = RowFirstIndexing<Columns, Rows>;

    static constexpr int get(int row, int column)
    {
        return row * Columns + column;
    }

    static constexpr int size()
    {
        return Rows * Columns;
    }
};

template<int Rows, int Columns>
struct RowFirstIndexing {
    using transposed = ColumnFirstIndexing<Columns, Rows>;

    static constexpr int get(int row, int column)
    {
        return column * Rows + row;
    }

    static constexpr int size()
    {
        return Rows * Columns;
    }
};

template<int N, bool IsColumnVector = true>
struct VectorIndexing {
    using transposed = VectorIndexing<N, !IsColumnVector>;

    static constexpr int get(int row, int column)
    {
        if constexpr (IsColumnVector)
            return row;
        else
            return column;
    }

    static constexpr int size()
    {
        return N;
    }
};


template <typename T, int Rows, int Columns, typename Index = ColumnFirstIndexing<Rows, Columns>>
struct Matrix
{
    T data[Index::size()];

    constexpr T& operator()(int row, int column) {
        return data[Index::get(row, column)];
    }

    constexpr const T& operator()(int row, int column) const {
        return data[Index::get(row, column)];
    }

    constexpr operator T() const requires(Rows == 1 && Columns == 1)
    {
        return data[0];
    }

    constexpr T& operator[](int index) requires(Rows == 1 || Columns == 1)
    {
        return data[index];
    }

    constexpr const T& operator[](int index) const requires(Rows == 1 || Columns == 1)
    {
        return data[index];
    }

    constexpr T* begin() { return data; }
    constexpr const T* begin() const { return data; }
    constexpr T* end() { return data + Index::size(); }
    constexpr const T* end() const { return data + Index::size(); }

    constexpr Matrix<T, Columns, Rows, typename Index::transposed> transposed() const {
        Matrix<T, Columns, Rows, typename Index::transposed> m;
        memcpy(&m, this, sizeof(*this));
        return m;
    }
};

template<typename T, int N>
using RowVector = Matrix<T, 1, N, VectorIndexing<N, false>>;

template<typename T, int N>
using ColumnVector = Matrix<T, N, 1, VectorIndexing<N, true>>;

template <typename Op1, typename Op2, typename T, typename U, int LhsRows, int LhsColumns, typename LhsIndex, typename RhsIndex>
constexpr decltype(Op1{}(T{}, U{})) applyColumnRowWise(Op1&& op1,
                                                 Op2&& op2,
                                                 int i,
                                                 int j,
                                                 const Matrix<T, LhsRows, LhsColumns, LhsIndex>& lhs,
                                                 const Matrix<U, LhsColumns, LhsRows, RhsIndex>& rhs)
{
    decltype(Op1{}(T{}, U{})) value = 0;
    for (int k = 0; k < LhsColumns; ++k) {
        value = op2(value, op1(lhs(i, k), rhs(k, j)));
    }
    return value;
}

template <typename T, typename U, int LhsRows, int LhsColumns, typename LhsIndex, typename RhsIndex>
constexpr Matrix<decltype(T{} * U{}), LhsRows, LhsRows> operator*(
                                                        const Matrix<T, LhsRows, LhsColumns, LhsIndex>& lhs,
                                                        const Matrix<U, LhsColumns, LhsRows, RhsIndex>& rhs)
{
    Matrix<decltype(T{} * U{}), LhsRows, LhsRows> m;
    for (int i = 0; i < LhsRows; ++i) {
        for (int j = 0; j < LhsRows; ++j) {
            m(i,j) = applyColumnRowWise([](auto t, auto u){ return t * u; }, [](auto t, auto u){ return t + u; }, i, j, lhs, rhs);
        }
    }
    return m;
}

template <typename Op, typename T, typename U, int Rows, int Columns, typename LhsIndex, typename RhsIndex>
constexpr Matrix<decltype(Op{}(T{}, U{})), Rows, Columns> combineElementWise(Op&& operation,
                                                        const Matrix<T, Rows, Columns, LhsIndex>& lhs,
                                                        const Matrix<U, Rows, Columns, RhsIndex>& rhs)
{
    Matrix<decltype(Op{}(T{}, U{})), Rows, Columns> m;
    for (int i = 0; i < Rows; ++i) {
        for (int j = 0; j < Columns; ++j) {
            m(i,j) = operation(lhs(i,j), rhs(i,j));
        }
    }
    return m;
}

template <typename T, typename U, int Rows, int Columns, typename LhsIndex, typename RhsIndex>
constexpr Matrix<decltype(T{} + U{}), Rows, Columns> operator+(
                                                        const Matrix<T, Rows, Columns, LhsIndex>& lhs,
                                                        const Matrix<U, Rows, Columns, RhsIndex>& rhs)
{
    return combineElementWise([](T t, U u) { return t + u; }, lhs, rhs);
}

template <typename T, typename U, int Rows, int Columns, typename LhsIndex, typename RhsIndex>
constexpr Matrix<decltype(T{} + U{}), Rows, Columns> operator-(
                                                        const Matrix<T, Rows, Columns, LhsIndex>& lhs,
                                                        const Matrix<U, Rows, Columns, RhsIndex>& rhs)
{
    return combineElementWise([](T t, U u) { return t - u; }, lhs, rhs);
}

template <typename R, typename Op, typename T, int Rows, int Columns, typename Index>
constexpr Matrix<R, Rows, Columns> applyElementWise(Op&& operation,
                                                        const Matrix<T, Rows, Columns, Index>& mat)
{
    Matrix<R, Rows, Columns> m;
    for (int i = 0; i < Rows; ++i) {
        for (int j = 0; j < Columns; ++j) {
            m(i,j) = operation(mat(i,j));
        }
    }
    return m;
}

template <typename U, typename T, int Rows, int Columns, typename Index>
constexpr Matrix<decltype(U{} * T{}), Rows, Columns> operator*(U&& lhs,
                                                        const Matrix<T, Rows, Columns, Index>& rhs)
{
    return applyElementWise<decltype(U{} * T{})>([&lhs](T t) { return lhs * t; }, rhs);
}

template <typename T, int Rows, int Columns, typename Index, typename U>
constexpr Matrix<decltype(T{} * U{}), Rows, Columns> operator*(
                                                        const Matrix<T, Rows, Columns, Index>& lhs, U&& rhs)
{
    return applyElementWise<decltype(T{} * U{})>([&rhs](T t) { return t * rhs; }, lhs);
}

template <typename U, typename T, int Rows, int Columns, typename Index>
constexpr Matrix<decltype(U{} / T{}), Rows, Columns> operator/(U&& lhs,
                                                        const Matrix<T, Rows, Columns, Index>& rhs)
{
    return applyElementWise<decltype(U{} / T{})>([&lhs](T t) { return lhs / t; }, rhs);
}

template <typename T, int Rows, int Columns, typename Index, typename U>
constexpr Matrix<decltype(T{} / U{}), Rows, Columns> operator/(
                                                        const Matrix<T, Rows, Columns, Index>& lhs, U&& rhs)
{
    return applyElementWise<decltype(T{} / U{})>([&rhs](T t) { return t / rhs; }, lhs);
}
