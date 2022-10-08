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

enum Codepoints : char32_t {
    // Basic Latin
    Null = 0x0000,
    StartOfHeading = 0x0001,
    StartOfText = 0x0002,
    EndOfText = 0x0003,
    EndOfTransmission = 0x0004,
    Enquiry = 0x0005,
    Acknowledge = 0x0006,
    Bell = 0x0007,
    Backspace = 0x0008,
    CharacterTabulation = 0x0009,
    HorizontalTabulation = CharacterTabulation,
    Tab = CharacterTabulation,
    LineFeed = 0x000a,
    LineTabulation = 0x000b,
    VerticalTabulation = LineTabulation,
    FormFeed = 0x000c,
    CarriageReturn = 0x000d,
    ShiftOut = 0x000e,
    ShiftIn = 0x000f,

    DataLinkEscape = 0x0010,
    DeviceControlOne = 0x0011,
    DeviceControlTwo = 0x0012,
    DeviceControlThree = 0x0013,
    DeviceControlFour = 0x0014,
    NegativeAcknowledge = 0x0015,
    SynchronousIdle = 0x0016,
    EndOfTransmissionBlock = 0x0017,
    Cancel = 0x0018,
    EndOfMedium = 0x0019,
    Substitute = 0x001a,
    Escape = 0x001b,
    InformationSeparatorFour = 0x001c,
    FileSeparator = InformationSeparatorFour,
    InformationSeparatorThree = 0x001d,
    GroupSeparator = InformationSeparatorThree,
    InformationSeparatorTwo = 0x001e,
    RecordSeparator = InformationSeparatorTwo,
    InformationSeparatorOne = 0x001f,
    UnitSeparator = InformationSeparatorOne,

    Space = 0x0020,
    ExclamationMark = 0x0021,
    QuotationMark = 0x0022,
    NumberSign = 0x0023,
    Pound = NumberSign,
    Hash = NumberSign,
    Crosshatch = NumberSign,
    Octothorpe = NumberSign,
    DollarSign = 0x0024,
    PercentSign = 0x0025,
    Ampersand = 0x0026,
    Apostrophe = 0x0027,
    LeftParenthesis = 0x0028,
    RightParenthesis = 0x0029,
    Asterisk = 0x002a,
    PlusSign = 0x002b,
    Comma = 0x002c,
    HyphenMinus = 0x002d,
    FullStop = 0x002e,
    Period = FullStop,
    Dot = FullStop,
    DecimalPoint = FullStop,
    Solidus = 0x002f,
    Slash = Solidus,

    DigitZero = 0x0030,
    DigitOne = 0x0031,
    DigitTwo = 0x0032,
    DigitThree = 0x0033,
    DigitFour = 0x0034,
    DigitFive = 0x0035,
    DigitSix = 0x0036,
    DigitSeven = 0x0037,
    DigitEight = 0x0038,
    DigitNine = 0x0039,
    Colon = 0x003a,
    Semicolon = 0x003b,
    LessThanSign = 0x003c,
    EqualsSign = 0x003d,
    GreaterThanSign = 0x003e,
    QuestionMark = 0x003f,

    CommercialAt = 0x0040,
    AtSign = CommercialAt,
    LatinCapitalLetterA = 0x0041,
    // ...
    LatinCapitalLetterZ = 0x005a,
    LeftSquareBracket = 0x005b,
    ReverseSolidus = 0x005c,
    Backslash = ReverseSolidus,
    RightSquareBracket = 0x005d,
    CircumflexAccent = 0x005e,
    LowLine = 0x005f,
    SpacingUnderscore = LowLine,

    GraveAccent = 0x0060,
    LatinSmallLetterA = 0x0061,
    // ...
    LatinSmallLetterZ = 0x007a,
    LeftCurlyBracket = 0x007b,
    VerticalLine = 0x007c,
    VerticalBar = VerticalLine,
    RightCurlyBracket = 0x007d,
    Tilde = 0x007e,
    Delete = 0x007f,

    // end of Basic Latin

    // Latin-1 Supplement
    PaddingCharacter = 0x0080,
    HighOctetPreset = 0x0081,
    BreakPermittedHere = 0x0082,
    NoBreakHere = 0x0083,
    IND = 0x0084,
    NextLine = 0x0085,
    StartOfSelectedArea = 0x0086,
    EndOfSelectedArea = 0x0087,
    CharacterTabulationSet = 0x0088,
    CharacterTabulationWithJustification = 0x0089,
    LineTabulationSet = 0x008a,
    PartialLineForward = 0x008b,
    PartialLineBackward = 0x008c,
    ReverseLineFeed = 0x008d,
    SingleShiftTwo = 0x008e,
    SingleShiftThree = 0x008f,

    DeviceControlString = 0x0090,
    PrivateUseOne = 0x0091,
    PrivateUseTwo = 0x0092,
    SetTransmitState = 0x0093,
    CancelCharacter = 0x0094,
    MessageWaiting = 0x0095,
    StartOfGuardedArea = 0x0096,
    EndOfGuardedArea = 0x0097,
    StartOfString = 0x0098,
    SingleGraphicCharacterIntroducer = 0x0099,
    SingleCharacterIntroducer = 0x009a,
    ControlSequenceIntroducer = 0x009b,
    StringTerminator = 0x009c,
    OperatingSystemCommand = 0x009d,
    PrivacyMessage = 0x009e,
    ApplicationProgramCommand = 0x009f,

    NoBreakSpace = 0x00a0,
    InvertedExclamationMark = 0x00a1,
    CentSign = 0x00a2,
    PoundSign = 0x00a3,
    PoundSterling = PoundSign,
    CurrencySign = 0x00a4,
    YenSign = 0x00a5,
    BrokenBar = 0x00a6,
    SectionSign = 0x00a7, // §
    Diaeresis = 0x00a8,
    CopyrightSign = 0x00a9,
    FeminineOrdinalIndicator = 0x00aa, // ª
    LeftPointingDoubleAngleQuotationMark = 0x00ab,
    LeftGuillemet = LeftPointingDoubleAngleQuotationMark,
    NotSign = 0x00ac,
    AngledDash = NotSign,
    SoftHyphen = 0x00ad,
    RegisteredSign = 0x00ae,
    RegisteredTrademarkSign = RegisteredSign,
    Macron = 0x00af,
    Overline = Macron,

    DegreeSign = 0x00b0, // °
    PlusMinusSign = 0x00b1,
    SuperscriptTwo = 0x00b2,
    SuperscriptThree = 0x00b3,
    AcuteAccent = 0x00b4,
    MicroSign = 0x00b5,
    PilcrowSign = 0x00b6,
    ParagraphSign = PilcrowSign,
    MiddleDot = 0x00b7,
    Cedilla = 0x00b8,
    SuperscriptOne = 0x00b9,
    MasculineOrdinalIndicator = 0x00ba, // º
    RightPointingDoubleAngleQuotationMark = 0x00bb,
    RightGuillemet = RightPointingDoubleAngleQuotationMark,
    VulgarFractionOneQuarter = 0x00bc, // ¼
    VulgarFractionOneHalf = 0x00bd, // ½
    VulgarFractionThreeQuarters = 0x00be, // ¾
    InvertedQuestionMark = 0x00bf,
    

    GreekSmallLetterPi = 0x03c0,

    // General Punctuation U+2000 - U+206F
    Bullet = 0x2022,
    Interrobang = 0x203d,

    // Arrows U+2190 - U+21FF
    LeftwardsArrow = 0x2190,
    UpwardsArrow = 0x2191,
    RightwardsArrow = 0x2192,
    DownwardsArrow = 0x2193,
    LeftRightArrow = 0x2194,
    UpDownArrow = 0x2195,

    // Mathematical Operators U+2200 - U+22FF
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

    // Private Use U+E000 - U+F8FF
    
    AppleMacBookArrowUp = 0xF700,
    AppleMacBookArrowDown = 0xF701,
    AppleMacBookArrowLeft = 0xF702,
    AppleMacBookArrowRight = 0xF703,
    // macbook fn + backspace = delete?
    AppleMacBookDelete = 0xF728
};

template <ranges_at_home::aRangeOf<char8_t> T>
struct CodepointsView {
    const T& inputRange;
    using InputIterator = ranges_at_home::iterator_t<T>;
    using InputSentinel = ranges_at_home::sentinel_t<T>;

    struct Sentinel {};

    struct Iterator {
        InputIterator mInput;
        InputSentinel mEnd;
        char32_t mCode{ 0 };
        int mLength{ 0 };

        constexpr char32_t operator*() const {
            return mCode;
        }

        constexpr Iterator& operator++() {
            while ((mInput != mEnd) && (mLength > 0)) {
                mInput++;
                mLength--;
            }
            updateValue();
            return *this;
        }

        constexpr bool operator!=(const Sentinel&) const {
            return mLength > 0 && mInput != mEnd;
        }

        constexpr void updateValue() {
            if (mInput != mEnd) {
                auto inputCopy = mInput;
                uint8_t inputChar = *(inputCopy++);

                int onesCount = std::countl_one(inputChar);
            
                // the length of the code point in the utf 8 stream
                mLength = 1;
                if (onesCount == 1 || onesCount > 4) {
                    // somethings a bit fishy about this string, it's invalid utf8
                    mCode = inputChar;
                    return;
                }
                // the resulting unicode codepoint
                mCode = (0b01111111 >> onesCount) & inputChar;
                while ((inputCopy != mEnd) && (--onesCount > 0)) {
                    mCode <<= 6;
                    mLength++;
                    inputChar = *(inputCopy++);
                    mCode |= inputChar & 0b00111111;
                }

            }
            else
            {
                mLength = 0;
                mCode = 0;
            }
        }

    };

    constexpr Iterator begin() const {
        auto it = Iterator{
            .mInput = ranges_at_home::begin(inputRange),
            .mEnd = ranges_at_home::end(inputRange),
        };
        it.updateValue();
        return it;
    }

    constexpr Sentinel end() const {
        return {};
    }
};

}
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

    compiletime std::optional<uint8_t> CharacterForCodepoint(char32_t codePoint) {
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
