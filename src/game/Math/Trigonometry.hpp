//
//  Trigonometry.hpp
//  Project256
//
//  Created by Andreas Stahl on 06.07.22.
//

#pragma once

#include "../defines.h"
#include <array>
#include <cstdint>
#include <numbers>


namespace {

compiletime uint64_t factorial(int n) {
    uint64_t x = n;
    for (int i = n - 1; i > 1; --i) {
        x *= i;
    }
    return x;
}

static_assert(factorial(0) == 0);
static_assert(factorial(1) == 1);
static_assert(factorial(2) == 2);
static_assert(factorial(3) == 6);
static_assert(factorial(4) == 24);

compiletime double power(double x, int n) {
    if (n == 0) {
        return 1.0;
    }
    double result = x;
    for (int i = 1; i < n; ++i) {
        result *= x;
    }
    return result;
}

compiletime bool isNear(auto a, auto b) {
    return (a - b) * (a - b) < 0.000002;
}

static_assert(isNear(power(2.0, 1), 2.0));
static_assert(isNear(power(2.0, 2), 4.0));
static_assert(isNear(power(2.0, 3), 8.0));
static_assert(isNear(power(2.0, 4), 16.0));

template <typename T, int K = 6>
compiletime T CosineTaylor(T rad) {
    T value{1};
    for (int n = 1; n <= K; ++n) {
        value += power(-1.0, n) * power(rad, 2 * n) / factorial(2 * n);
    }
    return value;
}

static const size_t N = 2048;

compiletime std::array<double, N> QuarterCosine = [](){
    std::array<double, N> result{};
    double step = std::numbers::pi / ((N * 2) + 1);
    double value = 0.0;
    for (int i = 0; i < N; ++i) {
        result[i] = CosineTaylor(value);
        value += step;
    }
    return result;
}();
}

template <std::floating_point T>
compiletime T myCos(T x) {
    if (x < 0) x = -x;
    T pi2 = (std::numbers::pi_v<T> * 2);
    int div = static_cast<int>(x / pi2);
    T remainder = x - div * (pi2);
    int index = static_cast<int>((remainder * N * 4) / pi2);

    int quadrant = index / N;
    index %= N;
    if (quadrant == 1 || quadrant == 3) {
        index = N - index - 1;
    }
    T value = static_cast<T>(QuarterCosine[index]);
    return (quadrant == 1 || quadrant == 2) ? -value : value;
}

template <std::floating_point T>
compiletime T mySin(T x) {
    constexpr T pi_2 = (std::numbers::pi_v<T> / 2);
    return myCos(pi_2 - x);
}

template <std::floating_point T>
compiletime T myTan(T x) {
    return mySin(x) / myCos(x);
}

compiletime void test_myCos() {

    for (double x = 0.0; x < 10.0; x += 0.01) {
        double my = myCos(x);
        double std = std::cos(x);
        assert(isNear(my, std));
        my = my; std = std;
    }

    for (double x = 0.0; x > -10.0; x -= 0.01) {
        double my = myCos(x);
        double std = std::cos(x);
        assert(isNear(my, std));
        my = my; std = std;
    }

    for (double x = -10.0; x < 10.0; x += 0.01) {
        double my = mySin(x);
        double std = std::sin(x);
        assert(isNear(my, std));
        my = my; std = std;
    }
}
