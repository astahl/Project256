//
//  Images.hpp
//  Project256
//
//  Created by Andreas Stahl on 02.07.22.
//

#pragma once

#include <cstdint>
#include <type_traits>
#include <array>

enum class ImageOrigin {
    BottomLeft,
    TopLeft,
    TopRight,
    BottomRight
};

enum class ImageLineOrder {
    BottomToTop,
    TopToBottom
};

template <typename Pixel, size_t Width, size_t Height, ImageOrigin O = ImageOrigin::BottomLeft, size_t Pitch = Width>
struct Image {

    template <typename T = Pixel>
    struct Line {
        using PixelType = T;
        T* firstPixel;
        constexpr T& pixel(ptrdiff_t x) {
            assert(x < Width);
            assert(x >= 0);
            return *(firstPixel + x);
        }
        constexpr size_t size() { return Width; }
        constexpr T* begin() { return firstPixel; }
        constexpr T* end() { return firstPixel + Width; }
    };

    template <bool Flipped, typename T = Pixel>
    struct LinesView {
        using PixelType = T;
        T* data;
        struct Sentinel {};
        struct Iterator {
            T* iteratorData;
            ptrdiff_t y {0};
            constexpr Line<T> operator*() const { return {.firstPixel = iteratorData + y * Pitch}; }
            constexpr Iterator& operator++() { ++y; return *this; }
            constexpr bool operator!=(Sentinel) const { return y != Height; }
        };
        struct FlippedIterator {
            T* iteratorData;
            ptrdiff_t y {Height - 1};
            constexpr Line<T> operator*() const { return {.firstPixel = iteratorData + y * Pitch}; }
            constexpr FlippedIterator& operator++() { --y; return *this; }
            constexpr bool operator!=(Sentinel) const { return y >= 0; }
        };

        constexpr auto begin() const {
            if constexpr (Flipped)
                return FlippedIterator{.iteratorData = data};
            else return Iterator{.iteratorData = data};
        }

        constexpr Sentinel end() const { return {}; }
        constexpr size_t size() const { return Height; }
        constexpr size_t length() const { return Width; }
    };

    using PixelType = Pixel;
    using LineType = Line<PixelType>;
    constant ImageOrigin Origin = O;
    constant ImageLineOrder LineOrder = Origin == ImageOrigin::BottomLeft ? ImageLineOrder::BottomToTop : ImageLineOrder::TopToBottom;

    std::array<Pixel, Pitch * Height> pixels;

    constexpr Pixel* data() {
        return pixels.data();
    }

    constexpr uint8_t* bytes() {
        return reinterpret_cast<uint8_t*>(pixels.data());
    }

    constexpr size_t bytesSize() {
        return pixels.size() * sizeof(PixelType);
    }

    constexpr size_t size() const {
        return pixels.size();
    }

    constexpr Pixel& pixel(ptrdiff_t x, ptrdiff_t y) {
        assert(x < Width);
        assert(y < Height);
        assert(x >= 0);
        assert(y >= 0);
        return pixels.at(x + y * Pitch);
    }

    constexpr Pixel& pixel(Vec2i pos) {
        return pixel(pos.x, pos.y);
    }

    constexpr LineType line(ptrdiff_t y) {
        assert(y >= 0);
        assert(y < Height);
        return { .firstPixel = &pixel(0, y) };
    }

    constexpr LineType lineFlipped(ptrdiff_t y) {
        assert(y >= 0);
        assert(y < Height);
        return { .firstPixel = &pixel(0, Height - y - 1 ) };
    }

    template <ImageLineOrder DesiredLineOrder = LineOrder>
    constexpr auto lines() {
        constexpr bool Flipped = (DesiredLineOrder != LineOrder);
        return LinesView<Flipped, Pixel> {.data = pixels.data() };
    }

    template <ImageLineOrder DesiredLineOrder = LineOrder>
    constexpr auto lines() const {
        constexpr bool Flipped = (DesiredLineOrder != LineOrder);
        return LinesView<Flipped, const Pixel> {.data = pixels.data() };
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

    constexpr int originX() const {
        return 0;
    }

    constexpr int originY() const {
        return 0;
    }

};


template <typename TImage, ImageOrigin O = ImageOrigin::BottomLeft>
struct ImageView {
    using ImageType = std::decay_t<TImage>;
    using Pixel = typename ImageType::PixelType;
    template <typename T = Pixel>
    struct Line {
        using PixelType = T;
        T* firstPixel;
        size_t width;
        constexpr T& pixel(ptrdiff_t x) {
            assert(x < width);
            assert(x >= 0);
            return *(firstPixel + x);
        }
        constexpr size_t size() { return width; }
        constexpr T* begin() { return firstPixel; }
        constexpr T* end() { return firstPixel + width; }
    };

    template <bool Flipped, typename T = Pixel>
    struct LinesView {
        using PixelType = T;
        T* data;
        const size_t width, height, pitch;
        struct Sentinel {};
        struct Iterator {
            T* iteratorData;
            const size_t width, height, pitch;
            ptrdiff_t y {0};
            constexpr Line<T> operator*() const { return {
                .firstPixel = iteratorData + y * pitch,
                .width = width,
            }; }
            constexpr Iterator& operator++() { ++y; return *this; }
            constexpr bool operator!=(Sentinel) const { return 0 != height - y; }
        };
        struct FlippedIterator {
            T* iteratorData;
            const size_t width, height, pitch;
            ptrdiff_t y {static_cast<ptrdiff_t>(height) - 1};
            constexpr Line<T> operator*() const { return {
                .firstPixel = iteratorData + y * pitch,
                .width = width
            }; }
            constexpr FlippedIterator& operator++() { --y; return *this; }
            constexpr bool operator!=(Sentinel) const { return y >= 0; }
        };

        constexpr auto begin() const {
            if constexpr (Flipped)
                return FlippedIterator{
                    .iteratorData = data,
                    .width = width,
                    .height = height,
                    .pitch = pitch,
                };
            else return Iterator{
                .iteratorData = data,
                .width = width,
                .height = height,
                .pitch = pitch
            };
        }

        constexpr Sentinel end() const { return {}; }
        constexpr size_t size() const { return height; }
        constexpr size_t length() const { return width; }
    };

    using PixelType = Pixel;
    using LineType = Line<PixelType>;

    constant ImageOrigin Origin = O;
    constant ImageLineOrder LineOrder = Origin == ImageOrigin::BottomLeft ? ImageLineOrder::BottomToTop : ImageLineOrder::TopToBottom;
    ImageType& image;

    size_t mWidth, mHeight;
    ptrdiff_t mOriginX, mOriginY;

    constexpr Pixel* data() {
        return &pixel(0,0);
    }

    constexpr size_t size() const {
        return mWidth * mHeight;
    }

    constexpr Pixel& pixel(ptrdiff_t x, ptrdiff_t y) {
        assert(x < (mWidth + mOriginX));
        assert(y < (mHeight + mOriginY));
        assert(x >= mOriginX);
        assert(y >= mOriginY);
        return image.pixel((x + mOriginX), (y + mOriginY));
    }

    constexpr Pixel& pixel(Vec2i pos) {
        return pixel(pos.x, pos.y);
    }

    template <ImageLineOrder DesiredLineOrder = LineOrder>
    constexpr auto lines() {
        constexpr bool Flipped = (DesiredLineOrder != LineOrder);
        return LinesView<Flipped, Pixel> {
            .data = image.data() + mOriginX + mOriginY * image.pitch(),
            .width = mWidth,
            .height = mHeight,
            .pitch = image.pitch()
        };
    }

    template <ImageLineOrder DesiredLineOrder = LineOrder>
    constexpr auto lines() const {
        constexpr bool Flipped = (DesiredLineOrder != LineOrder);
        return LinesView<Flipped, const Pixel> {
            .data = image.data() + mOriginX + mOriginY * image.pitch(),
            .width = mWidth,
            .height = mHeight,
            .pitch = image.pitch()
        };
    }

    constexpr size_t height() const {
        return mHeight;
    }

    constexpr size_t width() const {
        return mWidth;
    }

    constexpr size_t pitch() const {
        return image.pitch;
    }

    constexpr ptrdiff_t originX() const {
        return mOriginX;
    }

    constexpr ptrdiff_t originY() const {
        return mOriginY;
    }

};


template<typename T = uint8_t, size_t Width = sizeof(T) * 8>
struct BitmapCell {
    T bits;

    using MaskArray = std::array<T, Width>;

    compiletime MaskArray mask = []() {
        MaskArray masks{};
        T maskBit = 1;
        for (auto i = Width; i > 0; --i) {
            masks[i - 1] = maskBit;
            maskBit <<= 1;
        }
        return masks;
    }();

    constexpr size_t size() const {
        return sizeof(T) * 8;
    }

    constexpr bool get(int position) const {
        return mask[position] & bits;
    }

    constexpr void set(int position) {
        return bits |= mask[position];
    }

    constexpr void unset(int position) {
        return bits &= ~mask[position];
    }

};

template<size_t Width, size_t Height, ImageOrigin O = ImageOrigin::BottomLeft, typename T = uint8_t>
using BitmapImage = Image<BitmapCell<T>, Width / (sizeof(T) * 8), Height, O>;


constexpr uint64_t spread(const BitmapCell<uint8_t>& cell) {
    using Cell = BitmapCell<uint8_t>;
    uint64_t c = cell.bits;
    return (c & Cell::mask[0]) >> 7
        | (c & Cell::mask[1]) << 2
        | (c & Cell::mask[2]) << 11
        | (c & Cell::mask[3]) << 20
        | (c & Cell::mask[4]) << 29
        | (c & Cell::mask[5]) << 38
        | (c & Cell::mask[6]) << 47
        | (c & Cell::mask[7]) << 56;

}




template <typename T, ImageOrigin Origin = std::decay_t<T>::Origin>
constexpr ImageView<T, Origin> makeSubImage(T&& image, ptrdiff_t x, ptrdiff_t y, size_t width, size_t height)
{
    assert(x + width <= image.width());
    assert(y + height <= image.height());
    assert(x >= image.originX());
    assert(y >= image.originY());

    return ImageView<T, Origin>{
        .image = static_cast<T&&>(image),
        .mWidth = width,
        .mHeight = height,
        .mOriginX = x,
        .mOriginY = y,
    };
}

template <typename T, typename U>
constexpr void imageCopy(const T& source, U&& destination) {
    using DestinationType = std::decay_t<U>;
    using Pixel = typename DestinationType::PixelType;
    auto sourceLines = source.template lines<DestinationType::LineOrder>();
    auto destinationLines = destination.lines();

    const size_t lineWidth = std::min(sourceLines.length(), destinationLines.length()) * sizeof(Pixel);
    auto src = sourceLines.begin();
    auto srcEnd = sourceLines.end();
    auto dst = destinationLines.begin();
    auto dstEnd = destinationLines.end();

    while (src != srcEnd && dst != dstEnd) {
        memcpy((*dst).firstPixel, (*src).firstPixel, lineWidth);
        ++src; ++dst;
    }
}

template <typename T, typename U>
constexpr void imageBlitWithTransparentColor(const T& source, U&& destination, typename T::PixelType transparentColor) {
    using DestinationType = std::decay_t<U>;
    using SourceType = std::decay_t<T>;
    using SrcPixel = typename SourceType::PixelType;
    using Pixel = typename DestinationType::PixelType;
    auto sourceLines = source.template lines<DestinationType::LineOrder>();
    auto destinationLines = destination.lines();

    const size_t lineWidth = std::min(sourceLines.length(), destinationLines.length()) * sizeof(Pixel);
    auto src = sourceLines.begin();
    auto srcEnd = sourceLines.end();
    auto dst = destinationLines.begin();
    auto dstEnd = destinationLines.end();

    while (src != srcEnd && dst != dstEnd) {
        for (size_t i = 0; i < lineWidth; ++i) {
            SrcPixel srcColor = *((*src).firstPixel + i);
            if (srcColor != transparentColor)
                *((*dst).firstPixel + i) = static_cast<Pixel>(srcColor);
        }
        ++src; ++dst;
    }
}

