//
//  GameState.swift
//  Project256
//
//  Created by Andreas Stahl on 28.05.22.
//

import Foundation
import SwiftUI
import GameController

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

class GameState : ObservableObject {
    static var timingData = TimingData()
    let memory = UnsafeMutableRawPointer.allocate(byteCount: MemorySize, alignment: 128)
    var input = GameInput()
    let frameTime = Chronometer()
    var isMouseHidden = false
    var frameNumber: UInt64 = 0
    var upTime_microseconds: Int64 = 0
    var drawBuffer = DrawBuffer()
    @Published var tickScale: Double = 1.0

    init() {
        GameState.timingData.getPlatformTimeMicroseconds = {
            var time = timeval()
            gettimeofday(&time, nil)
            return Int64(time.tv_sec) * 1_000_000 + Int64(time.tv_usec)
        }

        profiling_time_initialise(&GameState.timingData)
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

    func setupControllers() {
        if let keyboard = GCKeyboard.coalesced {

            if input.controllers.0.subType.rawValue == 0 {
                input.controllers.0.subType = .init(rawValue: 1) // WTF why doesnt the enum work
            }
            if input.controllers.0.subType.rawValue == 2 {
                input.controllers.0.subType = .init(rawValue: 3)
            }
            if (input.controllerCount == 0) {
                input.controllerCount = 1
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
            }
            if let mouseInput = mouse.mouseInput {
                if mouseInput.mouseMovedHandler == nil {
                    mouseInput.mouseMovedHandler = mouseMoved(mouse:deltaX:deltaY:)
                }
            }
        }
    }

    func keyChanged(keyboard: GCKeyboardInput, key: GCControllerButtonInput, keyCode: GCKeyCode, pressed: Bool)
    {
        switch keyCode {
        case .keyW, .upArrow: input.controllers.0.stickLeft.up.pressed(pressed)
        case .keyA, .leftArrow: input.controllers.0.stickLeft.left.pressed(pressed)
        case .keyS, .downArrow: input.controllers.0.stickLeft.down.pressed(pressed)
        case .keyD, .rightArrow: input.controllers.0.stickLeft.right.pressed(pressed)
        default: break
        }
        input.controllers.0.stickLeft.digitalToAnalog()
        input.controllers.0.stickLeft.latches = true
    }

    func mouseMoved(mouse: GCMouseInput, deltaX: Float, deltaY: Float)
    {
        input.controllers.0.stickRight.end.x = deltaX
        input.controllers.0.stickRight.end.y = deltaY
        input.controllers.0.stickRight.analogToDigital(deadZone: 0.2)
    }


    func pollControllers() {


        if let controller = GCController.current {
            input.controllerCount = 2

            func dpadHandler(dpad: GCControllerDirectionPad, x: Float, y: Float)            {

                input.controllers.1.dPad.latches = true
                input.controllers.1.dPad.end = Vec2f(x: x, y: y)
                input.controllers.1.dPad.analogToDigital(deadZone: 0.0)
            }

            func leftStickHandler(dpad: GCControllerDirectionPad, x: Float, y: Float)            {

                input.controllers.1.stickLeft.latches = true
                input.controllers.1.stickLeft.end = Vec2f(x: x, y: y)
                input.controllers.1.stickLeft.analogToDigital(deadZone: 0.0)
            }
            if let extendedGamepad = controller.extendedGamepad {
                if extendedGamepad.leftThumbstick.valueChangedHandler == nil {
                    extendedGamepad.leftThumbstick.valueChangedHandler = leftStickHandler(dpad:x:y:)
                }
            }
        }
    }

    func tick() {
        setupControllers()
        pollControllers()
        profiling_time_interval(&GameState.timingData, eTimerTickToTick, eTimingTickToTick)
        profiling_time_set(&GameState.timingData, eTimerTickToTick)
        profiling_time_set(&GameState.timingData, eTimerTick)

        self.input.frameNumber = self.frameNumber
        self.frameNumber += 1
        let frameTime = self.frameTime.elapsed()
        self.upTime_microseconds += Int64(Double(frameTime.microseconds) * tickScale)

        self.input.upTime_microseconds =  self.upTime_microseconds
        self.input.elapsedTime_s = frameTime.seconds * tickScale
        // TODO finalize inputs
        profiling_time_interval(&GameState.timingData, eTimerTick, eTimingTickSetup)

        let output = doGameThings(&self.input, self.memory)
        profiling_time_interval(&GameState.timingData, eTimerTick, eTimingTickDo)

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
        profiling_time_interval(&GameState.timingData, eTimerTick, eTimingTickPost)
    }
}
