//
//  RangesAtHome.hpp
//  Project256
//
//  Created by Andreas Stahl on 11.06.22.
//

#pragma once
#include <concepts>
#include <ranges>
#include <vector>
namespace ranges_at_home {


template <typename T>
struct iterator { using type = decltype(std::declval<T&>().begin()); };

template <typename T>
using iterator_t = typename iterator<T>::type;

template <typename T>
struct iter_value { using type = decltype(*(std::declval<iterator_t<T>&>())); };

template <typename T>
using iter_value_t = typename iter_value<T>::type;

template<class...>struct types{using type=types;};
template <typename Signature> struct args;
template <typename R, typename...Args>
struct args<R(Args...)> : types<Args...>{};
template <typename Signature>
using args_t = typename args<Signature>::type;


template <typename Signature> struct returns;
template <typename R, typename...Args>
struct returns<R(Args...)> { using type = R; };
template <typename Signature>
using returns_t = typename returns<Signature>::type;


template<typename T, typename Func>
struct transform_view {
    using InputIterator = iterator_t<T>;
    using Input = std::remove_reference_t<T>;
    Input base;
    Func func;

    constexpr transform_view(T base, Func func)
    : base(base), func(func) {
    }

    struct iterator {
        InputIterator inputIterator;
        Func func;

        constexpr iterator(InputIterator inputIterator, Func func)
        : inputIterator(inputIterator), func(func) {
        }

        constexpr iterator& operator++() {
            ++inputIterator;
            return *this;
        }

        constexpr decltype(func(*inputIterator)) operator*() {
            const auto input = (*inputIterator);
            const auto result = func(input);
            return result;
        }

        constexpr bool operator!=(iterator& other) {
            return this->inputIterator != other.inputIterator;
        }
    };

    constexpr iterator begin() {
        return iterator(base.begin(), func);
    }

    constexpr iterator end() {
        return iterator(base.end(), func);
    }
};


template<typename T, typename Func>
struct filter_view {
    using InputIterator = iterator_t<T>;

    struct iterator {
        using value = decltype(*std::declval<InputIterator>());
        using reference = std::add_lvalue_reference_t<value>;

        InputIterator inputIterator;
        InputIterator endInput;
        Func func;

        constexpr iterator(InputIterator inputIterator, InputIterator endInputIterator, Func func)
        : inputIterator(inputIterator), endInput(endInputIterator), func(func) {
        }

        constexpr iterator& operator++() {
            do {
                ++inputIterator;
            }
            while (inputIterator != endInput && !func(*inputIterator));
            return *this;
        }

        constexpr decltype(*inputIterator) operator*() {
            return *inputIterator;
        }

        constexpr bool operator!=(iterator& other) {
            return this->inputIterator != other.inputIterator;
        }
    };


    T base;
    Func func;
    const iterator mEnd;

    constexpr filter_view(T base, Func func)
    : base(base), func(func), mEnd(iterator(base.end(), base.end(), func)) {
    }

    constexpr iterator begin() {
        auto it = iterator(base.begin(), base.end(), func);
        if (!func(*it)) {
            return ++it;
        }
        return it; 
    }

    constexpr iterator end() {
        return mEnd;
    }
};


template <typename Func>
struct transform {

    Func func;
    transform(Func func)
    : func(func) {}

    template <typename T>
    transform_view<T, Func> apply(T range)
    {
        return transform_view(range, func);
    }

};

template <typename Func>
struct filter {

    Func func;
    filter(Func func)
    : func(func) {}

    template <typename T>
    filter_view<T, Func> apply(T range)
    {
        return filter_view(range, func);
    }

};

template <typename Func>
struct forEach {
    Func func;
    forEach(Func func)
    : func(func) {}

    template <typename T>
    constexpr void apply(T range) {
        for(auto&& v : range) {
            func(v);
        }
    }
};

template <typename Func, typename V = args_t<decltype(std::declval<Func>())>>
struct reduce {
    Func func;
    V initial;

    reduce(Func func = Func{}, V initial = V{}) : func(func), initial{initial} {}

    template <typename T>
    constexpr auto apply(T range) {
        V value = initial;
        for(auto&& v : range) {
            value = func(v, value);
        }
        return value;
    }
};


template <typename T, typename U>
struct applicator {
    T left;
    U right;

    template <typename W>
    constexpr auto apply(W&& w) {
        return right.apply(left.apply(w));
    }

    constexpr auto begin() {
        return right.apply(left).begin();
    }

    constexpr auto end() {
        return right.apply(left).end();
    }

    constexpr auto operator()() {
        return right.apply(left);
    }

    constexpr auto run() {
        return this->operator()();
    }
};

template <typename T, typename U>
constexpr applicator<T, U> operator|(T left, U right)
{
    return applicator<T, U> {left, right};
}


}
