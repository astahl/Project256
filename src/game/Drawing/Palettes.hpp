//
//  Palettes.cpp
//  Project256
//
//  Created by Andreas Stahl on 04.06.22.
//
#pragma once

#include <cstdint>
#include <array>
#include <algorithm>
#include <limits>
#include <vector>

#include "../defines.h"


using ColorARGB = uint32_t;

template <typename T>
struct ColorComponents {
    T b, g, r, a;
};

using Color4i = ColorComponents<uint8_t>;
using Color4f = ColorComponents<float>;


template <typename T>
ColorComponents<T> operator-(const ColorComponents<T>& l, const ColorComponents<T>& r)
{
    return {
        .b = l.b - r.b,
        .g = l.g - r.g,
        .r = l.r - r.r,
        .a = l.a - r.a,
    };
}

enum class ColorMask : ColorARGB {
    Alpha = 0xFF'00'00'00,
    Red = 0x00'FF'00'00,
    Green = 0x00'00'FF'00,
    Blue = 0x00'00'00'FF,
};

enum class ColorOffset : uint8_t {
    Alpha = 24,
    Red = 16,
    Green = 8,
    Blue = 0,
};

template<ColorMask Mask, ColorOffset Offset>
compiletime uint8_t getComponent(ColorARGB color) {
    return static_cast<uint8_t>((static_cast<uint32_t>(color) & static_cast<uint32_t>(Mask)) >> static_cast<uint8_t>(Offset));
}

enum class WebColorRGB : ColorARGB {
    //Pink colors
    MediumVioletRed = 0xC7'15'85,
    DeepPink = 0xFF'14'93,
    PaleVioletRed = 0xDB'70'93,
    HotPink = 0xFF'69'B4,
    LightPink = 0xFF'B6'C1,
    Pink = 0xFF'C0'CB,
    // Red colors
    DarkRed = 0x8B'00'00,
    Red = 0xFF'00'00,
    Firebrick = 0xB2'22'22,
    Crimson = 0xDC'14'3C,
    IndianRed = 0xCD'5C'5C,
    LightCoral = 0xF0'80'80,
    Salmon = 0xFA'80'72,
    DarkSalmon = 0xE9'96'7A,
    LightSalmon = 0xFF'A0'7A,
    // Orange colors
    OrangeRed = 0xFF'45'00,
    Tomato = 0xFF'63'47,
    DarkOrange = 0xFF'8C'00,
    Coral = 0xFF'7F'50,
    Orange = 0xFF'A5'00,
    // Yellow colors
    DarkKhaki = 0xBD'B7'6B,
    Gold = 0xFF'D7'00,
    Khaki = 0xF0'E6'8C,
    PeachPuff = 0xFF'DA'B9,
    Yellow = 0xFF'FF'00,
    PaleGoldenrod = 0xEE'E8'AA,
    Moccasin = 0xFF'E4'B5,
    PapayaWhip = 0xFF'EF'D5,
    LightGoldenrodYellow = 0xFA'FA'D2,
    LemonChiffon = 0xFF'FA'CD,
    LightYellow = 0xFF'FF'E0,
    // Brown colors
    Maroon = 0x80'00'00,
    Brown = 0xA5'2A'2A,
    SaddleBrown = 0x8B'45'13,
    Sienna = 0xA0'52'2D,
    Chocolate = 0xD2'69'1E,
    DarkGoldenrod = 0xB8'86'0B,
    Peru = 0xCD'85'3F,
    RosyBrown = 0xBC'8F'8F,
    Goldenrod = 0xDA'A5'20,
    SandyBrown = 0xF4'A4'60,
    Tan = 0xD2'B4'8C,
    Burlywood = 0xDE'B8'87,
    Wheat = 0xF5'DE'B3,
    NavajoWhite = 0xFF'DE'AD,
    Bisque = 0xFF'E4'C4,
    BlanchedAlmond = 0xFF'EB'CD,
    Cornsilk = 0xFF'F8'DC,
    // Green colors
    DarkGreen = 0x00'64'00,
    Green = 0x00'80'00,
    DarkOliveGreen = 0x55'6B'2F,
    ForestGreen = 0x22'8B'22,
    SeaGreen = 0x2E'8B'57,
    Olive = 0x80'80'00,
    OliveDrab = 0x6B'8E'23,
    MediumSeaGreen = 0x3C'B3'71,
    LimeGreen = 0x32'CD'32,
    Lime = 0x00'FF'00,
    SpringGreen = 0x00'FF'7F,
    MediumSpringGreen = 0x00'FA'9A,
    DarkSeaGreen = 0x8F'BC'8F,
    MediumAquamarine = 0x66'CD'AA,
    YellowGreen = 0x9A'CD'32,
    LawnGreen = 0x7C'FC'00,
    Chartreuse = 0x7F'FF'00,
    LightGreen = 0x90'EE'90,
    GreenYellow = 0xAD'FF'2F,
    PaleGreen = 0x98'FB'98,
    // Cyan colors
    Teal = 0x00'80'80,
    DarkCyan = 0x00'8B'8B,
    LightSeaGreen = 0x20'B2'AA,
    CadetBlue = 0x5F'9E'A0,
    DarkTurquoise = 0x00'CE'D1,
    MediumTurquoise = 0x48'D1'CC,
    Turquoise = 0x40'E0'D0,
    Aqua = 0x00'FF'FF,
    Cyan = 0x00'FF'FF,
    Aquamarine = 0x7F'FF'D4,
    PaleTurquoise = 0xAF'EE'EE,
    LightCyan = 0xE0'FF'FF,
    // Blue colors
    Navy = 0x00'00'80,
    DarkBlue = 0x00'00'8B,
    MediumBlue = 0x00'00'CD,
    Blue = 0x00'00'FF,
    MidnightBlue = 0x19'19'70,
    RoyalBlue = 0x41'69'E1,
    SteelBlue = 0x46'82'B4,
    DodgerBlue = 0x1E'90'FF,
    DeepSkyBlue = 0x00'BF'FF,
    CornflowerBlue = 0x64'95'ED,
    SkyBlue = 0x87'CE'EB,
    LightSkyBlue = 0x87'CE'FA,
    LightSteelBlue = 0xB0'C4'DE,
    LightBlue = 0xAD'D8'E6,
    PowderBlue = 0xB0'E0'E6,
    //Purple, violet, and magenta colors
    Indigo = 0x4B'00'82,
    Purple = 0x80'00'80,
    DarkMagenta = 0x8B'00'8B,
    DarkViolet = 0x94'00'D3,
    DarkSlateBlue = 0x48'3D'8B,
    BlueViolet = 0x8A'2B'E2,
    DarkOrchid = 0x99'32'CC,
    Fuchsia = 0xFF'00'FF,
    Magenta = 0xFF'00'FF,
    SlateBlue = 0x6A'5A'CD,
    MediumSlateBlue = 0x7B'68'EE,
    MediumOrchid = 0xBA'55'D3,
    MediumPurple = 0x93'70'DB,
    Orchid = 0xDA'70'D6,
    Violet = 0xEE'82'EE,
    Plum = 0xDD'A0'DD,
    Thistle = 0xD8'BF'D8,
    Lavender = 0xE6'E6'FA,
    //White colors
    MistyRose = 0xFF'E4'E1,
    AntiqueWhite = 0xFA'EB'D7,
    Linen = 0xFA'F0'E6,
    Beige = 0xF5'F5'DC,
    WhiteSmoke = 0xF5'F5'F5,
    LavenderBlush = 0xFF'F0'F5,
    OldLace = 0xFD'F5'E6,
    AliceBlue = 0xF0'F8'FF,
    Seashell = 0xFF'F5'EE,
    GhostWhite = 0xF8'F8'FF,
    Honeydew = 0xF0'FF'F0,
    FloralWhite = 0xFF'FA'F0,
    Azure = 0xF0'FF'FF,
    MintCream = 0xF5'FF'FA,
    Snow = 0xFF'FA'FA,
    Ivory = 0xFF'FF'F0,
    White = 0xFF'FF'FF,
    // Gray and black colors
    Black = 0x00'00'00,
    DarkSlateGray = 0x2F'4F'4F,
    DimGray = 0x69'69'69,
    SlateGray = 0x70'80'90,
    Gray = 0x80'80'80,
    LightSlateGray = 0x77'88'99,
    DarkGray = 0xA9'A9'A9,
    Silver = 0xC0'C0'C0,
    LightGray = 0xD3'D3'D3,
    Gainsboro = 0xDC'DC'DC,
};


// uses SMPTE 240M conversion
compiletime ColorARGB YCbCrToARGB(double Y, double Cb, double Cr) {
    double KB = 0.087;
    double KR = 0.212;
    double KG = 1 - KB - KR;

    double R = Y + 0 * Cb + (2 - 2 * KR) * Cr;
    double G = Y + (-KB / KG * (2 - 2 * KB)) * Cb + (-KR / KG *  (2 - 2 * KR)) * Cr;
    double B = Y + (2 - 2 * KB) * Cb + 0 * Cr;

    return 0xFF'00'00'00 |
    std::clamp(static_cast<int>(256 * R), 0, 255) << 16 |
    std::clamp(static_cast<int>(256 * G), 0, 255) << 8 |
    std::clamp(static_cast<int>(256 * B), 0, 255);
}

compiletime ColorARGB makeARGB(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
{
    return static_cast<uint32_t>(alpha) << 24 |
    static_cast<uint32_t>(red) << 16 |
    static_cast<uint32_t>(green) << 8 | blue;
}

compiletime ColorARGB makeARGB(double red, double green, double blue, double alpha = 1.0)
{
    return static_cast<uint32_t>(alpha * 255) << 24 |
    static_cast<uint32_t>(red * 255) << 16 |
    static_cast<uint32_t>(green * 255) << 8 | static_cast<uint32_t>(blue * 255);
}

compiletime Color4i toColor4i(ColorARGB color) {
    return {
        .b = getComponent<ColorMask::Blue, ColorOffset::Blue>(color),
        .g = getComponent<ColorMask::Green, ColorOffset::Green>(color),
        .r = getComponent<ColorMask::Red, ColorOffset::Red>(color),
        .a = getComponent<ColorMask::Alpha, ColorOffset::Alpha>(color)
    };
}

compiletime Color4f toFP32(ColorARGB color) {
    return {
        .b = static_cast<float>(getComponent<ColorMask::Blue, ColorOffset::Blue>(color)) / 255.0f,
        .g = static_cast<float>(getComponent<ColorMask::Green, ColorOffset::Green>(color)) / 255.0f,
        .r = static_cast<float>(getComponent<ColorMask::Red, ColorOffset::Red>(color)) / 255.0f,
        .a = static_cast<float>(getComponent<ColorMask::Alpha, ColorOffset::Alpha>(color)) / 255.0f,
    };
}

template<uint8_t InputBitDepthPerChannel>
compiletime ColorARGB expandTo8Bit(uint32_t inputRGB, uint8_t alpha = 255)
{
    constexpr uint32_t componentMask = 0xFF & ~(0xFF << InputBitDepthPerChannel);
    double blue = static_cast<double>(inputRGB & componentMask) / componentMask;
    double green = static_cast<double>((inputRGB >> InputBitDepthPerChannel) & componentMask) / componentMask;
    double red = static_cast<double>((inputRGB >> (2*InputBitDepthPerChannel)) & componentMask) / componentMask;
    return makeARGB(red, green, blue, static_cast<double>(alpha) / 255);
}

template<uint8_t OutputBitDepthPerChannel>
compiletime ColorARGB quantizeToXBit(uint32_t inputRGB, uint8_t alpha = 255)
{
    constexpr int moveDownBy = 8 - OutputBitDepthPerChannel;
    uint8_t blue = ((inputRGB) & 0xFF) >> moveDownBy;
    uint8_t green = ((inputRGB >> 8) & 0xFF) >> moveDownBy;
    uint8_t red = ((inputRGB >> 16) & 0xFF) >> moveDownBy;
    return makeARGB(red, green, blue, alpha);
}

template <typename T>
struct FindNearestResult {
    T index;
    ColorARGB argb;
    float deltaSquared;
};

template <typename TColor, typename TColorSpace, typename TIndex = typename TColorSpace::difference_type>
compiletime FindNearestResult<TIndex>
findNearest(TColor color, const TColorSpace& colorSpace) {
    FindNearestResult<TIndex> result{ .deltaSquared = std::numeric_limits<float>::infinity() };
    Color4f a = toFP32(static_cast<ColorARGB>(color));
    TIndex index{};
    for (const ColorARGB candidateColor : colorSpace) {
        Color4f b = toFP32(candidateColor);
        Color4f c = a - b;
        float deltaSquared = (c.a * c.a + c.r * c.r + c.g * c.g + c.b * c.b);
        if (deltaSquared < result.deltaSquared) {
            result = {
                .index = index,
                .argb = candidateColor,
                .deltaSquared = deltaSquared,
            };
        }
        ++index;
    }
    return result;
}

union ColorArgbWrapper {
    Color4i components;
    ColorARGB value;
};

constexpr ColorArgbWrapper operator+(ColorArgbWrapper left, ColorArgbWrapper right)
{
    return ColorArgbWrapper {
        .components {
            .b = static_cast<uint8_t>(std::clamp(left.components.b + right.components.b, 0, 255)),
            .g = static_cast<uint8_t>(std::clamp(left.components.g + right.components.g, 0, 255)),
            .r = static_cast<uint8_t>(std::clamp(left.components.r + right.components.r, 0, 255)),
            .a = static_cast<uint8_t>(std::clamp(left.components.a + right.components.a, 0, 255)),
         }
    };
}

constexpr ColorArgbWrapper operator-(ColorArgbWrapper left, ColorArgbWrapper right)
{
    return ColorArgbWrapper {
        .components {
            .b = static_cast<uint8_t>(std::clamp(left.components.b - right.components.b, 0, 255)),
            .g = static_cast<uint8_t>(std::clamp(left.components.g - right.components.g, 0, 255)),
            .r = static_cast<uint8_t>(std::clamp(left.components.r - right.components.r, 0, 255)),
            .a = static_cast<uint8_t>(std::clamp(left.components.a - right.components.a, 0, 255)),
        }
    };
}

constexpr ColorArgbWrapper shiftRightMult(ColorArgbWrapper color, int shift, int mult)
{
    return ColorArgbWrapper {
        .components {
            .b = static_cast<uint8_t>((color.components.b * mult >> shift)),
            .g = static_cast<uint8_t>((color.components.g * mult >> shift)),
            .r = static_cast<uint8_t>((color.components.r * mult >> shift)),
            .a = static_cast<uint8_t>((color.components.a * mult >> shift)),
        }
    };
}

template <int Width, typename TColorSpace, typename T, bool Dither = true>
compiletime void ConvertBitmapFrom32BppToIndex(const ColorARGB* source, int width, int height, const TColorSpace& colorSpace, T* destination) {
    if constexpr (Dither) {
        constexpr int W = Width + 2;
        ColorArgbWrapper errors[(W) * 2]{};
        const ColorArgbWrapper* src = reinterpret_cast<const ColorArgbWrapper*>(source);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {

                ColorArgbWrapper sourceColor = src[x + y * width];
                ColorArgbWrapper errorAtSource = errors[x + 1];
                ColorArgbWrapper sourceWithError = sourceColor + errorAtSource;
                auto nearest = findNearest(sourceWithError.value, colorSpace);
                destination[x + y * width] = static_cast<T>(nearest.index);

                ColorArgbWrapper written{.value = nearest.argb};
                auto error = sourceWithError - written;
                errors[x + 2] = shiftRightMult(error, 4, 7) + errors[x + 2];
                errors[x + W] = shiftRightMult(error, 4, 3) + errors[x + W];
                errors[x + 1 + W] = shiftRightMult(error, 4, 5) + errors[x + 1 + W];
                errors[x + 2 + W] = shiftRightMult(error, 4, 1) + errors[x + 2 + W];
            }
            for (int x = 0; x < W; ++x) {
                errors[x] = errors[x + W];
                errors[x + W] = ColorArgbWrapper{};
            }
        }
    } else {
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x) {
                ColorARGB sourceColor = *(source++);
                auto nearest = findNearest(sourceColor, colorSpace);
                *(destination++) = nearest.index;
            }
    }
}



struct PaletteAppleII {

    using index_type = uint8_t;
    compiletime size_t count = 16;

    enum class Color : uint8_t {
        black, magenta, darkBlue, purple, darkGreen, grey1, mediumBlue,
        lightBlue, brown, orange, grey2, pink, green, yellow, aqua, white
    };

    compiletime std::array<uint32_t,16> colors = {
        YCbCrToARGB(0, 0, 0),
        YCbCrToARGB(0.25, 0, 0.5),
        YCbCrToARGB(0.25, 0.5, 0),
        YCbCrToARGB(0.5, 1, 1),
        YCbCrToARGB(0.25, 0, -0.5),
        YCbCrToARGB(0.5, 0, 0),
        YCbCrToARGB(0.5, 1, -1),
        YCbCrToARGB(0.75, 0.5, 0),
        YCbCrToARGB(0.25, -0.5, 0),
        YCbCrToARGB(0.5, -1, 1),
        YCbCrToARGB(0.5, 0, 0),
        YCbCrToARGB(0.75, 0, 0.5),
        YCbCrToARGB(0.5, -1, -1),
        YCbCrToARGB(0.75, -0.5, 0),
        YCbCrToARGB(0.75, 0, -0.5),
        YCbCrToARGB(1, 0, 0),
    };

    compiletime void writeTo(uint32_t* destination) {
        for (size_t i = 0; i < count; ++i) {
            destination[i] = colors[i];
        }
    }
};


struct PaletteHercules {
    compiletime size_t count = 2;

    enum class Color : uint8_t {
        black, white
    };

    compiletime std::array<uint32_t,2> colors = {
        0xFF000000,
        0xFFFFFFFF
    };

    compiletime void writeTo(uint32_t* destination) {
        for (size_t i = 0; i < count; ++i) {
            destination[i] = colors[i];
        }
    }
};

struct PaletteGameboy {
    compiletime size_t count = 4;

    compiletime std::array<uint32_t,4> colors = {
        0xFF000000,
        0xFF565656,
        0xFFACACAC,
        0xFFFFFFFF
    };

    compiletime void writeTo(uint32_t* destination) {
        for (size_t i = 0; i < count; ++i) {
            destination[i] = colors[i];
        }
    }
};



compiletime uint32_t egaTo8Bit(uint8_t input) {
    uint8_t swizzledTo2bpc =
    (input & 0b000001) << 1 |
    (input & 0b000010) << 2 |
    (input & 0b000100) << 3 |
    (input & 0b001000) >> 3 |
    (input & 0b010000) >> 2 |
    (input & 0b100000) >> 1 ;

    return expandTo8Bit<2>(swizzledTo2bpc);
}

struct PaletteEGA {
    compiletime size_t count = 16;

    enum class Color : uint8_t {
        black, blue, green, cyan, red, magenta, brown, lightGray,
        darkGray, lightBlue, lightGreen, lightCyan, lightRed, lightMagenta, yellow, white
    };

    compiletime std::array<uint32_t, 16> colors = [](){
        std::array<uint32_t, 16> result{};
        uint8_t pos = 0;
        for(auto colorSpaceIndex : {0, 1, 2, 3, 4, 5, 20, 7, 56, 57, 58, 59, 60, 61, 62, 63})
        {
            result[pos++] = egaTo8Bit(static_cast<uint8_t>(colorSpaceIndex));
        }
        return result;
    }();

    compiletime void writeTo(uint32_t* destination) {
        for (size_t i = 0; i < count; ++i) {
            destination[i] = colors[i];
        }
    }
};


template <typename T>
constexpr T lerp(int t, int steps, T from, T to) {
    return from + (t * (to - from)) / steps;
}

struct PaletteVGA {
    compiletime size_t count = 256;

    compiletime std::array<uint32_t, 256> colors =  [](){
        std::array<uint32_t, 256> result{};
        uint8_t pos = 0;
        for(auto colorSpaceIndex : {0, 1, 2, 3, 4, 5, 20, 7, 56, 57, 58, 59, 60, 61, 62, 63})
        {
            result[pos++] = egaTo8Bit(static_cast<uint8_t>(colorSpaceIndex));
        }
        for(auto grayValue : { 0x00, 0x10, 0x20, 0x35, 0x45, 0x55, 0x65, 0x75, 0x8A, 0x9A, 0xAA, 0xBA, 0xCA, 0xDF, 0xEF, 0xFF }) {
            result[pos++] = 0x010101 * grayValue | 0xFF000000;
        }
        double red[6] = {0.0, 0.917, 0.917, 1.0, 0.44, 0.44};
        double green[6] = {0.0, 0.2, 0.2, 1.0, 1.0, 1.0};
        double blue[6] = {1.0, 1.0, 0.14, 0.33, 0.3, 1.0};

        double ranges[] = {
            0.0, 1.0, 0.5, 1.0, 0.75, 1.0,
            0.0, 0.44, 0.22, 0.44, 0.33, 0.44,
            0.0, 0.25, 0.13, 0.25, 0.18, 0.25,
        };
        for (int rangeIdx = 0; rangeIdx < 9; ++rangeIdx) {
            double spread = ranges[2 * rangeIdx + 1] - ranges[2 * rangeIdx];
            double bias = ranges[2 * rangeIdx];

            // this is still a bit buggy, off by one errors probably :(
            for(int i = 0; i < 24; ++i) {
                double r{}, g{}, b{};
                int from{}, to{}, steps{1}, offset{};
                if (i < 4) {
                    steps = 4; offset = 0;
                    from = 0; to = 1;
                } else if (i < 8) {
                    steps = 4; offset = 4;
                    from = 1; to = 2;
                } else if (i < 12) {
                    steps = 4; offset = 8;
                    from = 2; to = 3;
                } else if (i < 16) {
                    steps = 4; offset = 12;
                    from = 3; to = 4;
                } else if (i < 20) {
                    steps = 4; offset = 16;
                    from = 4; to = 5;
                } else {
                    steps = 4; offset = 20;
                    from = 5; to = 0;
                }

                r = lerp(i - offset, steps, red[from], red[to]);
                g = lerp(i - offset, steps, green[from], green[to]);
                b = lerp(i - offset, steps, blue[from], blue[to]);

                r = r * spread + bias;
                g = g * spread + bias;
                b = b * spread + bias;

                result[pos++] = expandTo8Bit<6>(
                static_cast<uint32_t>(r * 0b111111) << 12
                | static_cast<uint32_t>(g * 0b111111) << 6
                | static_cast<uint32_t>(b * 0b111111));
            }
        }

        while (pos) {
            result[pos++] = 0xFF000000;
        }

        return result;
    }();


    compiletime void writeTo(uint32_t* destination) {
        for (size_t i = 0; i < count; ++i) {
            destination[i] = colors[i];
        }
    }
};


struct PaletteCGA {
    using index_type = uint8_t;

    enum class Color : uint8_t {
        black, blue, green, cyan, red, magenta, brown, lightGray,
        darkGray, lightBlue, lightGreen, lightCyan, lightRed, lightMagenta, yellow, white
    };

    enum class Mode {
        Mode4Palette0LowIntensity,
        Mode4Palette0HighIntensity,
        Mode4Palette1LowIntensity,
        Mode4Palette1HighIntensity,
        Mode5LowIntensity,
        Mode5HighIntensity,
    };

    compiletime size_t count = 4;

    compiletime std::array<uint32_t,16> colors = {
        0xFF'00'00'00,
        0xFF'00'00'AA,
        0xFF'00'AA'00,
        0xFF'00'AA'AA,
        0xFF'AA'00'00,
        0xFF'AA'00'AA,
        0xFF'AA'55'00, // brown has halved green value
        0xFF'AA'AA'AA,
        0xFF'55'55'55,
        0xFF'55'55'FF,
        0xFF'55'FF'55,
        0xFF'55'FF'FF,
        0xFF'FF'55'55,
        0xFF'FF'55'FF,
        0xFF'FF'FF'55,
        0xFF'FF'FF'FF,
    };

    compiletime void writeTo(uint32_t* destination, Mode mode = Mode::Mode4Palette0LowIntensity, Color background = Color::black) {
        std::array<Color, 4> colorNames{};
        switch (mode) {
            case Mode::Mode4Palette0LowIntensity:
                colorNames = { background, Color::green, Color::red, Color::brown };
                break;
            case Mode::Mode4Palette0HighIntensity:
                colorNames = { background, Color::lightGreen, Color::lightRed, Color::yellow };
                break;
            case Mode::Mode4Palette1LowIntensity:
                colorNames = { background, Color::cyan, Color::magenta, Color::lightGray };
                break;
            case Mode::Mode4Palette1HighIntensity:
                colorNames = { background, Color::lightCyan, Color::lightMagenta, Color::white };
                break;
            case Mode::Mode5LowIntensity:
                colorNames = { background, Color::cyan, Color::red, Color::lightGray };
                break;
            case Mode::Mode5HighIntensity:
                colorNames = { background, Color::lightCyan, Color::lightRed, Color::white };
                break;
        }
        for (size_t i = 0; i < count; ++i) {
            destination[i] = colors[static_cast<uint8_t>(colorNames[i])];
        }
    }
};


struct PaletteC64 {
    using index_type = uint8_t;
    compiletime size_t count = 16;

    compiletime std::array<uint32_t,16> colors = {
        0xFF'00'00'00,
        0xFF'FF'FF'FF,
        0xFF'67'37'2B,
        0xFF'6F'A3'B1,
        0xFF'6F'3C'85,
        0xFF'58'8C'43,
        0xFF'34'28'79,
        0xFF'B7'C6'6E,
        0xFF'6F'4F'25,
        0xFF'42'39'00,
        0xFF'99'66'59,
        0xFF'43'43'43,
        0xFF'6B'6B'6B,
        0xFF'9A'D1'83,
        0xFF'6B'5E'B4,
        0xFF'95'95'95,
    };

    enum class Color {
        black, white, red, cyan, purple, green, blue, yellow,
        orange, brown, lightRed, darkGray, gray, lightGreen, lightBlue, lightGray
    };

    compiletime void writeTo(uint32_t* destination) {
        for (size_t i = 0; i < count; ++i) {
            destination[i] = colors[i];
        }
    }
};


