//
//  Generators.cpp
//  Project256
//
//  Created by Andreas Stahl on 08.06.22.
//
#define CXX
#include "../Project256.h"

struct Rectangle {
    const Vec2i bottomLeft;
    const Vec2i topRight;

    struct Iterator {
        const Vec2i& bottomLeft;
        const Vec2i& topRight;
        Vec2i current;
        constexpr Vec2i operator*() {
            return current;
        }
        constexpr Iterator& operator++() {
            if (current.x == topRight.x) {
                ++current.y;
                current.x = bottomLeft.x;
            } else {
                ++current.x;
            }
            return *this;
        }

        constexpr bool operator!=(const Iterator& other) {
            return current.x != other.current.x || current.y != other.current.y;
        }
    };
    const Iterator mEnd = ++(Iterator{.bottomLeft = bottomLeft, .topRight = topRight, .current = topRight});

    constexpr Iterator begin() {
        return Iterator{
            .bottomLeft = bottomLeft,
            .topRight = topRight,
            .current = bottomLeft,
        };
    }

    constexpr Iterator end() {
        return mEnd;
    }
};
