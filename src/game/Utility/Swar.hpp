//
//  Swar.h
//  Project256
//
//  Created by Andreas Stahl on 07.03.23.
//

#pragma once



template <size_t Width, typename U = int, typename T = unsigned>
struct SWOR {
    static constexpr size_t size = sizeof(T) * 8 / Width;

    static constexpr T mask = [](){
        T t = 0;
        size_t x {Width};
        while (x--) {
            t <<= 1;
            t |= 1;
        }
        return t;
    }();

    static constexpr T odd_mask = [](){
        T t = 0;
        size_t x {size/2};
        while (x--) {
            t <<= 2 * Width;
            t |= mask;
        }
        return t;
    }();

    static constexpr T even_mask = ~odd_mask;

    T data;

    template<size_t I>
    constexpr U get() const noexcept {
        return static_cast<U>((data >> (Width * I)) & mask);
    }

    template<size_t I> requires std::signed_integral<U>
    constexpr U get() const noexcept {
        constexpr size_t usize = sizeof(U)*8;
        constexpr int right_offset = Width * I;
        constexpr int left_offset = usize - Width;
        constexpr int shift = left_offset - right_offset;
        if constexpr (shift < 0) {
            return static_cast<U>(data >> -shift) >> (usize - Width);
        } else {
            return static_cast<U>(data << shift) >> (usize - Width);
        }
    }

    template<size_t I>
    constexpr void set(U value) noexcept {
        constexpr T shifted_mask = mask << (Width * I);
        data = (data & ~shifted_mask) | ((static_cast<T>(value) & mask) << (Width * I));
    }
};

template <size_t Width, typename U, typename T>
constexpr SWOR<Width, U, T> operator+(SWOR<Width, U, T> lhs, SWOR<Width, U, T> rhs)
{
    constexpr T odd_mask = SWOR<Width, U, T>::odd_mask;
    constexpr T even_mask = SWOR<Width, U, T>::even_mask;
    T odd = (lhs.data & odd_mask) + (rhs.data & odd_mask);
    T even = (lhs.data & even_mask) + (rhs.data & even_mask);
    return { .data = (odd & odd_mask) | (even & even_mask) };
}


namespace std {

template <size_t Width, typename U, typename T>
struct tuple_size<SWOR<Width, U, T>> : std::integral_constant<size_t, SWOR<Width, U, T>::size>
{};

template <size_t I, size_t Width, typename U, typename T>
struct tuple_element<I, SWOR<Width, U, T>> {
    using type = U;
};



}

bool doSomething(unsigned i, unsigned j) {
    SWOR<1, bool> x { .data = i };
    SWOR<1, bool> y { .data = j };

    // x.set<3>(4);
    // y.set<3>(-3);
    auto z = x + y;

    return z.get<2>() || !z.get<3>();
    //return std::tuple_size<decltype(a)>::value;
}

