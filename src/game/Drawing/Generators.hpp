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

    struct Sentinel {};

    struct Iterator {
        Vec2i mD;
        int mYi;
        bool mSteep;
        Vec2i mCurrentPosition;
        Vec2i mTo;
        mutable bool mFinished;
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

        constexpr bool operator!=(const Sentinel& other) const {
            return mCurrentPosition.x != mTo.x + 1;
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
            .mTo = mTo,
            .mCurrentPosition = mFrom,
            .mCurrentError = 2 * mD.y - mD.x,
        };
    }

    constexpr Sentinel end() const {
        return Sentinel{};
    }

    constexpr size_t size() const {
        if (mSteep) {
            return abs(mTo.y - mFrom.y) + 1;
        } else {
            return abs(mTo.x - mFrom.x) + 1;
        }
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

struct Ellipsis {
    Vec2i mRadii;
    Vec2i mCenter{};

    struct Sentinel {
    };

    struct Iterator {
        long mRx2;
        long mRy2;
        long mError;
        Vec2i mCenter;
        Vec2i mOffset;
        int mQuadrant = -2;

        constexpr bool operator!=(const Sentinel&) const {
            return mOffset.y >= 0;
        }

        constexpr Vec2i operator*() const {
            switch (mQuadrant) {
                case 0: return mCenter + mOffset;
                case 1: return mCenter + swizzled<Vec2SwizzleMask::NegateX>(mOffset);
                case 2: return mCenter + swizzled<Vec2SwizzleMask::Negate>(mOffset);
                case 3: return mCenter + swizzled<Vec2SwizzleMask::NegateY>(mOffset);
                default:
                    return mCenter;
            }
        }

        constexpr Iterator& operator++() {
            if (mQuadrant < 4) {
                ++mQuadrant;
                return *this;
            }
            // all quadrants done, now calculate next offsets and errors
            mQuadrant = 0;

            long errorNext = mError * 2;
            if (errorNext < (2 * mOffset.x + 1) * mRy2) {
                mOffset.x += 1;
                mError += (2 * mOffset.x + 1) * mRy2;
            }
            if (errorNext > -(2* mOffset.y - 1) * mRx2) {
                mOffset.y -= 1;
                mError -= (2* mOffset.y - 1) * mRx2;
            }
            return *this;
        }
    };

    constexpr Iterator begin() const {
        long rx2 = static_cast<long>(mRadii.x) * mRadii.x;
        long ry2 = static_cast<long>(mRadii.y) * mRadii.y;
        return Iterator {
            .mRx2 = rx2,
            .mRy2 = ry2,
            .mError = ry2 - ( 2 * mRadii.y - 1) * rx2,
            .mCenter = mCenter,
            .mOffset = {.x = 0, .y = mRadii.y}
        };
    }

    constexpr Sentinel end() const {
        return Sentinel{};
    }
};

}
