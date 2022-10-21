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
#include <span>
#include <ranges>


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


template <typename T>
concept anImage = requires(T& t) {
    typename std::decay_t<T>::PixelType;
    { t.origin() } -> std::convertible_to<ImageOrigin>;
    { t.lineOrder() } -> std::convertible_to<ImageLineOrder>;
    { t.width() } -> std::convertible_to<size_t>;
    { t.height() } -> std::convertible_to<size_t>;
    { t.pitch() } -> std::convertible_to<size_t>;
    { t.originX() } -> std::convertible_to<ptrdiff_t>;
    { t.originY()} -> std::convertible_to<ptrdiff_t>;
    { t.at(Vec2i{}) } -> std::same_as<std::add_lvalue_reference_t<typename std::decay_t<T>::PixelType>>;
};

template <typename T, typename U>
concept anImageOf = anImage<T> && std::is_same_v<typename std::decay_t<T>::PixelType, U>;


template <typename T>
concept aStaticImage = anImage<T> && requires(T& t) {
    { T::Origin } -> std::convertible_to<ImageOrigin>;
    { T::LineOrder } -> std::convertible_to<ImageLineOrder>;
};

template <typename T, size_t Width, size_t Height, ImageOrigin O = ImageOrigin::BottomLeft, size_t Pitch = Width>
struct Image {

    using PixelType = T;
    using LineSpanType = std::span<PixelType, Width>;
    using PitchSpanType = std::span<PixelType, Pitch>;
    using PitchArrayType = std::array<PixelType, Pitch>;
    using LineStorageType = std::array<PitchArrayType, Height>;
    using PixelViewType = std::span<PixelType, Pitch * Height>;

    constant ImageOrigin Origin = O;
    constant ImageLineOrder LineOrder = (Origin == ImageOrigin::BottomLeft ? ImageLineOrder::BottomToTop : ImageLineOrder::TopToBottom);

    constant size_t VisiblePixelCount = Width * Height;
    constant size_t StoragePixelCount = Pitch * Height;

    template <typename TIterator, typename TPixel>
    struct LinesViewT {
        using BaseIterator = TIterator;
        using PixelType = TPixel;
        BaseIterator mBegin;
        BaseIterator mEnd;

        struct Iterator {
            BaseIterator pos;
            using LineSpan = std::span<PixelType, Width>;
            constexpr auto& operator++() {
                ++pos;
                return *this;
            }

            constexpr auto operator*() const {
                auto& line = *pos;
                return LineSpan{line};
            }

            constexpr bool operator!=(const Iterator& other) const {
                return pos != other.pos;
            }
        };

        Iterator begin() { return {mBegin}; }
        Iterator end() { return {mEnd}; }
        Iterator begin() const { return {mBegin}; }
        Iterator end() const { return {mEnd}; }
    };
    using LinesView = LinesViewT<typename LineStorageType::iterator, PixelType>;
    using ReverseLinesView = LinesViewT<typename LineStorageType::reverse_iterator, PixelType>;
    using ConstLinesView = LinesViewT<typename LineStorageType::const_iterator, const PixelType>;
    using ConstReverseLinesView = LinesViewT<typename LineStorageType::const_reverse_iterator, const PixelType>;

    template <typename TLinesView>
    struct PixelsViewT {
        using LinesIterator = typename TLinesView::Iterator;
        using PixelType = typename TLinesView::PixelType;
        TLinesView linesView;

        struct Iterator {
            LinesIterator line;
            LinesIterator endLine;
            using LineSpan = typename LinesIterator::LineSpan;
            using LineSpanIterator = typename LineSpan::iterator;

            LineSpanIterator pos;
            LineSpanIterator end;

            constexpr Iterator& operator++() {

                if (pos != end) {
                    ++pos;
                }
                if (!(pos != end)) {
                    ++line;
                    if (line != endLine) {
                        LineSpan currentLine = *line;
                        pos = currentLine.begin();
                        end = currentLine.end();
                    }
                }
                
                return *this;
            }

            constexpr PixelType& operator*() const {
                return *pos;
            }

            constexpr bool operator!=(const Iterator& other) const {
                return line != other.line;
            }
        };

        Iterator begin() {
            auto line = linesView.begin();
            return {
                .line = line,
                .endLine = linesView.end(),
                .pos = (*line).begin(),
                .end = (*line).end()
            };
        }

        Iterator end() {
            auto line = linesView.end();
            return {
                .line = line
            };
        }
        Iterator begin() const {
            auto line = linesView.begin();
            return {
                .line = line,
                .endLine = linesView.end(),
                .pos = (*line).cbegin(),
                .end = (*line).cend()
            };
        }

        Iterator end() const {
            auto line = linesView.end();
            return {
                .line = line
            };
        }
    };

    using PixelsView = PixelsViewT<LinesView>;
    using ConstPixelsView = PixelsViewT<ConstLinesView>;

    LineStorageType lines;

    constexpr PixelType* data() {
        return lines.front().data();
    }

    constexpr auto pixels() {
        return PixelsView{linesView()};
    }

    constexpr auto pixels() const {
        return ConstPixelsView{linesView()};
    }

    constexpr uint8_t* bytes() {
        return reinterpret_cast<uint8_t*>(data());
    }

    constexpr void fill(const PixelType& pixelValue) {
        for(PixelType& p : pixels()) {
            p = pixelValue;
        }
    }

    constexpr size_t bytesSize() const {
        return StoragePixelCount * sizeof(PixelType);
    }

    constexpr size_t size() const {
        return StoragePixelCount;
    }

    constexpr auto line(ptrdiff_t y) {
        return LineSpanType{lines.at(y)};
    }

    constexpr const auto line(ptrdiff_t y) const {
        return LineSpanType{lines.at(y)};
    }

    constexpr const PixelType& at(Vec2i pos) const {
        return lines.at(pos.y).at(pos.x);
    }

    constexpr PixelType& at(Vec2i pos) {
        return lines.at(pos.y).at(pos.x);
    }

    template <ImageLineOrder DesiredLineOrder = LineOrder>
    constexpr auto linesView() {
        if constexpr (DesiredLineOrder == LineOrder) {
            return LinesView{ .mBegin = lines.begin(), .mEnd = lines.end() };
        } else {
            return ReverseLinesView{ .mBegin = lines.rbegin(), .mEnd = lines.rend() };
        }
    }

    template <ImageLineOrder DesiredLineOrder = LineOrder>
    constexpr auto linesView() const {
        if constexpr (DesiredLineOrder == LineOrder) {
            return ConstLinesView{ .mBegin = lines.cbegin(), .mEnd = lines.cend() };
        } else {
            return ConstReverseLinesView{ .mBegin = lines.crbegin(), .mEnd = lines.crend() };
        }
    }

    constexpr size_t height() const {
        return Height;
    }

    constexpr size_t width() const {
        return Width;
    }

    constexpr Vec2i size2d() const {
        return { Width, Height };
    }

    constexpr Vec2i maxIndex() const {
        return { Width - 1, Height - 1};
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

    constexpr ImageLineOrder lineOrder() const {
        return LineOrder;
    }

    constexpr ImageOrigin origin() const {
        return Origin;
    }
};


template <typename TImage, ImageOrigin O = ImageOrigin::BottomLeft>
struct SubImageView {
    using ImageType = std::decay_t<TImage>;
    using Pixel = typename ImageType::PixelType;
    template <typename T = Pixel>
    struct Line {
        using PixelType = T;
        T* firstPixel;
        size_t width;
        constexpr T& pixel(ptrdiff_t x) {
            assert(x >= 0);
            assert(static_cast<size_t>(x) < width);
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
        return &at(0,0);
    }

    constexpr size_t size() const {
        return mWidth * mHeight;
    }

    constexpr Pixel& at(Vec2i pos) {
        assert(pos.x < (mWidth + mOriginX));
        assert(pos.y < (mHeight + mOriginY));
        assert(pos.x >= mOriginX);
        assert(pos.y >= mOriginY);
        return image.at(pos.x + mOriginX, pos.y + mOriginY);
    }

    constexpr const Pixel& at(Vec2i pos) const {
        assert(pos.x < (mWidth + mOriginX));
        assert(pos.y < (mHeight + mOriginY));
        assert(pos.x >= mOriginX);
        assert(pos.y >= mOriginY);
        return image.at(pos.x + mOriginX, pos.y + mOriginY);
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

    constexpr ImageLineOrder lineOrder() const {
        return LineOrder;
    }

    constexpr ImageOrigin origin() const {
        return Origin;
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

constexpr bool isValidImageIndex(anImage auto image, const Vec2i& index) {
    return index.x >= 0 && index.x < image.width() && index.y >= 0 && index.y < image.height();
}


template <anImage T, ImageOrigin Origin = std::decay_t<T>::Origin>
constexpr SubImageView<T, Origin> makeSubImage(T&& image, ptrdiff_t x, ptrdiff_t y, size_t width, size_t height)
{
    assert(x + width <= image.width());
    assert(y + height <= image.height());
    assert(x >= image.originX());
    assert(y >= image.originY());

    return SubImageView<T, Origin>{
        .image = static_cast<T&&>(image),
        .mWidth = width,
        .mHeight = height,
        .mOriginX = x,
        .mOriginY = y,
    };
}

template <anImage T, anImage U>
requires(std::same_as<typename T::PixelType, typename U::PixelType>)
constexpr void imageCopy(const T& source, U& destination) {
    using DestinationType = std::decay_t<U>;
    using Pixel = typename DestinationType::PixelType;
    auto sourceLines = source.template linesView<DestinationType::LineOrder>();
    auto destinationLines = destination.linesView();
    const size_t lineByteCount = std::min(source.width(), destination.width()) * sizeof(Pixel);

    for (auto [src, dst] : ranges_at_home::zip(sourceLines, destinationLines))
    {
        const Pixel* from = src.data();
        Pixel* to = dst.data();
        memcpy(to, from, lineByteCount);
    }
}

template <anImage T, anImage U>
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
            SrcPixel srcColor = (*src).pixel(i);
            if (srcColor != transparentColor)
                (*dst).pixel(i) = static_cast<Pixel>(srcColor);
        }
        ++src; ++dst;
    }
}

