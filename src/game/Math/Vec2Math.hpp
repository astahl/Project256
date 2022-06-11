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

template <typename T>
struct is_vec2 {
    using value = std::disjunction<std::is_same<T, Vec2i>, std::is_same<T, Vec2f>>;
};

template <typename T>
using is_vec2_v = typename is_vec2<T>::value;

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

template<typename V, typename = std::enable_if_t<is_vec2_v<V>{}>>
constexpr float length(V vec) {
    return sqrtf(dot(vec, vec));
}

template<typename V, typename = std::enable_if_t<is_vec2_v<V>{}>>
constexpr V normalized(V vec) {
    auto leng = length(vec);
    if (leng == 0.0) return V{};
    return vec / leng;
}

template<typename V, typename W, typename R = decltype(vec2_scalar_t<V>{} - vec2_scalar_t<W>{})>
constexpr vec2_t<R> operator-(V left, W right) {
    return vec2_t<R>{left.x - right.x, left.y - right.y};
}

template<typename V, typename W, typename R = decltype(vec2_scalar_t<V>{} - vec2_scalar_t<W>{})>
constexpr vec2_t<R> operator+(V left, W right) {
    return vec2_t<R>{left.x + right.x, left.y + right.y};
}

template<typename Vec1, typename Vec2>
constexpr bool operator<(Vec1 left, Vec2 right)
{
    return left.x < right.x && left.y < right.y;
}

template<typename Vec1, typename Vec2>
constexpr bool operator==(Vec1 left, Vec2 right)
{
    return left.x == right.x && left.y == right.y;
}

template<typename Scalar, typename Vec, typename R = decltype(Scalar{} * vec2_scalar_t<Vec>{})>
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

constexpr Vec2i round(Vec2f vec) {
    return Vec2i{static_cast<int>(lround(vec.x)), static_cast<int>(lround(vec.y))};
}

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
