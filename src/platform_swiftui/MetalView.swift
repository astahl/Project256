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
enum MyMTKViewErrors : Error {
    case InitError
}
enum MouseButton {
    case Left, Right, Other
}

enum Click {
    case Up, Down, Double
}

typealias TextInputHandler = (_ text: String) -> Void
typealias MouseMoveHandler = (_ relative: CGPoint, _ position: CGPoint?) -> Void
typealias MouseClickHandler = (_ mouseButton: MouseButton, _ click: Click, _ position: CGPoint?) -> Void
typealias BeforeDrawHandler = (_ buffer: DrawBuffer) -> Void

class MyMTKView : MTKView {
    var textInputHandler: TextInputHandler?
    var beforeDrawHandler: BeforeDrawHandler?
    private let drawBuffer = DrawBuffer()
    private var commandQueue: MTLCommandQueue?
    private var pipelineState: MTLRenderPipelineState?
    private var viewport = MTLViewport()
    private var scale = Vec2f()
    private var texture: MTLTexture?

    required init(coder: NSCoder) {
        super.init(coder: coder)
    }

    init( letterboxColor: Color?) throws {

        #if os(iOS)
//        addGestureRecognizer(UILongPressGestureRecognizer)
        #endif

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
        //self.isPaused = true
        //self.enableSetNeedsDisplay = true
    }

    func setLetterboxColor(_ color: Color) {
        if let clearColor = color.clearColor() {
            self.clearColor = clearColor
        }
    }

    func updateViewport(withDrawableSize size: CGSize) {
        self.viewport.height = size.height
        self.viewport.width = size.width
        self.scale = clipSpaceDrawBufferScale(UInt32(size.width), UInt32(size.height))
    }

    override func draw(_ dirtyRect: CGRect) {

        profiling_time_set(&GameState.timingData, eTimerDraw)
        self.beforeDrawHandler?(self.drawBuffer)
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



#if os(macOS)
    var mouseMoveHandler: MouseMoveHandler?
    var mouseClickHandler: MouseClickHandler?

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

    func positionOnBuffer(locationInWindow: NSPoint) -> CGPoint? {
        let normalizedPos = CGPoint(x: locationInWindow.x / self.frame.width, y: locationInWindow.y / self.frame.height)
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

    override func mouseMoved(with event: NSEvent) {
        if let pixelPosition = positionOnBuffer(locationInWindow: event.locationInWindow) {
            self.mouseMoveHandler?(CGPoint(x: event.deltaX, y: event.deltaY), pixelPosition)
        } else {
            self.mouseMoveHandler?(CGPoint(x: event.deltaX, y: event.deltaY), nil)
        }
    }

    override func mouseDragged(with event: NSEvent) {
        if let pixelPosition = positionOnBuffer(locationInWindow: event.locationInWindow) {
            self.mouseMoveHandler?(CGPoint(x: event.deltaX, y: event.deltaY), pixelPosition)
        } else {
            self.mouseMoveHandler?(CGPoint(x: event.deltaX, y: event.deltaY), nil)
        }
    }

    override func rightMouseDragged(with event: NSEvent) {
        if let pixelPosition = positionOnBuffer(locationInWindow: event.locationInWindow) {
            self.mouseMoveHandler?(CGPoint(x: event.deltaX, y: event.deltaY), pixelPosition)
        } else {
            self.mouseMoveHandler?(CGPoint(x: event.deltaX, y: event.deltaY), nil)
        }
    }

    override func mouseDown(with event: NSEvent) {
        self.mouseClickHandler?(.Left, .Down, positionOnBuffer(locationInWindow: event.locationInWindow))
    }

    override func mouseUp(with event: NSEvent) {
        self.mouseClickHandler?(.Left, .Up, positionOnBuffer(locationInWindow: event.locationInWindow))
    }

    override func rightMouseDown(with event: NSEvent) {
        self.mouseClickHandler?(.Right, .Down, positionOnBuffer(locationInWindow: event.locationInWindow))
    }

    override func rightMouseUp(with event: NSEvent) {
        self.mouseClickHandler?(.Right, .Up, positionOnBuffer(locationInWindow: event.locationInWindow))
    }

    override func otherMouseDown(with event: NSEvent) {
        self.mouseClickHandler?(.Other, .Down, positionOnBuffer(locationInWindow: event.locationInWindow))
    }

    override func otherMouseUp(with event: NSEvent) {
        self.mouseClickHandler?(.Other, .Up, positionOnBuffer(locationInWindow: event.locationInWindow))
    }
#endif
}

final class MetalView {
    private var letterboxColor: Color
    private var needsDisplay: Bool
    private var textInputHandler: TextInputHandler?
    private var mouseMoveHandler: MouseMoveHandler?
    private var beforeDrawHandler: BeforeDrawHandler?
    private var mouseClickHandler: MouseClickHandler?


    init() {
        self.letterboxColor = Color.black
        self.needsDisplay = false
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

    func mouseClick(_ handler: @escaping(MouseClickHandler)) -> MetalView {
        self.mouseClickHandler = handler
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
        return (try? MyMTKView(letterboxColor: self.letterboxColor))!
    }
    
    func updateNSView(_ nsView: MyMTKView, context: Context) {
        nsView.setLetterboxColor(self.letterboxColor)
        nsView.mouseMoveHandler = self.mouseMoveHandler
        nsView.beforeDrawHandler = self.beforeDrawHandler
        nsView.textInputHandler = self.textInputHandler
        nsView.mouseClickHandler = self.mouseClickHandler
        nsView.needsDisplay = self.needsDisplay
    }
}
#else
extension MetalView : UIViewRepresentable {
    typealias UIViewType = MyMTKView
    
    func makeUIView(context: Context) -> MyMTKView {
        return (try? MyMTKView(letterboxColor: self.letterboxColor))!
    }
    
    func updateUIView(_ uiView: MyMTKView, context: Context) {
        uiView.setLetterboxColor(self.letterboxColor)
        uiView.beforeDrawHandler = self.beforeDrawHandler
        uiView.needsDisplay = self.needsDisplay
    }
    
}
#endif
