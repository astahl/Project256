//
//  Text.hpp
//  Project256
//
//  Created by Andreas Stahl on 04.07.22.
//

#pragma once

#include "defines.h"
#include <cstdint>
#include <bit>
#include "../FML/RangesAtHome.hpp"

template <typename T>
struct Utf8CodepointsView {
    const T& inputRange;
    using InputIterator = ranges_at_home::iterator_t<T>;
    using InputSentinel = ranges_at_home::sentinel_t<T>;

    struct Sentinel {};

    struct Iterator {
        InputIterator mInput;
        InputSentinel mEnd;
        uint32_t mCode{0};
        bool atEnd{false};

        constexpr uint32_t operator*() const {
            return mCode;
        }

        constexpr Iterator& operator++() {
            if (!(mInput != mEnd)) {
                atEnd = true;
            }
            uint8_t inputChar = *(mInput++);

            int onesCount = std::__countl_one(inputChar);
            if(onesCount == 1 || onesCount > 4) {
                mCode = inputChar;
                return *this;
            }
            // the resulting unicode codepoint
            mCode = (0b01111111 >> onesCount) & inputChar;
            while ((mInput != mEnd) && (--onesCount > 0)) {
                mCode <<= 6;
                inputChar = *(mInput++);
                mCode |= inputChar & 0b00111111;
            }
            return *this;
        }

        constexpr bool operator!=(const Sentinel& other) const {
            return !atEnd;
        }
    };

    constexpr Iterator begin() const {
        auto it = Iterator{
            .mInput = ranges_at_home::begin(inputRange),
            .mEnd = ranges_at_home::end(inputRange),
        };
        ++it;
        return it;
    }

    constexpr Sentinel end() const {
        return {};
    }
};
