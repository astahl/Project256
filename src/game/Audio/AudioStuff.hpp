//
//  AudioStuff.hpp
//  Project256
//
//  Created by Andreas Stahl on 15.07.22.
//

#pragma once
#include <cmath>
#include <numbers>
#include <type_traits>
#include "../Math/Vec2Math.hpp"
#include "../Utility/CircularIndex.hpp"

template<typename T>
concept aSoundGenerator = requires (T& t) {
    typename T::AmplitudeType;
    t.value();
    t.advance(0.0f);
};

template<typename T>
concept aToneGenerator = aSoundGenerator<T> && requires (T& t) {
    t.frequency;
};

template<typename T>
struct Amplitude {};
template<>
struct Amplitude<float> {
    using Type = float;
    compiletime float Max = 1.0f;
    compiletime float Min = -1.0f;
    compiletime float Zero = 0.0f;
};


template<>
struct Amplitude<int16_t> {
    using Type = int16_t;
    compiletime int16_t Max = INT16_MAX;
    compiletime int16_t Min = INT16_MIN;
    compiletime int16_t Zero = 0;
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

template <typename T>
concept aTuneable = requires (T& t) {
    t.setNote(Note::A0);
    {t.currentNote()} -> std::same_as<Note>;
};

template <typename T>
concept aTriggerable = requires (T& t) {
    {t.isOn()} -> std::same_as<bool>;
    t.on(float{});
    t.off();
};

template <typename T>
concept anEnvelope = aTriggerable<T> && aSoundGenerator<T>;

template <typename T>
concept aRawVoice = aSoundGenerator<T> && aTriggerable<T>;

template<typename T>
concept aVoice = aRawVoice<T> && aTuneable<T>;

compiletime std::array<float, 128> Frequencies = ([]() {
    std::array<float, 128> result{};
    result[69] = 440.0f;
    result[70] = 466.16f;
    result[71] = 493.88f;
    result[72] = 523.25f;
    result[73] = 554.37f;
    result[74] = 587.33f;
    result[75] = 622.25f;
    result[76] = 659.26f;
    result[77] = 698.46f;
    result[78] = 739.99f;
    result[79] = 783.99f;
    result[80] = 830.61f;

    for (int i = 68; i >= 0; --i) {
        result[i] = 0.5f * result[i + 12];
    }

    for (int i = 81; i < 128; ++i) {
        result[i] = 2.0f * result[i - 12];
    }

    return result;
})();

compiletime float Frequency(Note note) {
    return Frequencies[static_cast<uint8_t>(note)];
}

template<typename T>
struct SineWave {
    using AmplitudeType = T;
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
    using AmplitudeType = T;
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
    using AmplitudeType = T;
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
    using AmplitudeType = T;
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
    using AmplitudeType = T;
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
    using AmplitudeType = T;
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

template<aToneGenerator T>
struct Sweep {
    using AmplitudeType = typename T::AmplitudeType;

    T generator;
    AmplitudeType amplitude;
    float frequency;
    float frequencyTo;
    float t;
    float rate; // 1 / duration
    constexpr AmplitudeType value() const {
        return generator.value();
    }

    void on(AmplitudeType a) {
        if (a > 0) {
            generator.amplitude = a;
        } else {
            off();
        }
    }

    void off() {
        t = 0;
        generator.amplitude = 0;
    }

    bool isOn() {
        return t > 0 && t <= 1;
    }

    void advance(float timeStep) {
        if (t <= 1.0f) {
            generator.frequency = std::lerp(frequency, frequencyTo, t);
        }
        generator.advance(timeStep);
        t += rate * timeStep;
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
    using AmplitudeType = T;

    float attack;
    float decay;
    float sustain;
    float release;
    bool percussive;

    AmplitudeType amplitude;
    float t;
    bool mIsOn;
    enum class Section {
        Ready, Attack, Decay, Sustain, Release
    } currentSection;
    AmplitudeType currentValue;
    AmplitudeType startValue;

    AmplitudeType value() {
        return currentValue;
    }

    void on(AmplitudeType amp) {
        if (!mIsOn && amp > 0) {
            mIsOn = true;
            amplitude = amp;
        }
        if (mIsOn && amp <= 0) {
            mIsOn = false;
        }
    }

    void off() {
        mIsOn = false;
    }

    bool isOn() {
        return mIsOn;
    }

    void advance(float timeStep) {
        if (currentSection == Section::Ready) {
            if (mIsOn == false) {
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
            if (mIsOn) {
                currentValue = sustain * amplitude;
            }
            else {
                currentSection = Section::Release;
                t = timeStep;
            }
        }
        if (currentSection == Section::Release) {
            if (mIsOn) {
                t = 0;
                currentSection = Section::Ready;
            } else if (t < release) { // also check if envelope was retriggered, skip to ready and attack subsequently
                float tRelease = t / release;
                currentValue = std::lerp(sustain * amplitude, T{}, tRelease);
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
    std::array<T, AudioFramesPerSecond * 2> buffer;
    CircularIndex<AudioFramesPerSecond * 2> read;
    CircularIndex<AudioFramesPerSecond * 2> write;
    float feedback;

    void put(T val) {
        buffer[write.value] = std::lerp(val, value(), feedback);
    }

    void setDelayTime(float seconds) {
        size_t offsetFrames = static_cast<size_t>(seconds * AudioFramesPerSecond);
        read = offsetFrames;
    }

    T value() {
        return buffer[read.value];
    }

    void advance(float /* timeStep */) {
        ++read;
        ++write;
    }
};



template <anEnvelope E, aToneGenerator V>
struct EnvelopeVoice {
    using AmplitudeType = typename E::AmplitudeType;

    E envelope;
    V wave;
    Note mCurrentNote;

    void setNote(Note note) {
        mCurrentNote = note;
        wave.frequency = Frequency(note);
    }

    void on(AmplitudeType amplitude) {
        envelope.on(amplitude);
    }

    void off() {
        envelope.off();
    }

    void advance(float timeStep) {
        envelope.advance(timeStep);
        wave.advance(timeStep);
    }

    AmplitudeType value() {
        return wave.value() * envelope.value();
    }

    bool isOn() {
        return envelope.isOn();
    }

    Note currentNote() {
        return mCurrentNote;
    }
};

template <anEnvelope E, aRawVoice V>
struct EnvelopeRawVoice {
    using AmplitudeType = typename E::AmplitudeType;

    E envelope;
    V wave;
    Note mCurrentNote;

    void setNote(Note note) {
        mCurrentNote = note;
        wave.frequency = Frequency(note);
    }

    void on(AmplitudeType amplitude) {
        envelope.on(amplitude);
        wave.on(amplitude);
    }

    void off() {
        envelope.off();
        wave.off();
    }

    void advance(float timeStep) {
        envelope.advance(timeStep);
        wave.advance(timeStep);
    }

    AmplitudeType value() {
        return wave.value() * envelope.value();
    }

    bool isOn() {
        return envelope.isOn();
    }

    Note currentNote() {
        return mCurrentNote;
    }
};


template <typename T>
using SineSynthVoice = EnvelopeVoice<EnvelopeAdsr<T>, SineWave<T>>;
template <typename T>
using SineSweepVoice = EnvelopeRawVoice<EnvelopeAdsr<T>, Sweep<SineWave<T>>>;


template <aVoice T, size_t N>
struct MultitimbralVoice {
    using AmplitudeType = typename T::AmplitudeType;

    std::array<T, N> voices;
    size_t from, to;

    void use(T voice) {
        voices.fill(voice);
    }

    void on(Note note, AmplitudeType amplitude) {
        for (size_t i = from; i != to; i = (i + 1) % voices.size()) {
            if (voices[i].currentNote() == note) {
                voices[i].setNote(note);
                voices[i].on(amplitude);
                return;
            }
        }
        voices[to].setNote(note);
        voices[to].on(amplitude);
        to += 1;
        to %= voices.size();
        if (to == from) {
            from = to + 1;
            from %= voices.size();
        }
    }

    void off(Note note) {
        for (size_t i = from; i != to; i = (i + 1) % voices.size()) {
            if (voices[i].currentNote() == note) {
                voices[i].off();
            }
        }
        while (from != to) {
            if (!voices[from].value()) {
                from = (from + 1) % voices.size();
            } else {
                break;
            }
        }
    }

    bool isOn() {
        return to != from;
    }

    Note currentNote() {
        size_t last = (to - 1) % voices.size();
        return voices[last].currentNote;
    }

    void advance(float timestep) {
        for (size_t i = from; i != to; i = (i + 1) % voices.size()) {
            voices[i].advance(timestep);
        }
    }

    AmplitudeType value() {
        AmplitudeType sum = {};
        float factor = 1.0f / N;

        for (size_t i = from; i != to; i = (i + 1) % voices.size()) {
            sum += static_cast<AmplitudeType>(factor * voices[i].value());
        }
        return sum;
    }
};


