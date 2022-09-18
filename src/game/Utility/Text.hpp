//
//  Text.hpp
//  Project256
//
//  Created by Andreas Stahl on 04.07.22.
//

#pragma once

#include "defines.h"
#include <cstdint>
#include <bit>
#include <string_view>
#include <optional>
#include "../FML/RangesAtHome.hpp"
namespace Unicode {

enum class Codepoints : uint32_t {
    LowLine = 0x005f,
    VerticalLine = 0x007c,
    MiddleDot = 0x00b7,
    GreekSmallLetterPi = 0x03c0,

    Bullet = 0x2022,

    LeftwardsArrow = 0x2190,
    UpwardsArrow = 0x2191,

    RingOperator = 0x2218,
    BulletOperator = 0x2219,

    BoxDrawingsLightHorizontal = 0x2500,
    BoxDrawingsHeavyHorizontal = 0x2501,
    BoxDrawingsLightVertical = 0x2502,
    BoxDrawingsHeavyVertical = 0x2503,
    BoxDrawingsLightVerticalAndHorizontal = 0x253c,
    BoxDrawingsHeavyVerticalAndHorizontal = 0x254b,
    BoxDrawingsLightArcDownAndRight = 0x256d,
    BoxDrawingsLightArcDownAndLeft = 0x256e,
    BoxDrawingsLightArcUpAndLeft = 0x256f,
    BoxDrawingsLightArcUpAndRight = 0x2570,
    BoxDrawingsLightDiagonalUpperRightToLowerLeft = 0x2571,
    BoxDrawingsLightDiagonalUpperLeftToLowerRight = 0x2572,
    BoxDrawingsLightDiagonalCross = 0x2572,

    UpperOneEigthBlock = 0x2594,
    LowerOneEighthBlock = 0x2581,

    BlackDiamond = 0x25c6,
    WhiteCircle = 0x25cb,
    BlackCircle = 0x25cf,
    WhiteBullet = 0x25e6,
    LargeCircle = 0x25ef,

    BlackSpadeSuit = 0x2660,
    BlackClubSuit = 0x2663,
    BlackHeartSuit = 0x2665,
    BlackDiamondSuit = 0x2666,

    HeavyBlackHeart = 0x2764,
};

}
template <typename T>
struct Utf8CodepointsView {
    const T& inputRange;
    using InputIterator = ranges_at_home::iterator_t<T>;
    using InputSentinel = ranges_at_home::sentinel_t<T>;

    struct Sentinel {};

    struct Iterator {
        InputIterator mInput;
        InputSentinel mEnd;
        uint32_t mCode{0};
        bool atEnd{false};

        constexpr uint32_t operator*() const {
            return mCode;
        }

        constexpr Iterator& operator++() {
            if (!(mInput != mEnd)) {
                atEnd = true;
            }
            uint8_t inputChar = *(mInput++);

            int onesCount = std::countl_one(inputChar);
            if(onesCount == 1 || onesCount > 4) {
                mCode = inputChar;
                return *this;
            }
            // the resulting unicode codepoint
            mCode = (0b01111111 >> onesCount) & inputChar;
            while ((mInput != mEnd) && (--onesCount > 0)) {
                mCode <<= 6;
                inputChar = *(mInput++);
                mCode |= inputChar & 0b00111111;
            }
            return *this;
        }

        constexpr bool operator!=(const Sentinel&) const {
            return !atEnd;
        }
    };

    constexpr Iterator begin() const {
        auto it = Iterator{
            .mInput = ranges_at_home::begin(inputRange),
            .mEnd = ranges_at_home::end(inputRange),
        };
        ++it;
        return it;
    }

    constexpr Sentinel end() const {
        return {};
    }
};

namespace CharacterRom {

struct PET {
    compiletime const std::string_view Filename = "CharacterRomPET8x8x256.bin";

    constant size_t ByteSize = 2048;

    compiletime std::array<uint8_t, 256> CharacterTable = []() {
        std::array<uint8_t, 256> result{};
        for (size_t i = 0; i < result.size(); ++i) result[i] = 0xFF;// map any unknown to checkerboard

        for (char c = ' '; c <= '?'; ++c) {
            result[c] = c;
        }
        for (char c = '@'; c <= ']'; ++c) {
            result[c] = c - '@';
        }

        for (char c = 'a'; c <= 'z'; ++c) {
            result[c] = c + 0x60;
        }

        return result;
    }();

    enum class SpecialCharacters : uint8_t {
        ArrowUp = 0x1e,
        ArrowLeft = 0x1f,
        HLineY3 = 0x40,
        HLine = HLineY3,
        SuitSpade = 0x41,
        VLineX3 = 0x42,
        HLineY4 = 0x43,
        HLineY5 = 0x44,
        HLineY6 = 0x45,
        HLineY2 = 0x46,
        VLineX2 = 0x47,
        VLineX5 = 0x48,
        ArcDownLeft = 0x49,
        ArcUpRight = 0x4a,
        ArcUpLeft = 0x4b,
        LineCornerLowerLeft = 0x4c,
        LineUpLeftDownRight = 0x4d,
        LineDownLeftUpRight = 0x4e,
        LineCornerUpperLeft = 0x4f,
        LineCornerUpperRight = 0x50,
        Bullet = 0x51,
        HLineY1 = 0x52,
        SuitHeart = 0x53,
        VLineX1 = 0x54,
        ArcDownRight = 0x55,
        LineCrossDiag = 0x56,
        Circle = 0x57,
        SuitClub = 0x58,
        VLineX6 = 0x59,
        SuitDiamond = 0x5a,
        LineCross = 0x5b,
        VLineX4 = 0x5d,
        VLine = VLineX4,
        Pi = 0x5e,
        HLineY7 = 0x63,
        HLineY0 = 0x64,
        VLineX0 = 0x65,
        HalfTone = 0x66,
        VLineX7 = 0x67,
        LineCornerLowerRight = 0x7a,
    };

    compiletime std::optional<uint8_t> CharacterForCodepoint(uint32_t codePoint) {
        // PET 0x20 - 0x3F is equal to ascii
        if (codePoint >= ' ' && codePoint <= '?') {
            return static_cast<uint8_t>(codePoint);
        }
        // Ascii Uppercase is at beginning of character rom
        if (codePoint >= '@' && codePoint <= ']') {
            return  static_cast<uint8_t>(codePoint - '@');
        }
        // Ascii lowercase (0x60 - 0x7A) is in 0xC0 - 0xDA of character rom
        if (codePoint >= 'a' && codePoint <= 'z') {
            return  static_cast<uint8_t>(codePoint + 0x60);
        }

        using enum SpecialCharacters;
        using namespace Unicode;
        auto cp = static_cast<Codepoints>(codePoint);

        switch (cp) {
            case Codepoints::VerticalLine:
            case Codepoints::BoxDrawingsLightVertical:
            case Codepoints::BoxDrawingsHeavyVertical:
                return static_cast<uint8_t>(VLine);

            case Codepoints::BoxDrawingsLightHorizontal:
            case Codepoints::BoxDrawingsHeavyHorizontal:
                return static_cast<uint8_t>(HLine);

            case Codepoints::BoxDrawingsLightDiagonalCross:
                return static_cast<uint8_t>(LineCrossDiag);
            case Codepoints::BoxDrawingsLightVerticalAndHorizontal:
            case Codepoints::BoxDrawingsHeavyVerticalAndHorizontal:
                return static_cast<uint8_t>(LineCross);

            case Codepoints::BoxDrawingsLightArcUpAndLeft:
                return static_cast<uint8_t>(ArcUpLeft);
            case Codepoints::BoxDrawingsLightArcUpAndRight:
                return static_cast<uint8_t>(ArcUpRight);
            case Codepoints::BoxDrawingsLightArcDownAndLeft:
                return static_cast<uint8_t>(ArcDownLeft);
            case Codepoints::BoxDrawingsLightArcDownAndRight:
                return static_cast<uint8_t>(ArcDownRight);

            case Codepoints::LowLine:
            case Codepoints::LowerOneEighthBlock:
                return static_cast<uint8_t>(HLineY0);
            case Codepoints::UpperOneEigthBlock:
                return static_cast<uint8_t>(HLineY7);
            case Codepoints::Bullet:
            case Codepoints::MiddleDot:
            case Codepoints::BlackCircle:
            case Codepoints::BulletOperator:
                return static_cast<uint8_t>(Bullet);
            case Codepoints::GreekSmallLetterPi:
                return static_cast<uint8_t>(Pi);
            case Codepoints::WhiteCircle:
            case Codepoints::RingOperator:
            case Codepoints::WhiteBullet:
            case Codepoints::LargeCircle:
                return static_cast<uint8_t>(Circle);
            case Codepoints::UpwardsArrow:
                return static_cast<uint8_t>(ArrowUp);
            case Codepoints::LeftwardsArrow:
                return static_cast<uint8_t>(ArrowLeft);
            case Codepoints::BlackSpadeSuit:
                return static_cast<uint8_t>(SuitSpade);
            case Codepoints::HeavyBlackHeart:
            case Codepoints::BlackHeartSuit:
                return static_cast<uint8_t>(SuitHeart);
            case Codepoints::BlackClubSuit:
                return static_cast<uint8_t>(SuitClub);
            case Codepoints::BlackDiamond:
            case Codepoints::BlackDiamondSuit:
                return static_cast<uint8_t>(SuitDiamond);
            default:
                return {};
        }
    }

};

}
