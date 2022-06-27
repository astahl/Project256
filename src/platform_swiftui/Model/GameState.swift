//
//  GameState.swift
//  Project256
//
//  Created by Andreas Stahl on 28.05.22.
//

import Foundation
import SwiftUI
import GameController
import Combine

extension GameButton {
    mutating func press() {
        self.pressed(true)
    }

    mutating func release() {
        self.pressed(false)
    }

    mutating func pressed(_ down: Bool) {
        self.transitionCount += 1
        self.endedDown = down
    }
}

extension Axis2 {
    mutating func digitalToAnalog() {
        self.end.y = self.up.endedDown ? 1 : self.down.endedDown ? -1 : 0
        self.end.x = self.right.endedDown ? 1 : self.left.endedDown ? -1 : 0
    }

    mutating func analogToDigital(deadZone: Float) {
        if self.end.y > deadZone {
            if abs(self.end.x) < self.end.y {
                self.up.press()
            } else {
                if (self.end.x > deadZone) {
                    self.right.press()
                } else if (self.end.x < -deadZone) {
                    self.left.press()
                }
            }
        } else if self.end.y < -deadZone {
            if abs(self.end.x) < abs(self.end.y) {
                self.down.press()
            } else {
                if (self.end.x > deadZone) {
                    self.right.press()
                } else if (self.end.x < -deadZone) {
                    self.left.press()
                }
            }
        }
    }
}

class Chronometer {
    var lastTimeIndex = 0
    var timevalues = UnsafeMutableBufferPointer<timeval>.allocate(capacity: 2)

    func elapsed() -> (microseconds: Int64, seconds: Double) {
        let nextTimeIndex = lastTimeIndex == 0 ? 1 : 0;
        gettimeofday(&timevalues[nextTimeIndex], nil)
        let t0 = timevalues[lastTimeIndex]
        let t1 = timevalues[nextTimeIndex]
        self.lastTimeIndex = nextTimeIndex
        let ms = Int64(t1.tv_sec - t0.tv_sec) * 1_000_000 + Int64(t1.tv_usec - t0.tv_usec)
        return (microseconds: ms, seconds: Double(ms) / 1_000_000);
    }

    init() {
        gettimeofday(&timevalues[lastTimeIndex], nil)
    }
}

func withUnsafeElementPointer<T, Result>(firstElement: inout T, offset: Int, body: @escaping (UnsafePointer<T>) throws -> Result) rethrows -> Result {
    do {
        return try withUnsafePointer(to: &firstElement) {
            beginPtr in
            let ptr = offset == 0 ? beginPtr : beginPtr.advanced(by: offset)
            do {
                return try body(ptr)
            } catch {
                throw error
            }
        }
    } catch {
        throw error
    }
}

func withUnsafeMutableElementPointer<T, Result>(firstElement: inout T, offset: Int, body: @escaping  (UnsafeMutablePointer<T>) throws -> Result) rethrows -> Result {
    do {
        return try withUnsafeMutablePointer(to: &firstElement) {
            beginPtr in
            let ptr = offset == 0 ? beginPtr : beginPtr.advanced(by: offset)
            do {
                return try body(ptr)
            } catch {
                throw error
            }
        }
    } catch {
        throw error
    }
}

func withUnsafeMutableBuffer<T, Result>(start: inout T, end: inout T, body: @escaping  (UnsafeMutableBufferPointer<T>) throws -> Result) rethrows -> Result {
    do {
        return try withUnsafeMutablePointer(to: &start) {
            startPtr in
            return try withUnsafeMutablePointer(to: &end) {
                endPtr in
                let count = startPtr.distance(to: endPtr) + 1
                do {
                    return try body(UnsafeMutableBufferPointer(start: startPtr, count: count))
                } catch {
                    throw error
                }
            }
        }
    } catch {
        throw error
    }
}

func loadDataDEBUG(filenamePtr: UnsafePointer<CChar>?, destination: UnsafeMutablePointer<UInt8>?, bufferSize: Int64) -> Int64 {
    let filename = String(cString: filenamePtr!)
    let url = Bundle.main.url(forResource: filename, withExtension: nil)
    let data = try? Data(contentsOf: url!)
    let count = min(bufferSize, Int64(data!.count))
    data!.copyBytes(to: destination!, count: Int(count))
    return count
}

func loadImageDEBUG(filenamePtr: UnsafePointer<CChar>?, destination: UnsafeMutablePointer<UInt32>?, width: Int32, height: Int32) -> Bool {
    let filename = String(cString: filenamePtr!)
    let url = Bundle.main.url(forResource: filename, withExtension: nil)
    let image = CIImage.init(contentsOf: url!)!
    let cgImage = image.cgImage ?? {
        let context = CIContext()
        return context.createCGImage(image, from: image.extent)!
    }()
    let targetColorSpace = CGColorSpace.init(name: CGColorSpace.sRGB)!;
    guard let context = CGContext.init(data: destination, width: Int(width), height: Int(height), bitsPerComponent: 8, bytesPerRow: 4 * Int(width), space: targetColorSpace, bitmapInfo: CGImageAlphaInfo.premultipliedFirst.rawValue) else {
        return false
    }

    context.draw(cgImage, in: CGRect(x: 0, y: 0, width: Int(width), height: Int(height)))
    // swizzle BGRA to ARBG with byte swap
    let buffer = UnsafeMutableBufferPointer(start: destination, count: Int(width * height))
    for i in 0..<buffer.count {
        buffer[i] = buffer[i].byteSwapped
    }
    return true
}

func timestamp() -> Int64 {
    var time = timeval()
    gettimeofday(&time, nil)
    return Int64(time.tv_sec) * 1_000_000 + Int64(time.tv_usec)
}

class GameState : ObservableObject {
    static var timingData: ProfilingTime? = {
        var value = ProfilingTime()
        value.getPlatformTimeMicroseconds = timestamp
        return value
    }()
    let memory = UnsafeMutableRawPointer.allocate(byteCount: MemorySize, alignment: 128)
    var input = GameInput()
    let frameTime = Chronometer()
    var isMouseHidden = false
    var frameNumber: UInt64 = 0
    var upTime_microseconds: Int64 = 0
    var drawBuffer = DrawBuffer()
    var controllerSubscription: AnyCancellable?;
    

    init() {
    }

    func addInputText(_ text: String) {
        let cString = text.utf8CString
        let offset = Int(input.textLength);
        let count = min(Int(InputMaxTextLength) - offset, Int(cString.count - 1))
        withUnsafeMutableElementPointer(firstElement: &input.text_utf8.0, offset: offset) {
            ptr in
            cString.withUnsafeBufferPointer {
                cStrPtr in
                ptr.assign(from: cStrPtr.baseAddress!, count: count)
            }
        }
        input.textLength += UInt32(count)
    }

    func addInputMouseMovement(relative: CGPoint, position: CGPoint?)
    {
        input.mouse.relativeMovement = Vec2f(x: Float(relative.x), y: Float(relative.y))
        if let pos = position {
            input.mouse.endedOver = true
            let offset = min(Int(input.mouse.trackLength), Int(InputMouseMaxTrackLength - 1))
            withUnsafeMutableElementPointer(firstElement: &input.mouse.track.0, offset: offset) {
                ptr in
                ptr.pointee = Vec2f(x: Float(pos.x), y: Float(pos.y))
            }
            input.mouse.trackLength += 1
        } else {
            input.mouse.endedOver = false
        }
    }

    func handleExtendedGamepadValueChange(gamepad: GCExtendedGamepad, element: GCControllerElement) {
        print(element)
    }

    func dpadHandler(dpad: GCControllerDirectionPad, x: Float, y: Float)            {

        input.controllers.1.dPad.latches = true
        input.controllers.1.dPad.end = Vec2f(x: x, y: y)
        input.controllers.1.dPad.analogToDigital(deadZone: 0.0)
    }

    func leftStickHandler(dpad: GCControllerDirectionPad, x: Float, y: Float)            {
        input.controllers.1.isActive = true
        input.controllers.1.isConnected = true
        input.controllers.1.stickLeft.latches = true
        input.controllers.1.stickLeft.end = Vec2f(x: x, y: y)
        input.controllers.1.stickLeft.analogToDigital(deadZone: 0.0)
    }

    func setupControllers() {
        GCController.shouldMonitorBackgroundEvents = true

        self.controllerSubscription = NotificationCenter.default.publisher(for: .GCControllerDidBecomeCurrent)
            .flatMap({ ($0.object as? GCController).publisher })
            .flatMap({ $0.extendedGamepad.publisher })
            .flatMap({
                gamepad in Timer.publish(every: 0.001, on: .current, in: .common)
                    .autoconnect()
                    .map({ _ in gamepad.capture() })
            })
            .sink(receiveValue: {
                extendedGamepad in
                self.input.controllers.1.isActive = true
                self.input.controllers.1.isConnected = true
                print(extendedGamepad.lastEventTimestamp)
                print(extendedGamepad.leftThumbstick.xAxis.value)
            })

        if let keyboard = GCKeyboard.coalesced {

            if input.controllers.0.subType.rawValue == 0 {
                input.controllers.0.subType = .init(rawValue: 1) // WTF why doesnt the enum work
            }
            if input.controllers.0.subType.rawValue == 2 {
                input.controllers.0.subType = .init(rawValue: 3)
            }
            if (input.controllerCount == 0) {
                input.controllerCount = 1
                input.controllers.0.isConnected = true
            }
            if let keyboardInput = keyboard.keyboardInput {
                if keyboardInput.keyChangedHandler == nil {
                    keyboardInput.keyChangedHandler = keyChanged(keyboard:key:keyCode:pressed:)
                }
            }
        }

        if let mouse = GCMouse.current {
            if input.controllers.0.subType.rawValue == 0 {
                input.controllers.0.subType = .init(rawValue: 2) // WTF why doesnt the enum work
            }
            if input.controllers.0.subType.rawValue == 1 {
                input.controllers.0.subType = .init(rawValue: 3)
            }
            if (input.controllerCount == 0) {
                input.controllerCount = 1
                input.controllers.0.isConnected = true
            }
            if let mouseInput = mouse.mouseInput {
                if mouseInput.mouseMovedHandler == nil {
                    mouseInput.mouseMovedHandler = mouseMoved(mouse:deltaX:deltaY:)
                }
                mouseInput.leftButton.valueChangedHandler = { self.input.controllers.0.buttonA.pressed($2) }
                mouseInput.rightButton?.valueChangedHandler = { self.input.controllers.0.buttonB.pressed($2) }
                mouseInput.middleButton?.valueChangedHandler = { self.input.controllers.0.buttonStickRight.pressed($2) }
            }
        }
    }

    func keyChanged(keyboard: GCKeyboardInput, key: GCControllerButtonInput, keyCode: GCKeyCode, pressed: Bool)
    {
        withUnsafeMutableElementPointer(firstElement: &input.controllers.0, offset: 0) {
            controller in
            switch keyCode {
            case .escape: controller.pointee.buttonBack.pressed(pressed)
            case .keyW, .upArrow: controller.pointee.stickLeft.up.pressed(pressed)
            case .keyA, .leftArrow: controller.pointee.stickLeft.left.pressed(pressed)
            case .keyS, .downArrow: controller.pointee.stickLeft.down.pressed(pressed)
            case .keyD, .rightArrow: controller.pointee.stickLeft.right.pressed(pressed)
            default: break
            }
            controller.pointee.stickLeft.digitalToAnalog()
            controller.pointee.stickLeft.latches = true
            controller.pointee.isActive = true
        }
    }


    func mouseMoved(mouse: GCMouseInput, deltaX: Float, deltaY: Float)
    {
        input.controllers.0.stickRight.end.x = deltaX
        input.controllers.0.stickRight.end.y = deltaY
        input.controllers.0.stickRight.analogToDigital(deadZone: 0.2)
        input.controllers.0.isActive = true
    }




    func tick(settings: GameSettings) {
        if (self.frameNumber == 0) {
            setupControllers()
        }
        GameState.timingData?.interval(timer: eTimerTickToTick, interval: eTimingTickToTick)
        GameState.timingData?.startTimer(eTimerTickToTick)
        GameState.timingData?.startTimer(eTimerTick)

        self.input.frameNumber = self.frameNumber
        self.frameNumber += 1
        let frameTime = self.frameTime.elapsed()
        self.upTime_microseconds += Int64(Double(frameTime.microseconds) * settings.timeScale)

        self.input.upTime_microseconds = self.upTime_microseconds
        self.input.elapsedTime_s = frameTime.seconds * settings.tickScale
        // TODO finalize inputs
        GameState.timingData?.interval(timer: eTimerTick, interval: eTimingTickSetup)

        let platformCallbacks = PlatformCallbacks(readFile: loadDataDEBUG(filenamePtr:destination:bufferSize:), readImage: loadImageDEBUG(filenamePtr:destination:width:height:))

        let output = doGameThings(&self.input, self.memory, platformCallbacks)
        GameState.timingData?.interval(timer: eTimerTick, interval: eTimingTickDo)

        if output.shouldQuit {
            exit(0)
        }
        if output.needTextInput {

        }
        #if os(macOS)
        if self.input.mouse.endedOver {
            self.isMouseHidden =
            setCursorVisible(output.shouldShowSystemCursor, currentlyHidden: self.isMouseHidden)
        } else {
            self.isMouseHidden = setCursorVisible(true, currentlyHidden: self.isMouseHidden)
        }
        #endif
        cleanInput(&input)
        GameState.timingData?.interval(timer: eTimerTick, interval: eTimingTickPost)
    }
}
