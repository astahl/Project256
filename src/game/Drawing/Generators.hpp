//
//  Generators.cpp
//  Project256
//
//  Created by Andreas Stahl on 08.06.22.
//
#pragma once

#define CXX
#include <algorithm>

#include "../Project256.h"
#include "../Math/Vec2Math.hpp"


namespace Generators {

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
            }
            else {
                ++current.x;
            }
            return *this;
        }

        constexpr bool operator!=(const Iterator& other) {
            return (current.x != other.current.x) || (current.y != other.current.y);
        }
    };

    using iterator = Iterator;

    const Iterator mEnd = ++(Iterator{ .bottomLeft = bottomLeft, .topRight = topRight, .current = topRight });

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

    constexpr size_t size() {
        return (1 + topRight.x - bottomLeft.x) * (1 + topRight.y - bottomLeft.y);
    }
};

struct Line {

    struct Iterator {
        const Line& mLine;
        Vec2i mCurrentPosition;
        int mCurrentError;

        constexpr Vec2i operator*() {
            if (mLine.mSteep)
                return Vec2i{mCurrentPosition.y, mCurrentPosition.x};
            return mCurrentPosition;
        }

        constexpr Iterator& operator++() {
            mCurrentPosition.x += 1;
            if (mCurrentError > 0) {
                mCurrentPosition.y += mLine.mYi;
                mCurrentError += 2 * (mLine.mD.y - mLine.mD.x);
            } else {
                mCurrentError += 2 * mLine.mD.y;
            }
            return *this;
        }

        constexpr bool operator!=(const Iterator& other) {
            return (mCurrentPosition.x != other.mCurrentPosition.x) || (mCurrentPosition.y != other.mCurrentPosition.y);
        }
    };


    Vec2i mFrom;
    Vec2i mTo;
    Vec2i mD;
    int mYi = 1;
    bool mSteep = false;
    Iterator mEnd = {*this, 0, 0};

    using iterator = Iterator;

    Line(Vec2i from, Vec2i to)
    {
        Vec2i diff = to - from;
        if (std::abs(diff.x) < std::abs(diff.y)) {
            mSteep = true;
            std::swap(to.x, to.y);
            std::swap(from.x, from.y);
        }

        if (from.x > to.x)
            std::swap(from, to);

        mFrom = from;
        mTo = to;
        mD = to - from;
        if (mD.y < 0)
        {
            mD.y = -mD.y;
            mYi = -1;
        }

        mEnd.mCurrentPosition = mTo;
        mEnd.mCurrentError = 2 * mD.y - mD.x;
        ++mEnd;
    }

    constexpr Iterator begin() {
        return Iterator{
            .mLine = *this,
            .mCurrentPosition = mFrom,
            .mCurrentError = 2 * mD.y - mD.x,
        };
    }

    constexpr Iterator end() {
        return mEnd;
    }
};





}
