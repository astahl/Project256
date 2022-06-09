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

typealias TextInputHandler = (_ text: String) -> Void
typealias MouseMoveHandler = (_ relative: CGPoint, _ position: CGPoint?) -> Void
typealias BeforeDrawHandler = () -> Void

class MyMTKView : MTKView {
    var textInputHandler: TextInputHandler?
    var beforeDrawHandler: BeforeDrawHandler?
    private var drawBuffer: DrawBuffer? = nil
    private var commandQueue: MTLCommandQueue?
    private var pipelineState: MTLRenderPipelineState?
    private var viewport = MTLViewport()
    private var quadScaleXY: [Float32] = [0.0, 0.0]
    private var texture: MTLTexture?

    required init(coder: NSCoder) {
        super.init(coder: coder)
    }

    init(drawBuffer: DrawBuffer, letterboxColor: Color?) throws {

        #if os(iOS)
//        addGestureRecognizer(UILongPressGestureRecognizer)
        #endif
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
        super.clearColor = letterboxColor?.clearColor() ?? MTLClearColor()
        self.texture = device.makeTexture(descriptor: MTLTextureDescriptor.texture2DDescriptor(pixelFormat: .bgra8Unorm, width: drawBuffer.width, height: drawBuffer.height, mipmapped: false))

        self.commandQueue = device.makeCommandQueue()
        self.viewport.zfar = 1.0
        self.preferredFramesPerSecond = 60
        self.isPaused = false
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

    override func draw() {
        super.draw()
        self.beforeDrawHandler?()

        update(withDrawableSize: drawableSize)

        guard let drawBuffer = drawBuffer else {
            return
        }

        let region = MTLRegion(origin: MTLOrigin(), size: MTLSize(width: drawBuffer.width, height: drawBuffer.height, depth: 1))

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



#if os(macOS)
    var mouseMoveHandler: MouseMoveHandler?

    override var acceptsFirstResponder: Bool {
        return true
    }

    override func viewDidMoveToWindow() {
        self.window?.acceptsMouseMovedEvents = true
    }

    override func keyDown(with event: NSEvent) {
        if let characters = event.characters {
            self.textInputHandler?(characters)
        }
    }

    override func mouseMoved(with event: NSEvent) {
        let normalizedPos = CGPoint(x: event.locationInWindow.x / self.frame.width, y: event.locationInWindow.y / self.frame.height)
        let scaleX = CGFloat(self.quadScaleXY[0])
        let scaleY = CGFloat(self.quadScaleXY[1])
        let pos = normalizedPos
            .applying(CGAffineTransform.init(translationX: -1, y: -1).scaledBy(x: 2, y: 2))

        if !CGRect.init(x: -1, y: -1, width: 2, height: 2).contains(pos) {
            self.mouseMoveHandler?(CGPoint(x: event.deltaX, y: event.deltaY), nil)
        } else {

            let pixelPosition = pos.applying(CGAffineTransform.init(translationX: 0.5, y: 0.5).scaledBy(x: 0.5 / scaleX, y: 0.5 / scaleY)).applying(CGAffineTransform.init(scaleX: CGFloat(drawBuffer?.width ?? 1), y: CGFloat(drawBuffer?.height ?? 1)))

            self.mouseMoveHandler?(CGPoint(x: event.deltaX, y: event.deltaY), pixelPosition)
        }
    }
#endif
}

final class MetalView {
    private var drawBuffer: DrawBuffer
    private var letterboxColor: Color
    private var textInputHandler: TextInputHandler?
    private var mouseMoveHandler: MouseMoveHandler?
    private var beforeDrawHandler: BeforeDrawHandler?


    init(drawBuffer: DrawBuffer) {
        self.drawBuffer = drawBuffer
        self.letterboxColor = Color.black
    }

    func letterboxColor(_ color: Color) -> MetalView {
        self.letterboxColor = color
        return self
    }

    func textInput(_ handler: @escaping(TextInputHandler)) -> MetalView {
        self.textInputHandler = handler
        return self
    }

    func mouseMove(_ handler: @escaping(MouseMoveHandler)) -> MetalView {
        self.mouseMoveHandler = handler
        return self
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
        return (try? MyMTKView(drawBuffer: self.drawBuffer, letterboxColor: self.letterboxColor))!
    }
    
    func updateNSView(_ nsView: MyMTKView, context: Context) {
        nsView.setLetterboxColor(self.letterboxColor)
        nsView.mouseMoveHandler = self.mouseMoveHandler
        nsView.beforeDrawHandler = self.beforeDrawHandler
        nsView.textInputHandler = self.textInputHandler
    }
}
#else
extension MetalView : UIViewRepresentable {
    typealias UIViewType = MyMTKView
    
    func makeUIView(context: Context) -> MyMTKView {
        return (try? MyMTKView(drawBuffer: self.drawBuffer, letterboxColor: self.letterboxColor))!
    }
    
    func updateUIView(_ uiView: MyMTKView, context: Context) {
        uiView.beforeDrawHandler = self.beforeDrawHandler
    }
    
}
#endif
