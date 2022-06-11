//
//  Sprites.hpp
//  Project256
//
//  Sprites, masks, collisions etc.
//  Created by Andreas Stahl on 11.06.22.
//

#pragma once
#include <cstdint>
#include <array>

template<int W, int H, typename T = uint8_t>
struct Buffer {
    static constexpr Vec2i size{W,H};
    static constexpr size_t pixelPitch = W;
    static constexpr size_t byteRowPitch = pixelPitch * sizeof(T);
    static constexpr size_t byteSize = H * byteRowPitch;
    std::array<T, W * H> data;
};

template<int W, int H, int N = 1, typename T = uint8_t>
struct SpritePicture {
    static constexpr size_t frameCount = N;
    static constexpr Vec2i frameSize{W,H};
    static constexpr size_t pixelPitch = W * N;
    static constexpr size_t byteRowPitch = pixelPitch * sizeof(T);
    static constexpr size_t byteSize = H * byteRowPitch;
    std::array<T, W*H*N> data;
};

template<typename S, typename B>
constexpr void blitSprite(const S& spritePicture, int frameNumber, B* buffer, int bufferPixelPitch, Vec2i position, Vec2i clipLowerLeft, Vec2i clipUpperRight) {
    for (Vec2i pos : Generators::Rectangle{Vec2i{0}, S::frameSize - Vec2i{1,1}}) {
        Vec2i bufferPos = pos + position;
        if (bufferPos < clipUpperRight && !(bufferPos < clipLowerLeft))
            buffer[bufferPos.x + bufferPos.y * bufferPixelPitch] = spritePicture.data[frameNumber * S::frameSize.x + pos.x + pos.y * S::pixelPitch];
    }
}
