//
//  Generators.cpp
//  Project256
//
//  Created by Andreas Stahl on 08.06.22.
//
#pragma once

#include <algorithm>

#include "../Project256.h"
#include "../Math/Vec2Math.hpp"


namespace Generators {

template <typename T>
compiletime T abs(T x) { return x < 0 ? -x : x; }
template <typename T>
compiletime void swap(T& x, T& y) { T temp = x; x = y; y = temp; }

struct Rectangle {
    Vec2i bottomLeft;
    Vec2i topRight;

    struct Iterator {
        Vec2i bottomLeft;
        Vec2i topRight;
        Vec2i current;
        
        constexpr Vec2i operator*() const {
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

        constexpr bool operator!=(const Iterator& other) const {
            return (current.x != other.current.x) || (current.y != other.current.y);
        }
    };

    using iterator = Iterator;

    const Iterator mEnd = ++(Iterator{ .bottomLeft = bottomLeft, .topRight = topRight, .current = topRight });

    constexpr Iterator begin() const {
        return Iterator{
            .bottomLeft = bottomLeft,
            .topRight = topRight,
            .current = bottomLeft,
        };
    }

    constexpr Iterator end() const {
        return mEnd;
    }

    constexpr size_t size() const {
        return (1 + topRight.x - bottomLeft.x) * (1 + topRight.y - bottomLeft.y);
    }
};

struct Line {

    struct Sentinel {
        int mXPos;
    };

    struct Iterator {
        Vec2i mD;
        int mYi;
        bool mSteep;
        Vec2i mCurrentPosition;
        int mCurrentError;

        constexpr Vec2i operator*() const {
            if (mSteep)
                return Vec2i{mCurrentPosition.y, mCurrentPosition.x};
            return mCurrentPosition;
        }

        constexpr Iterator& operator++() {
            mCurrentPosition.x += 1;
            if (mCurrentError > 0) {
                mCurrentPosition.y += mYi;
                mCurrentError += 2 * (mD.y - mD.x);
            } else {
                mCurrentError += 2 * mD.y;
            }
            return *this;
        }

        constexpr bool operator!=(const Iterator& other) const {
            return mCurrentPosition != other.mCurrentPosition;
        }

        constexpr bool operator!=(const Sentinel& other) const {
            return mCurrentPosition.x != other.mXPos;
        }
    };


    Vec2i mFrom{};
    Vec2i mTo{};
    Vec2i mD{};
    int mYi = 1;
    bool mSteep = false;

    using iterator = Iterator;

    constexpr Line(Vec2i from, Vec2i to)
        : mFrom(from)
        , mTo(to)
    {
        Vec2i diff = mTo - mFrom;
        if (abs(diff.x) < abs(diff.y)) {
            mSteep = true;
            swap(mTo.x, mTo.y);
            swap(mFrom.x, mFrom.y);
        }

        if (mFrom.x > mTo.x)
            swap(mFrom, mTo);

        mD = mTo - mFrom;
        if (mD.y < 0)
        {
            mD.y = -mD.y;
            mYi = -1;
        }
    }

    constexpr Iterator begin() const {
        return Iterator{
            .mD = mD,
            .mYi = mYi,
            .mSteep = mSteep,
            .mCurrentPosition = mFrom,
            .mCurrentError = 2 * mD.y - mD.x,
        };
    }

    constexpr Sentinel end() const {
        return Sentinel{mTo.x + 1};
    }

    constexpr size_t size() const {
        return abs(mTo.x - mFrom.x) + 1;
    }
};


struct Circle {
    int mRadius;
    Vec2i mCenter{};

    struct Sentinel {
    };

    struct Iterator {
        int mError;
        Vec2i mErrorChange;
        Vec2i mCenter;
        Vec2i mOffset;
        int mOctant = 4;

        constexpr bool operator!=(const Sentinel&) const {
            return mOffset.x <= mOffset.y;
        }

        constexpr Vec2i operator*() const {
            switch (mOctant) {
                case 0: return mCenter + swizzled<Vec2SwizzleMask::NegateX>(mOffset);
                case 1: return mCenter + swizzled<Vec2SwizzleMask::SwapNegateY>(mOffset);
                case 2: return mCenter + swizzled<Vec2SwizzleMask::SwapNegate>(mOffset);
                case 3: return mCenter + swizzled<Vec2SwizzleMask::Negate>(mOffset);
                case 4: return mCenter + mOffset;
                case 5: return mCenter + swizzled<Vec2SwizzleMask::NegateY>(mOffset);
                case 6: return mCenter + swizzled<Vec2SwizzleMask::Swap>(mOffset);
                case 7: return mCenter + swizzled<Vec2SwizzleMask::SwapNegateX>(mOffset);
                default:
                    assert(false);
                    return {};
            }
        }

        constexpr Iterator& operator++() {
            if (mOctant < 7) {
                ++mOctant;
                return *this;
            }
            // all octants done, now calculate next offsets and errors
            mOctant = 0;

            if (mError >= 0) {
                mOffset.y -= 1;
                mErrorChange.y += 2;
                mError += mErrorChange.y;
            }
            mOffset.x += 1;
            mErrorChange.x += 2;
            mError += mErrorChange.x + 1;
            return *this;
        }
    };

    constexpr Iterator begin() const {
        return Iterator {
            .mError = 1 - mRadius,
            .mErrorChange = Vec2i{.x = 0, .y = -2* mRadius },
            .mCenter = mCenter,
            .mOffset = {.x = 0, .y = mRadius}
        };
    }

    constexpr Sentinel end() const {
        return Sentinel{};
    }
};



}
