//
//  TestBed.hpp
//  Project256
//
//  Created by Andreas Stahl on 03.07.22.
//

#pragma once

#include "FML/RangesAtHome.hpp"
#include "Drawing/InterleavedBitmaps.hpp"
#include "Math/Vec2Math.hpp"
#include "Drawing/Sprites.hpp"
#include "Utility/Timers.hpp"
#include "Utility/Text.hpp"
#include "Drawing/Images.hpp"
#include "Drawing/Palettes.hpp"
#include "Drawing/Generators.hpp"
#include "Project256.h"


using DrawBuffer = Image<uint32_t, DrawBufferWidth, DrawBufferHeight>;
using VRAM = Image<uint8_t, DrawBufferWidth, DrawBufferHeight, ImageOrigin::BottomLeft>;

constant int TextCharacterW = 8;
constant int TextCharacterH = 8;
constant int TextLines = DrawBufferHeight / TextCharacterH;
constant int TextLineLength = DrawBufferWidth / TextCharacterW;

template<typename T>
concept aSoundGenerator = requires (T& t) {
    t.value();
    t.advance(0.0f);
};

template<typename T>
concept aToneGenerator = aSoundGenerator<T> && requires (T& t) {
    t.frequency;
};

template<typename T>
struct SineWave {
    compiletime float pi2 = 2 * std::numbers::pi_v<float>;
    float phase;
    T amplitude;
    float frequency;

    constexpr T value() const {
        return amplitude * mySin(phase);
    }

    void advance(float timeStep) {
        phase += pi2 * timeStep * frequency;
        if (phase > pi2) {
            phase = fmodf(phase, pi2);
        }
    }
};

template<typename T>
struct SawtoothWave {
    float phase;
    T amplitude;
    float frequency;

    constexpr T value() const {
        return amplitude * (phase - 1.0f);
    }

    void advance(float timeStep) {
        phase += timeStep * frequency;
        if (phase > 2.0f) {
            phase = fmodf(phase, 2.0f);
        }
    }
};

template<typename T>
struct TriangleWave {
    float phase;
    T amplitude;
    float frequency;


    constexpr T value() const {
        return amplitude * (std::fabs(phase * 4 - 2.0f) - 1.0f);
    }

    void advance(float timeStep) {
        phase += timeStep * frequency;
        if (phase > 1.0f) {
            phase = fmodf(phase, 1.0f);
        }
    }
};


template<typename T>
struct SquareWave {
    float phase;
    T amplitude;
    float frequency;


    constexpr T value() const {
        return phase < 0.5 ? amplitude : -amplitude;
    }

    void advance(float timeStep) {
        phase += timeStep * frequency;
        if (phase > 1.0f) {
            phase = fmodf(phase, 1.0f);
        }
    }
};

template<typename T>
struct PulseWave {
    float phase;
    T amplitude;
    float frequency;
    float pulseWidth;

    constexpr T value() const {
        return phase < pulseWidth ? amplitude : (phase - 0.5f < pulseWidth ? -amplitude : 0);
    }

    void advance(float timeStep) {
        phase += timeStep * frequency;
        if (phase > 1.0f) {
            phase = fmodf(phase, 1.0f);
        }
    }
};

template<typename T>
struct WhiteNoise {
    T amplitude;
    T mValue;

    constexpr T value() const {
        return mValue;
    }

    void advance(float) {
        auto rando = amplitude * static_cast<float>(rand()) / RAND_MAX;
        mValue = static_cast<T>(rando);
    }
};


template<aToneGenerator T, aSoundGenerator U>
struct FrequencyModulator {
    T carrier;
    U mod;
    float depth;

    constexpr auto value() const {
        return carrier.value();
    }

    void advance(float timeStep) {
        mod.advance(timeStep);
        carrier.advance(timeStep + (timeStep * depth * mod.value()));
    }
};

template<aToneGenerator T, aSoundGenerator U>
struct AmplitudeModulator {
    T carrier;
    U mod;
    float depth;

    constexpr auto value() const {
        return carrier.value() * std::lerp(1, mod.value(), depth);
    }

    void advance(float timeStep) {
        mod.advance(timeStep);
        carrier.advance(timeStep);
    }
};


template <typename T>
struct Step {
    T amplitude;
    float frequency;
};

template <typename T>
struct StepSequencer {
    std::array<T, 16> steps;
    // bpm * 4 / 60
    float stepsPerSecond;
    float t;
    int currentStep;

    void advance(float timeStep) {
        t += timeStep * stepsPerSecond;
        if (t >= steps.size()) {
            t = fmodf(t, static_cast<float>(steps.size()));
        }
        currentStep = static_cast<int>(t);
    }

    const T& value() const {
        return steps[currentStep];
    }
};

template <typename T>
struct EnvelopeAdsr {
    float attack;
    float decay;
    float sustain;
    float release;
    bool percussive;

    T amplitude;
    float t;
    bool on;
    enum class Section {
        Ready, Attack, Decay, Sustain, Release
    } currentSection;
    T currentValue;

    T value() {
        return currentValue;
    }

    void advance(float timeStep) {
        if (currentSection == Section::Ready) {
            if (on == false) {
                return;
            }

            currentSection = Section::Attack;
        }
        t += timeStep;
        if (currentSection == Section::Attack) {
            if (t < attack) {
                float tAttack = t / attack;
                currentValue = std::lerp(0.0f, amplitude, tAttack);
            }
            else {
                currentSection = Section::Decay;
                t -= attack;
            }
        }
        if (currentSection == Section::Decay) {
            if (t < decay) {
                float tDecay = t / decay;
                currentValue = std::lerp(amplitude, sustain * amplitude, tDecay);
            }
            else {
                currentSection = Section::Sustain;
                t -= decay;
            }
        }
        if (currentSection == Section::Sustain) {
            if (on && !percussive) {
                currentValue = sustain * amplitude;
            } 
            else {
                currentSection = Section::Release;
                t = timeStep;
            }
        }
        if (currentSection == Section::Release) {
            if (t < release &&
                (!on && !percussive)) { // also check if envelope was retriggered, skip to ready and attack subsequently
                float tRelease = t / release;
                currentValue = std::lerp(sustain * amplitude, 0, tRelease);
            }
            else {
                currentValue = 0;
                t = 0.0f;
                currentSection = Section::Ready;
            }
        }
    }
};

struct TestBedMemory {
    std::array<uint8_t, 1024 * 1024> scratch;
    // video
    alignas(128) VRAM vram;
    alignas(128) std::array<uint32_t, 256> palette;

    // text
    BitmapImage<TextCharacterW, TextCharacterH * 256> characterROM;
    std::array<uint8_t, TextLines * TextLineLength> textBuffer;
    std::array<uint8_t, TextLines * TextLineLength> textColors;
    int textFirstLine;
    int textLastLine;
    int textScroll;
    int textCursorPosition;
    AutoResettingTimer timerCursorBlink;
    bool isCursorOn;

    // bird
    Vec2f birdPosition;
    int birdSpeed;
    Vec2i birdTarget;
    Timer directionChangeTimer;
    FunctionPointer<void, TestBedMemory&> timerCallback;
    SpritePicture<5, 2, 4> sprite;
    Timer spriteAnimationTimer;
    int currentSpriteFrame;

    // images
    Image<uint8_t, 320, 256, ImageOrigin::TopLeft> imageDecoded;
    Image<uint8_t, 320, 256, ImageOrigin::TopLeft> faubigDecoded;
    Image<uint8_t, 32, 24, ImageOrigin::TopLeft> faufauDecoded;

    // mouse clicks
    Vec2i points[2];
    int currentPoint;

    // audio
    AmplitudeModulator<SineWave<float>, TriangleWave<float>> tone;
    StepSequencer<Step<float>> sequencer;
    PulseWave<float> sineWave;
    EnvelopeAdsr<float> envelope;
};


compiletime auto BufferSize = Vec2i{ DrawBufferWidth, DrawBufferHeight};
compiletime auto Center = BufferSize / 2;

void birdDirectionChange(TestBedMemory& memory) {
    memory.birdTarget = (rand2d() % BufferSize);
}

struct TestBed {
    using MemoryLayout = TestBedMemory;

    static GameOutput doGameThings(TestBedMemory& memory, const GameInput& input, const PlatformCallbacks& callbacks)
    {
        if (input.frameNumber == 0) {
#ifdef DEBUG
            test_myCos();
#endif
        }
        using namespace ranges_at_home;
        using namespace Generators;

        GameOutput output{};
        using Palette = PaletteVGA;
        using Text = CharacterRom::PET;
        const auto time = std::chrono::microseconds(input.upTime_microseconds);

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
            std::replace(memory.sprite.data.begin(), memory.sprite.data.end(), static_cast<uint8_t>(1), static_cast<uint8_t>(2));
            memory.currentSpriteFrame = 0;
            memory.birdSpeed = 5;
            {
                int64_t read = callbacks.readFile("CharacterRomPET8x8x256.bin", memory.characterROM.bytes(), memory.characterROM.bytesSize());
                assert(read == 2048);
                read = read;
            }
            memory.textFirstLine = 0;
            memory.textLastLine = 4;

            memory.timerCursorBlink = AutoResettingTimer(time, std::chrono::milliseconds(200));
            std::memset(memory.textColors.data(), 1, memory.textColors.size());
            std::memset(memory.textBuffer.data(), Text::CharacterTable[' '], memory.textBuffer.size());

            memory.textBuffer.at(1) = static_cast<uint8_t>(Text::SpecialCharacters::ArcDownRight);
            memory.textBuffer.at(2) = static_cast<uint8_t>(Text::SpecialCharacters::HLine);
            memory.textBuffer.at(3) = static_cast<uint8_t>(Text::SpecialCharacters::HLine);
            memory.textBuffer.at(4) = static_cast<uint8_t>(Text::SpecialCharacters::HLine);
            memory.textBuffer.at(5) = static_cast<uint8_t>(Text::SpecialCharacters::ArcDownLeft);

            {
                size_t read = callbacks.readFile("Faufau.brush", memory.scratch.data(), memory.scratch.size());
                ILBMDataParser<endian::big> parser{.data = memory.scratch.data(), .dataSize = static_cast<int>(read)};
                assert(parser.isValid());
                auto colorMap = parser.getColorMap();
                for (int i = 0; i < colorMap.size; ++i) {
                    auto color = colorMap.colors[i];
                    memory.palette[i] = makeARGB(color.red, color.green, color.blue);
                }
                parser.deinterleaveInto(memory.faufauDecoded.data(), memory.faufauDecoded.size(), memory.faufauDecoded.pitch());
            }

            {
                size_t read = callbacks.readFile("Faufau.ilbm", memory.scratch.data(), memory.scratch.size());
                ILBMDataParser<endian::big> parser{.data = memory.scratch.data(), .dataSize = static_cast<int>(read)};
                parser.inflateAndDeinterleaveInto(memory.faubigDecoded.data(), memory.faubigDecoded.size(), memory.faubigDecoded.pitch());
            }


            if (callbacks.readImage) {
                if (!callbacks.readImage("test.bmp", reinterpret_cast<uint32_t*>(memory.scratch.data()), 320, 256))
                     exit(3);
            }
            else {
                exit(4);
            }

            ConvertBitmapFrom32BppToIndex<320>(reinterpret_cast<uint32_t*>(memory.scratch.data()), 320, 256, memory.palette, memory.imageDecoded.data());


            //memory.tone.carrier.frequency = 300.0f;
            //memory.tone.carrier.amplitude = 1.0f;
            memory.tone.mod.frequency = 2.0f;
            memory.tone.mod.amplitude = 1.f;
            memory.tone.depth = 0.2f;

            memory.sequencer.stepsPerSecond = 4;
            memory.sequencer.steps[0] = { .amplitude = 1.0f, .frequency = 220.f };
            memory.sequencer.steps[1] = { .amplitude = 1.0f, .frequency = 250.f };
            memory.sequencer.steps[2] = { .amplitude = 1.0f, .frequency = 250.f };
            memory.sequencer.steps[3] = { .amplitude = 1.0f, .frequency = 440.f };;
            memory.sequencer.steps[12] = { .amplitude = 1.f, .frequency = 500.f };

            memory.envelope.attack = 0.01f;
            memory.envelope.sustain = 0.4f;
            memory.envelope.decay = 0.1f;
            memory.envelope.release = .5f;

        }

        constant auto black = static_cast<VRAM::PixelType>(findNearest(Colors::Black, memory.palette).index);
        constant auto white = static_cast<VRAM::PixelType>(findNearest(Colors::White, memory.palette).index);
        constant auto cyan = static_cast<VRAM::PixelType>(findNearest(Colors::Cyan, memory.palette).index);
        constant auto lightBlue = static_cast<VRAM::PixelType>(findNearest(Colors::LightBlue, memory.palette).index);
        constant auto red = static_cast<VRAM::PixelType>(findNearest(Colors::Red, memory.palette).index);
        constant auto green = static_cast<VRAM::PixelType>(findNearest(Colors::Green, memory.palette).index);
        constant auto whitePixel = [&](const auto& p) { memory.vram.pixel(p) = white; };
        constant auto redPixel = [&](const auto& p) { memory.vram.pixel(p) = red; };

        auto clearColor = black;
        if (input.closeRequested) {
            clearColor = cyan;
        }

        // handle timers
        if (memory.directionChangeTimer.hasFired(time) || memory.birdTarget == round(memory.birdPosition)) {
            memory.directionChangeTimer.reset(time, std::chrono::seconds(100 / memory.birdSpeed++));
            memory.timerCallback(memory);
        }

        if (memory.spriteAnimationTimer.hasFired(time)) {
            memory.spriteAnimationTimer.reset(time, std::chrono::milliseconds(1000 / memory.birdSpeed));
            memory.currentSpriteFrame = (memory.currentSpriteFrame + 1) % decltype(memory.sprite)::frameCount;
        }

        // clear the screen
        std::memset(memory.vram.data(), (uint8_t)clearColor, DrawBufferWidth * DrawBufferHeight);

        // draw the testimage
        imageCopy(memory.imageDecoded, memory.vram);



        // draw the palette in the first rows
    //    for (int y = 0; y < memory.palette.size() / 2; ++y) {
    //    for (int x = 0; x < DrawBufferWidth; ++x) {
    //        put(memory.vram.data(), Vec2i{ x, y }, (x * 16 / DrawBufferWidth) + y / 8 * 16);
    //    } }

        // draw faufau testimage
        imageCopy(memory.faufauDecoded, memory.vram);

        auto subImage = makeSubImage(memory.faufauDecoded, 0, 0, 16, 16);
        auto subImage2 = makeSubImage(memory.vram, 100, 100, 16, 16);
        imageBlitWithTransparentColor(subImage, subImage2, 0);

        if (input.textLength) {
            auto text = array_view<const char>{input.text_utf8, input.textLength};

            for (auto utf8 : Utf8CodepointsView<array_view<const char>>{text}) {
                if (utf8 < 128) {
                    // input is in ASCII range
                    switch (utf8) {
                        case '+': memory.textBuffer[memory.textCursorPosition]++; break;
                        case '-': memory.textBuffer[memory.textCursorPosition]--; break;
                        case 0x7F: // Delete == backspace on modern keyboards
                        case 0x08: // backspace
                            if (memory.textCursorPosition > 0) {
                                memory.textBuffer[--memory.textCursorPosition] = Text::CharacterTable[' '];
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
                            uint8_t outputChar = Text::CharacterTable[utf8];
                            if (outputChar != 0xFF) {
                                memory.textBuffer[memory.textCursorPosition++] = outputChar;
                            } else {
                                auto character = Text::CharacterForCodepoint(utf8);
                                if (character) {
                                    memory.textBuffer[memory.textCursorPosition++] = *character;
                                }
                                else {
                                    char upper = static_cast<uint8_t>(utf8) >> 4;
                                    char lower = static_cast<uint8_t>(utf8) & 0xF;
                                    memory.textBuffer[memory.textCursorPosition++] = Text::CharacterTable[upper > 9 ? (upper - 10) + 'a' : upper + '0'];
                                    memory.textBuffer[memory.textCursorPosition++] = Text::CharacterTable[lower > 9 ? (lower - 10) + 'a' : lower + '0'];
                                }
                            }
                        }
                    }
                } else {
                    switch (utf8) {
                        // MacBook Arrow Keys
                        case 0xF700: memory.textCursorPosition -= TextLineLength; break;
                        case 0xF701: memory.textCursorPosition += TextLineLength; break;
                        case 0xF702: memory.textCursorPosition -= 1; break;
                        case 0xF703: memory.textCursorPosition += 1; break;
                        // macbook fn + backspace = delete?
                        case 0xF728: memory.textBuffer[memory.textCursorPosition] = Text::CharacterTable[' ']; break;
                        default:
                        {
                            auto character = Text::CharacterForCodepoint(utf8);
                            if (character) {
                                memory.textBuffer[memory.textCursorPosition++] = *character;
                            }
                            printf("%x\n", utf8);
                        }
                    }
                }

            }

            memory.textCursorPosition = std::clamp(memory.textCursorPosition, 0, (memory.textLastLine - memory.textFirstLine + 1) * TextLineLength);
        }

        // do some experimentation in the vram
        compiletime auto wrap = [](auto p) { return wrapAround2d(p, Vec2i(), Vec2i{DrawBufferWidth, DrawBufferHeight});};
        compiletime auto clip = [=](const auto& p) { return (Vec2i{0,0} <= p) && (p < BufferSize); };

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
                    memory.vram.pixel(wrap(p)) = green;
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

        if (memory.points[0] != memory.points[1])
        for (auto p : Generators::Line{memory.points[0], memory.points[1]})
            whitePixel(p);

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
        auto birdDistance = itof(memory.birdTarget) - memory.birdPosition;
        memory.birdPosition = memory.birdPosition + static_cast<float>(input.elapsedTime_s) * memory.birdSpeed * normalized(birdDistance);
        memory.birdPosition = clamp(memory.birdPosition, Vec2f{}, Vec2f{DrawBufferWidth - 1, DrawBufferHeight - 1});

        memory.vram.pixel(memory.birdTarget) = lightBlue;

        memory.tone.mod.frequency = 5 * 100.f / (length(birdDistance) + 1);
        memory.tone.mod.amplitude = memory.birdPosition.x / 300;
        // draw
        blitSprite(memory.sprite, memory.currentSpriteFrame, memory.vram.data(), DrawBufferWidth, truncate(memory.birdPosition), Vec2i{}, Vec2i{DrawBufferWidth, DrawBufferHeight});

        // draw text buffer
        uint8_t* drawPointer = memory.vram.line(memory.vram.height() - TextCharacterH).firstPixel;
        uint8_t* textPointer = memory.textBuffer.data();
        uint8_t* textColorPointer = memory.textColors.data();

        int cursorPosition = memory.textCursorPosition;
        for (int line = 0; line < TextLines; ++line) {
            uint8_t* linePointer = drawPointer;
            if (line >= memory.textFirstLine && line <= memory.textLastLine) {
                for (int y = TextCharacterH - 1; y >= 0; --y) {
                    uint64_t* dst = reinterpret_cast<uint64_t*>(linePointer);
                    for (int pos = 0; pos < TextLineLength; ++pos)
                    {
                        const uint8_t t = textPointer[pos];
                        const uint8_t color = textColorPointer[pos];
                        const uint64_t pixels8 = spread(memory.characterROM.pixel(0, t * 8 + y));
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
            memory.isCursorOn = !memory.isCursorOn;
        }

        if (memory.isCursorOn) {
            int cursorY = (TextLines - 1) - memory.textCursorPosition / TextLineLength;
            int cursorX = memory.textCursorPosition % TextLineLength;
            (Rectangle{{cursorX * TextCharacterW, cursorY * TextCharacterH}, {(cursorX + 1) * TextCharacterW - 1, (cursorY + 1) * TextCharacterH - 1}} | forEach(whitePixel)).run();
        }

        return output;
    }

    static void writeDrawBuffer(TestBedMemory& memory, DrawBuffer& buffer) {
        uint8_t* vram = memory.vram.data();
        uint32_t* drawBuffer = buffer.data();

        constant auto stride = sizeof(uint64_t);
        constant auto width = DrawBuffer{}.width();
        constant auto height = DrawBuffer{}.height();
        constant auto destpitch = DrawBuffer{}.pitch();
        constant auto vrampitch = decltype(memory.vram){}.pitch();
        if constexpr (width % stride == 0)
        {
            const uint32_t* palette = memory.palette.data();
            for (uint32_t y = 0; y < height; ++y) {
                uint64_t* src = reinterpret_cast<uint64_t*>(vram + y * vrampitch);
                uint64_t* dst = reinterpret_cast<uint64_t*>(drawBuffer + y * destpitch);
                for (uint32_t x = 0; x < width; x += stride) {
                    const uint64_t sourcePixel8 = *src++;

                    dst[0] = static_cast<uint64_t>(palette[sourcePixel8 >> 0 & 0xff]) |
                        static_cast<uint64_t>(palette[sourcePixel8 >> 8 & 0xff]) << 32;
                    dst[1] = static_cast<uint64_t>(palette[sourcePixel8 >> 16 & 0xff]) |
                        static_cast<uint64_t>(palette[sourcePixel8 >> 24 & 0xff]) << 32;
                    dst[2] = static_cast<uint64_t>(palette[sourcePixel8 >> 32 & 0xff]) |
                        static_cast<uint64_t>(palette[sourcePixel8 >> 40 & 0xff]) << 32;
                    dst[3] = static_cast<uint64_t>(palette[sourcePixel8 >> 48 & 0xff]) |
                        static_cast<uint64_t>(palette[sourcePixel8 >> 56 & 0xff]) << 32;
                    dst += 4;
                }
            }
        }
        else {
            uint32_t* pixel = buffer.data();

            for (unsigned y = 0; y < DrawBufferHeight; ++y)
            for (unsigned x = 0; x < DrawBufferWidth; ++x)
                *pixel++ = memory.palette[*vram++];
        }
    }



    static void writeAudioBuffer(TestBedMemory& memory, void* buffer, const AudioBufferDescriptor& bufferDescriptor) {
        float timeStep = 1.0f / static_cast<float>(bufferDescriptor.sampleRate);

        struct Frame {
            int16_t left, right;
        };

        auto frames = reinterpret_cast<Frame*>(buffer);
        for (unsigned int i = 0; i < bufferDescriptor.framesPerBuffer; ++i) {
            auto& step = memory.sequencer.value();
            if (step.frequency > 0) {
                memory.tone.carrier.frequency = step.frequency;
            }
            if (!memory.envelope.on && step.amplitude > 0.0f) {
                memory.envelope.on = true;
                memory.envelope.amplitude = step.amplitude;
            } 
            if (memory.envelope.on && step.amplitude == 0.0f) {
                memory.envelope.on = false;
            }
            memory.tone.carrier.amplitude = memory.envelope.value();
            int16_t value = static_cast<int16_t>(2000 * memory.tone.value());
            frames[i].left = value;
            frames[i].right = value;
            memory.sequencer.advance(timeStep);
            memory.tone.advance(timeStep);
            memory.envelope.advance(timeStep);
        }

    }
};
