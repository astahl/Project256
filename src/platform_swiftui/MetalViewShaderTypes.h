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

typedef struct
{
    simd_float2 position;
    simd_float2 uv;
} Vertex;


#endif /* MetalViewShaderTypes_h */
