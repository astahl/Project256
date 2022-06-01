//
//  MetalView.swift
//  Project256
//
//  Created by Andreas Stahl on 25.05.22.
//

import SwiftUI
import MetalKit


enum MyMTKViewErrors : Error {
    case InitError
}

enum ScalingMode {
    case ScalingInteger
    case ScalingFull
}


class MyMTKView : MTKView {
    var scalingMode: ScalingMode = .ScalingFull
    private var drawBuffer: DrawBuffer? = nil
    private var commandQueue: MTLCommandQueue?
    private var pipelineState: MTLRenderPipelineState?
    private var viewport = MTLViewport.init()
    private var quadScaleXY: [Float32] = [0.0, 0.0]
    private var texture: MTLTexture?

    required init(coder: NSCoder) {
        super.init(coder: coder)
    }

    init(drawBuffer: DrawBuffer, scalingMode: ScalingMode) throws {
        self.drawBuffer = drawBuffer
        self.scalingMode = scalingMode

        guard let device = MTLCreateSystemDefaultDevice() else {
            throw MyMTKViewErrors.InitError
        }

        super.init(frame: CGRect.zero, device: device)

        guard let defaultLibrary = device.makeDefaultLibrary(),
                let vertexFunction = defaultLibrary.makeFunction(name: "vertexShader"),
                let fragmentFunction = defaultLibrary.makeFunction(name: "fragmentShader") else {
            throw MyMTKViewErrors.InitError
        }

        let pipelineDescriptor = MTLRenderPipelineDescriptor.init()
        pipelineDescriptor.fragmentFunction = fragmentFunction
        pipelineDescriptor.vertexFunction = vertexFunction
        pipelineDescriptor.label = "SimplePipeline"
        pipelineDescriptor.colorAttachments[0].pixelFormat = super.colorPixelFormat
        pipelineDescriptor.rasterSampleCount = super.sampleCount

        try? self.pipelineState = device.makeRenderPipelineState(descriptor: pipelineDescriptor)
        super.clearColor = MTLClearColor(red: 0, green: 0, blue: 0, alpha: 0)
        self.texture = device.makeTexture(descriptor: MTLTextureDescriptor.texture2DDescriptor(pixelFormat: .bgra8Unorm, width: drawBuffer.width, height: drawBuffer.height, mipmapped: false))

        self.commandQueue = device.makeCommandQueue()
        self.viewport.zfar = 1.0
    }

    func update(withDrawableSize size: CGSize) {
        self.viewport.height = size.height
        self.viewport.width = size.width
        // TODO fix this mess
        guard let drawBuffer = drawBuffer else {
            return
        }
        let width = UInt64(size.width)
        let height = UInt64(size.height)
        var widthOnTarget = width
        var heightOnTarget = height

        switch scalingMode {
        case .ScalingInteger:
            // integer scaling, only square pixels allowed...
            let factor = max(1, min(width / UInt64(drawBuffer.width), height / UInt64(drawBuffer.height)))
            widthOnTarget = factor * UInt64(drawBuffer.width)
            heightOnTarget = factor * UInt64(drawBuffer.height)
        case .ScalingFull:
            let targetAspectRatioFixPoint48_16 = (width << 16) / height
            let drawAspectRatioFixPoint48_16 = UInt64((DrawAspectH << 16) / DrawAspectV)

            if targetAspectRatioFixPoint48_16 < drawAspectRatioFixPoint48_16 {
                heightOnTarget = (widthOnTarget << 16) / drawAspectRatioFixPoint48_16
            } else {
                widthOnTarget = (heightOnTarget * drawAspectRatioFixPoint48_16) >> 16
            }
        }
        self.quadScaleXY[0] = Float(widthOnTarget) / Float32(size.width)
        self.quadScaleXY[1] = Float(heightOnTarget) / Float32(size.height)
    }

    override func draw() {
        super.draw()
        update(withDrawableSize: drawableSize)

        guard let drawBuffer = drawBuffer else {
            return
        }

        let region = MTLRegion(origin: MTLOrigin(), size: MTLSize(width: drawBuffer.width, height: drawBuffer.height, depth: 1))

        // todo can we move update tex to its own thread and just synchronize?
        writeDrawBuffer(UnsafeMutableRawPointer.init(bitPattern: 0), drawBuffer.data.baseAddress!)
        self.texture?.replace(region: region, mipmapLevel: 0, withBytes: drawBuffer.data.baseAddress!, bytesPerRow: drawBuffer.width * 4)

        guard let renderPassDescriptor = self.currentRenderPassDescriptor else {
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

            quadScaleXY.withUnsafeBytes {
                bytes in
                encoder.setVertexBytes(bytes.baseAddress!, length: bytes.count, index: Int(IndexQuadScaleXY.rawValue))
            }

            encoder.setFragmentTexture(self.texture, index: Int(IndexTexture.rawValue))

            encoder.drawPrimitives(type: .triangleStrip, vertexStart: 0, vertexCount: 4)

            encoder.endEncoding()
        }
        if let drawable = self.currentDrawable {
            commandBuffer.present(drawable)
        }
        commandBuffer.commit()
    }
}

final class MetalView : NSObject {
    var scalingMode: ScalingMode
    var drawBuffer: DrawBuffer

    init(scalingMode: ScalingMode, drawBuffer: DrawBuffer) {
        self.scalingMode = scalingMode
        self.drawBuffer = drawBuffer
    }
}

#if os(macOS)
extension MetalView : NSViewRepresentable {
    typealias NSViewType = MyMTKView
    
    func makeNSView(context: Context) -> MyMTKView {
        return (try? MyMTKView(drawBuffer: self.drawBuffer, scalingMode: self.scalingMode))!
    }
    
    func updateNSView(_ nsView: MyMTKView, context: Context) {
        nsView.scalingMode = self.scalingMode
        print(self)
    }

    static func dismantleNSView(_ nsView: MyMTKView, coordinator: ()) {
        print("dismantle")
    }
}
#else
extension MetalView : UIViewRepresentable {
    typealias UIViewType = MyMTKView
    
    func makeUIView(context: Context) -> MyMTKView {
        return (try? MyMTKView(drawBuffer: self.drawBuffer, scalingMode: self.scalingMode))!
    }
    
    func updateUIView(_ uiView: MyMTKView, context: Context) {
    }
    
}
#endif
