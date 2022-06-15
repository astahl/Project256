//
//  RangesAtHome.hpp
//  Project256
//
//  Created by Andreas Stahl on 11.06.22.
//

#pragma once
#include <concepts>
#include <ranges>

namespace ranges_at_home {

template<typename T, typename Func>
struct transform_view {
    using InputIterator = typename std::remove_reference_t<T>::iterator;
    T base;
    Func func;

    constexpr transform_view(T base, Func func)
    : base(base), func(func) {
    }

    struct iterator {
        InputIterator inputIterator;
        const Func& func;

        constexpr iterator(InputIterator inputIterator, const Func& func)
        : inputIterator(inputIterator), func(func) {
        }

        constexpr iterator& operator++() {
            ++inputIterator;
            return *this;
        }

        constexpr decltype(func(*inputIterator)) operator*() {
            return func(*inputIterator);
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
    using InputIterator = typename std::remove_reference_t<T>::iterator;

    struct iterator {
        InputIterator inputIterator;
        InputIterator endInput;
        const Func& func;

        constexpr iterator(InputIterator inputIterator, InputIterator endInputIterator, const Func& func)
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
        return iterator(base.begin(), base.end(), func);
    }

    constexpr iterator end() {
        return mEnd;
    }
};

}
