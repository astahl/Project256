#include <cstdint>
#include "Project256.h"
#include "Drawing/Palettes.hpp"
#include "Drawing/Generators.hpp"
#include "Drawing/Sprites.hpp"
#include "Drawing/InterleavedBitmaps.hpp"
#include "Math/Vec2Math.hpp"
#include "FML/RangesAtHome.hpp"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <cassert>

template<typename Return, typename ...Args>
using FunctionPointer = Return (*)(Args...);


struct Timer {
    std::chrono::microseconds firesAt;

    static Timer delay(std::chrono::microseconds currentTime, std::chrono::microseconds period)
    {
        return {.firesAt = currentTime + period };
    }

    bool hasFired(std::chrono::microseconds currentTime) const {
        return currentTime > firesAt;
    }
};


template<typename Color, typename Vec2 = Vec2i, int Pitch = DrawBufferWidth>
compiletime void put(uint8_t* drawBuffer, Vec2 position, Color color)
{
    assert(drawBuffer != nullptr);
    assert(position.x >= 0);
    assert(position.x < DrawBufferWidth);
    assert(position.y >= 0);
    assert(position.y < DrawBufferHeight);
    drawBuffer[static_cast<int>(position.x) + static_cast<int>(position.y) * Pitch] = static_cast<uint8_t>(color);
}


constant int TextCharacterW = 8;
constant int TextCharacterH = 8;
constant int TextCharacterBytes = 8;
constant int TextLines = DrawBufferHeight / TextCharacterH;
constant int TextLineLength = DrawBufferWidth / TextCharacterW;

compiletime std::array<uint8_t, 256> CharacterTable = []() {
    std::array<uint8_t, 256> result{};
    for (size_t i = 0; i < result.size(); ++i) result[i] = 0xFF;// map any unknown to checkerboard
    // PET 0x20 - 0x3F is equal to ascii
    for (char c = ' '; c <= '?'; ++c) {
        result[c] = c;
    }
    // Ascii Uppercase is at beginning of character rom
    for (char c = '@'; c <= ']'; ++c) {
        result[c] = c - '@';
    }
    // Ascii lowercase (0x60 - 0x7A) is in 0xC0 - 0xDA of character rom
    for (char c = 'a'; c <= 'z'; ++c) {
        result[c] = c + 0x60;
    }

    return result;
}();


template <typename Pixel, size_t Width, size_t Height, size_t Pitch = Width>
struct Image {

    std::array<Pixel, Pitch * Height> pixels;

    constexpr Pixel* data() {
        return pixels.data();
    }

    constexpr size_t size() const {
        return pixels.size();
    }

    constexpr Pixel& at(size_t x, size_t y) {
        assert(x < Width);
        assert(y < Height);
        return pixels.at(x + y * Pitch);
    }

    constexpr Pixel& at(Vec2i pos) {
        assert(pos.x >= 0);
        assert(pos.y >= 0);
        return at(static_cast<size_t>(pos.x), static_cast<size_t>(pos.y));
    }

    constexpr Pixel& atYFlipped(size_t x, size_t y) {
        return at(x, ((Height - 1) - y));
    }

    constexpr Pixel& atYFlipped(Vec2i pos) {
        assert(pos.x >= 0);
        assert(pos.y >= 0);
        return atYFlipped(pos.x, pos.y);
    }

    constexpr size_t height() const {
        return Height;
    }

    constexpr size_t width() const {
        return Width;
    }

    constexpr size_t pitch() const {
        return Pitch;
    }

    template<bool FlipY = false, size_t DestWidth, size_t DestHeight, size_t DestPitch>
    constexpr void copy(Image<Pixel, DestWidth, DestHeight, DestPitch>& destination)
    {
        constexpr size_t minHeight = min(Height, DestHeight);
        constexpr size_t lineWidth = min(Width, DestWidth) * sizeof(Pixel);

        for (size_t y = 0; y < minHeight; ++y) {
            void* src = nullptr;
            if constexpr (FlipY) {
                src = &atYFlipped(0, y);
            }
            else {
                src = &at(y);
            }

            memcpy(&destination.at(0, y), src, lineWidth);
        }
    }
};

struct GameMemory {
    std::array<uint8_t, 1024 * 1024> scratch;
    // video
    alignas(128) Image<uint8_t, DrawBufferWidth, DrawBufferHeight> vram;
    alignas(128) std::array<uint32_t, 32> palette;

    // text
    std::array<uint8_t, TextCharacterBytes * 256> characterROM;
    std::array<uint8_t, TextLines * TextLineLength> textBuffer;
    std::array<uint8_t, TextLines * TextLineLength> textColors;
    int textFirstLine;
    int textLastLine;
    int textScroll;
    int textCursorPosition;
    Timer timerCursorBlink;
    bool isCursorOn;

    // bird
    Vec2f birdPosition;
    int birdSpeed;
    Vec2i birdTarget;
    Timer directionChangeTimer;
    FunctionPointer<void, GameMemory&> timerCallback;
    SpritePicture<5, 2, 4> sprite;
    Timer spriteAnimationTimer;
    int currentSpriteFrame;

    // images
    Image<uint8_t, 320, 256> imageDecoded;
    Image<uint8_t, 320, 256> faubigDecoded;
    Image<uint8_t, 32, 24> faufauDecoded;

    // mouse clicks
    Vec2i points[2];
    int currentPoint;

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


void cleanInput(GameInput* input) {
    for (int i = 0; i < InputMaxControllers; ++i) {
        auto& controller = input->controllers[i];
        for (int axis1Index = 0; axis1Index < InputControllerAxis1Count; ++axis1Index)
        {
            controller.axes1[axis1Index].trigger.transitionCount = 0;
            controller.axes1[axis1Index].start = controller.axes1[axis1Index].end;
        }
        for (int axis2Index = 0; axis2Index < InputControllerAxis1Count; ++axis2Index)
        {
            controller.axes2[axis2Index].left.transitionCount = 0;
            controller.axes2[axis2Index].up.transitionCount = 0;
            controller.axes2[axis2Index].down.transitionCount = 0;
            controller.axes2[axis2Index].right.transitionCount = 0;
            controller.axes2[axis2Index].start = controller.axes2[axis2Index].end;
            if (!controller.axes2[axis2Index].latches)
                controller.axes2[axis2Index].end = Vec2f{};
        }
        for (int buttonIndex = 0; buttonIndex < InputControllerButtonCount; ++buttonIndex)
        {
            controller.buttons[buttonIndex].transitionCount = 0;
        }
    }
    auto& mouse = input->mouse;
    mouse.buttonLeft.transitionCount = 0;
    mouse.buttonRight.transitionCount = 0;
    mouse.buttonMiddle.transitionCount = 0;
    if (mouse.trackLength && mouse.endedOver) {
        mouse.track[0] = mouse.track[mouse.trackLength - 1];
        mouse.trackLength = 1;
    } else {
        mouse.trackLength = 0;
    }

    input->textLength = 0;
    input->closeRequested = false;
    input->tapCount = 0;
}


compiletime auto BufferSize = Vec2i{ DrawBufferWidth, DrawBufferHeight};
compiletime auto Center = BufferSize / 2;

void birdDirectionChange(GameMemory& memory) {
    memory.birdTarget = (rand2d() % BufferSize);
}


GameOutput doGameThings(GameInput* pInput, void* pMemory, PlatformCallbacks platform)
{
    assert(pInput != nullptr);
    assert(pMemory != nullptr);
    assert(platform.readFile != nullptr);

    using namespace ranges_at_home;
    using namespace Generators;

    GameOutput output{};
    using Palette = PaletteEGA;
    compiletime auto lookupColor = [](auto col) { return static_cast<uint8_t>(findNearest(col, Palette::colors).index); };
    compiletime uint8_t black = lookupColor(Colors::Black);
    compiletime uint8_t cyan = lookupColor(Colors::Cyan);
    compiletime uint8_t lightBlue = lookupColor(Colors::LightBlue);
    compiletime uint8_t red = lookupColor(Colors::Red);
    compiletime uint8_t white = lookupColor(Colors::White);
    compiletime uint8_t green = lookupColor(Colors::Green);

    compiletime auto clip = [=](const auto& p) {
        return (Vec2i{0,0} <= p) && (p < BufferSize);
    };

    auto clearColor = black;
    if (pInput->closeRequested) {
        clearColor = white;
    }

    GameMemory& memory = *reinterpret_cast<GameMemory*>(pMemory);
    GameInput& input = *pInput;

    // initialize main memory
    if (input.frameNumber == 0) {
        std::memset(&memory, 0, MemorySize);

        std::memset(memory.palette.data(), 0xFF, memory.palette.size() * 4);
        Palette::writeTo(memory.palette.data());
        memory.birdPosition = itof(Center);
        memory.timerCallback = birdDirectionChange;
        memory.sprite.data = {
            0, 0, 1, 0, 0,
            1, 1, 0, 1, 1,
            1, 1, 1, 1, 1,
            0, 0, 0, 0, 0,
            1, 1, 0, 1, 1,
            0, 0, 1, 0, 0,
            0, 0, 1, 0, 0,
            1, 1, 1, 1, 1,
        };
        std::replace(memory.sprite.data.begin(), memory.sprite.data.end(), static_cast<uint8_t>(1), cyan);
        memory.currentSpriteFrame = 0;
        memory.birdSpeed = 5;
        if (platform.readFile) {
            int64_t read = platform.readFile("CharacterRomPET8x8x256.bin", memory.characterROM.data(), memory.characterROM.size());
            if (read != 2048)
            {
                exit(2);
            }
        }
        else {
            exit(1);
        }
            
        memory.textFirstLine = 0;
        memory.textLastLine = 0;
        memory.textScroll = 0;
        std::memset(memory.textColors.data(), white, memory.textColors.size());
        std::memset(memory.textBuffer.data(), CharacterTable[' '], memory.textBuffer.size());
        {
        size_t read = platform.readFile("Faufau.brush", memory.scratch.data(), memory.scratch.size());
        ILBMDataParser<endian::big> parser{.data = memory.scratch.data(), .dataSize = static_cast<int>(read)};
        parser.isValid();
        auto header = parser.getHeader();
        printf("w: %d, h: %d", header.width.value, header.height.value);
        auto colorMap = parser.getColorMap();
        for (int i = 0; i < colorMap.size; ++i) {
            auto color = colorMap.colors[i];
            memory.palette[i] = makeARGB(color.red, color.green, color.blue);
        }
        parser.deinterleaveInto(memory.faufauDecoded.data(), memory.faufauDecoded.size(), memory.faufauDecoded.pitch());
        parser.deinterleaveInto(&memory.faufauDecoded.at(16,8), memory.faufauDecoded.size() - 16, memory.faufauDecoded.pitch());
        }

        {
        size_t read = platform.readFile("Faufau.ilbm", memory.scratch.data(), memory.scratch.size());
        ILBMDataParser<endian::big> parser{.data = memory.scratch.data(), .dataSize = static_cast<int>(read)};
        parser.inflateAndDeinterleaveInto(memory.faubigDecoded.data(), memory.faubigDecoded.size(), memory.faubigDecoded.pitch());
        }


        if (platform.readImage) {
            if (!platform.readImage("test.bmp", reinterpret_cast<uint32_t*>(memory.scratch.data()), 320, 256))
                exit(3);
        }
        else {
            exit(4);
        }

        ConvertBitmapFrom32BppToIndex<320>(reinterpret_cast<uint32_t*>(memory.scratch.data()), 320, 256, memory.palette, memory.imageDecoded.data());

    }

    constant auto whitePixel = [&](const auto& p) { put(memory.vram.data(), p, white); };
    constant auto redPixel = [&](const auto& p) { put(memory.vram.data(), p, red); };

    // handle timers
    const auto time = std::chrono::microseconds(input.upTime_microseconds);
    if (memory.directionChangeTimer.hasFired(time) || memory.birdTarget == round(memory.birdPosition)) {
        memory.directionChangeTimer = Timer::delay(time, std::chrono::seconds(100 / memory.birdSpeed++));
        memory.timerCallback(memory);
    }

    if (memory.spriteAnimationTimer.hasFired(time)) {
        memory.spriteAnimationTimer = Timer::delay(time, std::chrono::milliseconds(1000 / memory.birdSpeed));
        memory.currentSpriteFrame = (memory.currentSpriteFrame + 1) % decltype(memory.sprite)::frameCount;
    }

    // clear the screen
    std::memset(memory.vram.data(), (uint8_t)clearColor, DrawBufferWidth * DrawBufferHeight);

    // draw the testimage
    memory.imageDecoded.copy<true>(memory.vram);

    // draw the palette in the first rows
    for (int y = 0; y < memory.palette.size() / 2; ++y) {
    for (int x = 0; x < DrawBufferWidth; ++x) {
        // uncomment for all 256 colors
        put(memory.vram.data(), Vec2i{ x, y }, (x * 16 / DrawBufferWidth) + y / 8 * 16);
    } }

    // draw faufau testimage
    memory.faufauDecoded.copy<true>(memory.vram);

    for (unsigned int i = 0; i < input.textLength; ++i) {
        uint8_t inputChar = input.text_utf8[i];
        if ((inputChar & 0b10000000) == 0) {
            // input is ASCII (top bit is zero)
            switch (inputChar) {
                case 0x7F: // Delete == backspace on modern keyboards
                case 0x08: // backspace
                    if (memory.textCursorPosition > 0) {
                        memory.textBuffer[--memory.textCursorPosition] = CharacterTable[' '];
                    }
                    break;
                case 0x09: // tab
                    memory.textCursorPosition += 4 - ((memory.textCursorPosition) % 4);
                    break;
                case 0x19: // untab
                    memory.textCursorPosition -= 4 - ((memory.textCursorPosition) % 4);
                    break;
                case 0x0D: // Carriage Return (Enter on mac)
                {
                    int lineNumber = memory.textCursorPosition / TextLineLength;
                    memory.textCursorPosition = (lineNumber + 1) * TextLineLength;
                    break;
                }
                default: {
                    uint8_t outputChar = CharacterTable[inputChar];
                    if (outputChar != 0xFF) {
                        memory.textBuffer[memory.textCursorPosition++] = CharacterTable[inputChar];
                    } else {
                        char upper = inputChar >> 4;
                        char lower = inputChar & 0xF;
                        memory.textBuffer[memory.textCursorPosition++] = CharacterTable[upper > 9 ? (upper - 10) + 'a' : upper + '0'];
                        memory.textBuffer[memory.textCursorPosition++] = CharacterTable[lower > 9 ? (lower - 10) + 'a' : lower + '0'];
                    }
                }
            }
        } else {
            // chomp through UTF-8 encoding

            int onesCount = 0;
            for (;((inputChar & (0b10000000 >> onesCount)) != 0) && onesCount <= 5; ++onesCount) {
                // just chomping through bits until I find a zero, don't mind me!
            }
            assert(onesCount > 1);
                // starting with 10 is invalid for the first chunk of a utf8 code
            assert(onesCount != 5);
                // this should not happen in a utf-8 string
            // the resulting unicode codepoint
            uint32_t code = (0b01111111 >> onesCount) & inputChar;
            while ((++i < input.textLength) && (--onesCount > 0)) {
                code <<= 6;
                inputChar = input.text_utf8[i];
                code |= inputChar & 0b00111111;
            }

            switch (code) {
                // MacBook Arrow Keys
                case 0xF700: memory.textCursorPosition -= TextLineLength; break;
                case 0xF701: memory.textCursorPosition += TextLineLength; break;
                case 0xF702: memory.textCursorPosition -= 1; break;
                case 0xF703: memory.textCursorPosition += 1; break;
                // macbook fn + backspace = delete?
                case 0xF728: memory.textBuffer[memory.textCursorPosition] = CharacterTable[' ']; break;
                default:
                    printf("%x\n", code);
            }

        }
        memory.textCursorPosition = std::clamp(memory.textCursorPosition, 0, (memory.textLastLine - memory.textFirstLine + 1) * TextLineLength);
    }


    // do some experimentation in the vram
    compiletime auto wrap = [](auto p) { return wrapAround2d(p, Vec2i(), Vec2i{DrawBufferWidth, DrawBufferHeight});};

    if (memory.points[0] != memory.points[1])
    for (auto p : Generators::Line{memory.points[0], memory.points[1]})
        whitePixel(p);

    if (input.mouse.trackLength) {
        Vec2f mousePosition = input.mouse.track[input.mouse.trackLength - 1];
        Vec2i position = truncate(mousePosition);

        auto offset = [=](const auto& p) {
            return p + position;
        };

        if (input.mouse.buttonLeft.endedDown) {
            memory.points[memory.currentPoint] = position;
        } else if (input.mouse.buttonLeft.transitionCount) {
            memory.currentPoint = (memory.currentPoint + 1) % 2;
        }

        auto atMouse = transform(offset);
        compiletime auto clipped = filter(clip);
        compiletime auto wrapped = transform(wrap);

        compiletime auto rectangleGenerator = Rectangle{ Vec2i{-3,-3}, Vec2i{3, 3} };

        if (input.mouse.buttonLeft.endedDown) {
            for (auto p : rectangleGenerator | atMouse | clipped)
                put(memory.vram.data(), wrap(p), green);
        }

        compiletime auto crossGenerator = (HLine{{-3, 0}, 7} ^ VLine{{0, -3}, 7});
        compiletime auto cross = (crossGenerator | toArray<size(crossGenerator)>{}).run();
        (cross | atMouse | wrapped | forEach(whitePixel)).run();

        compiletime auto circleGenerator = Circle{.mRadius = 4};
        compiletime auto circle = (circleGenerator | toArray<size(circleGenerator)>{}).run();
        (circle | atMouse | wrapped | forEach(whitePixel)).run();

        auto mat = makeBase2dX(-normalized(Center - position));
        const auto pointAtCenter = [=](Vec2i p) {
            return truncate(mat * p); };

        const std::array<Vec2i, 4> points{ Vec2i{ 40, 0 }, Vec2i{-10, 20}, Vec2i{-5, 0}, Vec2i{-10, -20} };
        for (auto p : points
             | transform(pointAtCenter)
             | atMouse | wrapped) {
            redPixel(p);
        }

        for (auto p : points
             | transform(pointAtCenter)
             | atMouse
             | batch<2, 1, true>{}
             | transform([](std::array<Vec2i, 2> arr) {
                 Vec2i a = arr[0];
                 Vec2i b = arr[1];
                 return Line(a, b); })
             | flatten{}
             | wrapped ) {
            whitePixel(p);
        }

    }

    // write controller state to the screen
    for (int i = 0; i < InputMaxControllers; ++i) {
        auto& controller = input.controllers[i];
        if (!controller.isConnected)
            continue;

        Vec2i p{ 10, (i + 1) * 10 };
        for (auto& button : controller.buttons) {
            if (button.endedDown)
                whitePixel(p);
            p.x += 2;
        }

        for (auto& axis1 : controller.axes1) {
            if (axis1.trigger.endedDown)
                whitePixel(p);
            p.x += 2;
        }

        for (auto& axis2 : controller.axes2) {
            if (axis2.up.endedDown)
                whitePixel(p + Vec2i{ 1,1 });
            if (axis2.down.endedDown)
                whitePixel(p + Vec2i{ 1,-1 });
            if (axis2.left.endedDown)
                whitePixel(p + Vec2i{ });
            if (axis2.right.endedDown)
                whitePixel(p + Vec2i{ 2,0 });
            p.x += 4;
        }

        p.x = 10;
        p.y = (i + 1) * 10 + 3;
        if (controller.buttonBack.transitionCount != 0 && controller.buttonBack.endedDown) {
            output.shouldQuit = true;
        }
    }


    // update
    memory.birdPosition = memory.birdPosition + static_cast<float>(input.elapsedTime_s) * memory.birdSpeed * normalized(itof(memory.birdTarget) - memory.birdPosition);
    memory.birdPosition = clamp(memory.birdPosition, Vec2f{}, Vec2f{DrawBufferWidth - 1, DrawBufferHeight - 1});

    put(memory.vram.data(), memory.birdTarget, lightBlue);

    // draw
    blitSprite(memory.sprite, memory.currentSpriteFrame, memory.vram.data(), DrawBufferWidth, truncate(memory.birdPosition), Vec2i{}, Vec2i{DrawBufferWidth, DrawBufferHeight});

    // draw text buffer
    uint8_t* drawPointer = &memory.vram.at(0, memory.vram.height() - TextCharacterH);
    uint8_t* textPointer = memory.textBuffer.data();
    uint8_t* textColorPointer = memory.textColors.data();

    compiletime uint8_t mask0 = 1 << 7;
    compiletime uint8_t mask1 = 1 << 6;
    compiletime uint8_t mask2 = 1 << 5;
    compiletime uint8_t mask3 = 1 << 4;
    compiletime uint8_t mask4 = 1 << 3;
    compiletime uint8_t mask5 = 1 << 2;
    compiletime uint8_t mask6 = 1 << 1;
    compiletime uint8_t mask7 = 1 << 0;

    int cursorPosition = memory.textCursorPosition;
    for (int line = 0; line < TextLines; ++line) {
        uint8_t* linePointer = drawPointer;
        if (line >= memory.textFirstLine && line <= memory.textLastLine) {
            for (int y = TextCharacterH - 1; y >= 0; --y) {
                uint64_t* dst = reinterpret_cast<uint64_t*>(linePointer);
                for (int pos = 0; pos < TextLineLength; ++pos)
                {
                    const uint8_t t = textPointer[pos];
                    const uint64_t c = memory.characterROM[y + t * 8];
                    const uint8_t color = textColorPointer[pos];
                    uint64_t pixels8 =
                          (c & mask0) >> 7
                        | (c & mask1) << 2
                        | (c & mask2) << 11
                        | (c & mask3) << 20
                        | (c & mask4) << 29
                        | (c & mask5) << 38
                        | (c & mask6) << 47
                        | (c & mask7) << 56;
                    const uint64_t background = ~(pixels8 * 0xFF) / 0xFF;
                    // colors! top nibble is background, bottom nibble foreground
                    *dst++ = pixels8 * (color & 0xF) | background * (color >> 4);
                }
                linePointer += DrawBufferWidth;
            }
        }
        textPointer += TextLineLength;
        textColorPointer += TextLineLength;
        cursorPosition -= TextLineLength;
        drawPointer -= DrawBufferWidth * TextCharacterH;
    }

    if (memory.timerCursorBlink.hasFired(time)) {
        memory.timerCursorBlink = Timer::delay(time, std::chrono::milliseconds(200));
        memory.isCursorOn = !memory.isCursorOn;
    }

    if (memory.isCursorOn) {
        int cursorY = (TextLines - 1) - memory.textCursorPosition / TextLineLength;
        int cursorX = memory.textCursorPosition % TextLineLength;
        (Rectangle{{cursorX * TextCharacterW, cursorY * TextCharacterH}, {(cursorX + 1) * TextCharacterW - 1, (cursorY + 1) * TextCharacterH - 1}} | forEach(whitePixel)).run();
    }

    return output;
}

void writeDrawBuffer(void* pMemory, void* buffer)
{
    assert(buffer != nullptr);
    assert(pMemory != nullptr);

    GameMemory& memory = *reinterpret_cast<GameMemory*>(pMemory);
    uint8_t* vram = memory.vram.data();


    if constexpr (DrawBufferWidth % 8 == 0)
    {
        constexpr int count = DrawBufferWidth * DrawBufferHeight;
        uint64_t* src = reinterpret_cast<uint64_t*>(vram);
        uint64_t* dst = reinterpret_cast<uint64_t*>(buffer);
        const uint32_t* palette = memory.palette.data();
        for (int i = 0; i < count; i += 8) {
            const uint64_t sourcePixel8 = *src++;
            
            *dst = static_cast<uint64_t>(palette[sourcePixel8 >> 0 & 0xff]) | 
                static_cast<uint64_t>(palette[sourcePixel8 >> 8 & 0xff]) << 32;
            *(dst + 1) = static_cast<uint64_t>(palette[sourcePixel8 >> 16 & 0xff]) |
                static_cast<uint64_t>(palette[sourcePixel8 >> 24 & 0xff]) << 32;
            *(dst + 2) = static_cast<uint64_t>(palette[sourcePixel8 >> 32 & 0xff]) |
                static_cast<uint64_t>(palette[sourcePixel8 >> 40 & 0xff]) << 32;
            *(dst + 3) = static_cast<uint64_t>(palette[sourcePixel8 >> 48 & 0xff]) |
                static_cast<uint64_t>(palette[sourcePixel8 >> 56 & 0xff]) << 32;
            dst += 4;
        }
    }
    else {
        uint32_t* pixel = reinterpret_cast<uint32_t*>(buffer);

        for (unsigned y = 0; y < DrawBufferHeight; ++y)
        for (unsigned x = 0; x < DrawBufferWidth; ++x)
            *pixel++ = memory.palette[*vram++];
    }
}

}
