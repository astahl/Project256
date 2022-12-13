//
//  Test.h
//  Project256
//
//  Created by Andreas Stahl on 12.12.22.
//

#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>

namespace Matchers {

template <typename T>
concept AnExceptionMatcher = requires(T t) {
    typename T::exception_t;
};

template <typename E>
struct Throws { using exception_t = E; };

template <typename T, int precision>
struct IsNear {
    static_assert(precision >= 0, "precision must be at least 0");

    static constexpr T epsilon = [] {
        T val{1};
        for (int i = 0; i < precision; ++i) {
            val /= 10;
        }
        return val;
    }();
    T a;

    bool operator()(const T& b) const
    {
        auto d = a - b;
        return d < 0 ? -d <= epsilon : d <= epsilon;
    }
};


}

struct Test {

    std::ostream& os = std::cout;
    std::stringstream sstream;
    std::atomic_int countFailed;
    std::atomic_int countSuccess;

    std::vector<void(*)(Test&)> tests;

    constexpr std::ostream& expect(std::invocable auto&& value, Matchers::AnExceptionMatcher auto matcher) noexcept
    {
        using ex_t = typename decltype(matcher)::exception_t;
        try {
            std::invoke(value);
            return fail() << " no exception thrown";
        } catch (ex_t e) {
            return success();
        } catch (...) {
            return fail() << " unexpected exception that's not " << typeid(ex_t).name();
        }

    }

    constexpr std::ostream& expect(std::invocable auto&& value, auto matcher) noexcept
    {
        try {
            auto x = std::invoke(value);
            return expect(x, matcher);
        } catch (...) {
            return fail() << " unexpected exception";
        }
    }

    template<typename T, std::predicate<T> M>
    constexpr std::ostream& expect(T&& value, M matcher)
    {
        using value_t = std::remove_cvref_t<decltype(value)>;

        if (matcher(value))
            return success();
        else
            return fail() << " value " << value;
    }

    template<typename T, std::equality_comparable_with <T> M>
    constexpr std::ostream& expect(T&& value, M matcher)
    {
        if (matcher == value)
            return success();
        else
            return fail() << " value " << value << " equals (==) " << matcher;
    }

    std::ostream& success() {
        countSuccess++;
        sstream.clear();
        //os << "\nSuccess";
        return sstream;
    }

    std::ostream& fail() {
        countFailed++;
        os << "\nFailed";
        return os;
    }

    int run() {
        for (auto& test : tests) {
            test(*this);
        }
        os << "Succeeded: " << countSuccess << " Failed: " << countFailed << "\n";
        return countFailed ? -1 : 0;
    }

    constexpr void add(auto&& test) {
        tests.push_back(test);
    }
};
