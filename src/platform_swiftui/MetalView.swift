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


class MyMTKView : MTKView {
    var mouseMoveHandler: ((_ relative: CGPoint, _ position: CGPoint) -> Void)?

    private var drawBuffer: DrawBuffer? = nil
    private var commandQueue: MTLCommandQueue?
    private var pipelineState: MTLRenderPipelineState?
    private var viewport = MTLViewport.init()
    private var quadScaleXY: [Float32] = [0.0, 0.0]
    private var texture: MTLTexture?

    override var acceptsFirstResponder: Bool {
        return true
    }

    required init(coder: NSCoder) {
        super.init(coder: coder)

    }

    override func viewDidMoveToWindow() {
        self.window?.acceptsMouseMovedEvents = true
    }

    override func keyDown(with event: NSEvent) {
        // todo tell handler
//        event.characters?
    }

    override func mouseMoved(with event: NSEvent) {
        self.mouseMoveHandler?(CGPoint(x: event.deltaX, y: event.deltaY), event.locationInWindow)
    }

    init(drawBuffer: DrawBuffer, letterboxColor: Color?) throws {
        self.drawBuffer = drawBuffer

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
        super.clearColor = letterboxColor?.clearColor() ?? MTLClearColor(red: 0, green: 0, blue: 0, alpha: 1)
        self.texture = device.makeTexture(descriptor: MTLTextureDescriptor.texture2DDescriptor(pixelFormat: .bgra8Unorm, width: drawBuffer.width, height: drawBuffer.height, mipmapped: false))

        self.commandQueue = device.makeCommandQueue()
        self.viewport.zfar = 1.0
    }

    func setLetterboxColor(_ color: Color) {
        if let clearColor = color.clearColor() {
            self.clearColor = clearColor
        }
    }

    func update(withDrawableSize size: CGSize) {
        self.viewport.height = size.height
        self.viewport.width = size.width

        let scale = clipSpaceDrawBufferScale(UInt32(size.width), UInt32(size.height))
        self.quadScaleXY[0] = scale.x
        self.quadScaleXY[1] = scale.y

    }
    override func resize(withOldSuperviewSize oldSize: NSSize) {

        print("Appearance")
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

final class MetalView {
    var drawBuffer: DrawBuffer
    var letterboxColor: Color
    var mouseMoveHandler: ((CGPoint, CGPoint) -> Void)?

    init(drawBuffer: DrawBuffer) {
        self.drawBuffer = drawBuffer
        self.letterboxColor = Color.black
    }

    func letterboxColor(_ color: Color) -> MetalView {
        self.letterboxColor = color
        return self
    }

    func mouseMove(_ handler: @escaping(_ relative: CGPoint, _ position: CGPoint) -> Void) -> MetalView {
        self.mouseMoveHandler = handler
        return self
    }
}

#if os(macOS)
extension MetalView : NSViewRepresentable {
    typealias NSViewType = MyMTKView
    
    func makeNSView(context: Context) -> MyMTKView {
        return (try? MyMTKView(drawBuffer: self.drawBuffer, letterboxColor: self.letterboxColor))!
    }
    
    func updateNSView(_ nsView: MyMTKView, context: Context) {
        nsView.setLetterboxColor(self.letterboxColor)
        nsView.mouseMoveHandler = self.mouseMoveHandler
        print(self)
    }
}
#else
extension MetalView : UIViewRepresentable {
    typealias UIViewType = MyMTKView
    
    func makeUIView(context: Context) -> MyMTKView {
        return (try? MyMTKView(drawBuffer: self.drawBuffer, letterboxColor: self.letterboxColor))!
    }
    
    func updateUIView(_ uiView: MyMTKView, context: Context) {
    }
    
}
#endif
