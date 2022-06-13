//
//  MetalView.metal
//  Project256
//
//  Created by Andreas Stahl on 26.05.22.
//

#include <metal_stdlib>
using namespace metal;

#include "MetalViewShaderTypes.h"

// Vertex shader outputs and fragment shader inputs
struct RasterizerData
{
    float4 position [[position]];
    float2 uv;
};


vertex RasterizerData
vertexShader(uint vertexID [[vertex_id]],
             constant simd_float2 *viewportSizePointer [[buffer(IndexQuadScaleXY)]])
{
    constexpr float2 positions[4] = {{-1.0,-1.0}, {-1.0, 1.0}, {1.0, -1.0}, {1.0, 1.0}};
    constexpr float2 texCoords[4] = {{0.0,0.0}, {0.0, 1.0}, {1.0, 0.0}, {1.0, 1.0}};
    RasterizerData out;
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.uv = texCoords[vertexID];
    out.position.xy = positions[vertexID] * *viewportSizePointer;
    return out;
}

fragment half4 fragmentShader(RasterizerData in [[stage_in]],
                               texture2d<half> tex [[texture(IndexTexture)]])
{
    // sample weights
    const float bias = 0.5;
    const float spread = 0.3;

    constexpr sampler nnsampler(mag_filter::nearest, min_filter::linear);
    const simd_float2 stepSize = 0.25 * spread * simd_float2(1.0f / tex.get_width(), 1.0f / tex.get_height());
    half4 color{};
    const float remBias = (1.0 - bias) / 4;

    // +++++++++
    // +     1 +
    // +4      +
    // +   0   +
    // +      2+
    // + 3     +
    // +++++++++

    const simd_float2 samplePoint1 = in.uv + simd_float2(stepSize.x, 2 * stepSize.y);
    const simd_float2 samplePoint2 = in.uv + simd_float2(2 * stepSize.x, -stepSize.y);
    const simd_float2 samplePoint3 = in.uv + simd_float2(-stepSize.x, -2 * stepSize.y);
    const simd_float2 samplePoint4 = in.uv + simd_float2(-2 * stepSize.x, stepSize.y);

    color += bias * tex.sample(nnsampler, in.uv);
    color += remBias * tex.sample(nnsampler, samplePoint1);
    color += remBias * tex.sample(nnsampler, samplePoint2);
    color += remBias * tex.sample(nnsampler, samplePoint3);
    color += remBias * tex.sample(nnsampler, samplePoint4);
    return color;
}

