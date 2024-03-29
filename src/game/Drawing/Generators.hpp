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

struct Rectangle {
    Vec2i bottomLeft{};
    Vec2i topRight{};

    struct Sentinel {};

    struct Iterator {
        Vec2i bottomLeft;
        Vec2i topRight;
        Vec2i current;
        bool mFinished;
        
        constexpr Vec2i operator*() const {
            return current;
        }

        constexpr Iterator& operator++() {
            mFinished = !(current != topRight);
            if (current.x == topRight.x) {
                ++current.y;
                current.x = bottomLeft.x;
            }
            else {
                ++current.x;
            }

            return *this;
        }

        constexpr bool operator!=(const Sentinel&) const {
            return !mFinished;
        }
    };

    using iterator = Iterator;

    constexpr Rectangle(const Vec2i& corner1, const Vec2i& corner2) {
        if (corner1 < corner2) {
            bottomLeft = corner1;
            topRight = corner2;
        }
        else {
            bottomLeft = corner2;
            topRight = corner1;
        }
    }

    constexpr Iterator begin() const {
        return Iterator{
            .bottomLeft = bottomLeft,
            .topRight = topRight,
            .current = bottomLeft,
        };
    }

    constexpr Sentinel end() const {
        return Sentinel{};
    }

    constexpr size_t width() const {
        return static_cast<int64_t>(topRight.x) - bottomLeft.x + 1;
    }

    constexpr size_t height() const {
        return static_cast<int64_t>(topRight.y) - bottomLeft.y + 1;
    }

    constexpr size_t size() const {
        return width() * height();
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
        bool mFinished;
        int mCurrentError;

        constexpr Vec2i operator*() const {
            if (mSteep)
                return Vec2i{mCurrentPosition.y, mCurrentPosition.x};
            return mCurrentPosition;
        }

        constexpr Iterator& operator++() {
            mFinished = !(mCurrentPosition != mTo);
            mCurrentPosition.x += 1;
            if (mCurrentError > 0) {
                mCurrentPosition.y += mYi;
                mCurrentError += 2 * (mD.y - mD.x);
            } else {
                mCurrentError += 2 * mD.y;
            }
            return *this;
        }

        constexpr bool operator!=(const Sentinel&) const {
            return !mFinished;
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
        if (myabs(diff.x) < myabs(diff.y)) {
            mSteep = true;
            std::swap(mTo.x, mTo.y);
            std::swap(mFrom.x, mFrom.y);
        }

        if (mFrom.x > mTo.x)
            std::swap(mFrom, mTo);

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
            .mTo = mTo,
            .mCurrentError = 2 * mD.y - mD.x,
        };
    }

    constexpr Sentinel end() const {
        return Sentinel{};
    }

    constexpr size_t size() const {
        if (mSteep) {
            return abs(static_cast<int64_t>(mTo.y) - mFrom.y) + 1;
        } else {
            return abs(static_cast<int64_t>(mTo.x) - mFrom.x) + 1;
        }
    }
};


struct HLine {

    struct Sentinel {};

    struct Iterator {
        int mCount;
        Vec2i mCurrentPosition;

        constexpr Vec2i operator*() const {
            return mCurrentPosition;
        }

        constexpr Iterator& operator++() {
            mCurrentPosition.x += 1;
            mCount -= 1;
            return *this;
        }

        constexpr bool operator!=(const Sentinel&) const {
            return mCount > 0;
        }
    };


    Vec2i mFrom{};
    int mCount{};

    using iterator = Iterator;

    constexpr HLine(Vec2i from, Vec2i to)
    {
        int distance = to.x - from.x;
        if (distance > 0){
            mCount = distance;
            mFrom = from;
        } else {
            mCount = -distance;
            mFrom = to;
        }
        mCount += 1;
    }

    constexpr HLine(Vec2i from, int length)
    {
        mFrom = from;
        if (length > 0){
            mCount = length;
        } else {
            mCount = -length;
            mFrom.x += length  + 1;
        }
    }

    constexpr Iterator begin() const {
        return Iterator{
            .mCount = mCount,
            .mCurrentPosition = mFrom,
        };
    }

    constexpr Sentinel end() const {
        return Sentinel{};
    }

    constexpr size_t size() const {
        return mCount;
    }
};



struct VLine {

    struct Sentinel {};

    struct Iterator {
        int mCount;
        Vec2i mCurrentPosition;

        constexpr Vec2i operator*() const {
            return mCurrentPosition;
        }

        constexpr Iterator& operator++() {
            mCurrentPosition.y += 1;
            mCount -= 1;
            return *this;
        }

        constexpr bool operator!=(const Sentinel&) const {
            return mCount > 0;
        }
    };


    Vec2i mFrom{};
    int mCount{};

    using iterator = Iterator;

    constexpr VLine(Vec2i from, Vec2i to)
    {
        int distance = to.y - from.y;
        if (distance > 0){
            mCount = distance;
            mFrom = from;
        } else {
            mCount = -distance;
            mFrom = to;
        }
        mCount += 1;
    }

    constexpr VLine(Vec2i from, int length)
    {
        mFrom = from;
        if (length > 0){
            mCount = length;
        } else {
            mCount = -length;
            mFrom.y += length  + 1;
        }
    }

    constexpr Iterator begin() const {
        return Iterator{
            .mCount = mCount,
            .mCurrentPosition = mFrom,
        };
    }

    constexpr Sentinel end() const {
        return Sentinel{};
    }

    constexpr size_t size() const {
        return mCount;
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
