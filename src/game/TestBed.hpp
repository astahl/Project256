//
//  TestBed.hpp
//  Project256
//
//  Created by Andreas Stahl on 03.07.22.
//

#pragma once

#include "Audio/AudioStuff.hpp"
#include "Audio/Waves.hpp"
#include "FML/RangesAtHome.hpp"
#include "Drawing/InterleavedBitmaps.hpp"
#include "Math/Vec2Math.hpp"
#include "Drawing/Sprites.hpp"
#include "Utility/Timers.hpp"
#include "Utility/Text.hpp"
#include "Utility/FrameInput.hpp"
#include "Drawing/Images.hpp"
#include "Drawing/Palettes.hpp"
#include "Drawing/Generators.hpp"
#include "Project256.h"
#include <mutex>

#include <iostream>



using VRAM = Image<uint8_t, DrawBufferWidth, DrawBufferHeight, ImageOrigin::BottomLeft>;

constant int TextCharacterW = 8;
constant int TextCharacterH = 8;
constant int TextLines = DrawBufferHeight / TextCharacterH;
constant int TextLineLength = DrawBufferWidth / TextCharacterW;


globalvar std::mutex memoryMutex;

struct TestBedMemory {
    // video
    alignas(128) VRAM vram;
    alignas(128) std::array<uint32_t, 256> palette;

    // images
    alignas(8) Image<uint8_t, 320, 256, ImageOrigin::TopLeft> imageDecoded;
    alignas(8) Image<uint8_t, 320, 256, ImageOrigin::TopLeft> faubigDecoded;
    alignas(8) Image<uint8_t, 32, 24, ImageOrigin::TopLeft> faufauDecoded;

    // audio
    FrequencyModulator<SineWave<float>, TriangleWave<float>> tone;
    StepSequencer<Step<float>> sequencer;
    StepSequencer<Step<float>> drumSequencer;
    SineSweepVoice<float> pewpew;
    EnvelopeAdsr<float> envelope;
    EffectDelay<float> delay;
    MultitimbralVoice<SineSynthVoice<float>, 8> voice;

    bool isInitialized;

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

    // gamestate
    Vec2f birdPosition;
    int birdSpeed;
    Vec2i birdTarget;
    Timer directionChangeTimer;
    FunctionPointer<void, TestBedMemory&> timerCallback;
    SpritePicture<5, 2, 4> sprite;
    Timer spriteAnimationTimer;
    int currentSpriteFrame;

    // mouse clicks
    Vec2i mouseDownPosition;
    bool isMouseDown;
    
    std::array<uint8_t, 300 * 1024> scratch;
};


compiletime auto BufferSize = Vec2i{ DrawBufferWidth, DrawBufferHeight};
compiletime auto Center = BufferSize / 2;

void birdDirectionChange(TestBedMemory& memory) {
    memory.birdTarget = (rand2d() % BufferSize);
}

struct TestBed {
    using DrawBuffer = Image<uint32_t, DrawBufferWidth, DrawBufferHeight>;
    using AudioBuffer = Audio::PCM16StereoBuffer<AudioFramesPerBuffer>;
    using MemoryLayout = TestBedMemory;

    static GameOutput doGameThings(TestBedMemory& memory, const FrameInput::Input& input, const PlatformCallbacks& callbacks)
    {
        if (input.frameNumber == 0) {
#ifdef DEBUG
 //           test_myCos();
#endif
        }
        using namespace ranges_at_home;
        using namespace Generators;

        GameOutput output{};
        using Palette = PaletteVGA;
        using Text = CharacterRom::PET;
        const auto time = std::chrono::microseconds(input.upTime_microseconds);

        auto lock = std::scoped_lock(memoryMutex);
        // initialize main memory
        if (input.frameNumber == 0) {
            std::memset(&memory, 0, sizeof(TestBedMemory));

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

            memory.tone.depth = .5f;
            memory.tone.mod.amplitude = 1.0f;

            memory.sequencer = StepSequencer<>::withBpm(90);
            memory.sequencer.steps[0] = { .amplitude = 1.0f, .frequency = Frequency(Note::A3) };
            memory.sequencer.steps[4] = { .amplitude = 1.0f, .frequency = Frequency(Note::G4) };
            memory.sequencer.steps[8] = { .amplitude = 1.0f, .frequency = Frequency(Note::G4) };
            memory.sequencer.steps[12] = { .amplitude = .5f, .frequency = Frequency(Note::G4) };
            memory.sequencer.steps[14] = { .amplitude = .7f, .frequency = Frequency(Note::E4) };

            memory.envelope.percussive = false;
            memory.envelope.attack = 0.01f;
            memory.envelope.sustain = .4f;
            memory.envelope.decay = 0.05f;
            memory.envelope.release = .4f;

            memory.pewpew = {
                .envelope {
                    .attack = .01f,
                    .decay = .01f,
                    .sustain = 1.0f,
                    .release = .01f,
                    .percussive = true,
                },
                .wave {
                    .amplitude = 1.0f,
                    .frequency = 500.0f,
                    .frequencyTo = 50.0f,
                    .rate = 10.0f
                },
            };

            memory.drumSequencer = StepSequencer<>::withBpm(90);
            memory.drumSequencer.steps[0] = { .amplitude = 1.0f };
            memory.drumSequencer.steps[2] = { .amplitude = .5f };
            memory.drumSequencer.steps[4] = { .amplitude = 1.0f };
            memory.drumSequencer.steps[8] = { .amplitude = 1.0f };

            memory.delay.setDelayTime(1.0);
            memory.delay.feedback = 0.8f;

            SineSynthVoice<float> voice{
                .envelope {
                    .attack = 0.1f,
                    .decay = 0.1f,
                    .sustain = .8f,
                    .release = .5f,
                },
                .wave {
                    .amplitude = 1.0f
                }
            };
            memory.voice.use(voice);
            memory.isInitialized = true;
        }


        constant auto black = static_cast<VRAM::PixelType>(findNearest(WebColorRGB::Black, memory.palette).index);
        constant auto white = static_cast<VRAM::PixelType>(findNearest(WebColorRGB::White, memory.palette).index);
        constant auto cyan = static_cast<VRAM::PixelType>(findNearest(WebColorRGB::Cyan, memory.palette).index);
        constant auto lightBlue = static_cast<VRAM::PixelType>(findNearest(WebColorRGB::LightBlue, memory.palette).index);
        constant auto red = static_cast<VRAM::PixelType>(findNearest(WebColorRGB::Red, memory.palette).index);
        constant auto green = static_cast<VRAM::PixelType>(findNearest(WebColorRGB::Green, memory.palette).index);
        constant auto whitePixel = [&](const auto& p) { memory.vram.at(p) = white; };
        constant auto redPixel = [&](const auto& p) { memory.vram.at(p) = red; };
        constant auto greenPixel = [&](const auto& p) { memory.vram.at(p) = green; };
        constant auto lightBluePixel = [&](const auto& p) { memory.vram.at(p) = lightBlue; };

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

        if (!input.text.empty()) {
            auto text = input.text.span();

            for (auto codePoint : Unicode::CodepointsView<decltype(text)>{text}) {
                if (codePoint < 128) {
                    // input is in ASCII range
                    switch (codePoint) {
                        case Unicode::PlusSign:
                            memory.textBuffer[memory.textCursorPosition]++;
                            break;
                        case Unicode::HyphenMinus:
                            memory.textBuffer[memory.textCursorPosition]--;
                            break;
                        case Unicode::Delete: // Delete == backspace on modern keyboards
                        case Unicode::Backspace: // backspace
                            if (memory.textCursorPosition > 0) {
                                memory.textBuffer[--memory.textCursorPosition] = Text::CharacterTable[' '];
                            }
                            break;
                        case Unicode::Tab: // tab
                            memory.textCursorPosition += 4 - ((memory.textCursorPosition) % 4);
                            break;
                        case Unicode::EndOfMedium: // untab
                            memory.textCursorPosition -= 4 - ((memory.textCursorPosition) % 4);
                            break;
                        case Unicode::CarriageReturn: // Carriage Return (Enter on mac)
                        {
                            int lineNumber = memory.textCursorPosition / TextLineLength;
                            memory.textCursorPosition = (lineNumber + 1) * TextLineLength;
                            break;
                        }
                        default: {
                            uint8_t outputChar = Text::CharacterTable[codePoint];
                            if (outputChar != 0xFF) {
                                memory.textBuffer[memory.textCursorPosition++] = outputChar;
                            } else {
                                auto character = Text::CharacterForCodepoint(codePoint);
                                if (character) {
                                    memory.textBuffer[memory.textCursorPosition++] = *character;
                                }
                                else {
                                    char upper = static_cast<uint8_t>(codePoint) >> 4;
                                    char lower = static_cast<uint8_t>(codePoint) & 0xF;
                                    memory.textBuffer[memory.textCursorPosition++] = Text::CharacterTable[upper > 9 ? (upper - 10) + 'a' : upper + '0'];
                                    memory.textBuffer[memory.textCursorPosition++] = Text::CharacterTable[lower > 9 ? (lower - 10) + 'a' : lower + '0'];
                                }
                            }
                        }
                    }
                } else {
                    switch (codePoint) {
                        // MacBook Arrow Keys
                        case Unicode::AppleMacBookArrowUp: memory.textCursorPosition -= TextLineLength; break;
                        case Unicode::AppleMacBookArrowDown: memory.textCursorPosition += TextLineLength; break;
                        case Unicode::AppleMacBookArrowLeft: memory.textCursorPosition -= 1; break;
                        case Unicode::AppleMacBookArrowRight: memory.textCursorPosition += 1; break;
                        // macbook fn + backspace = delete?
                        case Unicode::AppleMacBookDelete: memory.textBuffer[memory.textCursorPosition] = Text::CharacterTable[' ']; break;
                        default:
                        {
                            auto character = Text::CharacterForCodepoint(codePoint);
                            if (character) {
                                memory.textBuffer[memory.textCursorPosition++] = *character;
                            }
                            else {
                                char* printBuffer = reinterpret_cast<char*>(memory.scratch.data());
                                snprintf(printBuffer, 100, "No Character for Codepoint: U+%05x in String \"%s\"", static_cast<uint32_t>(codePoint), reinterpret_cast<const char*>(text.data()));
                                callbacks.log(printBuffer);
                            }
                        }
                    }
                }

            }

            memory.textCursorPosition = std::clamp(memory.textCursorPosition, 0, (memory.textLastLine - memory.textFirstLine + 1) * TextLineLength);
        }

        // do some experimentation in the vram
        compiletime auto wrap = [](auto p) { return wrapAround2d(p, Vec2i(), Vec2i{DrawBufferWidth, DrawBufferHeight});};
        compiletime auto clip = [=](const auto& p) { return (Vec2i{0,0} <= p) && (p < BufferSize); };
        compiletime auto clipped = filter(clip);
        compiletime auto wrapped = transform(wrap);

        if (!input.mouse.track.empty()) {
            Vec2f mousePosition = input.mouse.track.back();
            Vec2i position = truncate(mousePosition);

            auto offset = [=](const auto& p) {
                return p + position;
            };

            if (input.mouse.buttonLeft.transitionCount) {
                memory.mouseDownPosition = position;
            }
            auto atMouse = transform(offset);

            compiletime auto rectangleGenerator = Rectangle{ Vec2i{-3,-3}, Vec2i{3, 3} };

            if (input.mouse.buttonLeft.endedDown) {
                for (auto p : rectangleGenerator | atMouse | clipped)
                    memory.vram.at(wrap(p)) = green;
            }
            
            compiletime auto crossGenerator = concat(HLine{{-3, 0}, 7}, VLine{{0, -3}, 7});
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


        if (input.mouse.buttonLeft.transitionCount) {
            localpersist Note note[4] = {Note::A4, Note::C5, Note::E5, Note::G5};
            localpersist int currentNote = 0;
            if (input.mouse.buttonLeft.endedDown ) {
                memory.voice.on(note[currentNote], 1.0f);
            }
            else {
                memory.voice.off(note[currentNote]);
                currentNote = (currentNote + 1) % 4;
            }
        }

        if (input.mouse.buttonLeft.endedDown && !input.mouse.track.empty()) {
            Vec2i mousePosition = truncate(input.mouse.track.back());
            for (auto p : Generators::Line(memory.mouseDownPosition, mousePosition)) {
                whitePixel(p);
            }
        }
        // write controller state to the screen
        for (int i = 0; i < InputMaxControllers; ++i) {
            auto& controller = input.controllers[i];

            Vec2i p{ 10, (i + 1) * 10 };
            for (auto x : Generators::HLine{p, -10}) {
                if (!controller.isConnected)
                    redPixel(x);
                else
                    greenPixel(x);
            }
            p.y += 1;
            for (auto x : Generators::HLine{p, -10}) {
                if (controller.isActive)
                    lightBluePixel(x);
            }
            p.x += 2;
            for (auto& button : array_view<const Button>{ &controller.shoulderLeft, InputControllerButtonCount }) {
                if (button.endedDown)
                    whitePixel(p);
                p.x += 2;
            }

            for (auto& axis1 : { controller.triggerLeft, controller.triggerRight } ) {
                if (axis1.trigger.endedDown)
                    whitePixel(p);
                p.x += 2;
            }

            for (auto& axis2 : {controller.stickLeft, controller.dPad, controller.stickRight}) {
                if (axis2.up.endedDown)
                    whitePixel(p + Vec2i{ 1,1 });
                if (axis2.down.endedDown)
                    whitePixel(p + Vec2i{ 1,-1 });
                if (axis2.left.endedDown)
                    whitePixel(p + Vec2i{ });
                if (axis2.right.endedDown)
                    whitePixel(p + Vec2i{ 2,0 });

                p.x += 8;

                for (auto x : Generators::Line{p, round(p + 2 * axis2.end)} | clipped ) {
                    whitePixel(x);
                }

                p.x += 8;
            }

            p.x = 10;
            p.y = (i + 1) * 10 + 3;
            if (controller.buttonBack.transitionCount != 0 && controller.buttonBack.endedDown) {
                output.shouldQuit = true;
            }
        }


        // update
        auto birdDistance = itof(memory.birdTarget) - memory.birdPosition;
        float seconds = static_cast<float>(input.elapsedTime_s.count());
        memory.birdPosition = memory.birdPosition + seconds * memory.birdSpeed * normalized(birdDistance);
        memory.birdPosition = clamp(memory.birdPosition, Vec2f{}, Vec2f{DrawBufferWidth - 1, DrawBufferHeight - 1});

        memory.vram.at(memory.birdTarget) = lightBlue;

        // draw
        blitSprite(memory.sprite, memory.currentSpriteFrame, memory.vram.data(), DrawBufferWidth, truncate(memory.birdPosition), Vec2i{}, Vec2i{DrawBufferWidth, DrawBufferHeight});

        // draw text buffer
        uint8_t* drawPointer = memory.vram.line(memory.vram.height() - TextCharacterH).data();
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
                        const uint64_t pixels8 = spread(memory.characterROM.at({0, t * 8 + y}));
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
        auto lock = std::scoped_lock(memoryMutex);
        if (!memory.isInitialized) return;
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



    static void writeAudioBuffer(TestBedMemory& memory, AudioBuffer& buffer, const AudioBufferDescriptor& bufferDescriptor) {

        auto lock = std::scoped_lock(memoryMutex);
        if (!memory.isInitialized) {
            buffer.clear();
            return;
        }

        float timeStep = 1.0f / static_cast<float>(bufferDescriptor.sampleRate);


        for (auto& frame : buffer.frames) {
            auto& step = memory.sequencer.value();
            if (step.frequency > 0) {
                memory.tone.carrier.frequency = step.frequency;
                memory.tone.mod.frequency = 3 * step.frequency;
            }
            memory.envelope.on(step.amplitude);

            auto& drumStep = memory.drumSequencer.value();
            memory.pewpew.on(drumStep.amplitude);

            memory.tone.carrier.amplitude = memory.envelope.value();
            memory.delay.put(memory.tone.value());
            auto mix = memory.voice.value() + memory.delay.value() + memory.tone.value() + memory.pewpew.value();

//            auto result = (1.0f - (1.0f / ((mix * mix) + 1.0f)));
//            result = mix < 0 ? -result : result;
            auto result = mix / 4;
            int16_t value = static_cast<int16_t>(std::numeric_limits<int16_t>::max() * result);

            frame.samples[0] = value;
            frame.samples[1] = value;
            memory.sequencer.advance(timeStep);
            memory.drumSequencer.advance(timeStep);
            memory.pewpew.advance(timeStep);
            memory.tone.advance(timeStep);
            memory.envelope.advance(timeStep);
            memory.delay.advance(timeStep);
            memory.voice.advance(timeStep);
        }

    }
};
