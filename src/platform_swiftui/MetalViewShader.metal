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
             constant Vertex *vertices [[buffer(IndexVertices)]],
             constant simd_float2 *viewportSizePointer [[buffer(IndexViewportSize)]])
{
    RasterizerData out;

    // Index into the array of positions to get the current vertex.
    // The positions are specified in pixel dimensions (i.e. a value of 100
    // is 100 pixels from the origin).
    float2 pixelSpacePosition = vertices[vertexID].position.xy;

    // Get the viewport size and cast to float.
    simd_float2 viewportSize = *viewportSizePointer;

    // To convert from positions in pixel space to positions in clip-space,
    //  divide the pixel coordinates by half the size of the viewport.
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.xy = pixelSpacePosition / (viewportSize / 2.0);

    // Pass the input color directly to the rasterizer.
    out.uv = vertices[vertexID].uv;

    return out;
}

fragment half4 fragmentShader(RasterizerData in [[stage_in]],
                              constant simd_float2 *viewportSizePointer [[buffer(IndexViewportSize)]],
                               texture2d<half> tex [[texture(IndexTexture)]])
{
    // sample weights
    const float bias = 0.5;
    const float spread = 0.5;

    constexpr sampler nnsampler(mag_filter::nearest, min_filter::linear);
    simd_float2 ps = *viewportSizePointer;
//    const simd_float2 stepSize = simd_float2(0.25 * spread, 0.25 * spread) / ps;
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

