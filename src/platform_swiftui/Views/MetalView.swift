//
//  MetalView.swift
//  Project256
//
//  Created by Andreas Stahl on 25.05.22.
//

import SwiftUI
import MetalKit


extension Color {
    func clearColor() -> MTLClearColor? {
        if let srgb = CGColorSpace(name: CGColorSpace.sRGB),
           let cgcolor = self.cgColor?.converted(to: srgb, intent: CGColorRenderingIntent.perceptual, options: nil),
           let c = cgcolor.components {
            return MTLClearColor(red: c[0], green: c[1], blue: c[2], alpha: c[3]);
        }
        return nil
    }
}

typealias BeforeDrawHandler = (_ buffer: DrawBuffer) -> DrawBuffer?
typealias PointConversion = (_ point: CGPoint) -> CGPoint?


class MyMTKView : MTKView {
    var beforeDrawHandler: BeforeDrawHandler?
    private var drawBuffer: DrawBuffer
    private var commandQueue: MTLCommandQueue?
    private var pipelineState: MTLRenderPipelineState?
    private var viewport = MTLViewport()
    private var scale = Vec2f()
    private var texture: MTLTexture?

    required init(coder: NSCoder) {
        self.drawBuffer = DrawBuffer()
        super.init(coder: coder)
    }

    init(frame: CGRect, drawBuffer: DrawBuffer?) {
        self.drawBuffer = drawBuffer ?? DrawBuffer()
        guard let device = MTLCreateSystemDefaultDevice() else {
            fatalError("Metal Device could not be created")
        }

        super.init(frame: frame, device: device)

        guard let defaultLibrary = device.makeDefaultLibrary(),
                let vertexFunction = defaultLibrary.makeFunction(name: "vertexShader"),
                let fragmentFunction = defaultLibrary.makeFunction(name: "fragmentShader") else {
            fatalError("Shaders could not be compiled")
        }

        let pipelineDescriptor = MTLRenderPipelineDescriptor.init()
        pipelineDescriptor.fragmentFunction = fragmentFunction
        pipelineDescriptor.vertexFunction = vertexFunction
        pipelineDescriptor.label = "SimplePipeline"
        #if os(macOS)
        self.layer?.isOpaque = false
        #else
        self.layer.isOpaque = false
        #endif
        pipelineDescriptor.colorAttachments[0].pixelFormat = super.colorPixelFormat
        pipelineDescriptor.rasterSampleCount = super.sampleCount

        try? self.pipelineState = device.makeRenderPipelineState(descriptor: pipelineDescriptor)
        super.clearColor = .init(red: 0, green: 0, blue: 0, alpha: 0)

        self.texture = device.makeTexture(descriptor: MTLTextureDescriptor.texture2DDescriptor(pixelFormat: .bgra8Unorm, width: self.drawBuffer.width, height: self.drawBuffer.height, mipmapped: false))

        self.commandQueue = device.makeCommandQueue()
        self.viewport.zfar = 1.0
        self.preferredFramesPerSecond = 60
        //self.isPaused = true
        //self.enableSetNeedsDisplay = true
    }

    func updateViewport(withDrawableSize size: CGSize) {
        self.viewport.height = size.height
        self.viewport.width = size.width
        self.scale = clipSpaceDrawBufferScale(UInt32(size.width), UInt32(size.height))
    }

    override func draw(_ dirtyRect: CGRect) {

        profiling_time_set(&GameState.timingData, eTimerDraw)
        if let newDrawBuffer = self.beforeDrawHandler?(self.drawBuffer) {
            self.drawBuffer = newDrawBuffer
        }
        profiling_time_interval(&GameState.timingData, eTimerDraw, eTimingDrawBefore)
        updateViewport(withDrawableSize: drawableSize)

        guard let renderPassDescriptor = self.currentRenderPassDescriptor else {
            return
        }

        let region = MTLRegion(origin: MTLOrigin(), size: MTLSize(width: drawBuffer.width, height: drawBuffer.height, depth: 1))

        self.texture?.replace(region: region, mipmapLevel: 0, withBytes: drawBuffer.data.baseAddress!, bytesPerRow: drawBuffer.width * 4)

        guard let commandBuffer = self.commandQueue?.makeCommandBuffer() else {
            return
        }
        commandBuffer.label = "MyCommand"
        guard let encoder = commandBuffer.makeRenderCommandEncoder(descriptor: renderPassDescriptor) else {
            return
        }

        encoder.label = "MyEncoder"
        profiling_time_interval(&GameState.timingData, eTimerDraw, eTimingDrawWaitAndSetup)

        encoder.setViewport(self.viewport)
        if let pipelineState = self.pipelineState {
            encoder.setRenderPipelineState(pipelineState)
            withUnsafeBytes(of: &self.scale) {
                bytes in
                encoder.setVertexBytes(bytes.baseAddress!, length: bytes.count, index: Int(IndexQuadScaleXY.rawValue))
            }
            encoder.setFragmentTexture(self.texture, index: Int(IndexTexture.rawValue))

            encoder.drawPrimitives(type: .triangleStrip, vertexStart: 0, vertexCount: 4)

            encoder.endEncoding()
            profiling_time_interval(&GameState.timingData, eTimerDraw, eTimingDrawEncoding)
        }
        if let drawable = self.currentDrawable {
            #if os(macOS)
            drawable.addPresentedHandler() {
                drawble in
                profiling_time_interval(&GameState.timingData, eTimerFrameToFrame, eTimingFrameToFrame)

                profiling_time_set(&GameState.timingData, eTimerFrameToFrame)
                profiling_time_interval(&GameState.timingData, eTimerDraw, eTimingDrawPresent)
            }
            #endif
            commandBuffer.present(drawable)
        }
        commandBuffer.commit()
    }

    func positionOnBuffer(locationInView: CGPoint) -> CGPoint? {
        let normalizedPos = CGPoint(x: locationInView.x / self.frame.width, y: locationInView.y / self.frame.height)
        let scaleX = CGFloat(self.scale.x)
        let scaleY = CGFloat(self.scale.y)
        let pos = normalizedPos
            .applying(CGAffineTransform.init(translationX: -1, y: -1).scaledBy(x: 2, y: 2))

        let scaled = pos.applying(CGAffineTransform.init(translationX: 0.5, y: 0.5).scaledBy(x: 0.5 / scaleX, y: 0.5 / scaleY))

        if CGRect.init(x: 0, y: 0, width: 1, height: 1).contains(scaled) {
            return scaled.applying(CGAffineTransform.init(scaleX: CGFloat(drawBuffer.width), y: CGFloat(drawBuffer.height)))
        }
        return nil
    }
}

final class MetalView {
    private var beforeDrawHandler: BeforeDrawHandler?
    var pixelPosition: PointConversion?
    var drawBuffer: DrawBuffer?

    init(drawBuffer: DrawBuffer?)
    {
        self.drawBuffer = drawBuffer
    }

    func beforeDraw(_ handler: @escaping(BeforeDrawHandler)) -> MetalView {
        self.beforeDrawHandler = handler
        return self
    }

}

#if os(macOS)
extension MetalView : NSViewRepresentable {
    typealias NSViewType = MyMTKView
    
    func makeNSView(context: Context) -> MyMTKView {
        return MyMTKView(frame: .zero, drawBuffer: drawBuffer)
    }
    
    func updateNSView(_ nsView: MyMTKView, context: Context) {
        nsView.beforeDrawHandler = self.beforeDrawHandler
        pixelPosition = nsView.positionOnBuffer(locationInView:)

    }
}
#else
extension MetalView : UIViewRepresentable {
    typealias UIViewType = MyMTKView
    
    func makeUIView(context: Context) -> MyMTKView {
        return MyMTKView(frame: .zero, drawBuffer: drawBuffer)
    }
    
    func updateUIView(_ uiView: MyMTKView, context: Context) {
        uiView.beforeDrawHandler = self.beforeDrawHandler
        pixelPosition = uiView.positionOnBuffer(locationInView:)
    }
}
#endif


struct MetalView_Previews: PreviewProvider {
    static var previews: some View {
        MetalView(
            drawBuffer: .init(withTestPattern: .Checkerboard(.init(gray: 0.0, alpha: 0.0), .init(red: 1.0, green: 0, blue: 0, alpha: 1), size: 10)))
        .background(.blue)
    }
}
