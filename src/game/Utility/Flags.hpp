#pragma once

#include <type_traits>
#include <concepts>


template <typename T>
concept anEnum = std::is_enum_v<T>;

template<anEnum T>
struct Flags {
    using underlying_t = std::underlying_type_t<T>;
    T value;

    template<std::same_as<T> ...Args>
    constexpr Flags(T first, Args ...args) {
        value = static_cast<T>((static_cast<underlying_t>(first) | ... | static_cast<underlying_t>(args)));
    }

    constexpr bool test(T i) const {
        return static_cast<underlying_t>(value) & static_cast<underlying_t>(i);
    }

    template<std::same_as<T> ...Args>
    constexpr void reset(T first, Args ...args) {
        value = static_cast<T>(
            static_cast<underlying_t>(value) & ~( (static_cast<underlying_t>(first) | ... | static_cast<underlying_t>(args)) ) 
            );
    }

    template<std::same_as<T> ...Args>
    constexpr void set(Args ...args) {
        value = static_cast<T>(
            (static_cast<underlying_t>(value) | ... | static_cast<underlying_t>(args))
            );
    }

    template<std::same_as<T> ...Args>
    constexpr void toggle(T first, Args ...args) {
        value = static_cast<T>(
            static_cast<underlying_t>(value) ^ ( (static_cast<underlying_t>(first) | ... | static_cast<underlying_t>(args)) )
            );
    }

    constexpr underlying_t underlying() const {
        return static_cast<underlying_t>(value);
    }

    constexpr bool operator==(T other) const {
        return value == other;
    }

    constexpr bool operator!=(T other) const {
        return value != other;
    }
};
