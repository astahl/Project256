//
//  MetalView.swift
//  Project256
//
//  Created by Andreas Stahl on 25.05.22.
//

import SwiftUI
import MetalKit


enum ScalingMode {
    case ScalingInteger
    case ScalingFull
}

class MetalViewBase : NSObject, MTKViewDelegate {

    let scalingMode = ScalingMode.ScalingFull
    var commandQueue: MTLCommandQueue?
    var pipelineState: MTLRenderPipelineState?
    var viewport = MTLViewport.init()
    var viewportSize: [Float32] = [0.0, 0.0]
    var quadVertices: [Vertex] = [
        Vertex(position: simd_float2(0, 0), uv: simd_float2(0, 0)),
        Vertex(position: simd_float2(0, Float(DrawBufferHeight)), uv: simd_float2(0, 1)),
        Vertex(position: simd_float2(Float(DrawBufferWidth), 0), uv: simd_float2(1, 0)),
        Vertex(position: simd_float2(Float(DrawBufferWidth), Float(DrawBufferHeight)), uv: simd_float2(1, 1))]
    var texture: MTLTexture?
    var drawBuffer = UnsafeMutableBufferPointer<UInt32>.allocate(capacity: Int(DrawBufferWidth * DrawBufferHeight))

    func makeView() -> MTKView {
        let mtkView = MTKView.init()
        guard let device = MTLCreateSystemDefaultDevice() else {
            return mtkView
        }


//        var sampleCount = 32
//        while !device.supportsTextureSampleCount(sampleCount) {
//            sampleCount >>= 1
//        }
//
//        print("SampleCount \(sampleCount)")
//        mtkView.sampleCount = sampleCount

        guard let defaultLibrary = device.makeDefaultLibrary(),
                let vertexFunction = defaultLibrary.makeFunction(name: "vertexShader"),
                let fragmentFunction = defaultLibrary.makeFunction(name: "fragmentShader") else {
            return mtkView
        }
        
        let pipelineDescriptor = MTLRenderPipelineDescriptor.init()
        pipelineDescriptor.fragmentFunction = fragmentFunction
        pipelineDescriptor.vertexFunction = vertexFunction
        pipelineDescriptor.label = "SimplePipeline"
        pipelineDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat

        pipelineDescriptor.rasterSampleCount = mtkView.sampleCount

        try? self.pipelineState = device.makeRenderPipelineState(descriptor: pipelineDescriptor)
        mtkView.delegate = self
        mtkView.device = device
        mtkView.clearColor = MTLClearColor(red: 1.0, green: 1.0, blue: 1.0, alpha: 1.0)

        self.texture = device.makeTexture(descriptor: MTLTextureDescriptor.texture2DDescriptor(pixelFormat: .bgra8Unorm, width: Int(DrawBufferWidth), height: Int(DrawBufferHeight), mipmapped: false))

        self.commandQueue = device.makeCommandQueue()
        self.viewport.zfar = 1.0
        return mtkView
    }
    
    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
        self.viewport.height = size.height
        self.viewport.width = size.width
        self.viewportSize[0] = Float32(size.width)
        self.viewportSize[1] = Float32(size.height)

        // TODO fix this mess

        let width = UInt64(size.width)
        let height = UInt64(size.height)
        var widthOnTarget = width
        var heightOnTarget = height

        switch scalingMode {
        case .ScalingInteger:
            // integer scaling, only square pixels allowed...
            let factor = max(1, min(width / UInt64(DrawBufferWidth), height / UInt64(DrawBufferHeight)))
            widthOnTarget = factor * UInt64(DrawBufferWidth)
            heightOnTarget = factor * UInt64(DrawBufferHeight)
        case .ScalingFull:
            let targetAspectRatioFixPoint48_16 = (width << 16) / height
            let drawAspectRatioFixPoint48_16 = UInt64((DrawAspectH << 16) / DrawAspectV)

            if targetAspectRatioFixPoint48_16 < drawAspectRatioFixPoint48_16 {
                heightOnTarget = (widthOnTarget << 16) / drawAspectRatioFixPoint48_16
            } else {
                widthOnTarget = (heightOnTarget * drawAspectRatioFixPoint48_16) >> 16
            }
        }

        let halfWidth = 0.5 * Float(widthOnTarget)
        let halfHeight = 0.5 * Float(heightOnTarget)

        self.quadVertices[0].position = simd_float2(-halfWidth, -halfHeight)
        self.quadVertices[1].position = simd_float2(-halfWidth, halfHeight)
        self.quadVertices[2].position = simd_float2(halfWidth, -halfHeight)
        self.quadVertices[3].position = simd_float2(halfWidth, halfHeight)
    }
    
    func draw(in view: MTKView) {
        let region = MTLRegion(origin: MTLOrigin(), size: MTLSize(width: Int(DrawBufferWidth), height: Int(DrawBufferHeight), depth: 1))

        writeDrawBuffer(UnsafeMutableRawPointer.init(bitPattern: 0), drawBuffer.baseAddress)
        self.texture?.replace(region: region, mipmapLevel: 0, withBytes: drawBuffer.baseAddress!, bytesPerRow: Int(DrawBufferWidth) * 4)

        guard let renderPassDescriptor = view.currentRenderPassDescriptor else {
            return
        }

        guard let commandBuffer = self.commandQueue?.makeCommandBuffer() else {
            return
        }
        commandBuffer.label = "MyCommand"
        guard let encoder = commandBuffer.makeRenderCommandEncoder(descriptor: renderPassDescriptor) else {
            return
        }
        
        encoder.label = "MyEncoder"
        
        encoder.setViewport(self.viewport)
        if let pipelineState = self.pipelineState {
            encoder.setRenderPipelineState(pipelineState)
            self.quadVertices.withUnsafeBytes {
                bytes in
                encoder.setVertexBytes(bytes.baseAddress!, length: bytes.count, index: Int(IndexVertices.rawValue))
            }
            viewportSize.withUnsafeBytes {
                bytes in
                encoder.setVertexBytes(bytes.baseAddress!, length: bytes.count, index: Int(IndexViewportSize.rawValue))
                encoder.setFragmentBytes(bytes.baseAddress!, length: bytes.count, index: Int(IndexViewportSize.rawValue))
            }

            encoder.setFragmentTexture(self.texture, index: Int(IndexTexture.rawValue))

            encoder.drawPrimitives(type: .triangleStrip, vertexStart: 0, vertexCount: 4)

            encoder.endEncoding()
        }
        if let drawable = view.currentDrawable {
            commandBuffer.present(drawable)
        }
        commandBuffer.commit()
    }
}

#if os(macOS)
final class MetalView : MetalViewBase, NSViewRepresentable {
    typealias NSViewType = MTKView
    
    func makeNSView(context: Context) -> MTKView {
        return makeView()
    }
    
    func updateNSView(_ nsView: MTKView, context: Context) {
    }
    
}
#else
final class MetalView : MetalViewBase, UIViewRepresentable {
    typealias UIViewType = MTKView
    
    func makeUIView(context: Context) -> MTKView {
        return makeView()
    }
    
    func updateUIView(_ uiView: MTKView, context: Context) {
    }
    
}
#endif
