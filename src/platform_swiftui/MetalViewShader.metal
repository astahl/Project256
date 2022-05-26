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
             constant AAPLVertex *vertices [[buffer(IndexVertices)]],
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
                               texture2d<half> tex [[texture(IndexTexture)]])
{
    constexpr sampler nnsampler(mag_filter::nearest, min_filter::linear);
    return tex.sample(nnsampler, in.uv);
}

