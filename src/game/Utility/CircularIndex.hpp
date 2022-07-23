#pragma once

#include <concepts>

template <size_t N, std::unsigned_integral T>
struct CircularIndex {
	T value;

	constexpr void set(auto val) {
		value = (((val) % N) + N) % N;
	}

	constexpr CircularIndex& operator+=(auto i) {
		set(value + i);
		return *this;
	}

	constexpr CircularIndex& operator++() {
		set(value + 1);
		return *this;
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

};
