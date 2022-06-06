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

class GameState : ObservableObject {
    let memory = UnsafeMutableRawPointer.allocate(byteCount: MemorySize, alignment: 128)
    var input = GameInput()
    let frameTime = Chronometer()
    var isMouseHidden = false
    var frameNumber: UInt64 = 0
    var upTime_microseconds: Int64 = 0
    var drawBuffer = DrawBuffer(width: Int(DrawBufferWidth), height: Int(DrawBufferHeight))

    func addInputText(text: String) {
        let cString = text.utf8CString
        let offset = Int(input.textLength);
        let count = min(Int(InputMaxTextLength) - offset, Int(cString.count - 1))
        withUnsafeMutablePointer(to: &input.text_utf8.0) {
            beginPtr in
            let ptr = offset == 0 ? beginPtr : beginPtr.advanced(by: offset)
            cString.withUnsafeBufferPointer {
                cStrPtr in
                ptr.assign(from: cStrPtr.baseAddress!, count: count)
            }
        }
        input.textLength += UInt32(count)
    }
}
