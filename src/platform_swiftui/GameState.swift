//
//  GameState.swift
//  Project256
//
//  Created by Andreas Stahl on 28.05.22.
//

import Foundation
import SwiftUI

struct Chronometer {
    var lastTimeIndex = 0
    var timevalues = UnsafeMutableBufferPointer<timeval>.allocate(capacity: 2)

    mutating func seconds() -> Double {
        let nextTimeIndex = lastTimeIndex == 0 ? 1 : 0;
        gettimeofday(&timevalues[nextTimeIndex], nil)
        let t0 = timevalues[lastTimeIndex]
        let t1 = timevalues[nextTimeIndex];
        self.lastTimeIndex = nextTimeIndex
        return Double(t1.tv_sec - t0.tv_sec) + Double(t1.tv_usec - t0.tv_usec) / 1_000_000.0;
    }

    init() {
        gettimeofday(&timevalues[lastTimeIndex], nil)
    }
}

class GameState : ObservableObject {
    let memory = UnsafeMutableRawPointer.allocate(byteCount: MemorySize, alignment: 128)
    var input = GameInput()
    var frameTime = Chronometer()
    var isMouseHidden = false
    var frameNumber: UInt64 = 0
    @Published var drawBuffer = DrawBuffer(width: Int(DrawBufferWidth), height: Int(DrawBufferHeight))
}
