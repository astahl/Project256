//
//  Vec2Math.cpp
//  Project256
//
//  Created by Andreas Stahl on 11.06.22.
//
#pragma once
#include "../Project256.h"
#include <cmath>
#include <type_traits>
#include <utility>


template <typename T>
using is_vec2 = std::bool_constant<std::disjunction_v<std::is_same<T, Vec2i>, std::is_same<T, Vec2f>>>;


template <typename T>
const bool is_vec2_v = is_vec2<T>::value;

template <typename T>
struct vec2 {};

template <> struct vec2<float> { using type = Vec2f; };
template <> struct vec2<int> { using type = Vec2i; };

template <typename T>
using vec2_t = typename vec2<T>::type;

template <typename T>
struct vec2_scalar {};

template <> struct vec2_scalar<vec2_t<float>> { using type = float; };
template <> struct vec2_scalar<vec2_t<int>> { using type = int; };
template <typename T>
using vec2_scalar_t = typename vec2_scalar<T>::type;


template<typename V, typename W, typename R = decltype(vec2_scalar_t<V>{} * vec2_scalar_t<W>{})>
constexpr R dot(V left, W right) {
    return left.x * right.x + left.y * right.y;
}

template<typename V, typename = std::enable_if_t<is_vec2_v<V>>>
constexpr float length(V vec) {
    return sqrtf(static_cast<float>(dot(vec, vec)));
}

template<typename V, std::enable_if_t<is_vec2_v<V>, bool> = true>
auto atan2(V vec) {
    return std::atan2(vec.y, vec.x);
}

template<typename V, typename R = vec2_t<decltype(vec2_scalar_t<V>{} / float{})>, typename = std::enable_if_t<is_vec2_v<V>>>
constexpr R normalized(V vec) {
    auto leng = length(vec);
    if (leng == 0.0) return R{1.0f, 0.0f};
    return vec / leng;
}

enum class Vec2SwizzleMask : uint8_t {
    None = 0,
    Swap = 1,
    NegateX = 2,
    SwapNegateX = 3,
    NegateY = 4,
    SwapNegateY = 5,
    Negate = 6,
    SwapNegate = 7
};

template<Vec2SwizzleMask mask, typename V, typename = std::enable_if_t<is_vec2_v<V>>>
constexpr V swizzled(V vec) {
    V result{};
    if constexpr ((static_cast<uint8_t>(mask) & static_cast<uint8_t>(Vec2SwizzleMask::Swap)) == static_cast<uint8_t>(Vec2SwizzleMask::Swap)) {
        result = { vec.y, vec.x };
    } else {
        result = vec;
    }
    if constexpr ((static_cast<uint8_t>(mask) & static_cast<uint8_t>(Vec2SwizzleMask::NegateX)) == static_cast<uint8_t>(Vec2SwizzleMask::NegateX)) {
        result.x = -result.x;
    }
    if constexpr ((static_cast<uint8_t>(mask) & static_cast<uint8_t>(Vec2SwizzleMask::NegateY)) == static_cast<uint8_t>(Vec2SwizzleMask::NegateY)) {
        result.y = -result.y;
    }
    return result;
}

template<typename V, typename W, typename R = decltype(vec2_scalar_t<V>{} - vec2_scalar_t<W>{})>
constexpr vec2_t<R> operator-(V left, W right) {
    return vec2_t<R>{left.x - right.x, left.y - right.y};
}

template<typename V, typename W, typename R = decltype(vec2_scalar_t<V>{} + vec2_scalar_t<W>{})>
constexpr vec2_t<R> operator+(V left, W right) {
    return vec2_t<R>{left.x + right.x, left.y + right.y};
}

template<typename V, typename W, typename R = decltype(vec2_scalar_t<V>{} * vec2_scalar_t<W>{})>
constexpr vec2_t<R> operator*(V left, W right) {
    return vec2_t<R>{left.x * right.x, left.y * right.y};
}

template<typename Vec1, typename Vec2, std::enable_if_t<std::conjunction_v<is_vec2<Vec1>, is_vec2<Vec2>>, bool> = true>
constexpr bool operator<(Vec1 left, Vec2 right)
{
    return left.x < right.x && left.y < right.y;
}

template<typename T, typename = std::enable_if_t<is_vec2_v<T>>>
constexpr T operator-(T vec)
{
    return T{ -vec.x, -vec.y };
}


template<typename Vec1, typename Vec2, std::enable_if_t<std::conjunction_v<is_vec2<Vec1>, is_vec2<Vec2>>, bool> = true>
constexpr bool operator<=(Vec1 left, Vec2 right)
{
    return left.x <= right.x && left.y <= right.y;
}

template<typename Vec1, typename Vec2, std::enable_if_t<std::conjunction_v<is_vec2<Vec1>, is_vec2<Vec2>>, bool> = true>
constexpr bool operator>(Vec1 left, Vec2 right)
{
    return left.x > right.x && left.y > right.y;
}

template<typename Vec1, typename Vec2, std::enable_if_t<std::conjunction_v<is_vec2<Vec1>, is_vec2<Vec2>>, bool> = true>
constexpr bool operator>=(Vec1 left, Vec2 right)
{
    return left.x >= right.x && left.y >= right.y;
}


template<typename Vec1, typename Vec2, std::enable_if_t<std::conjunction_v<is_vec2<Vec1>, is_vec2<Vec2>>, bool> = true>
constexpr bool operator==(Vec1 left, Vec2 right )
{
    return left.x == right.x && left.y == right.y;
}

template<typename Vec1, typename Vec2, std::enable_if_t<std::conjunction_v<is_vec2<Vec1>, is_vec2<Vec2>>, bool> = true>
constexpr bool operator!=(Vec1 left, Vec2 right )
{
    return !(left == right);
}

template<typename Scalar, typename Vec, typename R = decltype(Scalar{} * vec2_scalar_t<Vec>{}), typename = std::enable_if<std::is_scalar_v<Scalar>>>
constexpr vec2_t<R> operator*(Scalar left, Vec right)
{
    return vec2_t<R>{
        .x = left * right.x,
        .y = left * right.y };
}

template<typename Scalar, typename Vec,  typename R = decltype(Scalar{} * vec2_scalar_t<Vec>{})>
constexpr vec2_t<R> operator/(Vec left, Scalar right)
{
    return vec2_t<R>{
        .x = left.x / right,
        .y = left.y / right };
}

constexpr Vec2i truncate(Vec2f vec) {
    return Vec2i{static_cast<int>(vec.x), static_cast<int>(vec.y)};
}

#ifndef _WIN64
constexpr Vec2i round(Vec2f vec) {
    return Vec2i{static_cast<int>(lround(vec.x)), static_cast<int>(lround(vec.y))};
}
#else
Vec2i round(Vec2f vec) {
    return Vec2i{ static_cast<int>(lround(vec.x)), static_cast<int>(lround(vec.y)) };
}
#endif


constexpr Vec2f itof(Vec2i vec) {
    return Vec2f{ static_cast<float>(vec.x), static_cast<float>(vec.y) };
}

//template<typename Vec>
//constexpr Vec2f& operator=(Vec2f& left, Vec right)
//{
//    left.x = right.x;
//    left.y = right.y;
//    return left;
//}

constexpr Vec2i operator%(Vec2i left, Vec2i right) {
    return Vec2i{left.x % right.x, left.y % right.y};
}

constexpr Vec2i operator%(Vec2i left, int right) {
    return Vec2i{left.x % right, left.y % right};
}

Vec2i rand2d() {
    return Vec2i{rand(), rand()};
}

template<typename T, typename U1, typename U2>
constexpr T wrapAround(T a, U1 lowerBound, U2 upperBound) {
    auto width = upperBound - lowerBound;
    while (!(a < upperBound)) {
        a = a - width;
    }
    while (a < lowerBound) {
        a = a + width;
    }
    return a;
}

template<typename Vec, typename U1, typename U2>
constexpr Vec wrapAround2d(Vec a, U1 lowerBound, U2 upperBound) {
    return Vec{ wrapAround(a.x, lowerBound.x, upperBound.x),
        wrapAround(a.y, lowerBound.y, upperBound.y) };
}

template<typename V>
constexpr V clamp(V vec, V upper, V lower) {
    return V { std::clamp(vec.x, upper.x, lower.x), std::clamp(vec.y, upper.y, lower.y)};
}


template <typename T, size_t Rows, size_t Cols>
struct Matrix{
    struct Row {
        std::array<T, Cols> values;
    };
    std::array<Row, Rows> rows;
};

using Mat4i = Matrix<int, 4, 4>;
using Mat4f = Matrix<float, 4, 4>;

template <typename T, typename Vec, typename U = vec2_scalar_t<Vec>, typename R = vec2_t<decltype(T{} * U{})>>
constexpr R operator*(const Matrix<T, 2, 3>& matrix, const Vec& vector)
{
    return vec2<R>{
        .x = matrix.rows[0].values[0] * vector.x + matrix.rows[0].values[1] * vector.y + matrix.rows[0].values[2] * 1,
        .y = matrix.rows[1].values[0] * vector.x + matrix.rows[1].values[1] * vector.y + matrix.rows[1].values[2] * 1
    };
}

template <typename T, typename Vec, typename U = vec2_scalar_t<Vec>, typename R = vec2_t<decltype(T{} * U{})>>
constexpr R operator*(const Matrix<T, 2, 2>& matrix, const Vec& vector)
{
    return R{
        .x = matrix.rows[0].values[0] * vector.x + matrix.rows[0].values[1] * vector.y,
        .y = matrix.rows[1].values[0] * vector.x + matrix.rows[1].values[1] * vector.y
    };
}

template <typename T>
constexpr Matrix<T, 2, 2> makeScale2d(T uniform) {
    Matrix<T, 2, 2> result {};
    result.rows[0].values[0] = uniform;
    result.rows[1].values[1] = uniform;
    return result;
}

template <typename Vec, typename T = vec2_scalar_t<Vec>>
constexpr auto makeBase2d(Vec base1, Vec base2) {
    using M = Matrix<T, 2, 2>;
    using R = typename M::Row;
    M result{
        .rows{
            R({ base1.x, base2.x }),
            R({ base1.y, base2.y })
        }
    };
    return result;
}

template <typename Vec, typename T = vec2_scalar_t<Vec>>
constexpr Matrix<T, 2, 2> makeBase2dX(Vec newBaseX) {
    return makeBase2d(newBaseX, {-newBaseX.y, newBaseX.x});
}

template <typename Vec, typename T = vec2_scalar_t<Vec>>
constexpr Matrix<T, 2, 2> makeBase2dY(Vec newBaseY) {
    return makeBase2d({newBaseY.y, -newBaseY.x}, newBaseY);
}

constexpr Matrix<float, 2, 2> makeRotation2d(float angle) {
    Matrix<float, 2, 2> result {
        .rows {
            Matrix<float, 2, 2>::Row{ .values = { cos(angle), -sin(angle) } },
            Matrix<float, 2, 2>::Row{ .values = { sin(angle), cos(angle) } }
        }
    };
    return result;
}

template<typename Vec1, typename Vec2, std::enable_if_t<std::conjunction_v<is_vec2<Vec1>, is_vec2<Vec2>>, bool> = true>
constexpr bool isFurtherClockwise(const Vec1& vec1, const Vec2 vec2) {
    return dot(swizzled<Vec2SwizzleMask::SwapNegateX>(vec1), vec2) < 0;
}
