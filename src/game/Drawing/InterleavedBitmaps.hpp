//
//  InterleavedBitmaps.hpp
//  Project256
//
//  Parsing of interleaved bitmap files (deluxe paint)
//  https://wiki.amigaos.net/wiki/ILBM_IFF_Interleaved_Bitmap
//  Created by Andreas Stahl on 26.06.22.
//

#pragma once

#include <cstdint>
#include <optional>

enum class ILBMMasking: uint8_t {
    None = 0,
    HasMask = 1,
    HasTransparentColor = 2,
    Lasso = 3,
};

enum class ILBMCompression : uint8_t {
    None = 0,
    ByteRun1 = 1
};

enum class ILBMCRangeCycleFlags : int16_t {
    NoCycling = 0,
    Active = 1,
    Reverse = 2,
};

enum class ILBMCCRTCycleDirection : int16_t {
    NoCycling = 0,
    Forward = 1,
    Backwards = -1,
};

struct ILBMNames {
    compiletime char Form[4] = {'F','O','R','M'};
    compiletime char ILBM[4] = {'I','L','B','M'};
    compiletime char BitmapHeader[4] = {'B','M','H','D'};
    compiletime char ColorMap[4] = {'C','M','A','P'};
    compiletime char Grab[4] = {'G','R','A','B'};
    compiletime char Dest[4] = {'D','E','S','T'};
    compiletime char Sprite[4] = {'S','P','R','T'};
    compiletime char Amiga[4] = {'C','A','M','G'};
    compiletime char ColorRange[4] = {'C','R','N','G'};
    compiletime char ColorCyclingRangeAndTiming[4] = {'C','C','R','T'};
    compiletime char Body[4] = {'B','O','D','Y'};
};

compiletime bool ILBMCompareNames(const char* a, const char* b) {
    for (int i = 0; i < 4; ++i)
        if (a[i] != b[i])
            return false;
    return true;
}

#ifndef __cpp_lib_endian
enum class endian
{
#ifdef _WIN32
    little = 0,
    big    = 1,
    native = little
#else
    little = __ORDER_LITTLE_ENDIAN__,
    big    = __ORDER_BIG_ENDIAN__,
    native = __BYTE_ORDER__
#endif
};
#endif

template <typename T, endian Endianness = endian::native>
struct Endian {
    T value{};

    template<endian OtherEndianness>
    constexpr Endian(const Endian<T, OtherEndianness>& other) {
        if constexpr (Endianness == OtherEndianness) {
            value = other.value;
        } else {
            uint8_t* ptr = reinterpret_cast<uint8_t*>(&value);
            const uint8_t* otherPtr = reinterpret_cast<const uint8_t*>(&other.value);

            for (int i = 0; i < sizeof(T); ++i) {
                ptr[i] = otherPtr[sizeof(T) - 1 - i];
            }
        }
    }

    T native() const {
        Endian<T, endian::native> value(*this);
        return value.value;
    }
};

#pragma pack(push)
#pragma pack(1)

template <endian E = endian::native>
struct ILBMChunkPrelude {
    char chunkName[4];
    Endian<int, E> chunkLength;
};

template <endian E = endian::native>
struct ILBMHeader {
    Endian<uint16_t, E> width, height;
    Endian<int16_t, E> pixelPositionX, pixelPositionY;
    uint8_t planeCount;
    ILBMMasking masking;
    ILBMCompression compression;
    const uint8_t pad1{};
    Endian<uint16_t, E> transparentColorIndex;
    uint8_t pixelAspectX, pixelAspectY;
    Endian<int16_t, E> pageWidth, pageHeight;

    template <endian OtherEndianness>
    ILBMHeader(const ILBMHeader<OtherEndianness>& other)
    : width(other.width), height(other.height),
    pixelPositionX(other.pixelPositionX), pixelPositionY(other.pixelPositionY),
    planeCount(other.planeCount),
    masking(other.masking),
    compression(other.compression),
    transparentColorIndex(other.transparentColorIndex),
    pixelAspectX(other.pixelAspectX), pixelAspectY(other.pixelAspectY),
    pageWidth(other.pageWidth), pageHeight(other.pageHeight){};
};

template <endian E = endian::native>
struct ILBMGrab {
    Endian<int16_t, E> pointX, pointY;

    template <endian OtherEndianness>
    ILBMGrab(const ILBMGrab<OtherEndianness>& other)
    : pointX(other.pointX), pointY(other.pointY) {};
};

template <endian E = endian::native>
struct ILBMDestMerge {
    uint8_t depth;
    const uint8_t pad1{};
    Endian<uint16_t,E> planePick, planeOnOff, planeMask;

    template <endian OtherEndianness>
    ILBMDestMerge(const ILBMDestMerge<OtherEndianness>& other)
    : depth(other.depth)
    , planePick(other.planePick), planeOnOff(other.planeOnOff), planeMask(other.planeMask) {};
};

template <endian E = endian::native>
struct ILBMSprite {
    Endian<uint16_t,E> spritePrecedence;

    template <endian OtherEndianness>
    ILBMSprite(const ILBMSprite<OtherEndianness>& other)
    : spritePrecedence(other.spritePrecedence) {};
};

struct ILBMColor {
    uint8_t red, green, blue;
};
#pragma pack(pop)

// todo replace this with a range
struct ILBMColorMap {
    std::ptrdiff_t size;
    const ILBMColor* colors;

    const ILBMColor* begin() const { return colors; }
    const ILBMColor* end() const { return colors + size; }
};

// todo replace this with a range
struct ILBMBody {
    std::ptrdiff_t size;
    const uint8_t* data;

    const uint8_t* begin() const { return data; }
    const uint8_t* end() const { return data + size; }
};

static_assert(sizeof(ILBMHeader<endian::native>) == 20);

template <endian E = endian::big>
struct ILBMDataParser {
    const static std::ptrdiff_t HEADER_PRELUDE_OFFSET = 12;
    const static std::ptrdiff_t CHUNK_NOT_FOUND = -1;
    const static std::ptrdiff_t CHUNK_PRELUDE_SIZE = sizeof(ILBMChunkPrelude<E>);
    const uint8_t* data;
    const int dataSize;

    bool isValid() const {
        const uint8_t *readPointer = data;
        if (!ILBMCompareNames(reinterpret_cast<const char*>(readPointer), ILBMNames::Form)) {
            return false;
        }
        readPointer += 4;
        Endian<int> length = *reinterpret_cast<const Endian<int, E>*>(readPointer);
        if(length.value > dataSize - 8) {
            assert(false); // endianness problem or buffer was too small
            return false;
        }
        readPointer += 4;
        if (!ILBMCompareNames(reinterpret_cast<const char*>(readPointer), ILBMNames::ILBM)) {
            return false;
        }
        return true;
    }

    template <endian Target = endian::native>
    ILBMHeader<Target> getHeader() const {
        assert(dataSize > sizeof(ILBMHeader<E>) + HEADER_PRELUDE_OFFSET + CHUNK_PRELUDE_SIZE);
        ILBMHeader<Target> header = *reinterpret_cast<const ILBMHeader<E>*>(&data[HEADER_PRELUDE_OFFSET + CHUNK_PRELUDE_SIZE]);
        return header;
    }

    std::ptrdiff_t findChunk(const char* name, std::ptrdiff_t offset = HEADER_PRELUDE_OFFSET) const {
        auto chunkPrelude = reinterpret_cast<const ILBMChunkPrelude<E>*>(data + offset);
        Endian<int> chunkLength = chunkPrelude->chunkLength;
        while (offset < dataSize && !ILBMCompareNames(chunkPrelude->chunkName, name) && chunkLength.value != 0) {
            offset += chunkLength.value + CHUNK_PRELUDE_SIZE;
            chunkPrelude = reinterpret_cast<const ILBMChunkPrelude<E>*>(data + offset);
            chunkLength = chunkPrelude->chunkLength;
        }
        if (chunkLength.value == 0) {
            return CHUNK_NOT_FOUND;
        }
        if (offset > dataSize) {
            return CHUNK_NOT_FOUND;
        }
        return offset;
    }

    ILBMColorMap getColorMap() const {
        auto offset = findChunk(ILBMNames::ColorMap);
        if (offset == CHUNK_NOT_FOUND) {
            return ILBMColorMap{};
        }
        auto chunkPrelude = reinterpret_cast<const ILBMChunkPrelude<E>*>(data + offset);
        Endian<int> length = chunkPrelude->chunkLength;
        return { length.value / 3, reinterpret_cast<const ILBMColor*>(data + offset + CHUNK_PRELUDE_SIZE) };
    }

    template <endian Target = endian::native>
    std::optional<ILBMGrab<Target>> getGrab() const {
        auto offset = findChunk(ILBMNames::Grab);
        if (offset == CHUNK_NOT_FOUND) {
            return {};
        } else {
            return *reinterpret_cast<const ILBMGrab<E>*>(data + offset + CHUNK_PRELUDE_SIZE);
        }
    }

    template <endian Target = endian::native>
    std::optional<ILBMDestMerge<Target>> getDestMerge() const {
        auto offset = findChunk(ILBMNames::Dest);
        if (offset == CHUNK_NOT_FOUND) {
            return {};
        } else {
            return *reinterpret_cast<const ILBMDestMerge<E>*>(data + offset + CHUNK_PRELUDE_SIZE);
        }
    }

    template <endian Target = endian::native>
    std::optional<Endian<int, Target>> getAmigaDisplayModeFlags() const {
        auto offset = findChunk(ILBMNames::Amiga);
        if (offset == CHUNK_NOT_FOUND) {
            return {};
        } else {
            return *reinterpret_cast<const Endian<int, E>*>(data + offset + CHUNK_PRELUDE_SIZE);
        }
    }

    ILBMBody getBody() const {
        auto offset = findChunk(ILBMNames::Body);
        if (offset == CHUNK_NOT_FOUND) {
            return ILBMBody{};
        }
        auto chunkPrelude = reinterpret_cast<const ILBMChunkPrelude<E>*>(data + offset);
        Endian<int> length = chunkPrelude->chunkLength;
        return { length.value, reinterpret_cast<const uint8_t*>(data + offset + CHUNK_PRELUDE_SIZE) };
    }
};
