#define CXX
#include "Project256.h"
#include <cstdint>
#include "Drawing/Palettes.cpp"
#include "Drawing/Generators.cpp"
#include <iostream>
#include <chrono>

struct Timer {
    std::chrono::microseconds firesAt;

    Timer(std::chrono::microseconds currentTime, std::chrono::microseconds period)
    : firesAt{currentTime + period}{
    }

    bool hasFired(std::chrono::microseconds currentTime) const {
        return currentTime > firesAt;
    }
};

template<typename Vec>
constexpr Vec operator-(Vec left, Vec right)
{
    return Vec{left.x - right.x, left.y - right.y};
}

template<typename Vec>
constexpr Vec operator+(Vec left, Vec right)
{
    return Vec{left.x + right.x, left.y + right.y};
}

template<typename Vec1, typename Vec2>
constexpr bool operator<(Vec1 left, Vec2 right)
{
    return left.x < right.x && left.y < right.y;
}

template<typename Scalar, typename Vec>
constexpr Vec operator*(Scalar left, Vec right)
{
    using dimType = typeof(Vec::x);
    return Vec{
        .x = static_cast<dimType>(left * right.x),
        .y = static_cast<dimType>(left * right.y)};
}

template<typename Scalar, typename Vec>
constexpr Vec operator/(Vec left, Scalar right)
{
    using dimType = typeof(Vec::x);
    return Vec{
        .x = static_cast<dimType>(left.x / right),
        .y = static_cast<dimType>(left.y / right)};
}

constexpr Vec2i truncate(Vec2f vec) {
    return Vec2i{static_cast<int>(vec.x), static_cast<int>(vec.y)};
}

constexpr Vec2f itof(Vec2i vec) {
    return Vec2f{ static_cast<float>(vec.x), static_cast<float>(vec.y) };
}

//template<typename Vec>
//constexpr Vec2f& operator=(Vec2f& left, Vec right)
//{
//    left.x = right.x;
//    left.y = right.y;
//    return left;
//}

constexpr Vec2i operator%(Vec2i left, Vec2i right) {
    return Vec2i{left.x % right.x, left.y % right.y};
}

constexpr Vec2i operator%(Vec2i left, int right) {
    return Vec2i{left.x % right, left.y % right};
}

Vec2i rand2d() {
    return Vec2i{rand(), rand()};
}

template<typename T, typename U1, typename U2>
constexpr T wrapAround(T a, U1 lowerBound, U2 upperBound) {
    auto width = upperBound - lowerBound;
    while (!(a < upperBound)) {
        a = a - width;
    }
    while (!(lowerBound < a)) {
        a = a + width;
    }
    return a;
}

template<typename Vec, typename U1, typename U2>
constexpr Vec wrapAround2d(Vec a, U1 lowerBound, U2 upperBound) {
    return Vec{ wrapAround(a.x, lowerBound.x, upperBound.x),
        wrapAround(a.y, lowerBound.y, upperBound.y) };
}

template<typename Color, typename Vec2 = Vec2i, int Pitch = DrawBufferWidth>
constexpr void put(uint8_t* drawBuffer, Vec2 position, Color color)
{
    drawBuffer[static_cast<int>(position.x) + static_cast<int>(position.y) * Pitch] = static_cast<uint8_t>(color);
}

enum class GameState {
    Initialise,
    TitleScreen,
    MainMenu,
    TheThickOfIt
};

struct GameMemory {
    uint8_t vram[DrawBufferWidth * DrawBufferHeight];
    uint32_t palette[16];
    Vec2f dotPosition;
    Vec2f dotDirection;
    Timer directionChangeTimer;
    GameState state;
};

static_assert(sizeof(GameMemory) <= MemorySize, "MemorySize is too small to hold GameMemory struct");

extern "C" {

Vec2f clipSpaceDrawBufferScale(unsigned int viewportWidth, unsigned int viewportHeight)
{
    // we COULD assert that viewport dimensions are not greater than UINT16_MAX each...
    uint32_t viewportAspectRatio16_16 = (viewportWidth << 16) / viewportHeight;
    uint32_t drawAspectRatio16_16 = (DrawAspectH << 16) / DrawAspectV;

    uint32_t widthOnTarget = viewportWidth;
    uint32_t heightOnTarget = viewportHeight;

    if (viewportAspectRatio16_16 > drawAspectRatio16_16) {
        // window is wider than the draw target, shrink on x
        widthOnTarget = (heightOnTarget * drawAspectRatio16_16) >> 16;
    }
    else if (drawAspectRatio16_16 > viewportAspectRatio16_16) {
        heightOnTarget = (widthOnTarget << 16) / drawAspectRatio16_16;
    }

    return {
        .x = static_cast<float>(widthOnTarget) / viewportWidth,
        .y = static_cast<float>(heightOnTarget) / viewportHeight,
    };
}

GameOutput doGameThings(GameInput* pInput, void* pMemory)
{
    using Palette = PaletteC64;
    GameMemory& memory = *reinterpret_cast<GameMemory*>(pMemory);
    GameInput& input = *pInput;
    if (input.textLength)
        std::cout << input.text_utf8;

    if (input.frameNumber == 0) {
        for(int i = 0; i < DrawBufferWidth * DrawBufferHeight; ++i)
            memory.vram[i] = (i + input.frameNumber) % Palette::count;

        Palette::writeTo(memory.palette);
        memory.dotPosition = 0.5 * Vec2f{DrawBufferWidth, DrawBufferHeight};
        for(int i = 0; i < DrawBufferWidth * DrawBufferHeight; ++i)
            memory.vram[i] = 0x4;
    }



    const auto time = std::chrono::microseconds(input.upTime_microseconds);
    if (memory.directionChangeTimer.hasFired(time)) {
        memory.directionChangeTimer = Timer(time, std::chrono::seconds(1));
        memory.dotDirection = itof((rand2d() % 100) - Vec2i{50, 50});
    }

    if (input.hasMouse) {
        if (input.mouse.trackLength) {
            Vec2f mousePosition = input.mouse.track[input.mouse.trackLength - 1];

            Vec2i position = truncate(mousePosition);

            for (auto p : Rectangle {.bottomLeft = position - Vec2i{0,8}, .topRight = position + Vec2i{4, 0}})
                put(memory.vram, wrapAround2d(p, Vec2i{}, Vec2i{DrawBufferWidth, DrawBufferHeight}), Palette::Color::green);

        } else {

        }
    }
    // clear
    put(memory.vram, memory.dotPosition, 0x0);
    // update
    memory.dotPosition = memory.dotPosition + input.elapsedTime_s * memory.dotDirection;
    memory.dotPosition = wrapAround2d(memory.dotPosition, Vec2i{0,0}, Vec2i{DrawBufferWidth, DrawBufferHeight});
    // draw
    put(memory.vram, memory.dotPosition, Palette::Color::lightRed);

	return GameOutput{
		//.shouldQuit = input.closeRequested,
	};
}

void writeDrawBuffer(void* pMemory, void* buffer)
{
    if (buffer == nullptr) {
        return;
    }
    uint32_t* pixel = reinterpret_cast<uint32_t*>(buffer);

    if (pMemory == nullptr) {
        for (unsigned y = 0; y < DrawBufferHeight; ++y)
        for (unsigned x = 0; x < DrawBufferWidth; ++x)
        {
            *pixel++ = 0xFF000000 | ((y % 2) && (x % 2) ? 0xFFFFFF : y) ; // argb
        }
        return;
    }

    GameMemory& memory = *reinterpret_cast<GameMemory*>(pMemory);
    uint8_t* vram = memory.vram;
    
    for (unsigned y = 0; y < DrawBufferHeight; ++y)
    for (unsigned x = 0; x < DrawBufferWidth; ++x)
        *pixel++ = memory.palette[*vram++];
}

}
