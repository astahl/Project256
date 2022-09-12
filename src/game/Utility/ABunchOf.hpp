#pragma once

#include <array>
#include <numeric>
#include <cassert>


template<typename T, size_t N>
struct ABunchOf {
    std::size_t count = 0;
    std::ptrdiff_t insertIndex = 0;
    std::array<std::ptrdiff_t, N> insertMap = {};
    std::array<T, N> storage = {};

    constexpr ABunchOf() {
        reset();
    }

    constexpr void reset() {
        count = 0;
        insertIndex = 0;
        std::iota(insertMap.begin(), insertMap.end(), 1);
    }

    constexpr size_t insert(T&& value) {
        assert(insertIndex != N && "container is full");

        const size_t insertAt = insertIndex;
        storage[insertAt] = value;
        insertIndex = insertMap[insertAt];
        insertMap[insertAt] = -1;
        ++count;
        return insertAt;
    }

    constexpr void free(size_t index) {
        assert((index < N) && "index out of range");
        assert((insertMap[index] == -1) && "index is already freed");

        insertMap[index] = insertIndex;
        insertIndex = index;
        --count;
    }

    constexpr T& operator[](size_t index) {
        assert((index < N) && "index out of range");
        assert((insertMap[index] == -1) && "index is free");

        return storage[index];
    }

    constexpr const T& operator[](size_t index) const {
        assert((index < N) && "index out of range");
        assert((insertMap[index] == -1) && "index is free");

        return storage[index];
    }

    constexpr T& at(size_t index) {
        return storage.at(index);
    }

    constexpr const T& at(size_t index) const {
        return storage.at(index);
    }

    constexpr bool empty() const {
        return count == 0;
    }

    constexpr size_t size() const{
        return count;
    }

    constexpr size_t max_size() const {
        return N;
    }
};
