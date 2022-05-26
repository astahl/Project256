//
//  MetalViewShaderTypes.h
//  Project256
//
//  Created by Andreas Stahl on 26.05.22.
//

#ifndef MetalViewShaderTypes_h
#define MetalViewShaderTypes_h


#include <simd/simd.h>

// Buffer index values shared between shader and C code to ensure Metal shader buffer inputs
// match Metal API buffer set calls.
typedef enum AAPLVertexInputIndex
{
    IndexVertices,
    IndexViewportSize,
    IndexTexture,
} AAPLVertexInputIndex;

//  This structure defines the layout of vertices sent to the vertex
//  shader. This header is shared between the .metal shader and C code, to guarantee that
//  the layout of the vertex array in the C code matches the layout that the .metal
//  vertex shader expects.
typedef struct
{
    simd_float2 position;
    simd_float2 uv;
} AAPLVertex;


#endif /* MetalViewShaderTypes_h */
