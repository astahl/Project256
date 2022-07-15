//
//  AudioStuff.hpp
//  Project256
//
//  Created by Andreas Stahl on 15.07.22.
//

#pragma once
#include <cmath>
#include <numbers>
#include "../Math/Vec2Math.hpp"

template<typename T>
concept aSoundGenerator = requires (T& t) {
    t.value();
    t.advance(0.0f);
};

template<typename T>
concept aToneGenerator = aSoundGenerator<T> && requires (T& t) {
    t.frequency;
};

enum class Note : uint8_t {
    A0 = 21,
    A0Sharp = 22,
    B0Flat = A0Sharp,
    B0 = 23,
    C1 = 24,
    C1Sharp = 25,
    D1Flat = C1Sharp,
    D1 = 26,
    D1Sharp = 27,
    E1Flat = D1Sharp,
    E1 = 28,
    F1 = 29,
    F1Sharp = 30,
    G1Flat = F1Sharp,
    G1 = 31,
    G1Sharp = 32,
    A1Flat = G1Sharp,
    A1 = 33,
    A1Sharp = 34,
    B1Flat = A1Sharp,
    B1 = 35,
    C2 = 36,
    C2Sharp = 37,
    D2Flat = C2Sharp,
    D2 = 38,
    D2Sharp = 39,
    E2Flat = D2Sharp,
    E2 = 40,
    F2 = 41,
    F2Sharp = 42,
    G2Flat = F2Sharp,
    G2 = 43,
    G2Sharp = 44,
    A2Flat = G2Sharp,
    A2 = 45,
    A2Sharp = 46,
    B2Flat = A2Sharp,
    B2 = 47,
    C3,
    C3Sharp,
    D3Flat = C3Sharp,
    D3,
    D3Sharp,
    E3Flat = D3Sharp,
    E3,
    F3,
    F3Sharp,
    G3Flat = F3Sharp,
    G3,
    G3Sharp,
    A3Flat = G3Sharp,
    A3,
    A3Sharp,
    B3Flat = A3Sharp,
    B3,
    C4,
    C4Sharp,
    D4Flat = C4Sharp,
    D4,
    D4Sharp,
    E4Flat = D4Sharp,
    E4,
    F4,
    F4Sharp,
    G4Flat = F4Sharp,
    G4,
    G4Sharp,
    A4Flat = G4Sharp,
    A4,
    A4Sharp,
    B4Flat = A4Sharp,
    B4,
    C5,
    C5Sharp,
    D5Flat = C5Sharp,
    D5,
    D5Sharp,
    E5Flat = D5Sharp,
    E5,
    F5,
    F5Sharp,
    G5Flat = F5Sharp,
    G5,
    G5Sharp,
    A5Flat = G5Sharp,
    A5,
    A5Sharp,
    B5Flat = A5Sharp,
    B5,

    C6,
    C6Sharp,
    D6Flat = C6Sharp,
    D6,
    D6Sharp,
    E6Flat = D6Sharp,
    E6,
    F6,
    F6Sharp,
    G6Flat = F6Sharp,
    G6,
    G6Sharp,
    A6Flat = G6Sharp,
    A6,
    A6Sharp,
    B6Flat = A6Sharp,
    B6,

    C7,
    C7Sharp,
    D7Flat = C7Sharp,
    D7,
    D7Sharp,
    E7Flat = D7Sharp,
    E7,
    F7,
    F7Sharp,
    G7Flat = F7Sharp,
    G7,
    G7Sharp,
    A7Flat = G7Sharp,
    A7,
    A7Sharp,
    B7Flat = A7Sharp,
    B7,

    C8,
    C8Sharp,
    D8Flat = C8Sharp,
    D8,
    D8Sharp,
    E8Flat = D8Sharp,
    E8,
    F8,
    F8Sharp,
    G8Flat = F8Sharp,
    G8,
    G8Sharp,
    A8Flat = G8Sharp,
    A8,
    A8Sharp,
    B8Flat = A8Sharp,
    B8,

    C9,
    C9Sharp,
    D9Flat = C9Sharp,
    D9,
    D9Sharp,
    E9Flat = D9Sharp,
    E9,
    F9,
    F9Sharp,
    G9Flat = F9Sharp,
    G9,
    G9Sharp,
    A9Flat = G9Sharp,

};
static_assert(static_cast<uint8_t>(Note::A4) == 69);
static_assert(static_cast<uint8_t>(Note::A9Flat) == 128);

compiletime std::array<float, 128> Frequencies = ([]() {
    std::array<float, 128> result{};
    result[69] = 440.0;
    result[70] = 466.16;
    result[71] = 493.88;
    result[72] = 523.25;
    result[73] = 554.37;
    result[74] = 587.33;
    result[75] = 622.25;
    result[76] = 659.26;
    result[77] = 698.46;
    result[78] = 739.99;
    result[79] = 783.99;
    result[80] = 830.61;

    for (int i = 68; i >= 0; --i) {
        result[i] = 0.5 * result[i + 12];
    }

    for (int i = 81; i < 128; ++i) {
        result[i] = 2 * result[i - 12];
    }

    return result;
})();

compiletime float Frequency(Note note) {
    return Frequencies[static_cast<uint8_t>(note)];
}

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

template <typename T = Step<float>>
struct StepSequencer {
    std::array<T, 16> steps;
    // bpm * 4 / 60
    float stepsPerSecond;
    float t;
    int currentStep;

    static StepSequencer withBpm(float bpm) {
        return {
            .stepsPerSecond = bpm * 4 / 60
        };
    }

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
    T startValue;

    T value() {
        return currentValue;
    }

    void triggerValue(T amp, T threshold = 0) {
        if (!on && amp > threshold) {
            on = true;
            amplitude = amp;
        }
        if (on && amp <= threshold) {
            on = false;
        }
    }

    void advance(float timeStep) {
        if (currentSection == Section::Ready) {
            if (on == false) {
                return;
            }
            startValue = currentValue;
            currentSection = Section::Attack;
        }
        t += timeStep;
        if (currentSection == Section::Attack) {
            if (t < attack) {
                float tAttack = t / attack;
                currentValue = std::lerp(startValue, amplitude, tAttack);
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
                currentSection = percussive ? Section::Release : Section::Sustain;
                t -= decay;
            }
        }
        if (currentSection == Section::Sustain) {
            if (on) {
                currentValue = sustain * amplitude;
            }
            else {
                currentSection = Section::Release;
                t = timeStep;
            }
        }
        if (currentSection == Section::Release) {
            if (on) {
                t = 0;
                currentSection = Section::Ready;
            } else if (t < release) { // also check if envelope was retriggered, skip to ready and attack subsequently
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


template <typename T>
struct EffectDelay {
    std::array<T, AudioFramesPerSecond * 10> buffer;
    size_t read;
    size_t write;
    float feedback;

    void put(T val) {
        buffer[write] = std::lerp(val, value(), feedback);
    }

    T value() {
        return buffer[read];
    }

    void advance(float timeStep) {
        ++read;
        ++write;
        read %= buffer.size();
        write %= buffer.size();
    }
};

