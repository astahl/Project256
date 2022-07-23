#pragma once

#include <concepts>

template <size_t N, std::signed_integral T = std::ptrdiff_t>
struct CircularIndex {
	T value;

    constexpr CircularIndex(auto val) {
        value = (((val) % N) + N) % N;
    }

	constexpr CircularIndex& operator=(auto val) {
		value = (((val) % N) + N) % N;
        return *this;
	}

	constexpr CircularIndex& operator+=(auto i) {
		return *this = (value + i);
	}

	constexpr CircularIndex& operator++() {
		return *this = value + 1;
	}

	constexpr CircularIndex& operator-=(auto i) {
		set(value - i);
		return *this;
	}

	constexpr CircularIndex& operator--() {
		set(value - 1);
		return *this;
	}

	constexpr bool operator==(const CircularIndex& other) const {
		return other.value == value;
	}

	constexpr bool operator!=(const CircularIndex& other) const {
		return other.value != value;
	}

    constexpr T operator-(const CircularIndex& other) const {
        return ((value + N) - other.value) % N;
    }

};
