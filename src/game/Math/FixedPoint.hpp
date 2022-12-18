//
//  FixedPoint.hpp
//  Project256
//
//  Created by Andreas Stahl on 18.12.22.
//

#pragma once
#include <concepts>
#include <type_traits>

template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

struct RawTag {};

template <int P, std::integral T>
struct FixedPointReal {
    static const int precision = P;

    T data;

    constexpr FixedPointReal() {
        data = T{};
    }

    constexpr FixedPointReal(T value, RawTag) {
        data = value;
    }

    template <int Q>
    constexpr FixedPointReal(FixedPointReal<Q, T> other) {
        constexpr int difference = Q - P;
        if constexpr (difference == 0) {
            data = other.data;
        }
        if constexpr (difference > 0) {
            data = other.data >> difference;
        }
        if constexpr (difference < 0) {
            data = other.data << -difference;
        }
    }

    explicit constexpr FixedPointReal(std::integral auto value) {
        if constexpr (P == 0) {
            data = static_cast<T>(value);
        }
        if constexpr (P > 0) {
            data = static_cast<T>(value << P);
        }
        if constexpr (P < 0) {
            data = static_cast<T>(value >> -P);
        }
    }

    explicit constexpr FixedPointReal(std::floating_point auto&& value) {
        if constexpr (P == 0) {
            data = static_cast<T>(value);
        }
        if constexpr (P > 0) {
            data = static_cast<T>((T{1} << P) * value);
        }
        if constexpr (P < 0) {
            data = static_cast<T>(value / (T{1} << -P));
        }
    }

    template<std::integral U = int>
    constexpr U toInt() {
        if constexpr (P == 0) {
            return static_cast<U>(data);
        }
        if constexpr (P > 0) {
            return static_cast<U>(data >> P);
        }
        if constexpr (P < 0) {
            return static_cast<U>(data << -P);
        }
    }

    template <std::floating_point U = float>
    constexpr U toFloat() {
        if constexpr (P == 0) {
            return static_cast<U>(data);
        }
        if constexpr (P > 0) {
            return static_cast<U>(data) / (1 << P);
        }
        if constexpr (P < 0) {
            return static_cast<U>(data) * (1 << -P);
        }
    }
};

template <int P, std::integral T>
constexpr FixedPointReal<P, T> operator*(const FixedPointReal<P, T>& a, arithmetic auto&& b)
{
    return { static_cast<T>(a.data * b), RawTag{} };
}

template <int P, int Q, std::integral T>
constexpr FixedPointReal<P + Q, T> operator*(const FixedPointReal<P, T>& a, const FixedPointReal<Q, T>& b)
{
    return { a.data * b.data, RawTag{} };
}

template <int P, std::integral T>
constexpr FixedPointReal<P, T> operator+(const FixedPointReal<P, T>& a, const FixedPointReal<P, T>& b)
{
    return { a.data + b.data, RawTag{} };
}

template <int P, std::integral T>
constexpr FixedPointReal<P, T> operator+(const FixedPointReal<P, T>& a, arithmetic auto&& b)
{
    return a + FixedPointReal<P, T>{std::forward<decltype(b)>(b)};
}

template <int P, std::integral T>
constexpr FixedPointReal<P, T> operator-(const FixedPointReal<P, T>& a, const FixedPointReal<P, T>& b)
{
    return { a.data - b.data, RawTag{} };
}

template <int P, std::integral T>
constexpr FixedPointReal<P, T> operator-(const FixedPointReal<P, T>& a, arithmetic auto&& b)
{
    return a - FixedPointReal<P, T>{std::forward<decltype(b)>(b)};
}
