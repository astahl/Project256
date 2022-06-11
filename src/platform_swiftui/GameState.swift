//
//  GameState.swift
//  Project256
//
//  Created by Andreas Stahl on 28.05.22.
//

import Foundation
import SwiftUI

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
    var drawBuffer = DrawBuffer(width: Int(DrawBufferWidth), height: Int(DrawBufferHeight))

    init() {
        GameState.timingData.getPlatformTimeMicroseconds = {
            var time = timeval()
            gettimeofday(&time, nil)
            return Int64(time.tv_sec) * 1_000_000 + Int64(time.tv_usec)
        }

        profiling_time_initialise(&GameState.timingData)
    }

    func addInputText(text: String) {
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
            input.mouse.endedOver = eTRUE
            let offset = min(Int(input.mouse.trackLength), Int(InputMouseMaxTrackLength - 1))
            withUnsafeMutableElementPointer(firstElement: &input.mouse.track.0, offset: offset) {
                ptr in
                ptr.pointee = Vec2f(x: Float(pos.x), y: Float(pos.y))
            }
            input.mouse.trackLength += 1
        } else {
            input.mouse.endedOver = eFALSE
        }
    }

    func clearInput() {
        var oldInputCopy = input

        input = GameInput()
        input.hasMouse = eTRUE
        if oldInputCopy.mouse.endedOver == eTRUE && oldInputCopy.mouse.trackLength > 0 {
            let oldMousePosition = withUnsafeElementPointer(firstElement: &oldInputCopy.mouse.track.0, offset: Int(oldInputCopy.mouse.trackLength - 1)) { $0.pointee }
            input.mouse.endedOver = eTRUE
            input.mouse.track.0 = oldMousePosition
            input.mouse.trackLength = 1
        }
        input.mouse.buttonLeft.endedDown = oldInputCopy.mouse.buttonLeft.endedDown
        input.mouse.buttonRight.endedDown = oldInputCopy.mouse.buttonRight.endedDown
        input.mouse.buttonMiddle.endedDown = oldInputCopy.mouse.buttonMiddle.endedDown
    }
}
