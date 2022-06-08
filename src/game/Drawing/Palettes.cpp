//
//  Palettes.cpp
//  Project256
//
//  Created by Andreas Stahl on 04.06.22.
//

#include <cstdint>
#include <array>

// uses SMPTE 240M conversion
constexpr uint32_t YCbCrToARGB(double Y, double Cb, double Cr) {
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


struct PaletteAppleII {

    constexpr static size_t count = 16;

    enum class Color : uint8_t {
        black, magenta, darkBlue, purple, darkGreen, grey1, mediumBlue,
        lightBlue, brown, orange, grey2, pink, green, yellow, aqua, white
    };

    constexpr static std::array<uint32_t,16> colors = {
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

    static void writeTo(uint32_t* destination) {
        for (size_t i = 0; i < count; ++i) {
            destination[i] = colors[i];
        }
    }
};

struct PaletteCGA {

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

    constexpr static size_t count = 4;

    constexpr static std::array<uint32_t,16> colors = {
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

    static void writeTo(uint32_t* destination, Mode mode = Mode::Mode4Palette0LowIntensity, Color background = Color::black) {
        std::array<Color, 4> colorNames;
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

    constexpr static size_t count = 16;

    constexpr static std::array<uint32_t,16> colors = {
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

    static void writeTo(uint32_t* destination) {
        for (size_t i = 0; i < count; ++i) {
            destination[i] = colors[i];
        }
    }
};
