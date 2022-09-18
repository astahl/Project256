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
#include <utility>
#include <memory>
#include <type_traits>
#include <algorithm>

namespace ranges_at_home {

template <typename T, typename U>
using is_not_same = std::bool_constant<std::negation_v<std::is_same<T, U>>>;

template <typename T, size_t N = 0>
struct iterator {};

template <typename T>
struct iterator<T> { using type = decltype(std::declval<T&>().begin()); };

template <typename T>
struct iterator<const T> { using type = decltype(std::declval<const T&>().begin()); };
//
//template <typename T, size_t N>
//struct iterator<std::array<T, N>> { using type = T*; };
//
//template <typename T, size_t N>
//struct iterator<const std::array<T, N>> { using type = const T*; };

template <typename T, size_t N>
struct iterator<T[N]> { using type = T*; };

template <typename T, size_t N>
struct iterator<const T[N]> { using type = const T*; };

template <typename T, size_t N = 0>
struct sentinel {};

template <typename T>
struct sentinel<T> { using type = decltype(std::declval<T&>().end()); };

template <typename T>
struct sentinel<const T> { using type = decltype(std::declval<const T&>().end()); };

//template <typename T, size_t N>
//struct sentinel<std::array<T, N>> { using type = T*; };
//
//template <typename T, size_t N>
//struct sentinel<const std::array<T, N>> { using type = const T*; };

template <typename T, size_t N>
struct sentinel<T[N]> { using type = T*; };

template <typename T, size_t N>
struct sentinel<const T[N]> { using type = const T*; };

template <typename T>
using iterator_t = typename iterator<T>::type;

template <typename T>
using sentinel_t = typename sentinel<T>::type;

template <typename T>
struct iter_value { using type = std::decay_t<decltype(*(std::declval<iterator_t<T>&>()))>; };

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


template <typename T>
constexpr iterator_t<const T> begin(const T& t) { return t.begin(); }

template <typename T, size_t N = std::extent_v<T>, typename U = T[N]>
constexpr iterator_t<const T> begin(const U& t) { return &t[0]; }

template <typename T>
constexpr sentinel_t<const T> end(const T& t) { return t.end(); }

template <typename T, size_t N = std::extent_v<T>, typename U = T[N]>
constexpr sentinel_t<const T> end(const U& t) { return &t[N - 1]; }

template <typename T>
constexpr size_t size(const T& t) {
    size_t sz = 0;
    for (auto _ : t) { _ = _; sz++;}
    return sz;
}

template <typename T, typename T::size>
constexpr size_t size(const T& t) { return t.size(); }

template <typename T, size_t N = std::extent_v<T>, typename U = T[N]>
constexpr size_t size(const U& t) { return N; }

template <typename T, typename U>
concept aOneWayInequalityComparable = requires(T& t, U& u) {
    t != u;
};

template <typename T>
concept anIncrementable = requires(T& t) {
    {++t} -> std::same_as<T&>;
};


template <typename T>
concept aDereferencable = requires(T& t) {
    *t;
};

template <typename T>
concept anIterator = anIncrementable<T> && aDereferencable<T>;

template <typename R>
concept aRange = requires(R& r) {
    typename ranges_at_home::iter_value_t<R>;
    typename ranges_at_home::sentinel_t<R>;
    typename ranges_at_home::iterator_t<R>;

    ranges_at_home::begin(r);
    ranges_at_home::end(r);
}
&& anIterator<iterator_t<R>>
&& aOneWayInequalityComparable<iterator_t<R>, sentinel_t<R>>;


template <typename T>
struct array_view {
    T* mPtr;
    size_t mSize;

    constexpr T* begin() const {
        return mPtr;
    }

    constexpr T* end() const {
        return mPtr + mSize;
    }

    constexpr size_t size() const {
        return mSize;
    }
};

template<aRange T, typename Func>
struct transform_view final {
    using InputIterator = iterator_t<const T>;
    using InputSentinel = sentinel_t<const T>;
    using InputValue = decltype(*std::declval<InputIterator>());
    using Value = decltype(std::declval<Func>()(std::declval<InputValue>()));
    const T& base;
    const Func& func;

    constexpr transform_view(const T& base, const Func& func)
    : base(base), func(func) {
    }

    struct sentinel {
        const InputSentinel mInputSentinel;
    };

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

        constexpr Value operator*() const {
            InputValue input = (*inputIterator);
            Value result = func(input);
            return result;
        }

        constexpr bool operator!=(const iterator& other) const {
            return this->inputIterator != other.inputIterator;
        }

        constexpr bool operator!=(const sentinel& other) const {
            return this->inputIterator != other.mInputSentinel;
        }
    };

    constexpr iterator begin() const {
        return iterator{ranges_at_home::begin(base), func};
    }

    constexpr sentinel end() const {
        return sentinel{ranges_at_home::end(base)};
    }

    constexpr size_t size() const {
        return base.size();
    }
};


template<aRange T, typename U = iter_value_t<T>>
struct enumerate_view final {
    using InputIterator = iterator_t<const T>;
    using InputSentinel = sentinel_t<const T>;
    const T& base;

    struct enumerated final {
        U value;
        int position;
    };

    constexpr enumerate_view(const T& base)
    : base(base) {
    }

    struct sentinel {
        InputSentinel mEndIt;
    };

    struct iterator {
        InputIterator inputIterator;
        int position = 0;

        constexpr iterator(InputIterator inputIterator)
        : inputIterator(inputIterator) {
        }

        constexpr iterator& operator++() {
            ++inputIterator;
            ++position;
            return *this;
        }

        constexpr enumerated operator*() const {
            U input = (*inputIterator);
            auto result = enumerated{.value = input, .position = position};
            return result;
        }

        constexpr bool operator!=(const iterator& other) const {
            return this->inputIterator != other.inputIterator;
        }

        constexpr bool operator!=(const sentinel& other) const {
            return this->inputIterator != other.mEndIt;
        }
    };

    constexpr iterator begin() const {
        return iterator{ranges_at_home::begin(base)};
    }

    constexpr sentinel end() const {
        return sentinel{ranges_at_home::end(base)};
    }

    constexpr size_t size() const {
        return base.size();
    }
};


template<aRange T, typename Func>
struct filter_view final {
    using InputIterator = iterator_t<const T>;
    using InputSentinel = sentinel_t<const T>;
    struct sentinel {
        InputSentinel mEndIt;
    };

    struct iterator {
        using value = iter_value_t<const T>;

        InputIterator mInputIterator;
        InputSentinel mInputSentinel;
        value mCurrentValue;
        const Func& mFunc;

        constexpr iterator(InputIterator inputIterator, InputSentinel inputSentinel, const Func& func)
        : mInputIterator(inputIterator), mInputSentinel(inputSentinel), mCurrentValue{*inputIterator}, mFunc(func) {
        }

        constexpr iterator& operator++() {
            if (mInputIterator != mInputSentinel) {
                do {
                    ++mInputIterator;
                    mCurrentValue = *mInputIterator;
                }
                while (needsToAdvance());
            }

            return *this;
        }

        constexpr value operator*() const {
            return mCurrentValue;
        }

        constexpr bool operator!=(const iterator& other) const {
            return mInputIterator != other.mInputIterator;
        }

        constexpr bool operator!=(const sentinel& other) const {
            return mInputIterator != other.mEndIt;
        }

        constexpr bool needsToAdvance() const {
            return (mInputIterator != mInputSentinel) && !mFunc(mCurrentValue);
        }
    };

    const T& base;
    const Func& mFunc;

    constexpr filter_view(const T& base, const Func& func)
    : base(base), mFunc(func) {
    }

    constexpr iterator begin() const {
        auto it = iterator{ranges_at_home::begin(base), ranges_at_home::end(base), mFunc};
        while (it.needsToAdvance())
            ++it;
        return it; 
    }

    constexpr sentinel end() const {
        return sentinel{ranges_at_home::end(base)};
    }

    constexpr size_t size() const {
        return std::distance(begin(), end());
    }
};



template<aRange T>
struct skip_view {
    using InputIterator = iterator_t<const T>;
    using InputSentinel = sentinel_t<const T>;
    using InputValue = iter_value_t<const T>;

    int mCount;
    const T& mInputRange;

    struct Sentinel {};

    struct Iterator {
        InputIterator mInputIt;
        InputSentinel mEnd;
        int mCount;

        constexpr void doTheSkip() {
            while (mCount > 0 && mInputIt != mEnd) {
                mCount -= 1;
                ++mInputIt;
            }
        }

        constexpr Iterator& operator++() {
            ++mInputIt;
            return *this;
        }

        constexpr InputValue operator*() const {
            return *mInputIt;
        }

        constexpr bool operator!=(const Sentinel&) const {
            return mInputIt != mEnd;
        }
    };

    constexpr Iterator begin() const {
        auto it = Iterator{
            .mInputIt = ranges_at_home::begin(mInputRange),
            .mEnd = ranges_at_home::end(mInputRange),
            .mCount = mCount
        };
        it.doTheSkip();
        return it;
    }

    constexpr Sentinel end() const {
        return {};
    }
};


template<aRange T>
struct take_view {
    using InputIterator = iterator_t<const T>;
    using InputSentinel = sentinel_t<const T>;
    using InputValue = iter_value_t<const T>;

    int mCount;
    const T& mInputRange;

    struct Sentinel {};

    struct Iterator {
        InputIterator mInputIt;
        InputSentinel mEnd;
        int mCount;

        constexpr Iterator& operator++() {
            mCount--;
            ++mInputIt;
            return *this;
        }

        constexpr InputValue operator*() const {
            return *mInputIt;
        }

        constexpr bool operator!=(const Sentinel&) const {
            return mCount > 0 && mInputIt != mEnd;
        }
    };

    constexpr Iterator begin() const {
        auto it = Iterator{
            .mInputIt = ranges_at_home::begin(mInputRange),
            .mEnd = ranges_at_home::end(mInputRange),
            .mCount = mCount
        };
        return it;
    }

    constexpr Sentinel end() const {
        return {};
    }
};


template<aRange T, size_t N, size_t Stride, bool Wrap>
struct batch_view {
    using InputIterator = iterator_t<const T>;
    using InputSentinel = sentinel_t<const T>;
    using InputValue = iter_value_t<const T>;

    int mCount;
    const T& mInputRange;

    struct Sentinel {};

    struct Iterator {
        InputIterator mBegin;
        InputIterator mInputIt;
        InputSentinel mEnd;

        constexpr Iterator& operator++() {
            for (size_t i = 0; i < Stride && mInputIt != mEnd; ++i) {
                ++mInputIt;
            }

            return *this;
        }

        constexpr std::array<InputValue, N> operator*() const {
            std::array<InputValue, N> result{};
            size_t i = 0;
            if constexpr (Wrap) {
                if constexpr (!std::is_copy_assignable_v<InputIterator>) {
                    std::unique_ptr<InputIterator> inputCopy = std::make_unique<InputIterator>(mInputIt);
                    while (i < N) {
                        result[i] = *(*inputCopy);
                        ++*inputCopy;
                        ++i;
                        if (!(*inputCopy != mEnd)) {
                            inputCopy.reset(new InputIterator{mBegin});
                        }
                    }
                } else {
                    InputIterator inputCopy{mInputIt};
                    while (i < N) {
                        result[i] = *(inputCopy);
                        ++inputCopy;
                        ++i;
                        if (!(inputCopy != mEnd)) {
                            inputCopy = mBegin;
                        }
                    }
                }

            } else {
                auto inputCopy{mInputIt};
                while (i < N && inputCopy != mEnd) {
                    result[i] = *inputCopy;
                    ++inputCopy;
                    ++i;
                }
            }
            return result;
        }

        constexpr bool operator!=(const Sentinel&) const {
            return mInputIt != mEnd;
        }
    };

    constexpr Iterator begin() const {
        const auto begin = ranges_at_home::begin(mInputRange);
        auto it = Iterator{
            .mBegin = begin,
            .mInputIt = begin,
            .mEnd = ranges_at_home::end(mInputRange),
        };
        return it;
    }

    constexpr Sentinel end() const {
        return {};
    }
};



template<aRange T>
struct flatten_view {
    using InputIterator = iterator_t<const T>;
    using InputSentinel = sentinel_t<const T>;
    using InputValue = iter_value_t<const T>;
    using FlattenedInputIterator = iterator_t<InputValue>;
    using FlattenedInputSentinel = sentinel_t<InputValue>;
    using FlattenedValue = iter_value_t<InputValue>;

    const T& mInputRange;

    struct Sentinel {};

    struct Iterator {
        InputIterator mInputIt;
        InputSentinel mEnd;
        FlattenedInputIterator mFlattenedIt;
        FlattenedInputSentinel mFlattenedSentinel;

        constexpr Iterator& operator++() {
            if (mFlattenedIt != mFlattenedSentinel) {
                ++mFlattenedIt;
            } else if (mInputIt != mEnd) {
                ++mInputIt;
                if (mInputIt != mEnd) {
                    mFlattenedIt = ranges_at_home::begin(*mInputIt);
                    mFlattenedSentinel = ranges_at_home::end(*mInputIt);
                }
            }
            return *this;
        }

        constexpr FlattenedValue operator*() const {
            return *mFlattenedIt;
        }

        constexpr bool operator!=(const Sentinel&) const {
            return mInputIt != mEnd;
        }
    };

    constexpr Iterator begin() const {
        const auto begin = ranges_at_home::begin(mInputRange);
        const auto end = ranges_at_home::end(mInputRange);
        if (begin != end)
        {
            auto flattenedIt = ranges_at_home::begin(*begin);
            auto flattenedSentinel = ranges_at_home::end(*begin);
            return Iterator{
                .mInputIt = begin,
                .mEnd = end,
                .mFlattenedIt = flattenedIt,
                .mFlattenedSentinel = flattenedSentinel
            };
        }
        return Iterator {
            .mInputIt = begin,
            .mEnd = end,
        };
    }

    constexpr Sentinel end() const {
        return {};
    }
};



template<aRange T, aRange U>
struct pairwise_view {
    using InputIteratorLeft = iterator_t<const T>;
    using InputSentinelLeft = sentinel_t<const T>;
    using InputValueLeft = iter_value_t<const T>;
    using InputIteratorRight = iterator_t<const U>;
    using InputSentinelRight = sentinel_t<const U>;
    using InputValueRight = iter_value_t<const U>;

    const T& mInputRangeLeft;
    const U& mInputRangeRight;

    struct Sentinel {};

    struct Iterator {
        InputIteratorLeft mInputItLeft;
        InputSentinelLeft mEndLeft;
        InputIteratorRight mInputItRight;
        InputSentinelRight mEndRight;

        constexpr Iterator& operator++() {
            ++mInputItLeft;
            ++mInputItRight;
            return *this;
        }

        constexpr std::tuple<InputValueLeft, InputValueRight> operator*() const {
            return std::make_tuple(*mInputItLeft, *mInputItRight);
        }

        constexpr bool operator!=(const Sentinel&) const {
            return mInputItLeft != mEndLeft && mInputItRight != mEndRight;
        }
    };

    constexpr Iterator begin() const {
        const auto beginLeft = ranges_at_home::begin(mInputRangeLeft);
        const auto endLeft= ranges_at_home::end(mInputRangeLeft);
        const auto beginRight = ranges_at_home::begin(mInputRangeRight);
        const auto endRight = ranges_at_home::end(mInputRangeRight);
        return {.mInputItLeft = beginLeft, .mEndLeft = endLeft, .mInputItRight = beginRight, .mEndRight = endRight};
    }

    constexpr Sentinel end() const {
        return {};
    }

    constexpr size_t size() const {
        return std::min(ranges_at_home::size(mInputRangeLeft), ranges_at_home::size(mInputRangeRight));
    }
};

template <typename Func>
struct transform final {

    const Func& func;
    constexpr transform(const Func& func)
    : func(func) {}

    template <aRange T>
    constexpr transform_view<T, Func> apply(const T& range) const
    {
        return transform_view(range, func);
    }

};


template <typename Func>
struct filter {

    const Func& func;
    constexpr filter(const Func& func)
    : func(func) {}

    template <aRange T>
    constexpr filter_view<T, Func> apply(const T& range) const
    {
        return filter_view(range, func);
    }

};

struct enumerate {
    template <aRange T>
    constexpr enumerate_view<T> apply(const T& range) const
    {
        return enumerate_view(range);
    }
};

struct skip {
    int mCount;

    template <aRange T>
    constexpr skip_view<T> apply(const T& range) const
    {
        return skip_view<T> {
            .mCount = mCount,
            .mInputRange = range,
        };
    }
};

struct take {
    int mCount;

    template <aRange T>
    constexpr take_view<T> apply(const T& range) const
    {
        return take_view<T> {
            .mCount = mCount,
            .mInputRange = range,
        };
    }
};


template <size_t N, size_t Stride = N, bool Wrap = true>
struct batch {
    static_assert(Stride > 0, "Stride must be greater than 0");
    template <aRange T>
    constexpr auto apply(const T& range) const {
        return batch_view<T, N, Stride, Wrap> {
            .mInputRange = range
        };
    }
};


struct flatten final {

    template <aRange T>
    constexpr flatten_view<T> apply(const T& range) const
    {
        return flatten_view<T>{.mInputRange = range};
    }

};


template <typename Func>
struct forEach {
    const Func& mFunc;

    constexpr forEach(const Func& func) : mFunc(func) {}

    template <aRange T>
    constexpr void apply(const T& range) const {
        for(const auto& v : range) {
            mFunc(v);
        }
    }
};

template <typename Func, typename V>
struct reduce {
    const Func& mFunc;
    const V initial;

    constexpr reduce(const Func& func, V initial) : mFunc(func), initial{initial} {}

    template <aRange T>
    constexpr auto apply(const T& range) const {
        V value = initial;
        for(const auto& v : range) {
            value = mFunc(value, v);
        }
        return value;
    }
};


template <size_t N>
struct toArray {

    template<aRange U>
    constexpr auto apply(const U& range) const
    {
        using T = iter_value_t<U>;
        using V = typename enumerate_view<U>::enumerated;
        using A = std::array<T,N>;
        return (enumerate{}
                | reduce([](A& arr, const V& en) {
                            arr[en.position] = en.value;
                            return arr;
                        }, A{}))
        .apply(range);
    }
};


template <size_t N, typename Compare>
struct toSortedArray {
    Compare mCompare{};

    template<aRange U>
    auto apply(const U& range) const
    {
        using T = iter_value_t<U>;
        using V = typename enumerate_view<U>::enumerated;
        auto array = toArray<N>().apply(range);
        std::sort(array.begin(), array.end(), mCompare);
        return array;
    }
};


template <typename T, typename U>
struct applicator final {
    T left;
    U right;

    template <aRange W>
    constexpr auto apply(const W& w) const {
        return right.apply(left.apply(w));
    }

    constexpr auto begin() const {
        return ranges_at_home::begin(right.apply(left));
    }

    constexpr auto end() const {
        return ranges_at_home::end(right.apply(left));
    }

    constexpr auto operator()() const {
        return right.apply(left);
    }

    constexpr auto run() const {
        return this->operator()();
    }

    constexpr size_t size() const {
        return right.apply(left).size();
    }
};

template <typename T, typename U>
constexpr applicator<T, U> operator|(T&& left, U&& right)
{
    return applicator<T, U> {
        .left{static_cast<T&&>(left)},
        .right{static_cast<U&&>(right)}};
}

template <aRange T, aRange U>
requires std::same_as<iter_value_t<T>, iter_value_t<U>>
struct concatenator final {
    T left;
    U right;

    using InputIteratorLeft = iterator_t<T>;
    using InputIteratorRight = iterator_t<U>;
    using InputSentinelLeft = sentinel_t<T>;
    using InputSentinelRight = sentinel_t<U>;

    struct sentinel {
        InputSentinelRight mRightSentinel;
    };

    struct iterator {
        using Value = iter_value_t<T>;
        InputIteratorLeft mLeft;
        InputIteratorRight mRight;
        InputSentinelLeft mLeftSentinel;

        constexpr iterator& operator++() {
            if (mLeft != mLeftSentinel) {
                ++mLeft;
            } else {
                ++mRight;
            }
            return *this;
        }

        constexpr Value operator*() const {
            if (mLeft != mLeftSentinel) {
                return *mLeft;
            } else {
                return *mRight;
            }
        }

        constexpr bool operator!=(const sentinel& other) const {
            return mRight != other.mRightSentinel;
        }
    };

    constexpr auto begin() const {
        return iterator {
            .mLeft = ranges_at_home::begin(left),
            .mRight = ranges_at_home::begin(right),
            .mLeftSentinel = ranges_at_home::end(left),
        };
    }

    constexpr auto end() const {
        return sentinel {
            .mRightSentinel = ranges_at_home::end(right)
        };
    }

    constexpr size_t size() const {
        return left.size() + right.size();
    }
};


template <aRange T, aRange U>
requires std::same_as<iter_value_t<T>, iter_value_t<U>>
struct alternator final {
    T left;
    U right;

    using InputIteratorLeft = iterator_t<T>;
    using InputIteratorRight = iterator_t<U>;
    using InputSentinelLeft = sentinel_t<T>;
    using InputSentinelRight = sentinel_t<U>;

    struct sentinel {
    };

    struct iterator {
        using Value = iter_value_t<T>;
        InputIteratorLeft mLeft;
        InputIteratorRight mRight;
        InputSentinelLeft mLeftSentinel;
        InputSentinelRight mRightSentinel;
        bool mRightNext;

        constexpr iterator& operator++() {
            if (mRightNext && mRight != mRightSentinel) {
                ++mRight;
            } else if (mLeft != mLeftSentinel) {
                ++mLeft;
            }
            mRightNext = !mRightNext;
            return *this;
        }

        constexpr Value operator*() const {
            if (!mRightNext) {
                return *mLeft;
            } else {
                return *mRight;
            }
        }

        constexpr bool operator!=(const sentinel& other) const {
            return mRight != mRightSentinel || mLeft != mLeftSentinel;
        }
    };

    constexpr auto begin() const {
        return iterator {
            .mLeft = ranges_at_home::begin(left),
            .mRight = ranges_at_home::begin(right),
            .mLeftSentinel = ranges_at_home::end(left),
            .mRightSentinel = ranges_at_home::end(right)
        };
    }

    constexpr auto end() const {
        return sentinel {};
    }

    constexpr size_t size() const {
        return left.size() + right.size();
    }
};

template <ranges_at_home::aRange T, ranges_at_home::aRange U>
requires std::same_as<ranges_at_home::iter_value_t<T>, ranges_at_home::iter_value_t<U>>
constexpr ranges_at_home::alternator<T, U> alternate(T&& left, U&& right)
{
    return ranges_at_home::alternator<T, U> {static_cast<T&&>(left), static_cast<U&&>(right)};
}


}

template <ranges_at_home::aRange T, ranges_at_home::aRange U>
requires std::same_as<ranges_at_home::iter_value_t<T>, ranges_at_home::iter_value_t<U>>
constexpr ranges_at_home::concatenator<T, U> operator^(T&& left, U&& right)
{
    return ranges_at_home::concatenator<T, U> {static_cast<T&&>(left), static_cast<U&&>(right)};
}

template <ranges_at_home::aRange T, ranges_at_home::aRange U>
constexpr ranges_at_home::pairwise_view<T, U> operator&(T&& left, U&& right)
{
    return ranges_at_home::pairwise_view<T, U> {static_cast<T&&>(left), static_cast<U&&>(right)};
}

