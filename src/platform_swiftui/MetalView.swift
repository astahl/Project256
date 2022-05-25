//
//  MetalView.swift
//  Project256
//
//  Created by Andreas Stahl on 25.05.22.
//

import SwiftUI
import MetalKit

class MetalViewBase : NSObject, MTKViewDelegate {
    
    var commandQueue: MTLCommandQueue?
    var pipelineState: MTLRenderPipelineState?
    var viewport = MTLViewport.init()
    var viewportSize: [UInt32] = [0, 0]
    
    func makeView() -> MTKView {
        let mtkView = MTKView.init()
        mtkView.sampleCount = 4
        guard let device = MTLCreateSystemDefaultDevice() else {
            return mtkView
        }
        
        guard let defaultLibrary = device.makeDefaultLibrary() else {
            return mtkView
        }
        
        guard let vertexFunction = defaultLibrary.makeFunction(name: "vertexShader") else {
            return mtkView
        }
        guard let fragmentFunction = defaultLibrary.makeFunction(name: "fragmentShader") else {
            return mtkView
        }
        
        let pipelineDescriptor = MTLRenderPipelineDescriptor.init()
        pipelineDescriptor.fragmentFunction = fragmentFunction
        pipelineDescriptor.vertexFunction = vertexFunction
        pipelineDescriptor.label = "SimplePipeline"
        pipelineDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat
        pipelineDescriptor.rasterSampleCount = mtkView.sampleCount
        try? pipelineState = device.makeRenderPipelineState(descriptor: pipelineDescriptor)
        mtkView.delegate = self
        mtkView.device = device
        mtkView.clearColor = MTLClearColor(red: 0.0, green: 0.0, blue: 0.0, alpha: 1.0)
        self.commandQueue = device.makeCommandQueue()
        self.viewport.zfar = 1.0
        return mtkView
    }
    
    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
        self.viewport.height = size.height
        self.viewport.width = size.width
        self.viewportSize[0] = UInt32(size.width)
        self.viewportSize[1] = UInt32(size.height)
    }
    
    func draw(in view: MTKView) {
        let triangleVertices: [AAPLVertex] = [
            AAPLVertex.init(position: simd_float2(250, -250), color: simd_float4(1, 0, 0, 1)),
            AAPLVertex.init(position: simd_float2(-250, -250), color: simd_float4(0, 1, 0, 1)),
            AAPLVertex.init(position: simd_float2(0, 250), color: simd_float4(0, 0, 1, 1))]
    
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
        
        encoder.setRenderPipelineState(self.pipelineState!)
        triangleVertices.withUnsafeBytes {
            bytes in
            encoder.setVertexBytes(bytes.baseAddress!, length: bytes.count, index: Int(AAPLVertexInputIndexVertices.rawValue))
        }
        
        viewportSize.withUnsafeBytes {
            bytes in
            encoder.setVertexBytes(bytes.baseAddress!, length: bytes.count, index: Int(AAPLVertexInputIndexViewportSize.rawValue))
        }
        
        encoder.drawPrimitives(type: .triangle, vertexStart: 0, vertexCount: 3)
        
        encoder.endEncoding()
        commandBuffer.present(view.currentDrawable!)
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
