//
//  Palettes.cpp
//  Project256
//
//  Created by Andreas Stahl on 04.06.22.
//

#include <cstdint>
#include <array>
#include <ranges>

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

    constexpr static uint32_t colors[16] = {
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
        for (size_t i = 0; i < 4; ++i) {
            destination[i] = colors[static_cast<uint8_t>(colorNames[i])];
        }
    }
};
